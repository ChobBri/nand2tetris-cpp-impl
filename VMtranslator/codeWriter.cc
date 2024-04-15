#include <fstream>
#include <string>
#include <stdexcept>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <filesystem>
#include <array>

#include "commandType.h"
#include "codeWriter.h"

using namespace std;

CodeWriter::CodeWriter(const string& fileName)
    :
    m_currentFunctionName(""),
    m_returnAddressLabelCounter(0)
{
    m_outputStream = ofstream(fileName, ios::trunc | ios::binary);

    if (!m_outputStream.is_open())
    {
        throw bad_exception();
    }

    m_arithLogicCommandSet = {"add", "sub", "neg", "eq", "gt", "lt", "and", "or", "not"};
    m_memoryCommandMap = {{"push", C_PUSH}, {"pop", C_POP}};
    m_memorySegmentSet = {"argument", "local", "static", "constant", "this", "that", "pointer", "temp"};
    m_fileNameStem = "";

    writeBoostrap();
}

void CodeWriter::writeBoostrap()
{
    stringstream instrStream;
    instrStream << "@256\n";
    instrStream << "D=A\n";
    instrStream << "@SP\n";
    instrStream << "M=D\n";
    m_outputStream << instrStream.str();

    writeCall("Sys.init", 0);
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

void CodeWriter::writeLabel(const std::string& label)
{
    stringstream instrStream;
    instrStream << "(" << label << ")\n";
    m_outputStream << instrStream.str();
}

void CodeWriter::writeGoto(const std::string& label)
{
    stringstream instrStream;
    instrStream << "@" << label << "\n";
    instrStream << "0;JMP\n";
    m_outputStream << instrStream.str();
}

void CodeWriter::writeIf(const std::string& label)
{
    stringstream instrStream;
    instrStream << stackPop();
    instrStream << "@" << label << "\n";
    instrStream << "D;JNE\n";
    m_outputStream << instrStream.str();
}

void CodeWriter::writeFunction(const std::string& funcName, const int nVars)
{
    m_currentFunctionName = funcName;
    m_returnAddressLabelCounter = 0;

    stringstream instrStream;
    instrStream << "(" << funcName << ")\n";
    for (int i = 0; i < nVars; i++)
    {
        instrStream << "@0\n";
        instrStream << "D=A\n";
        instrStream << stackPush();
    }
    m_outputStream << instrStream.str();
}

void CodeWriter::writeCall(const std::string& funcName, const int nArgs)
{
    stringstream instrStream;

    const std::string labelName = m_currentFunctionName + "$ret."s + to_string(m_returnAddressLabelCounter++);

    // Save return address
    instrStream << "@" << labelName << "\n";
    instrStream << "D=A\n";
    instrStream << stackPush();

    // Save segments
    static const array<const string, 4> saveSegments = {"LCL", "ARG", "THIS", "THAT"};
    for (const string& segment : saveSegments)
    {
        instrStream << "@" << segment << "\n";
        instrStream << "D=M\n";
        instrStream << stackPush();
    }

    // Reposition ARG
    instrStream << "@" << nArgs + 5 << "\n";
    instrStream << "D=A\n";
    instrStream << "@SP\n";
    instrStream << "D=M-D\n";
    instrStream << "@ARG\n";
    instrStream << "M=D\n";

    // Reposition LCL
    instrStream << "@SP\n";
    instrStream << "D=M\n";
    instrStream << "@LCL\n";
    instrStream << "M=D\n";

    // Jump to function
    instrStream << "@" << funcName << "\n";
    instrStream << "0;JMP\n";

    // Return label
    instrStream << "(" << labelName << ")\n";

    m_outputStream << instrStream.str();
}

void CodeWriter::writeReturn()
{
    stringstream instrStream;

    // place endFrame at R13
    instrStream << "@LCL\n";
    instrStream << "D=M\n";
    instrStream << "@R13\n";
    instrStream << "M=D\n";  

    // place retAddr at R14
    instrStream << "@5\n";
    instrStream << "A=D-A\n";
    instrStream << "D=M\n";
    instrStream << "@R14\n";
    instrStream << "M=D\n";  

    // Place returned value at ARG
    instrStream << stackPop();
    instrStream << "@ARG\n";
    instrStream << "A=M\n";
    instrStream << "M=D\n";

    // SP placed after return value
    instrStream << "D=A\n"; 
    instrStream << "@SP\n";
    instrStream << "M=D+1\n"; 

    // Restore segments
    static const array<const string, 4> restoreSegments = {"THAT", "THIS", "ARG", "LCL"};
    for (const string& segment : restoreSegments)
    {
        instrStream << "@R13\n";
        instrStream << "AM=M-1\n";
        instrStream << "D=M\n";
        instrStream << "@" << segment << "\n";
        instrStream << "M=D\n";
    }

    // Jump to return address
    instrStream << "@R14\n";
    instrStream << "A=M\n";
    instrStream << "0;JMP\n";

    m_outputStream << instrStream.str();
}

void CodeWriter::setFileName(const std::string& fileName)
{
    m_fileNameStem = filesystem::path(fileName).stem().string();
}

void CodeWriter::close()
{
    m_outputStream.close();
}
