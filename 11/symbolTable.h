#pragma once

#include <string>
#include <fstream>
#include <queue>
#include <unordered_map>
#include <tuple>
#include <array>

class SymbolTable
{
public:
    enum IdentifierType {STATIC, FIELD, ARG, VAR, NUM_OF_TYPES};

private:
    enum SymbolElementEntry {NAME, TYPE, KIND, INDEX, NUM_OF_ENTRIES};

public:
    SymbolTable();
    void reset();
    void define(const std::string& name, const std::string& type, const IdentifierType identifierType);
    int varCount(const IdentifierType identifierType) const;
    IdentifierType kindOf(const std::string& name) const;
    std::string typeOf(const std::string& name) const;
    int indexOf(const std::string& name) const;
    bool contains(const std::string& name) const;

private:
    typedef std::tuple<std::string, std::string, IdentifierType, int> SymbolTableElement;
    std::unordered_map<std::string, SymbolTableElement> symbolTable;
    std::array<int, NUM_OF_TYPES> kindCount;
};