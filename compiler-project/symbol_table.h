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

        // Error and warning messages
        std::vector<std::string> errors;
        std::vector<std::string> warnings;

        void printSymbolTable(int programNum);

        // For optimization-- tracing
        int getScope() const { return currentScope; };

    private:
        std::vector<std::map<std::string, Symbol>> scopeStack;
        std::vector<Symbol> allSymbols;
        int currentScope;
};

#endif // SYMBOL_TABLE_H