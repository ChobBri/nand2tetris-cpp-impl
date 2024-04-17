#include <string>
#include <cstdint>
#include <map>

class SymbolTable
{
public:
    SymbolTable();
    ~SymbolTable() = default;

    void addEntry(const std::string& symbol, const uint16_t address);
    bool contains(const std::string& symbol) const;
    uint16_t getAddress(const std::string& symbol) const;

private:
    std::map<std::string, uint16_t> symbolTable;
};