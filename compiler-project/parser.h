#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include <string>
#include "token.h"
#include "cst_node.h"

class Parser {
public:
    Parser(std::vector<Token> tokens);
    ~Parser();
    CSTNode* parse();
    std::vector<std::string> errors;

private:
    std::vector<Token> tokens;
    int current;

    Token currentToken();
    Token peek();
    bool check(TokenType type);
    CSTNode* match(TokenType expected);
    bool isAtEnd();

    CSTNode* parseProgram();
    CSTNode* parseBlock();
    CSTNode* parseStatementList();
    CSTNode* parseStatement();
    CSTNode* parsePrintStatement();
    CSTNode* parseAssignmentStatement();
    CSTNode* parseVarDecl();
    CSTNode* parseWhileStatement();
    CSTNode* parseIfStatement();
    CSTNode* parseExpr();
    CSTNode* parseIntExpr();
    CSTNode* parseStringExpr();
    CSTNode* parseBooleanExpr();
    CSTNode* parseId();
    CSTNode* parseCharList();
};

#endif
