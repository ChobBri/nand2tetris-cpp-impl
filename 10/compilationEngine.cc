#include <memory>
#include <exception>
#include <unordered_map>
#include <iostream>

#include "compilationEngine.h"
#include "jackTokenizer.h"

using namespace std;

CompilationEngine::CompilationEngine(const std::string& inputFileName, const std::string& outputFileName)
    :
    m_depth(0)
{

    m_outputStream = ofstream(outputFileName, ios::trunc | ios::binary);

    if (!m_outputStream.is_open())
    {
        throw runtime_error("Could not open file.");
    }

    m_tokenizer = new JackTokenizer(inputFileName);
    m_tokenizer->advance();
}

CompilationEngine::~CompilationEngine()
{
    delete m_tokenizer;
}

void CompilationEngine::compile()
{
    compileClass();
    m_outputStream.close();
}

void CompilationEngine::writeTabs(int tabCount)
{
    for (int i = 0; i < tabCount; i++)
    {
        m_outputStream << "  ";
    }
}

void CompilationEngine::writeLine(const string& tokenType, const string& token)
{
    writeTabs(m_depth);
    cout << token << endl;
    m_outputStream << "<" << tokenType << "> " << token << " </" << tokenType << ">\n"; 
}

void CompilationEngine::writeOpenTag(const string& token)
{
    writeTabs(m_depth++);
    m_outputStream << "<" << token << ">\n"; 
}

void CompilationEngine::writeCloseTag(const string& token)
{
    writeTabs(--m_depth);
    m_outputStream << "</" << token << ">\n"; 
}

void CompilationEngine::writeKeyword(const std::string& keyword)
{
    if (m_tokenizer->tokenType() != JackTokenizer::TokenType::KEYWORD)
    {
        throw logic_error("Expected keyword.");
    }

    static const unordered_map<JackTokenizer::KeyWord, string> keyWordMap = {
        {JackTokenizer::KeyWord::BOOLEAN, "boolean"},
        {JackTokenizer::KeyWord::CHAR, "char"},
        {JackTokenizer::KeyWord::CLASS, "class"},
        {JackTokenizer::KeyWord::CONSTRUCTOR, "constructor"},
        {JackTokenizer::KeyWord::DO, "do"},
        {JackTokenizer::KeyWord::ELSE, "else"},
        {JackTokenizer::KeyWord::FALSE, "false"},
        {JackTokenizer::KeyWord::FIELD, "field"},
        {JackTokenizer::KeyWord::FUNCTION, "function"},
        {JackTokenizer::KeyWord::IF, "if"},
        {JackTokenizer::KeyWord::INT, "int"},
        {JackTokenizer::KeyWord::LET, "let"},
        {JackTokenizer::KeyWord::METHOD, "method"},
        {JackTokenizer::KeyWord::NILL, "null"},
        {JackTokenizer::KeyWord::RETURN, "return"},
        {JackTokenizer::KeyWord::STATIC, "static"},
        {JackTokenizer::KeyWord::THIS, "this"},
        {JackTokenizer::KeyWord::TRUE, "true"},
        {JackTokenizer::KeyWord::VAR, "var"},
        {JackTokenizer::KeyWord::VOID, "void"},
        {JackTokenizer::KeyWord::WHILE, "while"},
    };

    string tokenKeyword = keyWordMap.at(m_tokenizer->keyWord());

    if (tokenKeyword != keyword)
    {
        throw logic_error("Received wrong keyword");
    }

    writeLine("keyword", tokenKeyword);

    if (m_tokenizer->hasMoreTokens())
    {
        m_tokenizer->advance();
    }
}

void CompilationEngine::writeIdentifier()
{
    if (m_tokenizer->tokenType() != JackTokenizer::TokenType::IDENTIFIER)
    {
        throw logic_error("Expected identifier.");
    }

    writeLine("identifier", m_tokenizer->identifier());

    if (m_tokenizer->hasMoreTokens())
    {
        m_tokenizer->advance();
    }
}

void CompilationEngine::writeSymbol(const std::string& symbol)
{
    if (m_tokenizer->tokenType() != JackTokenizer::TokenType::SYMBOL)
    {
        throw logic_error("Expected symbol.");
    }

    static const unordered_map<string, string> specialSymbolMap = {
        {"<", "&lt;"},
        {">", "&gt;"},
        {"\"", "&quot;"},
        {"&", "&amp;"},
    };

    string tokenSymbol = string(1, m_tokenizer->symbol());
    if (tokenSymbol != symbol)
    {
        throw logic_error("Expected: " + symbol + ", Received: "+ tokenSymbol);
    }

    if (specialSymbolMap.find(tokenSymbol) != specialSymbolMap.end())
    {
        tokenSymbol = specialSymbolMap.at(tokenSymbol);
    }

    writeLine("symbol", tokenSymbol);

    if (m_tokenizer->hasMoreTokens())
    {
        m_tokenizer->advance();
    }
}

