#pragma once

#include <string>
#include <fstream>
#include <queue>
#include <memory>
#include <unordered_map>

class VMWriter
{
public:
    enum SegmentType {CONSTANT, ARGUMENT, LOCAL, STATIC, THIS, THAT, POINTER, TEMP};
    enum ArithmeticCommand {ADD, SUB, NEG, EQ, GT, LT, AND, OR, NOT};

public:
    VMWriter(const std::string& outputFileName);

public:
    void writePush(const SegmentType segment, const int index);
    void writePop(const SegmentType segment, const int index);
    void writeArithmetic(const ArithmeticCommand command);
    void writeLabel(const std::string& label);
    void writeGoto(const std::string& label);
    void writeIf(const std::string& label);
    void writeCall(const std::string& name, const int nArgs);
    void writeFunction(const std::string& name, const int nVars);
    void writeReturn();
    void close();

private:
    std::ofstream m_outputStream;

private:
    std::unordered_map<SegmentType, std::string> m_segmentMap = {
        {CONSTANT, "constant"},
        {ARGUMENT, "argument"},
        {LOCAL, "local"},
        {STATIC, "static"},
        {THIS, "this"},
        {THAT, "that"},
        {POINTER, "pointer"},
        {TEMP, "temp"},
    };

    std::unordered_map<ArithmeticCommand, std::string> m_arithmeticMap = {
        {ADD, "add"},
        {SUB, "sub"},
        {NEG, "neg"},
        {EQ, "eq"},
        {GT, "gt"},
        {LT, "lt"},
        {AND, "and"},
        {OR, "or"},
        {NOT, "not"},
    };
};