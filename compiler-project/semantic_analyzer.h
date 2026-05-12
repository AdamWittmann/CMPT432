#ifndef SEMANTIC_ANALYZER_H
#define SEMANTIC_ANALYZER_H

#include "cst_node.h"
#include "symbol_table.h"
#include <vector>
#include <string>

class SemanticAnalyzer {
    public: 

        // Constructor
        SemanticAnalyzer(CSTNode* cst);
        // Deconstructor
        ~SemanticAnalyzer();

        //Will just call visit(cst) and return results
        CSTNode* analyze();

        std::vector<std::string> errors;
        std::vector<std::string> warnings;
        SymbolTable symbolTable;
        std::vector<std::string> traces;

        private:
            CSTNode* cst;

            CSTNode* visit(CSTNode* node);
            std::string resolveType(CSTNode* node);
            std::string collectCharList(CSTNode* node);
            void flattenStatementList(CSTNode* statementList, CSTNode* astBlock);

};

#endif 