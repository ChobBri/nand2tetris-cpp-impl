#include <memory>
#include <exception>
#include <unordered_map>
#include <iostream>

#include "compilationEngine.h"
#include "jackTokenizer.h"

using namespace std;

CompilationEngine::CompilationEngine(const std::string& inputFileName, const std::string& outputFileName)
    :
    m_labelCounter(0)
{
    m_tokenizer = new JackTokenizer(inputFileName);
    m_tokenizer->advance();

    m_vmWriter = new VMWriter(outputFileName);
}

CompilationEngine::~CompilationEngine()
{
    delete m_tokenizer;
    delete m_vmWriter;
}

void CompilationEngine::compile()
{
    compileClass();
    m_vmWriter->close();
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

        m_currentVarType = typeMap.at(kw);
        writeKeyword(typeMap.at(kw));
    }
    else if (m_tokenizer->tokenType() == JackTokenizer::TokenType::IDENTIFIER)
    {
        m_currentVarType = m_tokenizer->identifier();
        writeIdentifier();
    }
    else
    {
        throw logic_error("Invalid type.");
    }
}

void CompilationEngine::compileClass()
{
    writeKeyword("class");
    m_className = m_tokenizer->identifier();
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
        m_subroutineSymbolTable.reset();
        compileSubroutineDec(); // *multipletimes;
    }

    writeSymbol("}");
}

void CompilationEngine::compileClassVarDec()
{
    if (m_tokenizer->tokenType() != JackTokenizer::TokenType::KEYWORD)
    {
        throw logic_error("Expected keyword.");        
    }

    if (m_tokenizer->keyWord() == JackTokenizer::KeyWord::STATIC)
    {
        m_currentIdentifierType = SymbolTable::IdentifierType::STATIC;
        writeKeyword("static");
    }
    else if (m_tokenizer->keyWord() == JackTokenizer::KeyWord::FIELD)
    {
        m_currentIdentifierType = SymbolTable::IdentifierType::FIELD;
        writeKeyword("field");
    }
    else
    {
        throw logic_error("Token was neither 'static' nor 'field'.");
    }

    writeType();
    m_classSymbolTable.define(m_tokenizer->identifier(), m_currentVarType, m_currentIdentifierType);
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
        m_classSymbolTable.define(m_tokenizer->identifier(), m_currentVarType, m_currentIdentifierType);
        writeIdentifier();
    }

    writeSymbol(";");
}

void CompilationEngine::compileSubroutineDec()
{
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

    m_currentSubroutineType = kw;

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

    m_subroutineName = m_tokenizer->identifier();
    writeIdentifier();
    writeSymbol("(");
    compileParameterList();
    writeSymbol(")");
    compileSubroutineBody();
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
    m_currentIdentifierType = SymbolTable::IdentifierType::ARG;
    if (!isType())
    {
        return;
    }

    if (m_currentSubroutineType == JackTokenizer::KeyWord::METHOD)
    {
        m_subroutineSymbolTable.define("this", m_className, m_currentIdentifierType);
    }

    writeType();
    m_subroutineSymbolTable.define(m_tokenizer->identifier(), m_currentVarType, m_currentIdentifierType);
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
        m_subroutineSymbolTable.define(m_tokenizer->identifier(), m_currentVarType, m_currentIdentifierType);
        writeIdentifier();
    }
}

void CompilationEngine::compileSubroutineBody()
{
    writeSymbol("{");

    int varCount = 0;

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
        varCount += compileVarDec();
    }

    if (m_currentSubroutineType == JackTokenizer::KeyWord::CONSTRUCTOR)
    {
        m_vmWriter->writeFunction(m_className + ".new", varCount);
        m_vmWriter->writePush(VMWriter::SegmentType::CONSTANT, m_classSymbolTable.varCount(SymbolTable::IdentifierType::FIELD));
        m_vmWriter->writeCall("Memory.alloc", 1);
        m_vmWriter->writePop(VMWriter::SegmentType::POINTER, 0);
    }
    else if (m_currentSubroutineType == JackTokenizer::KeyWord::METHOD)
    {
        m_vmWriter->writeFunction(m_className + "." + m_subroutineName, varCount);
        m_vmWriter->writePush(VMWriter::SegmentType::ARGUMENT, 0);
        m_vmWriter->writePop(VMWriter::SegmentType::POINTER, 0);
    }
    else if (m_currentSubroutineType == JackTokenizer::KeyWord::FUNCTION)
    {
        m_vmWriter->writeFunction(m_className + "." + m_subroutineName, varCount);
    }
    
    compileStatements();

    writeSymbol("}");
}

