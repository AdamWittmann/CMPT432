#include "token.h"
#include <iostream>

//Token Constructor implementation with initializer list
Token::Token(TokenType t, std::string v, int l, int c)
    : type(t), value(v), line(l), column(c) {}

void Token::print() const {
    std::cout << "DEBUG " << typeToString() << " [ " << value << " ] " << "found at (" << line << "," << column << ")" << std::endl;
}

std::string Token::typeToString() const{
    switch(type){
        //SYMBOLS
        case LEFT_PAREN: return "LEFT_PAREN";
        case RIGHT_PAREN: return "RIGHT_PAREN";
        case LEFT_BRACE: return "LEFT_BRACE";
        case RIGHT_BRACE: return "RIGHT_BRACE";
        case QUOTE: return "QUOTE";

        //KEYWORDS
        case PRINT: return "PRINT";
        case WHILE: return "WHILE";
        case INT: return "INT";
        case STRING: return "STRING";
        case BOOLEAN: return "BOOLEAN";
        case IF: return "IF";

        //BOOLEAN
        case BOOL: return "BOOL";

        //OPERATORS
        case ASSIGN: return "ASSIGN";
        case DOUBLE_EQUALS: return "DOUBLE_EQUALS";
        case NOT_EQUALS: return "NOT_EQUALS";
        case PLUS: return "PLUS";
        
        //LITERALS
        case ID: return "ID";
        case CHAR: return "CHAR";
        case DIGIT: return "DIGIT";
        

        //SPECIAL
        case EOP: return "EOP";
        case EOF_TOKEN: return "EOF_TOKEN";

        //DEFAULT
        default: return "UNKNOWN";
    }
}