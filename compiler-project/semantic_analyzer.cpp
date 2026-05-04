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
        // Helper function to bypass the statementList and adding the correct statement directly
        flattenStatementList(node->children[1],astNode);
        symbolTable.exitScope();

        // Combine errors from symbolTable and analyzer
        errors.insert(errors.end(), symbolTable.errors.begin(), symbolTable.errors.end());
        warnings.insert(warnings.end(), symbolTable.warnings.begin(), symbolTable.warnings.end());
        // Clear the errors from symbolTable-- so next program doesnt produce the previous warnings.
        symbolTable.errors.clear();
        symbolTable.warnings.clear();
        return astNode;

    }
    if(label == "VarDecl"){
        // get type from first child's token value
        std::string type = node->children[0]->token->value;
        // get name from second child's first child's token value
        std::string name = node->children[1]->children[0]->token->value;

        // declare in symbol table
        symbolTable.declared(name,type);

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
            std::string exprType = resolveType(node->children[2]);
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
        astNode->addChild(visit(node->children[2]));
        return astNode;
    }

    if(label == "PrintStatement"){
        CSTNode* expr = node->children[2];
        //build AST node
        CSTNode* astNode = new CSTNode("PrintStatement");
        astNode->addChild(visit(expr));
        return astNode;
    }

    if(label == "Expr"){
        return visit(node->children[0]);
    }

    if(label == "IntExpr"){
        CSTNode* astNode = new CSTNode("IntExpr");
        astNode->addChild(node->children[0]); // digit leaf
        if(node->children.size() == 3){
            astNode->addChild(node->children[1]); // + leaf
            astNode->addChild(visit(node->children[2])); // recurse on Expr
        }
        return astNode;
    }

    if(label == "StringExpr"){
        std::string val = collectCharList(node->children[1]);
        CSTNode* astNode = new CSTNode("StringExpr");
        astNode->addChild(new CSTNode(val));
        return astNode;
    }

    if(label == "BooleanExpr"){
        CSTNode* astNode = new CSTNode("BooleanExpr");
        if(node->children.size() == 5){
            astNode->addChild(visit(node->children[1]));
            astNode->addChild(node->children[2]);
            astNode->addChild(visit(node->children[3]));
        }
        else if(node->children.size() == 1){
            astNode->addChild(node->children[0]);
        }
        return astNode;
    }

    if(label == "Id"){
        std::string name = node->children[0]->token->value;
        Symbol* sym = symbolTable.lookup(name);
        // if not pointing to definition return undeclared
        if(sym == nullptr){
            errors.push_back("Error: undeclared variable '" + name + "' at (" +
            std::to_string(node->children[0]->token->line) + "," +
            std::to_string(node->children[0]->token->column) + ")");
        } else {
            if(!sym->isInit){
                warnings.push_back("WARNING: variable '" + name + "' used without being initialized");
            }
            // mark used if used
            symbolTable.markUsed(name);
        }
        CSTNode* astNode = new CSTNode("Id");
        astNode->addChild(new CSTNode(name));
        return astNode;

    }

    if(label == "WhileStatement"){
        CSTNode* astNode = new CSTNode("WhileStatement");
        astNode->addChild(visit(node->children[1])); // Boolop
        astNode->addChild(visit(node->children[2])); // Block
        return astNode;
    }

    if(label == "IfStatement"){
        CSTNode* astNode = new CSTNode("IfStatement");
        astNode->addChild(visit(node->children[1]));
        astNode->addChild(visit(node->children[2]));
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

std::string collectCharList(CSTNode* node){
    if(node == nullptr) return "";
    if(node->label == "Epsilon") return "";
    if(node->label == "CharList"){
        if(node->children[0]->label == "Epsilon") return "";
        // child[0] is the char leaf, [2] is another charList
        std::string ch = node->children[0]->token->value;
        return ch + collectCharList(node->children[1]);
    }
    return "";
}

void flattenStatementList(CSTNode* statementList, CSTNode* astBlock){
    if(statementList == nullptr) return;
    if(statementList->label == "Epsilon") return;

    for(CSTNode* child : statementList->children){
        if(child->label == "Epsilon") continue;
        if(child->label == "StatementList"){
            flattenStatementList(child, astBlock);
        }else if (child->label == "Statement"){
                CSTNode* result = visit(child->children[0]);
                if(result != nullptr) astBlock->addChild(result);
        }
    }
}