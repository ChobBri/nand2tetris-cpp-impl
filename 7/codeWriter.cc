#include <fstream>
#include <string>
#include <stdexcept>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <filesystem>

#include "commandType.h"
#include "codeWriter.h"

using namespace std;

CodeWriter::CodeWriter(const string& fileName)
{
    m_outputStream = ofstream(fileName, ios::trunc | ios::binary);

    if (!m_outputStream.is_open())
    {
        throw bad_exception();
    }

    m_arithLogicCommandSet = {"add", "sub", "neg", "eq", "gt", "lt", "and", "or", "not"};
    m_memoryCommandMap = {{"push", C_PUSH}, {"pop", C_POP}};
    m_memorySegmentSet = {"argument", "local", "static", "constant", "this", "that", "pointer", "temp"};
    m_fileNameStem = filesystem::path(fileName).stem().string();
}

void CodeWriter::unaryOp(const std::string& op, std::string& instructions)
{
    stringstream instrStream;
    instrStream << "@SP\n";
    instrStream << "AM=M-1\n";
    instrStream << "M=" << op << "M\n";
    instrStream << "@SP\n";
    instrStream << "M=M+1\n";
    instructions = instrStream.str();
}

void CodeWriter::binaryOp(const string& op, string& instructions)
{
    stringstream instrStream;
    instrStream << "@SP\n";
    instrStream << "AM=M-1\n";
    instrStream << "D=M\n";
    instrStream << "@SP\n";
    instrStream << "AM=M-1\n";
    instrStream << "M=M" << op << "D\n";
    instrStream << "@SP\n";
    instrStream << "M=M+1\n";
    instructions = instrStream.str();
}

void CodeWriter::compOp(const string& command, string& instructions)
{
    static int condCounter = 0;

    stringstream instrStream;
    instrStream << "@SP\n";
    instrStream << "AM=M-1\n";
    instrStream << "D=M\n";
    instrStream << "@SP\n";
    instrStream << "AM=M-1\n";
    instrStream << "D=M-D\n";
    instrStream << "@COND" << condCounter << "\n";
    instrStream << "D;J" << command << "\n";
    instrStream << "D=0\n";
    instrStream << "@END" << condCounter << "\n";
    instrStream << "0;JMP\n";
    instrStream << "(COND" << condCounter << ")\n";
    instrStream << "D=-1\n";
    instrStream << "(END" << condCounter << ")\n";
    instrStream << "@SP\n";
    instrStream << "A=M\n";
    instrStream << "M=D\n";
    instrStream << "@SP\n";
    instrStream << "M=M+1\n";
    condCounter++;
    instructions = instrStream.str();
}

void CodeWriter::writeArithmetic(const string& command)
{
    static const unordered_set<string> compSet = {"eq", "lt", "gt"};
    static const unordered_map<string, string> binaryMap = {{"add", "+"}, {"sub", "-"}, {"and", "&"}, {"or", "|"}};
    static const unordered_map<string, string> unaryMap = {{"not", "!"}, {"neg", "-"}};
    
    if (unaryMap.find(command) != unaryMap.end())
    {
        string instructions = "";
        unaryOp(unaryMap.at(command), instructions);
        m_outputStream << instructions;
    }
    else if (binaryMap.find(command) != binaryMap.end())
    {
        string instructions = "";
        binaryOp(binaryMap.at(command), instructions);
        m_outputStream << instructions;
    }
    else if (compSet.find(command) != compSet.end())
    {
        string instructions = "";
        string upperCommand = command;
        transform(upperCommand.begin(), upperCommand.end(), upperCommand.begin(), ::toupper);
        compOp(upperCommand, instructions);
        m_outputStream << instructions;
    }
}

string CodeWriter::stackPush()
{
    stringstream instrStream;
    instrStream << "@SP\n"; 
    instrStream << "A=M\n"; 
    instrStream << "M=D\n"; 
    instrStream << "@SP\n";
    instrStream << "M=M+1\n";
    return instrStream.str();
}
string CodeWriter::stackPop()
{
    stringstream instrStream;
    instrStream << "@SP\n";
    instrStream << "AM=M-1\n";
    instrStream << "D=M\n";
    return instrStream.str();
}

void CodeWriter::writePushPop(const CommandType command, string& segment, const int index)
{
    static const unordered_map<string, string> progPtrTranslator = {
        {"local", "LCL"},
        {"argument", "ARG"},
        {"this", "THIS"},
        {"that", "THAT"}
    };

    static const unordered_map<string, int> progSpecPtrTranslator = {
        {"temp", 5},
        {"pointer", 3}
    };

    if (command == C_PUSH)
    {
        if (segment == "constant")
        {
            stringstream instrStream;
            instrStream << "@" << index << "\n";
            instrStream << "D=A\n";
            instrStream << stackPush();
            m_outputStream << instrStream.str();
        }
        else if (progPtrTranslator.find(segment) != progPtrTranslator.end())
        {
            stringstream instrStream;
            instrStream << "@" << progPtrTranslator.at(segment) << "\n";
            instrStream << "D=M\n";
            instrStream << "@" << index << "\n"; 
            instrStream << "A=D+A\n";
            instrStream << "D=M\n";
            instrStream << stackPush();
            m_outputStream << instrStream.str();
        }
        else if (progSpecPtrTranslator.find(segment) != progSpecPtrTranslator.end())
        {
            stringstream instrStream;
            instrStream << "@R" << progSpecPtrTranslator.at(segment) + index << "\n";
            instrStream << "D=M\n";
            instrStream << stackPush();
            m_outputStream << instrStream.str();
        }
        else if (segment == "static")
        {
            stringstream instrStream;
            instrStream << "@" << m_fileNameStem << "." << index << "\n";
            instrStream << "D=M\n";
            instrStream << stackPush();
            m_outputStream << instrStream.str();
        }
    }
    else if (command == C_POP)
    {
        if (progPtrTranslator.find(segment) != progPtrTranslator.end())
        {
            stringstream instrStream;
            instrStream << "@" << progPtrTranslator.at(segment) << "\n";
            instrStream << "D=M\n";
            instrStream << "@" << index << "\n"; 
            instrStream << "D=D+A\n";
            instrStream << "@R13\n";
            instrStream << "M=D\n";  // save addr to R13
            instrStream << stackPop();
            instrStream << "@R13\n";
            instrStream << "A=M\n";
            instrStream << "M=D\n";  // save RAM[SP] to RAM[addr]
            m_outputStream << instrStream.str();
        }
        else if (progSpecPtrTranslator.find(segment) != progSpecPtrTranslator.end())
        {
            stringstream instrStream;
            instrStream << stackPop();
            instrStream << "@R" << progSpecPtrTranslator.at(segment) + index << "\n";
            instrStream << "M=D\n";
            m_outputStream << instrStream.str();
        }
        else if (segment == "static")
        {
            stringstream instrStream;
            instrStream << stackPop();
            instrStream << "@" << m_fileNameStem << "." << index << "\n";
            instrStream << "M=D\n";
            m_outputStream << instrStream.str();
        }
    }
}

void CodeWriter::close()
{
    m_outputStream.close();
}

