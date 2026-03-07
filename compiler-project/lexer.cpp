#include <iostream>
#include "lexer.h"
#include "token.h"
#include <map>

// Constructor

Lexer::Lexer(std::string source)
    // Cpp initialization list
    // Lexer will always start at pos 0, line 1, and col 1.
    : source(source), pos(0), line(1), col(1) {}

Lexer::~Lexer() = default;

// Returns Current Character
char Lexer::current(){
    return source[pos];
}

// Peek ahead 1 character
// For '==' '!=' ...
char Lexer::peek(){
    if (pos + 1 < (int)source.length()){
        return source[pos+1];
    }
    return '\0';
}

// Advance to next character
// If next character is new line, reset position. Else increment column and position
void Lexer::advance(){
    if(current() == '\n'){
        line++;
        col = 1;
    }
    else{
        col++;
        
    }
    pos++;
}

// Did we finish scanning the entire file??
// This makes sense instead of when reaching and EOP bc multiple programs, and error catching.
bool Lexer::isEnd(){
    return (pos >= int(source.length()));
}

// Skip the white space bc the lexer doesnt gaf about it
void Lexer::skipWhiteSpace(){
    while(!isEnd() && (current() == ' ' || current() =='\n' || current() =='\t' || current() == '\r')){
        advance();
    }
}
// Consume entire (key)words
// std::string Lexer::consumeWord(){

//     std::string word= "";
//     while(!isEnd() && current() >= 'a' && current() <= 'z'){
//         word += current();
//         advance();
//     }
//     return word;
// }

// // Match keywords
// // Using a map instead of a lame ass if chain
// TokenType Lexer::matchKeyword(std::string word){
//     std::map<std::string, TokenType> keywords {
//         {"print", PRINT},
//         {"int", INT},
//         {"while", WHILE},
//         {"if", IF},
//         {"string", STRING},
//         {"boolean", BOOLEAN},
//         {"true", BOOL},
//         {"false", BOOL}
//     };
    
//     //if its a keyword return the keyword type
//     if(keywords.count(word)) return keywords[word];
//     // If a 1 letter word return ID.
//     if(word.length() == 1) return ID;

//     return ID;
//     // ERROR HANDLING
// }

// Main loop, aka the scanner
// Purpose-- read in source (file) and produce tokens
std::vector<LexResult> Lexer::lex(){
    // empty vector to store results 
    std::vector<LexResult> results;
    // empty vector to store tokens
    std::vector<Token> tokens;
    
    // empty vector to store error messages
    std::vector<std::string> errors;
    // While not at the end of file.
    while(!isEnd()){
        // Skip whitespace
        skipWhiteSpace();

        // Check for end of file
        if(isEnd()) {
            break;
        }

        int tokenLine = line;
        int tokenCol = col;
        char c = current();

        //Check for symbols, operators, literals, digits, letters, and unrecognized characters
        switch(c){
            case '{':

                tokens.push_back(Token(LEFT_BRACE,"{", tokenLine, tokenCol));
                advance();
                break;

            case '}':

                tokens.push_back(Token(RIGHT_BRACE, "}", tokenLine, tokenCol));
                advance();
                break;

            case '(':

                tokens.push_back(Token(LEFT_PAREN, "(", tokenLine, tokenCol));
                advance();
                break;

            case ')':

                tokens.push_back(Token(RIGHT_PAREN,")", tokenLine, tokenCol));
                advance();
                break;

            case '+':

                tokens.push_back(Token(PLUS,"+", tokenLine, tokenCol));
                advance();
                break;

            case '$':
                tokens.push_back(Token(EOP,"$", tokenLine, tokenCol));
                results.push_back({tokens, errors});
                tokens.clear();
                errors.clear();
                advance();
                break;

            case '=':

                // HANDLE = or ==. USE PEEK MAYBE
                if(peek() == '='){

                    tokens.push_back(Token(DOUBLE_EQUALS,"==", tokenLine, tokenCol));
                    
                    // Advance twice
                    advance(); // skips the first =
                    advance(); // skips the second =
                }else {
                    tokens.push_back(Token(ASSIGN,"=", tokenLine, tokenCol));
                    advance();
                }
                break;

            case '!':

                // HANDLE ! or !=
                if(peek() == '='){
                    tokens.push_back(Token(NOT_EQUALS, "!=", tokenLine, tokenCol));

                    // Advance twice
                    advance(); // skips the =
                    advance(); // skips the !
                }else {
                    // Error lone ! is not valid in this grammer
                    errors.push_back("Error: '!' is not valid at (" + std::to_string(line) + "," + std::to_string(col) + ")"+ ".\nDid you mean '!='?");
                    advance();
                }
                break;

            case '"':

                // Push starting quote
                tokens.push_back(Token(QUOTE, "\"", tokenLine, tokenCol));
                advance();

                // HANDLE STRING LITERALS 
                // Return unterminated quote error if no closing quote
                while(!isEnd() && current() != '"'){
                    tokens.push_back(Token(CHAR,std::string(1, current()), line, col));
                    advance();
                }

                // We reach the closing quote
                if(!isEnd()){
                    tokens.push_back(Token(QUOTE, "\"", tokenLine, tokenCol));
                    advance();
                }else{

                    // Error: Unterminated string literal
                    errors.push_back("Error: Unterminated string literal at (" + std::to_string(line) + "," + std::to_string(col) + ")" + "\nAre you forgetting a quote?");
                }
                break;

                case '/':

                    // Handle comments
                    if(peek() == '*'){
                        advance();
                        advance();
                        
                        // Skip until found end comment
                        while(!isEnd()){
                            if(current() == '*' && peek() == '/'){
                                advance();
                                advance();
                                break;
                            }
                            advance();
                        }
                        // If reaches end of the file before finding terminating comment symbol
                        if(isEnd()){
                            // Return error for unterminated comment
                            errors.push_back("Error: Unterminated comment at line:(" + std::to_string(line) + "," + std::to_string(col) + ")" +"\nAre you forgetting a '*/'?");
                        }
                    }else{
                        // Lone / is not valid return error
                        errors.push_back("Error: Unrecognized character at (" + std::to_string(line) + "," + std::to_string(col) + ")" + "Did you mean to make a comment?");
                    }
                    break;

            // Handle digits, ids, and errors
            default:

                // Digit
                if(c >= '0' && c <= '9'){
                    tokens.push_back(Token(DIGIT, std::string(1, c), tokenLine, tokenCol));
                    advance();
                }
                // ID
                else if(c >= 'a' && c <= 'z'){
                    std::map<std::string, TokenType> keywords {
                        {"print", PRINT},
                        {"int", INT},
                        {"while", WHILE},
                        {"if", IF},
                        {"string", STRING},
                        {"boolean", BOOLEAN},
                        {"true", BOOL},
                        {"false", BOOL}
                    };

                    bool matched = false;
                    for(auto& [kw, type] : keywords){
                        // Check if keyword fits at current position
                        if(source.substr(pos, kw.length()) == kw){
                            for(int i = 0; i < (int)kw.length(); i++) advance();
                            tokens.push_back(Token(type, kw, tokenLine, tokenCol));
                            matched = true;
                            break;
                        }
                    }
                    if(!matched){
                        // Single char ID
                        tokens.push_back(Token(ID, std::string(1, c), tokenLine, tokenCol));
                        advance();
                    }
                }
                break;
        }
    }
    return results;
}
