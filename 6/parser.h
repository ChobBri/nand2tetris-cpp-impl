#include <string>
#include <iostream>
#include <fstream>
#include <unordered_set>
#include "symbolTable.h"

enum Command_Type {A_COMMAND, C_COMMAND, L_COMMAND};

class Parser
{
public:
    Parser(const std::string& fileName);
    ~Parser() = default;

    bool hasMoreCommands();
    void advance();
    Command_Type commandType();
    std::string symbol();
    std::string dest();
    std::string comp();
    std::string jump();

private:
    void fillSymbolTable(const std::string& fileName);
private:
    std::ifstream m_inputStream;
    Command_Type m_commandType;
    std::string m_symbol;
    std::string m_dest;
    std::string m_comp;
    std::string m_jump;
    SymbolTable symbolTable;
    uint16_t symbolIndex = 16;
    uint16_t numberOfLines = 0;
    uint16_t lineIndex = 0;
};
