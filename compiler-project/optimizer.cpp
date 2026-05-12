#include "optimizer.h"

Optimizer::Optimizer(CSTNode* ast):
    ast(ast) {}

Optimizer::~Optimizer() {}

CSTNode* Optimizer::optimize() {
    return visit(ast);
}

bool Optimizer::isConstant(CSTNode* node){
    if(node == nullptr) return false;

    if(node->label == "IntExpr"){
        if(node->children.size() == 1){
            // single digit literal - always constant
            return true;
        } else if(node->children.size() == 3){
            // constant only if right side expr is also fully constant
            return isConstant(node->children[2]);
        }
    }

    // variables are never compile-time constants
    if(node->label == "ID") return false;

    return false;
}

int Optimizer::evaluate(CSTNode* node){
    if(node == nullptr) return 0;

    if(node->label == "IntExpr"){
        int leftVal = std::stoi(node->children[0]->token.value);
        if(node->children.size() == 1){
            return leftVal;
        } else if(node->children.size() == 3){
            int rightVal = evaluate(node->children[2]);
            return leftVal + rightVal;
        }
    }

    // Should never evaluate non-constant nodes
    errors.push_back("Error: attempted to evaluate non-constant expression during optimization");
    return 0;
}

CSTNode* Optimizer::foldIntExpr(CSTNode* node){
    if(node == nullptr) return node;

    if(node->label == "IntExpr" && isConstant(node)){
        int result = evaluate(node);
        traces.push_back("Constant folding: folded expression to " + std::to_string(result));

        // Create a new leaf token with the computed value
        Token foldedToken(DIGIT, std::to_string(result), 0, 0);
        CSTNode* digitLeaf = new CSTNode("DIGIT", foldedToken);

        // Return a single-child IntExpr replacing the original subtree
        CSTNode* folded = new CSTNode("IntExpr");
        folded->addChild(digitLeaf);
        return folded;
    }

    // Not foldable - return unchanged
    return node;
}

CSTNode* Optimizer::visit(CSTNode* node){
    if(node == nullptr) return nullptr;

    // Recursively visit children first (post-order traversal)
    for(int i = 0; i < (int)node->children.size(); i++){
        node->children[i] = visit(node->children[i]);
    }

    // After visiting children, attempt to fold if this is an IntExpr
    if(node->label == "IntExpr"){
        return foldIntExpr(node);
    }

    // For all other nodes, return as-is
    return node;
}