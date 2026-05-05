#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <string>
#include <map>
#include <vector>

struct Symbol{
    std::string name;
    std::string type;
    int scope;
    bool isInit;
    bool isUsed;
    int line;
    int column;
};

class SymbolTable{
    public:
        // Constructor
        SymbolTable();
        // Deconstructor
        ~SymbolTable();

        // Scope
        void enterScope();
        void exitScope();

        // Symbol Management
        bool declared(std::string name, std::string type, int line, int column);
        Symbol* lookup(std::string name);
        bool markInitialized(std::string name);
        bool markUsed(std::string name);
        // Duplicate Symbols for the print out
        std::vector<Symbol> allSymbols;

        // Error and warning messages
        std::vector<std::string> errors;
        std::vector<std::string> warnings;

        //Verbose print of symbol table
        void printSymbolTable();
    private:
        std::vector<std::map<std::string, Symbol>> scopeStack;
        int currentScope;
};

#endif // SYMBOL_TABLE_H