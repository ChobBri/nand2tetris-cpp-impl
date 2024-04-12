#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <sstream>

#include <unordered_set>
#include <unordered_map>

#include "jackTokenizer.h"

using namespace std;

JackTokenizer::JackTokenizer(const string& fileName)
    :
    m_hasMoreTokens(true),
    m_tokenType(KEYWORD),
    m_keyWord(CLASS),
    m_symbol(0),
    m_identifier(""),
    m_intVal(0),
    m_stringVal("")
{
    m_inputStream = ifstream(fileName);

    if (!m_inputStream.is_open())
    {
        throw bad_exception();
    }

    getNextToken();
}

bool JackTokenizer::hasMoreTokens() const
{
    return m_hasMoreTokens;
}

void JackTokenizer::advance()
{
    analyseToken();
    getNextToken();
}

void JackTokenizer::analyseToken()
{
    static const unordered_map<string, KeyWord> keywordMap = {
        {"class", CLASS}, {"constructor", CONSTRUCTOR}, {"function", FUNCTION},
        {"method", METHOD}, {"field", FIELD}, {"static", STATIC},
        {"var", VAR}, {"int", INT}, {"char", CHAR}, {"boolean", BOOLEAN},
        {"void", VOID}, {"true", TRUE}, {"false", FALSE}, {"null", NILL},
        {"this", THIS}, {"let", LET}, {"do", DO}, {"if", IF},
        {"else", ELSE}, {"while", WHILE}, {"return", RETURN}
        };

    static const unordered_set<string> symbolSet = {
        "{", "}", "(", ")", "[", "]", ".", ",", ";", 
        "+", "-", "*", "/", "&", "|", "<", ">", "=", "~"
    };

    int len = m_token.size();

    if (keywordMap.find(m_token) != keywordMap.end())
    {
        m_keyWord = keywordMap.at(m_token);
        m_tokenType = KEYWORD;
        return;
    }

    if (symbolSet.find(m_token) != symbolSet.end())
    {
        m_symbol = m_token[0];
        m_tokenType = SYMBOL;
        return;
    }

    stringstream ss(m_token);
    int number = 0;

    if (ss >> number)
    {
        if (to_string(number) != m_token)
        {
            throw logic_error("Invalid token.");
        }

        if (number < 0 || number > 32767)
        {
            throw logic_error("Invalid token.");
        }

        m_intVal = number;
        m_tokenType = INT_CONST;
        return;
    }

    if (len >= 2 && m_token[0] == '"' && m_token[len - 1] == '"')
    {
        string strVal = m_token.substr(1, len - 2);
        if (count(strVal.begin(), strVal.end(), '"') != 0)
        {
            throw logic_error("Invalid token.");
        }

        m_stringVal = strVal;
        m_tokenType = STRING_CONST;
        return;
    }

    auto isValidIdChar = [](char c) {
        return
            c == '_' ||
            (c >= '0' && c <= '9') ||
            (c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z');
    };

    if (isValidIdChar(m_token[0]) && !isdigit(m_token[0]))
    {
        auto it = find_if(m_token.begin(), m_token.end(), [&](char c){ return !isValidIdChar(c); });
        if (it != m_token.end())
        {
            throw logic_error("Invalid token.");
        }

        m_identifier = m_token;
        m_tokenType = IDENTIFIER;
        return;
    }

    throw logic_error("Invalid token.");
}

void JackTokenizer::getNextToken()
{
    static const unordered_map<string, KeyWord> keywordMap = {
        {"class", CLASS}, {"constructor", CONSTRUCTOR}, {"function", FUNCTION},
        {"method", METHOD}, {"field", FIELD}, {"static", STATIC},
        {"var", VAR}, {"int", INT}, {"char", CHAR}, {"boolean", BOOLEAN},
        {"void", VOID}, {"true", TRUE}, {"false", FALSE}, {"null", NILL},
        {"this", THIS}, {"let", LET}, {"do", DO}, {"if", IF},
        {"else", ELSE}, {"while", WHILE}, {"return", RETURN}
    };

    static const unordered_set<string> symbolSet = {
        "{", "}", "(", ")", "[", "]", ".", ",", ";", 
        "+", "-", "*", "/", "&", "|", "<", ">", "=", "~"
    };

    if (!m_tokenQueue.empty())
    {
        m_token = m_tokenQueue.front();
        m_tokenQueue.pop();
        return;
    }

    while (m_inputStream.is_open())
    {
        string buf;
        if (!getline(m_inputStream, buf))
        {
            m_hasMoreTokens = false;
            m_inputStream.close();
            break;
        }

        if (buf.size() == 0)
        {
            continue;
        }



        while (buf.size() > 0)
        {
            auto it = find_if(buf.begin(), buf.end(), [](char c){ return c != ' '; });
            if (it != buf.begin())
            {
                int len = it - buf.begin();
                buf.erase(0, len);
                continue;
            }

            int nextSpacePos = buf.find(" ");


            if (buf[0] == '"')
            {
                int pos = buf.find("\"", 1);
                if (pos == string::npos)
                {
                    throw logic_error("Could not tokenize string.");
                }
                m_tokenQueue.push(buf.substr(0, pos + 1));
                buf.erase(0, pos + 1);
            }
            else if (buf.find("//", 0, 2) == 0)
            {
                break;
            }
            else if (buf.find("/*", 0, 2) == 0)
            {
                do
                {
                    int pos = buf.find("*/");

                    if (pos != string::npos)
                    {
                        buf.erase(0, pos + 2);
                        break;
                    }
                } while (getline(m_inputStream, buf));
            }
            else if (symbolSet.find(string(1, buf[0])) != symbolSet.end())
            {
                m_tokenQueue.push(string(1, buf[0]));
                buf.erase(0, 1);
            }
            else if (buf[0] == '"')
            {
                int pos = buf.find("\"", 1);
                if (pos == string::npos)
                {
                    throw logic_error("Could not tokenize string.");
                }
                m_tokenQueue.push(buf.substr(pos + 1));
                buf.erase(0, pos + 1);
            }
            else if (isdigit(buf[0]))
            {
                auto it = find_if(buf.begin(), buf.end(), [](char c){
                    return !isdigit(c);
                });
                int len = it - buf.begin();
                m_tokenQueue.push(buf.substr(0, len));
                buf.erase(0, len);
            }
            else if (keywordMap.find(buf) != keywordMap.end())
            {
                m_tokenQueue.push(buf);
                buf = "";
                break;
            }

            else
            {
                auto it = find_if(buf.begin(), buf.end(), [](char c){
                    return (symbolSet.find(string(1, c)) != symbolSet.end()) || (c == ' ');
                });

                int len = it - buf.begin();
                string sub = buf.substr(0, len);
                m_tokenQueue.push(sub);
                buf.erase(0, len);
            }
        }

        if (m_tokenQueue.empty())
        {
            continue;
        }

        m_token = m_tokenQueue.front();
        m_tokenQueue.pop();

        break;
    }
}

JackTokenizer::TokenType JackTokenizer::tokenType() const
{
    return m_tokenType;
}

JackTokenizer::KeyWord JackTokenizer::keyWord() const
{
    return m_keyWord;
}

char JackTokenizer::symbol() const
{
    return m_symbol;
}

std::string JackTokenizer::identifier() const
{
    return m_identifier;
}

int JackTokenizer::intVal() const
{
    return m_intVal;
}

std::string JackTokenizer::stringVal() const
{
    return m_stringVal;
}
