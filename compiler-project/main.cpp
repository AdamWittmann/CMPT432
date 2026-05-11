#include <iostream> 
#include <fstream>
#include "lexer.h"
#include "token.h"
#include "parser.h"
#include "cst_node.h"
#include "semantic_analyzer.h"
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
                std::cout << "Parse successful" << std::endl;
                
                // Semantic analysis
                std::cout << "\n Semantic analyzing" << std::endl;
                SemanticAnalyzer analyzer(cst);
                CSTNode* ast = analyzer.analyze();
                
                for(const std::string& err : analyzer.errors){
                    std::cerr << err << std::endl;
                }
                for(const std::string& warn : analyzer.warnings){
                    std::cout << warn << std::endl;
                }
                
                if(analyzer.errors.empty()){
                    std::cout << "Semantic analysis passed with " << analyzer.warnings.size() << " warnings." << std::endl;
                    if(verbose){
                        analyzer.symbolTable.printSymbolTable(programNum);
                    }
                    
                    // Code generation
                    std::cout << "\nGenerating code for program " << programNum << std::endl;
                    CodeGenerator codegen(ast);
                    if(codegen.generate()){
                        std::cout << "Code generation successful." << std::endl;
                        codegen.printImage();
                    } else {
                        for(const std::string& err : codegen.errors){
                            std::cerr << err << std::endl;
                        }
                        std::cout << "Code generation failed with " << codegen.errors.size() << " errors." << std::endl;
                    }
                    delete ast;
                } else {
                    std::cout << "Semantic analysis failed with " << analyzer.errors.size() << " errors." << std::endl;
                    delete ast;
                    return 1;
                }
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
            return 1;
        }
        programNum++;
    }
    return 0;

}