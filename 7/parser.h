#include <fstream>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "commandType.h"

class Parser
{
public:
    Parser(const std::string& fileName);
    ~Parser() = default;

public:
    bool hasMoreCommands() const;
    void advance();
    CommandType commandType() const;
    std::string arg1() const;
    int arg2() const;

private:
    int numberOfCommands(const std::string& fileName);

private:
    std::ifstream m_inputStream;
    std::unordered_set<std::string> m_arithLogicCommandSet;
    std::unordered_map<std::string, CommandType> m_memoryCommandMap;
    std::unordered_set<std::string> m_memorySegmentSet;
    std::string m_arg1;
    int m_arg2;
    CommandType m_commandType;
    int m_lineCount;
    int m_lineIndex;
};
