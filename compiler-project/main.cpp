#include <iostream> 
#include <fstream>
#include "lexer.h"
#include "token.h"
#include "parser.h"
#include "cst_node.h"
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
    std::vector<LexResult> results = lexer.lex();

    int programNum = 1;
    for(const LexResult& result : results){
        std::cout << "Lexing program " << programNum << std::endl;
        
        for(const Token& token : result.tokens){
            token.print();
        }
        for(const std::string& err : result.errors){
            std::cerr << err << std::endl;
        }
        
        std::cout << "Lexer completed program " << programNum 
                << " with " << result.errors.size() << " errors." << std::endl;
        
        // If lexer returned 0 errors move onto parser
        if(result.errors.empty()){
            std::cout << "\nParsing program " << programNum << std::endl;
            Parser parser(result.tokens);
            CSTNode* cst = parser.parse();
            
            if(parser.errors.empty()){
                std::cout << "Parse successful, CST:" << std::endl;
                cst->print();
            } else {
                for(const std::string& err : parser.errors){
                    std::cerr << err << std::endl;
                }
                std::cout << "Parser completed program " << programNum 
                        << " with " << parser.errors.size() << " errors." << std::endl;
                delete cst;
                return 1;
            }
            delete cst;
        } else {
        // lexer had errors
        return 1; // signal failure
    }
    
        programNum++;
    }
    return 0;

}