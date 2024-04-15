#include <memory>
#include <exception>
#include <unordered_map>
#include <iostream>
#include <sstream>

#include "VMWriter.h"

using namespace std;

VMWriter::VMWriter(const std::string& outputFileName)
{
    m_outputStream = ofstream(outputFileName, ios::trunc | ios::binary);

    if (!m_outputStream.is_open())
    {
        throw runtime_error("Could not open file.");
    }
}

void VMWriter::writePush(const SegmentType segment, const int index)
{
    stringstream strStream;
    strStream << "push " << m_segmentMap.at(segment) << " " << index << endl;
    m_outputStream << strStream.str();
}

void VMWriter::writePop(const SegmentType segment, const int index)
{
    stringstream strStream;
    strStream << "pop " << m_segmentMap.at(segment) << " " << index << endl;
    m_outputStream << strStream.str();
}

void VMWriter::writeArithmetic(const ArithmeticCommand command)
{
    stringstream strStream;
    strStream << m_arithmeticMap.at(command) << endl;
    m_outputStream << strStream.str();
}

void VMWriter::writeLabel(const std::string& label)
{
    stringstream strStream;
    strStream << "label " << label << endl;
    m_outputStream << strStream.str();
}

void VMWriter::writeGoto(const std::string& label)
{
    stringstream strStream;
    strStream << "goto " << label << endl;
    m_outputStream << strStream.str();
}

void VMWriter::writeIf(const std::string& label)
{
    stringstream strStream;
    strStream << "if-goto " << label << endl;
    m_outputStream << strStream.str();
}

void VMWriter::writeCall(const std::string& name, const int nArgs)
{
    stringstream strStream;
    strStream << "call " << name << " " << nArgs << endl;
    m_outputStream << strStream.str();
}

void VMWriter::writeFunction(const std::string& name, const int nVars)
{
    stringstream strStream;
    strStream << "function " << name << " " << nVars << endl;
    m_outputStream << strStream.str();
}

void VMWriter::writeReturn()
{
    stringstream strStream;
    strStream << "return" << endl;
    m_outputStream << strStream.str();
}

void VMWriter::close()
{
    m_outputStream.close();
}
