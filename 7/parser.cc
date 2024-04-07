#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <sstream>

#include "parser.h"

using namespace std;

Parser::Parser(const std::string& fileName)
    :
    m_arg1(""),
    m_arg2(0),
    m_lineIndex(0),
    m_lineCount(0)
{
    m_inputStream = ifstream(fileName);

    if (!m_inputStream.is_open())
    {
        throw bad_exception();
    }

    m_arithLogicCommandSet = {"add", "sub", "neg", "eq", "gt", "lt", "and", "or", "not"};
    m_memoryCommandMap = {{"push", C_PUSH}, {"pop", C_POP}};
    m_memorySegmentSet = {"argument", "local", "static", "constant", "this", "that", "pointer", "temp"};

    m_lineCount = numberOfCommands(fileName); 
}

int Parser::numberOfCommands(const string& fileName)
{
    ifstream inputStream(fileName);

    if (!inputStream.is_open())
    {
        throw bad_exception();
    }

    string currentLine;
    int count = 0;
    while (getline(inputStream, currentLine))
    {
        stringstream ss = stringstream(currentLine);
        string token;
        if (ss >> token)
        {
            if (token.find("//") == 0)
            {
                continue;
            }

            count++;
        }
    }

    inputStream.close();

    return count;
}

bool Parser::hasMoreCommands() const
{
    return m_lineIndex < m_lineCount;
}

void Parser::advance()
{
    string currentLine;
    while (getline(m_inputStream, currentLine))
    {
        stringstream ss = stringstream(currentLine);
        string token;

        if (ss >> token)
        {
            m_arg1 = token;

            if (token.find("//") == 0)
            {
                continue;
            }

            if (m_arithLogicCommandSet.find(token) != m_arithLogicCommandSet.end())
            {
                m_commandType = C_ARITHMETIC;
            }
            else if (m_memoryCommandMap.find(token) != m_memoryCommandMap.end())
            {
                m_commandType = m_memoryCommandMap.at(token);
                string segment;
                int index;
                if (ss >> segment >> index)
                {
                    m_arg1 = segment;
                    m_arg2 = index;
                }
                else
                {
                    throw runtime_error(string(__func__) + "Invalid syntax");
                }
            }
            else
            {
                throw runtime_error(string(__func__) + "Invalid syntax");
            }

            m_lineIndex++;
            break;
        }
    }

    if (!hasMoreCommands())
    {
        m_inputStream.close();
    }
}

CommandType Parser::commandType() const
{
    return m_commandType;
}

std::string Parser::arg1() const
{
    return m_arg1;
}
int Parser::arg2() const
{
    return m_arg2;
}