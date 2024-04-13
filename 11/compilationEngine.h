#pragma once

#include <string>
#include <fstream>
#include <queue>
#include <memory>
#include <unordered_set>

#include "jackTokenizer.h"
#include "VMWriter.h"
#include "symbolTable.h"

class CompilationEngine
{
public:
    CompilationEngine(const std::string& inputFileName, const std::string& outputFileName);
    ~CompilationEngine();

public:
    void compile();

private:
    void compileClass();
    void compileClassVarDec();
    void compileSubroutineDec();
    void compileParameterList();
    void compileSubroutineBody();
    int compileVarDec();
    void compileStatements();
    void compileLet();
    void compileIf();
    void compileWhile();
    void compileDo();
    void compileReturn();
    void compileExpression();
    void compileTerm();
    int compileExpressionList();

private:
    void writeTabs(int tabCount);
    void writeLine(const std::string& tokenType, const std::string& token);
    void writeOpenTag(const std::string& token);
    void writeCloseTag(const std::string& token);

    void writeKeyword(const std::string& keyword);
    void writeIdentifier();
    void writeSymbol(const std::string& symbol);

    void writeType();
    bool isType();
    void writeUnaryOp();
    bool isUnaryOp();
    void writeOp();
    bool isOp();

    void writeIntegerConstant();
    bool isIntegerConstant();
    void writeStringConstant();
    bool isStringConstant();
    void writeKeywordConstant();
    bool isKeywordConstant();

    void writeVMPush(const std::string& identifier);
    void writeVMPop(const std::string& identifier);
private:
    JackTokenizer* m_tokenizer;
    VMWriter* m_vmWriter;

    SymbolTable::IdentifierType m_currentIdentifierType;
    std::string m_currentVarType;
    SymbolTable m_classSymbolTable;
    SymbolTable m_subroutineSymbolTable;
    std::string m_className;
    std::string m_subroutineName;
    JackTokenizer::KeyWord m_currentSubroutineType;
    int m_labelCounter;
    
};