#include "symbol_table.h"

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

bool SymbolTable::declared(std::string name, std::string type){
    // does name already exist in current scope (redeclaration error)
    if(scopeStack.back().count(name)){
        errors.push_back("Error: redeclaration of variable '" + name + "' at scope " + std::to_string(currentScope));
        return false;
    }
    // add new symbol to current scope
    Symbol s;
    s.name = name;
    s.type = type;
    s.scope = currentScope;
    s.isInit = false;
    s.isUsed = false;
    scopeStack.back()[name] = s;
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


