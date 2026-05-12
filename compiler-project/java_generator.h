#ifndef JAVA_GENERATOR_H
#define JAVA_GENERATOR_H

// ─────────────────────────────────────────────────────────────
//  java_generator.h
//  Generates readable Java source code from the AST.
//  Output is a valid .java file that can be compiled with javac
//  and run on the JVM. Serves as a comparison target against
//  the 6502 output running on SvegOS.
//
//  Generated with Claude assistance — May 11, 2025
// ─────────────────────────────────────────────────────────────

#include "cst_node.h"
#include <vector>
#include <string>

class JavaGenerator {
public:
    JavaGenerator(CSTNode* ast, int programNum);
    ~JavaGenerator();

    // Entry point — returns generated Java source as a string
    std::string generate();

    std::vector<std::string> errors;
    std::vector<std::string> traces;

private:
    CSTNode* ast;
    int programNum;   // used for class name e.g. AlanProgram1
    int indentLevel;  // tracks current indentation depth
    std::string output; // accumulates the generated source

    // Emit a line with proper indentation
    void emit(const std::string& line);

    // Increment/decrement indent
    void indent();
    void dedent();

    // AST visitors -- one per node type
    void genBlock(CSTNode* node);
    void genVarDecl(CSTNode* node);
    void genAssignment(CSTNode* node);
    void genPrint(CSTNode* node);
    void genWhile(CSTNode* node);
    void genIf(CSTNode* node);
    std::string genExpr(CSTNode* node);
    std::string genIntExpr(CSTNode* node);
    std::string genStringExpr(CSTNode* node);
    std::string genBooleanExpr(CSTNode* node);
    std::string genId(CSTNode* node);
};

#endif // JAVA_GENERATOR_H