void CompilationEngine::writeType()
{
    if (m_tokenizer->tokenType() == JackTokenizer::TokenType::KEYWORD)
    {
        static const unordered_map<JackTokenizer::KeyWord, string> typeMap = {
            {JackTokenizer::KeyWord::INT, "int"},
            {JackTokenizer::KeyWord::CHAR, "char"},
            {JackTokenizer::KeyWord::BOOLEAN, "boolean"},
        };

        JackTokenizer::KeyWord kw = m_tokenizer->keyWord();
        if (typeMap.find(kw) == typeMap.end())
        {
            throw logic_error("Invalid type.");
        }

        writeKeyword(typeMap.at(kw));
    }
    else if (m_tokenizer->tokenType() == JackTokenizer::TokenType::IDENTIFIER)
    {
        writeIdentifier();
    }
    else
    {
        throw logic_error("Invalid type.");
    }
}

void CompilationEngine::compileClass()
{
    writeOpenTag("class");
    writeKeyword("class");
    writeIdentifier();
    writeSymbol("{");

    auto isCVDPossible = [this]() -> bool {
        if (m_tokenizer->tokenType() == JackTokenizer::TokenType::KEYWORD)
        {
            JackTokenizer::KeyWord kw = m_tokenizer->keyWord();
            return kw == JackTokenizer::KeyWord::STATIC ||
                   kw == JackTokenizer::KeyWord::FIELD;
        }
        return false;
    };

    while (isCVDPossible())
    {
        compileClassVarDec(); // *multiple times
    }

    auto isSubPossible = [this]() -> bool {
        if (m_tokenizer->tokenType() == JackTokenizer::TokenType::KEYWORD)
        {
            JackTokenizer::KeyWord kw = m_tokenizer->keyWord();
            return kw == JackTokenizer::KeyWord::CONSTRUCTOR ||
                   kw == JackTokenizer::KeyWord::FUNCTION ||
                   kw == JackTokenizer::KeyWord::METHOD;
        }
        return false;
    };

    while (isSubPossible())
    {
        compileSubroutineDec(); // *multipletimes;
    }

    writeSymbol("}");
    writeCloseTag("class");
}

void CompilationEngine::compileClassVarDec()
{
    writeOpenTag("classVarDec");
    if (m_tokenizer->tokenType() != JackTokenizer::TokenType::KEYWORD)
    {
        throw logic_error("Expected keyword.");        
    }

    if (m_tokenizer->keyWord() == JackTokenizer::KeyWord::STATIC)
    {
        writeKeyword("static");
    }
    else if (m_tokenizer->keyWord() == JackTokenizer::KeyWord::FIELD)
    {
        writeKeyword("field");
    }
    else
    {
        throw logic_error("Token was neither 'static' nor 'field'.");
    }

    writeType();
    writeIdentifier();

    auto isMoreVarPossible = [this]() -> bool {
        if (m_tokenizer->tokenType() == JackTokenizer::TokenType::SYMBOL)
        {
            char symbol = m_tokenizer->symbol();
            return symbol == ',';
        }
        return false;
    };

    while (isMoreVarPossible())
    {
        writeSymbol(",");
        writeIdentifier();
    }

    writeSymbol(";");
    writeCloseTag("classVarDec");
}

void CompilationEngine::compileSubroutineDec()
{
    writeOpenTag("subroutineDec");

    if (m_tokenizer->tokenType() != JackTokenizer::TokenType::KEYWORD)
    {
        throw logic_error("Expected keyword.");        
    }

    JackTokenizer::KeyWord kw = m_tokenizer->keyWord();
    static const unordered_map<JackTokenizer::KeyWord, string> functionTypeMap = {
        {JackTokenizer::KeyWord::CONSTRUCTOR, "constructor"},
        {JackTokenizer::KeyWord::FUNCTION, "function"},
        {JackTokenizer::KeyWord::METHOD, "method"},
    };

    if (functionTypeMap.find(kw) == functionTypeMap.end())
    {
        throw logic_error("Invalid keyword.");
    }

    writeKeyword(functionTypeMap.at(kw));

    if (m_tokenizer->tokenType() == JackTokenizer::TokenType::KEYWORD)
    {
        if (m_tokenizer->keyWord() == JackTokenizer::KeyWord::VOID)
        {
            writeKeyword("void");
        }
        else
        {
            writeType();
        }
    }
    else if (m_tokenizer->tokenType() == JackTokenizer::TokenType::IDENTIFIER)
    {
        writeType();
    }
    else
    {
        throw logic_error("Invalid return type.");
    }

    writeIdentifier();
    writeSymbol("(");
    compileParameterList();
    writeSymbol(")");
    compileSubroutineBody();
    writeCloseTag("subroutineDec");
}

