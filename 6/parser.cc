#include "parser.h"
#include <string>
#include <iostream>
#include <algorithm>

using namespace std;

Parser::Parser(const string& fileName)
{
    fillSymbolTable(fileName);

    // Read from the text file
    m_inputStream = ifstream(fileName);

    if (!m_inputStream.is_open())
    {
        throw bad_exception();
    }

}

void Parser::fillSymbolTable(const std::string& fileName)
{
    ifstream symbolInputStream = ifstream(fileName);
    if (!symbolInputStream.is_open())
    {
        throw bad_exception();
    }

    string currentLine;
    while (getline(symbolInputStream, currentLine))
    {
        if (currentLine.size() == 0)
        {
            continue;
        }

        auto spaceComp = [](unsigned char ch) { return !isspace(ch); };
        currentLine.erase(currentLine.begin(), find_if(currentLine.begin(), currentLine.end(), spaceComp));
        currentLine.erase(find_if(currentLine.rbegin(), currentLine.rend(), spaceComp).base(), currentLine.end());
        int strLen = currentLine.size();

        char startChar = currentLine[0];
        if (startChar == '/')
        {
            continue;
        }

        if (startChar == '(')
        {
            auto leftIt = find(currentLine.begin(), currentLine.end(), '(');
            auto rightIt = find(currentLine.begin(), currentLine.end(), ')');

            if (rightIt == currentLine.end())
            {
                throw runtime_error("Invalid syntax");
            }

            int symbolLen = (rightIt - leftIt) - 1;

            if (symbolLen < 1)
            {
                throw runtime_error("Invalid syntax");
            }

            string symbol = currentLine.substr(1, symbolLen);
            if (!symbolTable.contains(symbol))
            {
                symbolTable.addEntry(symbol, numberOfLines);
            }
            continue;
        }

        numberOfLines++;
    }

    symbolInputStream.close();
}

bool Parser::hasMoreCommands()
{
    return lineIndex < numberOfLines;
}

void Parser::advance()
{
    static const std::unordered_set<std::string> m_destSet {"M", "D", "MD", "A", "AM", "AD", "AMD"};
    static const std::unordered_set<std::string> m_compSet {"0", "1", "-1", "D", "A", "!D", "!A", "-D", "-A", "D+1", "A+1", "D-1", "A-1", "D+A", "D-A", "A-D", "D&A", "D|A",
                                                            "M", "!M", "-M", "M+1", "M-1", "M+D", "D+M", "D-M", "M-D", "D&M", "D|M"};
    static const std::unordered_set<std::string> m_jumpSet {"JGT", "JEQ", "JGE", "JLT", "JNE", "JLE", "JMP"};

    string currentLine;

    while (getline(m_inputStream, currentLine))
    {
        if (currentLine.size() == 0)
        {
            continue;
        }

        auto spaceComp = [](unsigned char ch) { return !isspace(ch); };
        currentLine.erase(currentLine.begin(), find_if(currentLine.begin(), currentLine.end(), spaceComp));
        currentLine.erase(find_if(currentLine.rbegin(), currentLine.rend(), spaceComp).base(), currentLine.end());

        int strLen = currentLine.size();

        char startChar = currentLine[0];
        if (startChar == '/')
        {
            if (strLen == 1 || currentLine[1] != '/')
            {
                throw runtime_error("Invalid syntax");
            }

            // Comment line detected. Advance to next line.
            continue;
        }

        m_symbol = "";
        m_dest = "";
        m_comp = "";
        m_jump = "";

        if (startChar == '@')
        {
            if (strLen == 1)
            {
                throw runtime_error("Invalid syntax");
            }

            m_commandType = A_COMMAND;

            string symbol = currentLine.substr(1);

            if (symbolTable.contains(symbol))
            {
                m_symbol = to_string(symbolTable.getAddress(symbol));
            }
            else
            {
                if (!std::isdigit(symbol[0]))
                {
                    symbolTable.addEntry(symbol, symbolIndex++);
                    m_symbol = to_string(symbolTable.getAddress(symbol));
                }
                else
                {
                    m_symbol = symbol;
                }
            }

            lineIndex++;
            return;
        }

        if (startChar == '(')
        {
            auto leftIt = find(currentLine.begin(), currentLine.end(), '(');
            auto rightIt = find(currentLine.begin(), currentLine.end(), ')');

            if (rightIt == currentLine.end())
            {
                throw runtime_error("Invalid syntax");
            }

            int symbolLen = (rightIt - leftIt) - 1;

            if (symbolLen < 1)
            {
                throw runtime_error("Invalid syntax");
            }
            m_symbol = currentLine.substr(1, symbolLen);
            m_commandType = L_COMMAND;

            return;
        }

        m_commandType = C_COMMAND;

        int eqIndex = find(currentLine.begin(), currentLine.end(), '=') - currentLine.begin();
        int semiIndex = find(currentLine.begin(), currentLine.end(), ';') - currentLine.begin();

        if (eqIndex >= strLen && semiIndex >= strLen)
        {
            throw runtime_error("Invalid syntax");
        }

        if (eqIndex != strLen && semiIndex != strLen && semiIndex < eqIndex)
        {
            throw runtime_error("Invalid syntax");
        }

        if (eqIndex < strLen)
        {
            m_dest = currentLine.substr(0, eqIndex);
            if (m_destSet.count(m_dest) == 0)
            {
                throw runtime_error("Invalid syntax");
            }

            if (eqIndex == strLen - 1)
            {
                throw runtime_error("Invalid syntax");
            }
            m_comp = currentLine.substr(eqIndex + 1);
            if (m_compSet.count(m_comp) == 0)
            {
                throw runtime_error("Invalid syntax");
            }
        }

        if (semiIndex < strLen)
        {
            if (semiIndex == strLen - 1 || semiIndex == 0)
            {
                throw runtime_error("Invalid syntax");
            }

            m_comp = currentLine.substr(0, semiIndex);
            if (m_compSet.count(m_comp) == 0)
            {
                throw runtime_error("Invalid syntax");
            }
            m_jump = currentLine.substr(semiIndex + 1);
            if (m_jumpSet.count(m_jump) == 0)
            {
                throw runtime_error("Invalid syntax");
            }
        }

        if (eqIndex < strLen && semiIndex < strLen)
        {
            if (eqIndex == strLen - 1)
            {
                throw runtime_error("Invalid syntax");
            }
            int compLen = (semiIndex - eqIndex) - 1;
            m_comp = currentLine.substr(semiIndex + 1, compLen);
            if (m_compSet.count(m_comp) == 0)
            {
                throw runtime_error("Invalid syntax");
            }
        }

        lineIndex++;
        break;
    }

    if (!hasMoreCommands())
    {
        m_inputStream.close();
    }
}

Command_Type Parser::commandType()
{
    return m_commandType;
}

string Parser::symbol()
{
    return m_symbol;
}

string Parser::dest()
{
    return m_dest;
}

string Parser::comp()
{
    return m_comp;
}

string Parser::jump()
{
    return m_jump;
}

