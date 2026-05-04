#include "semantic_analyzer.h"

// Constructor
SemanticAnalyzer::SemanticAnalyzer(CSTNode* cst):
    cst(cst) {}

// Deconstructor
SemanticAnalyzer::~SemanticAnalyzer() {}

// Analyze - entry point
CSTNode* SemanticAnalyzer:: analyze() {
    return visit(cst);
}

// Visit
// This builds ast nodes if the child isnt null
CSTNode* SemanticAnalyzer::visit(CSTNode* node){
    if(node == nullptr) return nullptr;

    std::string label = node->label;

    if(label == "Block"){
        CSTNode* astNode = new CSTNode("Block");
        symbolTable.enterScope();

        for(CSTNode* child : node->children){
            CSTNode* result = visit(child);

            if(result != nullptr){
                astNode->addChild(result);
            }
        }
        symbolTable.exitScope();
        return astNode;

    }
    if(label == "VarDecl"){
        // get type from first child's token value
        std::string type = node->children[0]->token->value;
        // get name from second child's first child's token value
        std::string name = node->children[1]->children[0]->token->value;

        // declare in symbol table
        if(!symbolTable.declared(name,type)){
            errors.push_back("Error: ");
        }

        // build node
        CSTNode* astNode = new CSTNode("VarDecl");
        astNode->addChild(new CSTNode(type));
        astNode->addChild(new CSTNode(name));
        return astNode;
    }
    if(label == "AssignmentStatement"){
        std::string id = node->children[0]->token->value;
        

        // check if declared
        Symbol* sym = symbolTable.lookup(id);
        if(sym == nullptr){
            errors.push_back("Error: undeclared variable '" + id + "' at (" +
                std::to_string(node->children[0]->token->line) + "," +
                std::to_string(node->children[0]->token->column) + ")");
        } else {
            // resolve type of expression
            std::string exprType = resolveType(node->children[1]);
            // check type matches
            if(sym->type != exprType){
                errors.push_back("Error: type mismatch for variable '" + id + 
                    "'. Expected " + sym->type + " but got " + exprType);
            }
            symbolTable.markInitialized(id);
        }
        // build node
        CSTNode* astNode = new CSTNode("AssignmentStatement");
        astNode->addChild(new CSTNode(id));
        astNode->addChild(visit(node->children[1]));
        return astNode;
    }

    if(label == "PrintStatement"){
        CSTNode* expr = node->children[2];

        // if expr is an ID mark it as used
        if(expr->children[0]->label == "Id"){
            std::string name = expr->children[0]->children[0]->token->value;
            Symbol* sym = symbolTable.lookup(name);
            // if not pointing to definition return undeclared
            if(sym == nullptr){
                errors.push_back("Error: undeclared variable '" + name + "' at (" +
                std::to_string(expr->children[0]->children[0]->token->line) + "," +
                std::to_string(expr->children[0]->children[0]->token->column) + ")");
            } else {
                if(!sym->isInit){
                    warnings.push_back("WARNING: variable '" + name + "' used without being initialized");
                }
                // mark used if used
                symbolTable.markUsed(name);
            }

        }

        //build AST node
        CSTNode* astNode = new CSTNode("PrintStatement");
        astNode->addChild(visit(expr));
        return astNode;
    }
    return nullptr;
}

std::string SemanticAnalyzer::resolveType(CSTNode* node){
    if(node == nullptr) return "unknown";

    std::string label = node->label;

    if(label == "IntExpr") return "int";
    if(label == "StringExpr") return "string";
    if(label == "BooleanExpr") return "boolean";
    if(label == "Expr") return resolveType(node->children[0]);
    if(label == "Id"){
        std::string name = node->children[0]->token->value;
        Symbol* sym = symbolTable.lookup(name);
        if(sym == nullptr) return "unknown";
        return sym->type;
    }
    return "unknown";
}