#include <tuple>
#include <iostream>

#include "symbolTable.h"

using namespace std;

SymbolTable::SymbolTable() : kindCount({}) {}

void SymbolTable::reset()
{
    symbolTable.clear();
    kindCount.fill(0);
}

void SymbolTable::define(const string& name, const string& type, const IdentifierType identifierType)
{
    if (symbolTable.find(name) != symbolTable.end())
    {
        throw logic_error(name + " already exists.");
    }

    // cout << name << type << identifierType << kindCount[identifierType] << endl;
    SymbolTableElement elem = {name, type, identifierType, kindCount[identifierType]++};
    symbolTable.insert({name, elem});
}

int SymbolTable::varCount(const IdentifierType identifierType) const
{
    return kindCount.at(identifierType);
}

SymbolTable::IdentifierType SymbolTable::kindOf(const string& name) const
{
    return get<KIND>(symbolTable.at(name));
}

string SymbolTable::typeOf(const string& name) const
{
    return get<TYPE>(symbolTable.at(name));
}

int SymbolTable::indexOf(const string& name) const
{
    return get<INDEX>(symbolTable.at(name));
}

bool SymbolTable::contains(const std::string& name) const
{
    return symbolTable.find(name) != symbolTable.end();
}