int CompilationEngine::compileVarDec()
{
    int varCount = 0;
    m_currentIdentifierType = SymbolTable::IdentifierType::VAR;
    writeKeyword("var");
    writeType();
    m_subroutineSymbolTable.define(m_tokenizer->identifier(), m_currentVarType, m_currentIdentifierType);
    writeIdentifier();
    varCount++;

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
        m_subroutineSymbolTable.define(m_tokenizer->identifier(), m_currentVarType, m_currentIdentifierType);
        writeIdentifier();
        varCount++;
    }

    writeSymbol(";");
    return varCount;
}

void CompilationEngine::compileStatements()
{
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
}

void CompilationEngine::writeVMPush(const string& identifier)
{
    static const unordered_map<SymbolTable::IdentifierType, VMWriter::SegmentType> typeMap = {
        {SymbolTable::IdentifierType::STATIC, VMWriter::SegmentType::STATIC},
        {SymbolTable::IdentifierType::FIELD, VMWriter::SegmentType::THIS},
        {SymbolTable::IdentifierType::ARG, VMWriter::SegmentType::ARGUMENT},
        {SymbolTable::IdentifierType::VAR, VMWriter::SegmentType::LOCAL},
    };

    if (m_subroutineSymbolTable.contains(identifier))
    {
        SymbolTable::IdentifierType itype =  m_subroutineSymbolTable.kindOf(identifier);
        int index = m_subroutineSymbolTable.indexOf(identifier);
        m_vmWriter->writePush(typeMap.at(itype), index);
    }
    else if (m_classSymbolTable.contains(identifier))
    {
        SymbolTable::IdentifierType itype =  m_classSymbolTable.kindOf(identifier);
        int index = m_classSymbolTable.indexOf(identifier);
        m_vmWriter->writePush(typeMap.at(itype), index);
    }
    else
    {
        throw logic_error("Cannot find symbol: " + identifier);
    }
}

void CompilationEngine::writeVMPop(const string& identifier)
{
    static const unordered_map<SymbolTable::IdentifierType, VMWriter::SegmentType> typeMap = {
        {SymbolTable::IdentifierType::STATIC, VMWriter::SegmentType::STATIC},
        {SymbolTable::IdentifierType::FIELD, VMWriter::SegmentType::THIS},
        {SymbolTable::IdentifierType::ARG, VMWriter::SegmentType::ARGUMENT},
        {SymbolTable::IdentifierType::VAR, VMWriter::SegmentType::LOCAL},
    };

    if (m_subroutineSymbolTable.contains(identifier))
    {
        SymbolTable::IdentifierType itype =  m_subroutineSymbolTable.kindOf(identifier);
        int index = m_subroutineSymbolTable.indexOf(identifier);
        m_vmWriter->writePop(typeMap.at(itype), index);
    }
    else if (m_classSymbolTable.contains(identifier))
    {
        SymbolTable::IdentifierType itype =  m_classSymbolTable.kindOf(identifier);
        int index = m_classSymbolTable.indexOf(identifier);
        m_vmWriter->writePop(typeMap.at(itype), index);
    }
    else
    {
        throw logic_error("Cannot find symbol: " + identifier);
    }
}

