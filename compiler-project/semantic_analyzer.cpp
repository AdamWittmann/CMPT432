#include "semantic_analyzer.h"
#include <iostream>
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
        
        // Trace scope entry before entering
        traces.push_back("Entering scope " + std::to_string(symbolTable.getScope() + 1));
        symbolTable.enterScope();
        flattenStatementList(node->children[1], astNode);
        symbolTable.exitScope();
        traces.push_back("Exiting scope " + std::to_string(symbolTable.getScope() + 1));

        // Drain symbol table errors and warnings then clear for next scope
        errors.insert(errors.end(), symbolTable.errors.begin(), symbolTable.errors.end());
        warnings.insert(warnings.end(), symbolTable.warnings.begin(), symbolTable.warnings.end());
        symbolTable.errors.clear();
        symbolTable.warnings.clear();
        return astNode;
    }

    if(label == "Program"){
        return visit(node->children[0]);
    }

    // Used claude web for tracing optimization. 
    if(label == "VarDecl"){
        // Extract type and name from CST children
        std::string type = node->children[0]->token.value;
        std::string name = node->children[1]->children[0]->token.value;
        int line = node->children[1]->children[0]->token.line;
        int column = node->children[1]->children[0]->token.column;

        // Register in symbol table and trace the declaration
        symbolTable.declared(name, type, line, column);
        traces.push_back("Declared '" + name + "' (" + type + ") at (" + std::to_string(line) + "," + std::to_string(column) + ")");

        // Build AST node with type and name as children
        CSTNode* astNode = new CSTNode("VarDecl");
        astNode->addChild(new CSTNode(type));
        astNode->addChild(new CSTNode(name));
        return astNode;
    }

    // Used claude web for tracing optimization.
    if(label == "AssignmentStatement"){
    std::string id = node->children[0]->token.value;
    Symbol* sym = symbolTable.lookup(id);

    if(sym == nullptr){
        // Variable used before declaration
        errors.push_back("Error: undeclared variable '" + id + "' at (" +
            std::to_string(node->children[0]->token.line) + "," +
            std::to_string(node->children[0]->token.column) + ")");
            traces.push_back("✗ Assignment to undeclared variable '" + id + "'");
        } else {
            // Resolve and verify expression type matches declared type
            std::string exprType = resolveType(node->children[2]);
            if(sym->type != exprType){
                errors.push_back("Error: type mismatch for variable '" + id +
                    "'. Expected " + sym->type + " but got " + exprType);
                traces.push_back("✗ Type check: '" + id + "' expects " + sym->type + " but got " + exprType);
            } else {
                traces.push_back("✓ Type check: '" + id + "' = " + exprType + " ✓");
            }
            symbolTable.markInitialized(id);
        }

        // Build AST node
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
        CSTNode* leaf = node ->children[0];
        astNode->addChild(new CSTNode(leaf->label, leaf->token)); // digit leaf
        if(node->children.size() == 3){
            CSTNode* leaf = node ->children[1];
            astNode->addChild(new CSTNode(leaf->label, leaf->token));; // + leaf
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
            CSTNode* leaf = node ->children[2];
            astNode->addChild(new CSTNode(leaf->label, leaf->token));;
            astNode->addChild(visit(node->children[3]));
        }
        else if(node->children.size() == 1){
            CSTNode* leaf = node ->children[0];
            astNode->addChild(new CSTNode(leaf->label, leaf->token));;
        }
        return astNode;
    }

    // Used claude web for tracing optimization.
    if(label == "ID"){
        std::string name = node->children[0]->token.value;
        Symbol* sym = symbolTable.lookup(name);

        if(sym == nullptr){
            // Variable referenced before declaration
            errors.push_back("Error: undeclared variable '" + name + "' at (" +
                std::to_string(node->children[0]->token.line) + "," +
                std::to_string(node->children[0]->token.column) + ")");
            traces.push_back("✗ Referenced undeclared variable '" + name + "'");
        } else {
            // Warn if used before initialization
            if(!sym->isInit){
                warnings.push_back("Warning: variable '" + name + "' used without being initialized");
                traces.push_back("⚠ '" + name + "' referenced before initialization");
            } else {
                traces.push_back("→ Referenced '" + name + "' (" + sym->type + ")");
            }
            symbolTable.markUsed(name);
        }

        // Build AST node
        CSTNode* astNode = new CSTNode("ID");
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
    if(label == "ID"){
        std::string name = node->children[0]->token.value;
        Symbol* sym = symbolTable.lookup(name);
        if(sym == nullptr) return "unknown";
        return sym->type;
    }
    return "unknown";
}

std::string SemanticAnalyzer::collectCharList(CSTNode* node){
    if(node == nullptr) return "";
    if(node->label == "Epsilon") return "";
    if(node->label == "CharList"){
        if(node->children[0]->label == "Epsilon") return "";
        // child[0] is the char leaf, [2] is another charList
        std::string ch = node->children[0]->token.value;
        return ch + collectCharList(node->children[1]);
    }
    return "";
}

void SemanticAnalyzer::flattenStatementList(CSTNode* statementList, CSTNode* astBlock){
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