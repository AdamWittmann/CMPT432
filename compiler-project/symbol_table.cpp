#include "symbol_table.h"
#include <iomanip>
#include <iostream>
SymbolTable::SymbolTable():
    currentScope(0) {}

SymbolTable::~SymbolTable() {}

void SymbolTable::enterScope(){
    // push new empty map onto scopeStack
    scopeStack.push_back(std::map<std::string, Symbol>());
    // increment scope
    currentScope++;
}

void SymbolTable::exitScope(){
    // check for warnings on all symbols in scope
    for(auto& [name,symbol] : scopeStack.back()){
        if(!symbol.isUsed){
            warnings.push_back("WARNING: Variable '" + name + "' declared but never used at scope " + std::to_string(currentScope));
        }
        if(!symbol.isInit){
            warnings.push_back("WARNING: Variable '" + name + "' delcare but never initialized at scope " + std::to_string(currentScope));
        }
    }
    // pop
    scopeStack.pop_back();
    // decrement
    currentScope--;
}

bool SymbolTable::declared(std::string name, std::string type, int line, int column){
    // does name already exist in current scope (redeclaration error)
    if(scopeStack.back().count(name)){
        errors.push_back("Error: redeclaration of variable '" + name + "' at scope " + std::to_string(currentScope));
        return false;
    }
    // add new symbol to current scope
    Symbol s;
    s.name = name;
    s.type = type;
    s.line = line;
    s.column = column;
    s.scope = currentScope;
    s.isInit = false;
    s.isUsed = false;
    scopeStack.back()[name] = s;
    allSymbols.push_back(s);
    return true;
}
Symbol* SymbolTable::lookup(std::string name){
    // Search from top of stack downwards
    for(int i = scopeStack.size() - 1; i >= 0; i--){
        if(scopeStack[i].count(name)){
            return &scopeStack[i][name];
        }
    }
    // Return pointer to symbol if found
    // return nullptr if not found
    return nullptr;
}
bool SymbolTable::markInitialized(std::string name){
    // find symbol via lookup
    // set isInit = true
    // return false if not found
    Symbol* s = lookup(name);
    if(s == nullptr){
        return false;
    }
    s->isInit = true;
    return true;
}
bool SymbolTable::markUsed(std::string name){
    // find symbol via lookup
    // set isUsed = true
    // return false if not found
    Symbol* s = lookup(name);
    if(s == nullptr){
        return false;
    }
    s->isUsed = true;
    return true;
}

void SymbolTable::printSymbolTable(){
    // I used claude to do this
    std::cout << std::left
              << std::setw(10) << "Name"
              << std::setw(10) << "Type"
              << std::setw(8)  << "Scope"
              << std::setw(6)  << "Line"
              << std::setw(6)  << "Col"
              << std::endl;
    std::cout << std::string(40, '-') << std::endl;

    for(const auto& symbol : allSymbols){
        std::cout << std::left
                << std::setw(10) << symbol.name
                << std::setw(10) << symbol.type
                << std::setw(8)  << symbol.scope
                << std::setw(6)  << symbol.line
                << std::setw(6)  << symbol.column
        << std::endl;
    }

}

