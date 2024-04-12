#pragma once


#include <string>
#include <fstream>
#include <queue>

class JackTokenizer
{
public:
    enum TokenType {KEYWORD, SYMBOL, IDENTIFIER, INT_CONST, STRING_CONST};
    enum KeyWord {CLASS, METHOD, FUNCTION, CONSTRUCTOR, INT, BOOLEAN, CHAR, VOID, VAR, STATIC, FIELD, LET, DO, IF, ELSE, WHILE, RETURN, TRUE, FALSE, NILL, THIS};

public:
    JackTokenizer(const std::string& fileName);
    bool hasMoreTokens() const;
    void advance();
    TokenType tokenType() const;
    KeyWord keyWord() const;
    char symbol() const;
    std::string identifier() const;
    int intVal() const;
    std::string stringVal() const;

private:
    void getNextToken();
    void analyseToken();

private:
    std::string m_token;
    bool m_hasMoreTokens;
    std::ifstream m_inputStream;
    TokenType m_tokenType;
    KeyWord m_keyWord;
    char m_symbol;
    std::string m_identifier;
    int m_intVal;
    std::string m_stringVal;

    std::queue<std::string> m_tokenQueue;
};