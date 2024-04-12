#pragma once

#include <string>
#include <fstream>
#include <queue>
#include <memory>
#include <unordered_set>

#include "jackTokenizer.h"

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
    void compileVarDec();
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
private:
    JackTokenizer* m_tokenizer;
    std::ofstream m_outputStream;
    int m_depth;
};