void CompilationEngine::compileLet()
{
    writeKeyword("let");

    string identifier = m_tokenizer->identifier();
    writeIdentifier();

    auto isArrPossible = [this]() -> bool {
        if (m_tokenizer->tokenType() == JackTokenizer::TokenType::SYMBOL)
        {
            return m_tokenizer->symbol() == '[';
        }
        return false;
    };

    bool isArr = false;

    if (isArrPossible())
    {
        isArr = true;
        writeSymbol("[");
        writeVMPush(identifier);
        compileExpression();
        m_vmWriter->writeArithmetic(VMWriter::ArithmeticCommand::ADD);
        m_vmWriter->writePop(VMWriter::SegmentType::POINTER, 1);
        writeSymbol("]");
    }

    writeSymbol("=");
    compileExpression();
    if (isArr)
    {
        m_vmWriter->writePop(VMWriter::SegmentType::THAT, 0);
    }
    else
    {
        writeVMPop(identifier);
    }
    writeSymbol(";");
}

void CompilationEngine::compileIf()
{
    writeKeyword("if");
    writeSymbol("(");
    compileExpression();
    writeSymbol(")");
    int currentLabelCounter = m_labelCounter;
    m_labelCounter += 2;
    m_vmWriter->writeArithmetic(VMWriter::ArithmeticCommand::NOT);
    m_vmWriter->writeIf(m_className + ".L" + to_string(currentLabelCounter));
    writeSymbol("{");
    compileStatements();
    m_vmWriter->writeGoto(m_className + ".L" + to_string(currentLabelCounter + 1));
    writeSymbol("}");

    m_vmWriter->writeLabel(m_className + ".L" + to_string(currentLabelCounter));
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
    m_vmWriter->writeLabel(m_className + ".L" + to_string(currentLabelCounter + 1));

}

void CompilationEngine::compileWhile()
{
    writeKeyword("while");
    writeSymbol("(");
    int currentLabelCounter = m_labelCounter;
    m_labelCounter += 2;
    m_vmWriter->writeLabel(m_className + ".L" + to_string(currentLabelCounter));
    compileExpression();
    m_vmWriter->writeArithmetic(VMWriter::ArithmeticCommand::NOT);
    m_vmWriter->writeIf(m_className + ".L" + to_string(currentLabelCounter + 1));
    writeSymbol(")");
    writeSymbol("{");
    compileStatements();
    writeSymbol("}");
    m_vmWriter->writeGoto(m_className + ".L" + to_string(currentLabelCounter));
    m_vmWriter->writeLabel(m_className + ".L" + to_string(currentLabelCounter + 1));
}

void CompilationEngine::compileDo()
{
    writeKeyword("do");
    if (m_tokenizer->tokenType() == JackTokenizer::TokenType::IDENTIFIER)
    {
        string identifier = m_tokenizer->identifier();
        writeIdentifier();

        if (m_tokenizer->tokenType() == JackTokenizer::TokenType::SYMBOL)
        {
            char symbol = m_tokenizer->symbol();
            if (symbol == '(')
            {
                writeSymbol("(");
                m_vmWriter->writePush(VMWriter::SegmentType::POINTER, 0);
                int nArgs = compileExpressionList();
                writeSymbol(")");
                m_vmWriter->writeCall(m_className + "." + identifier, nArgs + 1);
            }
            else if (symbol == '.')
            {
                static const unordered_map<SymbolTable::IdentifierType, VMWriter::SegmentType> typeMap = {
                    {SymbolTable::IdentifierType::STATIC, VMWriter::SegmentType::STATIC},
                    {SymbolTable::IdentifierType::FIELD, VMWriter::SegmentType::THIS},
                    {SymbolTable::IdentifierType::ARG, VMWriter::SegmentType::ARGUMENT},
                    {SymbolTable::IdentifierType::VAR, VMWriter::SegmentType::LOCAL},
                };

                string subroutineName;
                int nArgs = 0;
                writeSymbol(".");
                if (m_subroutineSymbolTable.contains(identifier))
                {
                    subroutineName = m_subroutineSymbolTable.typeOf(identifier) + "." + m_tokenizer->identifier();
                    SymbolTable::IdentifierType itype = m_subroutineSymbolTable.kindOf(identifier);
                    m_vmWriter->writePush(typeMap.at(itype), m_subroutineSymbolTable.indexOf(identifier));
                    nArgs++;
                }
                else if (m_classSymbolTable.contains(identifier))
                {
                    subroutineName = m_classSymbolTable.typeOf(identifier) + "." + m_tokenizer->identifier();
                    SymbolTable::IdentifierType itype = m_classSymbolTable.kindOf(identifier);
                    m_vmWriter->writePush(typeMap.at(itype), m_classSymbolTable.indexOf(identifier));
                    nArgs++;
                }
                else
                {
                    subroutineName = identifier + "." + m_tokenizer->identifier();
                }

                writeIdentifier();
                writeSymbol("(");
                nArgs += compileExpressionList();
                writeSymbol(")");
                m_vmWriter->writeCall(subroutineName, nArgs);
            }
        }
    }
    writeSymbol(";");
    m_vmWriter->writePop(VMWriter::SegmentType::TEMP, 0);
}