bool CompilationEngine::isType()
{
    if (m_tokenizer->tokenType() == JackTokenizer::TokenType::KEYWORD)
    {
        static const unordered_map<JackTokenizer::KeyWord, string> typeMap = {
            {JackTokenizer::KeyWord::INT, "int"},
            {JackTokenizer::KeyWord::CHAR, "char"},
            {JackTokenizer::KeyWord::BOOLEAN, "boolean"},
        };

        JackTokenizer::KeyWord kw = m_tokenizer->keyWord();
        if (typeMap.find(kw) != typeMap.end())
        {
            return true;
        }
    }
    else if (m_tokenizer->tokenType() == JackTokenizer::TokenType::IDENTIFIER)
    {
        return true;
    }

    return false;
}

void CompilationEngine::compileParameterList()
{
    writeOpenTag("parameterList");

    if (!isType())
    {
        writeCloseTag("parameterList");
        return;
    }

    writeType();
    writeIdentifier();
    
    auto isMoreArgsPossible = [this]() -> bool {
        if (m_tokenizer->tokenType() == JackTokenizer::TokenType::SYMBOL)
        {
            char symbol = m_tokenizer->symbol();
            return symbol == ',';
        }
        return false;
    };

    while (isMoreArgsPossible())
    {
        writeSymbol(",");
        writeType();
        writeIdentifier();
    }

    writeCloseTag("parameterList");
}

void CompilationEngine::compileSubroutineBody()
{
    writeOpenTag("subroutineBody");
    writeSymbol("{");

    auto isVarDecPossible = [this]() -> bool {
        if (m_tokenizer->tokenType() == JackTokenizer::TokenType::KEYWORD)
        {
            JackTokenizer::KeyWord kw = m_tokenizer->keyWord();
            return kw == JackTokenizer::KeyWord::VAR;
        }
        return false;
    };

    while (isVarDecPossible())
    {
        compileVarDec();
    }

    compileStatements();
    writeSymbol("}");
    writeCloseTag("subroutineBody");
}

void CompilationEngine::compileVarDec()
{
    writeOpenTag("varDec");
    
    writeKeyword("var");
    writeType();
    writeIdentifier();

    auto isMoreArgsPossible = [this]() -> bool {
        if (m_tokenizer->tokenType() == JackTokenizer::TokenType::SYMBOL)
        {
            char symbol = m_tokenizer->symbol();
            return symbol == ',';
        }
        return false;
    };

    while (isMoreArgsPossible())
    {
        writeSymbol(",");
        writeIdentifier();
    }

    writeSymbol(";");
    writeCloseTag("varDec");
}

void CompilationEngine::compileStatements()
{
    writeOpenTag("statements");

    auto isMoreStatementsPossible = [this]() -> bool {
        if (m_tokenizer->tokenType() == JackTokenizer::TokenType::KEYWORD)
        {
            JackTokenizer::KeyWord kw = m_tokenizer->keyWord();
            return kw == JackTokenizer::KeyWord::LET ||
                   kw == JackTokenizer::KeyWord::IF ||
                   kw == JackTokenizer::KeyWord::WHILE ||
                   kw == JackTokenizer::KeyWord::DO ||
                   kw == JackTokenizer::KeyWord::RETURN;
        }
        return false;
    };

    while (isMoreStatementsPossible())
    {
        JackTokenizer::KeyWord kw = m_tokenizer->keyWord();
        switch (kw)
        {
            case JackTokenizer::KeyWord::LET:
            {
                compileLet();
                break;
            }
            case JackTokenizer::KeyWord::IF:
            {
                compileIf();
                break;
            }
            case JackTokenizer::KeyWord::WHILE:
            {
                compileWhile();
                break;
            }
            case JackTokenizer::KeyWord::DO:
            {
                compileDo();
                break;
            }
            case JackTokenizer::KeyWord::RETURN:
            {
                compileReturn();
                break;
            }
        }
    }
    writeCloseTag("statements");
}

