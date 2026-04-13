#include "parser.h"
#include <iostream>


// initialize the vector just set it to 0
Parser::Parser(std::vector<Token> tokens):
    tokens(tokens), current(0) {}

// Destructor
Parser::~Parser() {}

// check if at the end
bool Parser::isAtEnd(){
    return currentToken().type == EOP;
}

// returns current token
Token Parser::currentToken(){
    return tokens[current];
}

// lookahead #LL(1) grammar
Token Parser::peek(){
    if(!isAtEnd()){
        return tokens[current + 1];
    }else {
        return tokens[current];
    }
}

bool Parser::check(TokenType type){
    return currentToken().type == type;
}

CSTNode* Parser::match(TokenType expected){
    if(check(expected)){
        // make a leaf node from current token
        Token t = currentToken();
        // advance current
        current++;
        // return the leaf node
        return new CSTNode(t.typeToString(), &tokens[current - 1]);
    } else {
        // push error with line/col info
        Token temp(expected, "", 0, 0);
        errors.push_back("Error: expected " + temp.typeToString() + 
                 " but found '" + currentToken().value + "'" +
                 " at (" + std::to_string(currentToken().line) + 
                 "," + std::to_string(currentToken().column) + ")");
        return nullptr;
    }
}

CSTNode* Parser::parse(){
    return parseProgram();

}

CSTNode* Parser::parseProgram(){
    CSTNode* node = new CSTNode("Program");
    node->addChild(parseBlock());
    node->addChild(match(EOP));
    return node;
}

CSTNode* Parser::parseBlock(){
    CSTNode* node = new CSTNode("Block");
    node->addChild(match(LEFT_BRACE));
    node->addChild(parseStatementList());
    node->addChild(match(RIGHT_BRACE));
    return node;
}

CSTNode* Parser::parseStatementList(){
    CSTNode* node = new CSTNode("StatementList");
    //Check for epsilon prod before trying to continue
    if(check(RIGHT_BRACE)){
        node->addChild(new CSTNode("Epsilon"));
    }else {
        node->addChild(parseStatement());
        node->addChild(parseStatementList());
    }
    return node;
}

CSTNode* Parser::parseStatement(){
    CSTNode* node = new CSTNode("Statement");
    if(check(PRINT)){
        node->addChild(parsePrintStatement());
    }
    else if(check(ID)){
        node->addChild(parseAssignmentStatement());
    }
    else if(check(INT) || check(STRING) || check(BOOLEAN)){
        node->addChild(parseVarDecl());
    }
    else if(check(WHILE)){
        node->addChild(parseWhileStatement());
    }
    else if(check(IF)){
        node->addChild(parseIfStatement());
    }
    else if(check(LEFT_BRACE)){
        node->addChild(parseBlock());
    }
    else{
        errors.push_back("Error: unexpected token '" + currentToken().value + 
                 "' at (" + std::to_string(currentToken().line) + 
                 "," + std::to_string(currentToken().column) + 
                 "). Expected a statement.");
    }
        return node;

}

CSTNode* Parser::parsePrintStatement(){
    CSTNode* node = new CSTNode("PrintStatement");
    node->addChild(match(PRINT));
    node->addChild(match(LEFT_PAREN));
    node->addChild(parseExpr());
    node->addChild(match(RIGHT_PAREN));
    return node;
}

CSTNode* Parser::parseAssignmentStatement(){
    CSTNode* node = new CSTNode("AssignmentStatement");
    node->addChild(match(ID));
    node->addChild(match(ASSIGN));
    node->addChild(parseExpr());
    return node;
}

CSTNode* Parser::parseVarDecl(){
    CSTNode* node = new CSTNode("VarDecl");
    if(check(INT)) node->addChild(match(INT));
    else if(check(STRING)) node->addChild(match(STRING));
    else node->addChild(match(BOOLEAN));
    node->addChild(parseId());
    return node;
}

CSTNode* Parser::parseWhileStatement(){
    CSTNode* node = new CSTNode("WhileStatement");
    node->addChild(match(WHILE));
    node->addChild(parseBooleanExpr());
    node->addChild(parseBlock());
    return node;
}

CSTNode* Parser::parseIfStatement(){
    CSTNode* node = new CSTNode("IfStatement");
    node->addChild(match(IF));
    node->addChild(parseBooleanExpr());
    node->addChild(parseBlock());
    return node;
}

CSTNode* Parser::parseExpr(){
    CSTNode* node = new CSTNode("Expr");
    if(check(DIGIT)){
    node->addChild(parseIntExpr());
    }
    else if(check(QUOTE)){
    node->addChild(parseStringExpr());
    }
    else if(check(LEFT_PAREN) || check(BOOL)){
    node->addChild(parseBooleanExpr());
    }
    else if(check(ID)){
    node->addChild(parseId());
    }else {
    errors.push_back("Error: expected an expression but found '" + 
                     currentToken().value + "'" +
                     " at (" + std::to_string(currentToken().line) + 
                     "," + std::to_string(currentToken().column) + ")");
    }
    return node;
}

CSTNode* Parser::parseIntExpr(){
    CSTNode* node = new CSTNode("IntExpr");
    node->addChild(match(DIGIT));
    // this one has two cases so verify that the next one is plus before proceeding
    if(check(PLUS)){
        node->addChild(match(PLUS));
        node->addChild(parseExpr());
    }
    return node;
}

CSTNode* Parser::parseStringExpr(){
    CSTNode* node = new CSTNode("StringExpr");
    node->addChild(match(QUOTE));
    node->addChild(parseCharList());
    node->addChild(match(QUOTE));
    return node;
}

CSTNode* Parser::parseBooleanExpr(){
    CSTNode* node = new CSTNode("BooleanExpr");
    if(check(BOOL)){
        node->addChild(match(BOOL));
    }else {
        node->addChild(match(LEFT_PAREN));
        node->addChild(parseExpr());
        if(check(DOUBLE_EQUALS)){
            node->addChild(match(DOUBLE_EQUALS));
        } else if(check(NOT_EQUALS)){
            node->addChild(match(NOT_EQUALS));
        } else {
            errors.push_back("Error: expected '==' or '!=' but found '" + 
                            currentToken().value + "'" +
                            " at (" + std::to_string(currentToken().line) + 
                            "," + std::to_string(currentToken().column) + ")");
        }
        node->addChild(parseExpr());
        node->addChild(match(RIGHT_PAREN));
    }
    
    return node;
}

CSTNode* Parser::parseId(){
    CSTNode* node = new CSTNode("ID");
    node->addChild(match(ID));
    return node;
}

CSTNode* Parser::parseCharList(){
    CSTNode* node = new CSTNode("CharList");
    if(check(CHAR)){
        node->addChild(match(CHAR));
        node->addChild(parseCharList());
    }
    else {
        node->addChild(new CSTNode("Epsilon"));
    }
    return node;
}
