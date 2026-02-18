#ifndef TOKEN_H
#define TOKEN_H

#include <string>

enum TokenType {
    //SYMBOLS
    LEFT_PAREN,
    RIGHT_PAREN,
    LEFT_BRACE,
    RIGHT_BRACE,
    QUOTE,

    //KEYWORDS
    PRINT,
    WHILE,
    INT,
    STRING,
    BOOLEAN,
    IF,

    //BOOLEAN
    BOOL,

    //OPERATORS
    ASSIGN,
    DOUBLE_EQUALS,
    NOT_EQUALS,
    PLUS,

    //LITERALS
    ID,
    CHAR,
    DIGIT,

    //SPECIAL
    EOP,
    EOF_TOKEN
};

class Token{
public:
    TokenType type;
    std::string value;
    int line;
    int column;

    Token(TokenType t, std::string v, int l, int c);
    void print() const;
    std::string typeToString() const;
};

#endif