void CompilationEngine::compileLet()
{
    writeOpenTag("letStatement");
    writeKeyword("let");
    writeIdentifier();

    auto isArrPossible = [this]() -> bool {
        if (m_tokenizer->tokenType() == JackTokenizer::TokenType::SYMBOL)
        {
            return m_tokenizer->symbol() == '[';
        }
        return false;
    };

    if (isArrPossible())
    {
        writeSymbol("[");
        compileExpression();
        writeSymbol("]");
    }

    writeSymbol("=");
    compileExpression();
    writeSymbol(";");
    writeCloseTag("letStatement");
}

void CompilationEngine::compileIf()
{
    writeOpenTag("ifStatement");
    writeKeyword("if");
    writeSymbol("(");
    compileExpression();
    writeSymbol(")");
    writeSymbol("{");
    compileStatements();
    writeSymbol("}");

    auto isElsePossible = [this]() -> bool {
        if (m_tokenizer->tokenType() == JackTokenizer::TokenType::KEYWORD)
        {
            return m_tokenizer->keyWord() == JackTokenizer::KeyWord::ELSE;
        }
        return false;
    };

    if (isElsePossible())
    {
        writeKeyword("else");
        writeSymbol("{");
        compileStatements();
        writeSymbol("}");
    }
    writeCloseTag("ifStatement");
}

void CompilationEngine::compileWhile()
{
    writeOpenTag("whileStatement");
    writeKeyword("while");
    writeSymbol("(");
    compileExpression();
    writeSymbol(")");
    writeSymbol("{");
    compileStatements();
    writeSymbol("}");
    writeCloseTag("whileStatement");
}

void CompilationEngine::compileDo()
{
    writeOpenTag("doStatement");
    writeKeyword("do");
    if (m_tokenizer->tokenType() == JackTokenizer::TokenType::IDENTIFIER)
    {
        writeIdentifier();

        if (m_tokenizer->tokenType() == JackTokenizer::TokenType::SYMBOL)
        {
            char symbol = m_tokenizer->symbol();
            if (symbol == '(')
            {
                writeSymbol("(");
                compileExpressionList();
                writeSymbol(")");
            }
            else if (symbol == '.')
            {
                writeSymbol(".");
                writeIdentifier();
                writeSymbol("(");
                compileExpressionList();
                writeSymbol(")");
            }
        }
    }
    writeSymbol(";");
    writeCloseTag("doStatement");
}

void CompilationEngine::compileReturn()
{
    writeOpenTag("returnStatement");
    writeKeyword("return");

    auto isTermPossible = [this]() -> bool {
        if (m_tokenizer->tokenType() == JackTokenizer::TokenType::SYMBOL)
        {
            char symbol = m_tokenizer->symbol();
            return symbol != ';';
        }
        return true;
    };
    
    if (isTermPossible())
    {
        compileExpression();
    }

    writeSymbol(";");

    writeCloseTag("returnStatement");
}

void CompilationEngine::writeIntegerConstant()
{
    if (!isIntegerConstant())
    {
        throw logic_error("Expected integer constant.");
    }

    writeLine("integerConstant", to_string(m_tokenizer->intVal()));

    if (m_tokenizer->hasMoreTokens())
    {
        m_tokenizer->advance();
    }
}

bool CompilationEngine::isIntegerConstant()
{
    return m_tokenizer->tokenType() == JackTokenizer::TokenType::INT_CONST;
}

void CompilationEngine::writeStringConstant()
{
    if (!isStringConstant())
    {
        throw logic_error("Expected string constant.");
    }

    writeLine("stringConstant", m_tokenizer->stringVal());

    if (m_tokenizer->hasMoreTokens())
    {
        m_tokenizer->advance();
    }
}

bool CompilationEngine::isStringConstant()
{
    return m_tokenizer->tokenType() == JackTokenizer::TokenType::STRING_CONST;
}

void CompilationEngine::writeUnaryOp()
{
    if (!isUnaryOp())
    {
        throw logic_error("Expected unary operation.");
    }

    writeSymbol(string(1, m_tokenizer->symbol()));
}

bool CompilationEngine::isUnaryOp()
{
    if (m_tokenizer->tokenType() == JackTokenizer::TokenType::SYMBOL)
    {
        char symbol = m_tokenizer->symbol();
        return symbol == '-' || symbol == '~';
    }
    return false;
}

