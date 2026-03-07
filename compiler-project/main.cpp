#include <iostream> 
#include <fstream>
#include "lexer.h"
#include "token.h"

int main(int argc, char* argv[]){

    // Check if there is a file
    if(argc < 2){
        std::cerr << "Usage: ./compiler <source_file>" << std::endl;
        return 1;
    }


    // Read in file
    std::ifstream file(argv[1]);
    // Check if it can be opened
    if(!file.is_open()){
        std::cerr << "Error: Unable to open file: " << argv[1] << std::endl;
        return 1;
    }

    // Read in the file
    std::string line;
    std::string source;
    while(std::getline(file, line)){
        source += line + "\n";
    }

    // Pass to lexer
    Lexer lexer(source);
    LexResult result = lexer.lex();

    // Print Errors
    for(const std::string& err : result.errors){
        std::cerr << err << std::endl;
    }
    // Print tokens
    for(const Token& token : result.tokens){
        token.print();
    }
    if(!result.errors.empty()){
    return 1;
    }
      
    return 0;

}