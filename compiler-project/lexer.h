#ifndef LEXER_H
#define LEXER_H

#include <vector>
#include "token.h"
#include <string>

// Struct to access tokens and errors from main
struct LexResult {
    std::vector<Token> tokens;
    std::vector<std::string> errors;
};
class Lexer {

   public:

        // Lexer takes source string
      Lexer(std::string source);

        // destructor
      ~Lexer();

      std::vector<LexResult> lex();

      
   private:

   // To keep track of state
   std::string source;
   int pos;
   int line;
   int col;

   // Helper Methods
   char current();
   char peek();
   void advance();
   bool isEnd();

   void skipWhiteSpace();

  //  std::string consumeWord();
  //  TokenType matchKeyword(std::string word);
   

};

#endif // LEXER_H