void CompilationEngine::writeOp()
{
    if (!isOp())
    {
        throw logic_error("Expected binary operation.");
    }

    writeSymbol(string(1, m_tokenizer->symbol()));
}

bool CompilationEngine::isOp()
{
    if (m_tokenizer->tokenType() == JackTokenizer::TokenType::SYMBOL)
    {
        char symbol = m_tokenizer->symbol();
        return symbol == '+' || symbol == '-' || symbol == '*' ||
               symbol == '/' || symbol == '&' || symbol == '|' ||
               symbol == '<' || symbol == '>' || symbol == '=';
    }
    return false;
}

void CompilationEngine::compileTerm()
{
    writeOpenTag("term");

    if (isIntegerConstant())
    {
        writeIntegerConstant();
    }
    else if (isStringConstant())
    {
        writeStringConstant();
    }
    else if (isKeywordConstant())
    {
        writeKeywordConstant();
    }
    else if (m_tokenizer->tokenType() == JackTokenizer::TokenType::IDENTIFIER)
    {
        writeIdentifier();

        if (m_tokenizer->tokenType() == JackTokenizer::TokenType::SYMBOL)
        {
            char symbol = m_tokenizer->symbol();
            if (symbol == '(')
            {
                writeSymbol("(");
                compileExpressionList();
                writeSymbol(")");
            }
            else if (symbol == '[')
            {
                writeSymbol("[");
                compileExpression();
                writeSymbol("]");
            }
            else if (symbol == '.')
            {
                writeSymbol(".");
                writeIdentifier();
                writeSymbol("(");
                compileExpressionList();
                writeSymbol(")");
            }
        }
    }
    else if (m_tokenizer->tokenType() == JackTokenizer::TokenType::SYMBOL)
    {
        char symbol = m_tokenizer->symbol();
        if (isUnaryOp())
        {
            writeUnaryOp();
            compileTerm();
        }
        else if (symbol == '(')
        {
            writeSymbol("(");
            compileExpression();
            writeSymbol(")");
        }
    }
    else
    {
        throw logic_error("Expected term." + m_tokenizer->keyWord());
    }

    writeCloseTag("term");
}

void CompilationEngine::writeKeywordConstant()
{
    if (!isKeywordConstant())
    {
        throw logic_error("Expected keyword constant.");
    }

    static const unordered_map<JackTokenizer::KeyWord, string> keywordMap = {
        {JackTokenizer::KeyWord::TRUE, "true"},
        {JackTokenizer::KeyWord::FALSE, "false"},
        {JackTokenizer::KeyWord::NILL, "null"},
        {JackTokenizer::KeyWord::THIS, "this"},
    };

    writeKeyword(keywordMap.at(m_tokenizer->keyWord()));
}

bool CompilationEngine::isKeywordConstant()
{
    if (m_tokenizer->tokenType() == JackTokenizer::TokenType::KEYWORD)
    {
        JackTokenizer::KeyWord kw = m_tokenizer->keyWord();
        return kw == JackTokenizer::KeyWord::TRUE ||
               kw == JackTokenizer::KeyWord::FALSE ||
               kw == JackTokenizer::KeyWord::NILL ||
               kw == JackTokenizer::KeyWord::THIS;
    }
    return false;
}

void CompilationEngine::compileExpression()
{
    writeOpenTag("expression");
    compileTerm();
    while (isOp())
    {
        writeOp();
        compileTerm();
    }
    writeCloseTag("expression");
}


int CompilationEngine::compileExpressionList()
{
    writeOpenTag("expressionList");
    int expressionCount = 0;

    auto isExpressionPossible = [this]() -> bool {
        if (m_tokenizer->tokenType() != JackTokenizer::TokenType::SYMBOL)
        {
            return true;
        }

        char symbol = m_tokenizer->symbol();
        return symbol != ')' && symbol != ']';
    };

    if (isExpressionPossible())
    {
        compileExpression();
        expressionCount++;

        auto isMorePossible = [this]() -> bool {
            if (m_tokenizer->tokenType() == JackTokenizer::TokenType::SYMBOL)
            {
                char symbol = m_tokenizer->symbol();
                return symbol == ',';
            }
            return false;
        };

        while (isMorePossible())
        {
            writeSymbol(",");
            compileExpression();
            expressionCount++;
        }
    }

    writeCloseTag("expressionList");
    return expressionCount;
}
