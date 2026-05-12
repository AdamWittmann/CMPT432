#ifndef OPTIMIZER_H
#define OPTIMIZER_H

// ─────────────────────────────────────────────────────────────
//  optimizer.h
//  AST optimization pass — runs after semantic analysis and
//  before code generation. Currently implements constant folding:
//  integer expressions with all-literal operands are evaluated
//  at compile time and replaced with a single digit node.
//
//  Generated with Claude assistance — May 11, 2025
// ─────────────────────────────────────────────────────────────

#include "cst_node.h"
#include <vector>
#include <string>

class Optimizer {
public:
    // Constructor takes the AST produced by SemanticAnalyzer
    Optimizer(CSTNode* ast);
    ~Optimizer();

    // Entry point — walks the AST and returns an optimized version
    CSTNode* optimize();

    // Trace log of optimizations applied (for verbose output)
    std::vector<std::string> traces;

    // Errors encountered during optimization (should be rare)
    std::vector<std::string> errors;

private:
    CSTNode* ast;

    // Recursively walk and optimize a node, returning the result
    CSTNode* visit(CSTNode* node);

    // Attempt to fold an IntExpr — returns folded node or original
    CSTNode* foldIntExpr(CSTNode* node);

    // Check if an IntExpr node is a compile-time constant (no variables)
    bool isConstant(CSTNode* node);

    // Evaluate a constant IntExpr to its integer value
    int evaluate(CSTNode* node);
};

#endif // OPTIMIZER_H