void CompilationEngine::compileReturn()
{
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
    else
    {
        m_vmWriter->writePush(VMWriter::SegmentType::CONSTANT, 0);
    }

    writeSymbol(";");
    m_vmWriter->writeReturn();
}

void CompilationEngine::writeIntegerConstant()
{
    if (!isIntegerConstant())
    {
        throw logic_error("Expected integer constant.");
    }

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
    if (isIntegerConstant())
    {
        m_vmWriter->writePush(VMWriter::SegmentType::CONSTANT, m_tokenizer->intVal());
        writeIntegerConstant();
    }
    else if (isStringConstant())
    {
        string strConstant = m_tokenizer->stringVal();
        int strLen = strConstant.size();
        m_vmWriter->writePush(VMWriter::SegmentType::CONSTANT, strLen);
        m_vmWriter->writeCall("String.new", 1);

        for (char c : strConstant)
        {
            m_vmWriter->writePush(VMWriter::SegmentType::CONSTANT, c);
            m_vmWriter->writeCall("String.appendChar", 2);
        }
        writeStringConstant();
    }
    else if (isKeywordConstant())
    {
        JackTokenizer::KeyWord kw = m_tokenizer->keyWord();
        if (kw == JackTokenizer::KeyWord::TRUE)
        {
            m_vmWriter->writePush(VMWriter::SegmentType::CONSTANT, 1);
            m_vmWriter->writeArithmetic(VMWriter::ArithmeticCommand::NEG);
        }
        else if (kw == JackTokenizer::KeyWord::FALSE)
        {
            m_vmWriter->writePush(VMWriter::SegmentType::CONSTANT, 0);
        }
        else if (kw == JackTokenizer::KeyWord::NILL)
        {
            m_vmWriter->writePush(VMWriter::SegmentType::CONSTANT, 0);
        }
        else if (kw == JackTokenizer::KeyWord::THIS)
        {
            m_vmWriter->writePush(VMWriter::SegmentType::POINTER, 0);
        }

        writeKeywordConstant();
    }
    else if (m_tokenizer->tokenType() == JackTokenizer::TokenType::IDENTIFIER)
    {
        string identifier = m_tokenizer->identifier();
        writeIdentifier();

        if (m_tokenizer->tokenType() == JackTokenizer::TokenType::SYMBOL)
        {
            char symbol = m_tokenizer->symbol();
            if (symbol == '(')  // Subroutine call
            {
                writeSymbol("(");
                m_vmWriter->writePush(VMWriter::SegmentType::POINTER, 0);
                int nArgs = compileExpressionList();
                writeSymbol(")");
                m_vmWriter->writeCall(m_className + "." + identifier, nArgs + 1);
            }
            else if (symbol == '[')  // Array access
            {
                writeSymbol("[");
                m_vmWriter->writePush(VMWriter::SegmentType::POINTER, 1);
                m_vmWriter->writePop(VMWriter::SegmentType::TEMP, 1);
                writeVMPush(identifier);
                compileExpression();
                m_vmWriter->writeArithmetic(VMWriter::ArithmeticCommand::ADD);
                m_vmWriter->writePop(VMWriter::SegmentType::POINTER, 1);
                m_vmWriter->writePush(VMWriter::SegmentType::THAT, 0);
                m_vmWriter->writePop(VMWriter::SegmentType::TEMP, 0);  // save array value
                m_vmWriter->writePush(VMWriter::SegmentType::TEMP, 1);
                m_vmWriter->writePop(VMWriter::SegmentType::POINTER, 1); // load previous pointer
                m_vmWriter->writePush(VMWriter::SegmentType::TEMP, 0);   // return array value
                writeSymbol("]");
            }
            else if (symbol == '.')  // Member subroutine access
            {
                static const unordered_map<SymbolTable::IdentifierType, VMWriter::SegmentType> typeMap = {
                    {SymbolTable::IdentifierType::STATIC, VMWriter::SegmentType::STATIC},
                    {SymbolTable::IdentifierType::FIELD, VMWriter::SegmentType::THIS},
                    {SymbolTable::IdentifierType::ARG, VMWriter::SegmentType::ARGUMENT},
                    {SymbolTable::IdentifierType::VAR, VMWriter::SegmentType::LOCAL},
                };

                string subroutineName;
                int nArgs = 0;
                writeSymbol(".");
                if (m_subroutineSymbolTable.contains(identifier))
                {
                    subroutineName = m_subroutineSymbolTable.typeOf(identifier) + "." + m_tokenizer->identifier();
                    SymbolTable::IdentifierType itype = m_subroutineSymbolTable.kindOf(identifier);
                    m_vmWriter->writePush(typeMap.at(itype), m_subroutineSymbolTable.indexOf(identifier));
                    nArgs++;
                }
                else if (m_classSymbolTable.contains(identifier))
                {
                    subroutineName = m_classSymbolTable.typeOf(identifier) + "." + m_tokenizer->identifier();
                    SymbolTable::IdentifierType itype = m_classSymbolTable.kindOf(identifier);
                    m_vmWriter->writePush(typeMap.at(itype), m_classSymbolTable.indexOf(identifier));
                    nArgs++;
                }
                else
                {
                    subroutineName = identifier + "." + m_tokenizer->identifier();
                }

                writeIdentifier();
                writeSymbol("(");
                nArgs += compileExpressionList();
                writeSymbol(")");
                m_vmWriter->writeCall(subroutineName, nArgs);
            }
            else
            {
                writeVMPush(identifier);  // variable access
            }
        }
    }
    else if (m_tokenizer->tokenType() == JackTokenizer::TokenType::SYMBOL)
    {
        char symbol = m_tokenizer->symbol();
        if (isUnaryOp())
        {
            char c = m_tokenizer->symbol();
            writeUnaryOp();
            compileTerm();
            VMWriter::ArithmeticCommand ac = c == '~' ? VMWriter::ArithmeticCommand::NOT : VMWriter::ArithmeticCommand::NEG;
            m_vmWriter->writeArithmetic(ac);
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
    compileTerm();
    while (isOp())
    {
        static const unordered_map<char, VMWriter::ArithmeticCommand> symbolCommandMap = {
            {'+', VMWriter::ArithmeticCommand::ADD},
            {'-', VMWriter::ArithmeticCommand::SUB},
            {'=', VMWriter::ArithmeticCommand::EQ},
            {'&', VMWriter::ArithmeticCommand::AND},
            {'>', VMWriter::ArithmeticCommand::GT},
            {'<', VMWriter::ArithmeticCommand::LT},
            {'|', VMWriter::ArithmeticCommand::OR},
        };

        static const unordered_map<char, string> symbolSubroutineMap = {
            {'*', "Math.multiply"},
            {'/', "Math.divide"},
        };
        char c = m_tokenizer->symbol();
        writeOp();
        compileTerm();
        if (symbolCommandMap.find(c) != symbolCommandMap.end())
        {
            m_vmWriter->writeArithmetic(symbolCommandMap.at(c));
        }
        else if (symbolSubroutineMap.find(c) != symbolSubroutineMap.end())
        {
            m_vmWriter->writeCall(symbolSubroutineMap.at(c), 2);
        }
    }
}


int CompilationEngine::compileExpressionList()
{
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

    return expressionCount;
}
