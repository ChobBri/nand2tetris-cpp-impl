#include <fstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <cstdint>
#include <stack>
#include <sstream>

#include "commandType.h"
class CodeWriter
{
public:
    CodeWriter(const std::string& fileName);
    ~CodeWriter() = default;

public:
    void setFileName(const std::string& fileName);
    void writeArithmetic(const std::string& command);
    void writePushPop(const CommandType command, std::string& segment, const int index);
    void writeLabel(const std::string& label);
    void writeGoto(const std::string& command);
    void writeIf(const std::string& command);
    void writeFunction(const std::string& funcName, const int nVars);
    void writeCall(const std::string& funcName, const int nArgs);
    void writeReturn();
    void close();

private:
    void unaryOp(const std::string& op, std::string& instructions);
    void binaryOp(const std::string& op, std::string& instructions);
    void compOp(const std::string& command, std::string& instructions);
    void writeBoostrap();
    /**
     * Assumes D is the value to push
    */
    std::string stackPush();

    /**
     * Returns popped value into D
    */
    std::string stackPop();

private:
    std::ofstream m_outputStream;
    std::unordered_set<std::string> m_arithLogicCommandSet;
    std::unordered_map<std::string, CommandType> m_memoryCommandMap;
    std::unordered_set<std::string> m_memorySegmentSet;
    std::string m_fileNameStem;
    std::string m_currentFunctionName;
    int m_returnAddressLabelCounter;
};
