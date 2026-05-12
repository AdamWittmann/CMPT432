#include <iostream> 
#include <fstream>
#include <iomanip>
#include "lexer.h"
#include "token.h"
#include "parser.h"
#include "cst_node.h"
#include "semantic_analyzer.h"
#include "code_generator.h"
#include "optimizer.h"

// ─────────────────────────────────────────────────────────────
//  Changelog
//  May 11, 2025 — Restructured output for terminal visibility.
//  Added styled phase headers, status indicators, and semantic
//  trace output (scope entry/exit, declarations, type checks).
//  Print helpers centralize all formatting so phases stay clean.
// ─────────────────────────────────────────────────────────────

// ── Print Helpers ─────────────────────────────────────────────

// Program header box using Unicode box-drawing characters
void printHeader(int programNum, const std::string& filename) {
    std::string title = "  Compiling Program " + std::to_string(programNum) + " -- " + filename + "  ";
    std::string top = "+", bot = "+";
    for(size_t i = 0; i < title.size(); i++){ top += "="; bot += "="; }
    top += "+"; bot += "+";
    std::cout << "\n" << top << "\n|" << title << "|\n" << bot << "\n";
}

// Phase label e.g. [ LEXER ]
void printPhase(const std::string& name) {
    std::cout << "\n[ " << name << " ]\n";
}

// Success indicator
void printOk(const std::string& msg) {
    std::cout << "  OK  " << msg << "\n";
}

// Error indicator
void printErr(const std::string& msg) {
    std::cerr << "  ERR " << msg << "\n";
}

// Warning indicator
void printWarn(const std::string& msg) {
    std::cout << "  /!\\ " << msg << "\n";
}

// Trace indicator for verbose semantic output
void printTrace(const std::string& msg) {
    std::cout << "  --> " << msg << "\n";
}

// Visual divider for separating sections
void printDivider() {
    std::cout << "  " << std::string(50, '-') << "\n";
}

// ── Main ──────────────────────────────────────────────────────

int main(int argc, char* argv[]){
    bool verbose = true;

    if(argc < 2){
        std::cerr << "Usage: ./compiler <source_file>" << std::endl;
        return 1;
    }

    // Open and read source file
    std::ifstream file(argv[1]);
    if(!file.is_open()){
        std::cerr << "Error: Unable to open file: " << argv[1] << std::endl;
        return 1;
    }

    std::string fileLine, source;
    while(std::getline(file, fileLine)){
        source += fileLine + "\n";
    }

    // Strip path from filename for display
    std::string filename = argv[1];
    size_t slash = filename.find_last_of("/\\");
    if(slash != std::string::npos) filename = filename.substr(slash + 1);

    // ── Lexer ──────────────────────────────────────────────────
    Lexer lexer(source);
    lexer.setVerbose(verbose);
    std::vector<LexResult> results = lexer.lex();

    int programNum = 1;
    for(const LexResult& result : results){

        printHeader(programNum, filename);

        // ── Lexer Phase ───────────────────────────────────────
        printPhase("LEXER");

        if(verbose){
            for(const Token& token : result.tokens){
                token.print();
            }
            printDivider();
        }

        for(const std::string& warn : result.warnings){
            printWarn(warn);
        }
        for(const std::string& err : result.errors){
            printErr(err);
        }

        if(!result.errors.empty()){
            printErr("Lexer failed with " + std::to_string(result.errors.size()) + " error(s)");
            return 1;
        }
        printOk("Lexed " + std::to_string(result.tokens.size()) + " tokens with "
                + std::to_string(result.warnings.size()) + " warning(s)");

        // ── Parser Phase ──────────────────────────────────────
        printPhase("PARSER");

        Parser parser(result.tokens);
        CSTNode* cst = parser.parse();

        for(const std::string& err : parser.errors){
            printErr(err);
        }

        if(!parser.errors.empty()){
            printErr("Parser failed with " + std::to_string(parser.errors.size()) + " error(s)");
            delete cst;
            return 1;
        }

        printOk("Parse successful");

        if(verbose){
            printDivider();
            std::cout << "  CST:\n";
            cst->print(2);
            printDivider();
        }

        // ── Semantic Analysis Phase ───────────────────────────
        printPhase("SEMANTIC ANALYSIS");

        SemanticAnalyzer analyzer(cst);
        CSTNode* ast = analyzer.analyze();

        // Print scope/declaration/type-check traces
        if(verbose){
            for(const std::string& trace : analyzer.traces){
                printTrace(trace);
            }
            if(!analyzer.traces.empty()) printDivider();
        }

        for(const std::string& warn : analyzer.warnings){
            printWarn(warn);
        }
        for(const std::string& err : analyzer.errors){
            printErr(err);
        }

        if(!analyzer.errors.empty()){
            printErr("Semantic analysis failed with " + std::to_string(analyzer.errors.size()) + " error(s)");
            delete ast;
            return 1;
        }
        printOk("Semantic analysis passed with " + std::to_string(analyzer.warnings.size()) + " warning(s)");
        // ── Optimizer Phase ───────────────────────────────────────
        printPhase("OPTIMIZER");

        Optimizer optimizer(ast);
        CSTNode* optimizedAst = optimizer.optimize();

        if(verbose){
            for(const std::string& trace : optimizer.traces){
                printTrace(trace);
            }
            if(!optimizer.traces.empty()) printDivider();
        }

        if(optimizer.traces.empty()){
            printOk("No constant folding opportunities found");
        } else {
            printOk("Constant folding applied " + std::to_string(optimizer.traces.size()) + " optimization(s)");
        }
        // ── Symbol Table ──────────────────────────────────────
        if(verbose){
            printPhase("SYMBOL TABLE");
            analyzer.symbolTable.printSymbolTable(programNum);
        }

        // ── Code Generation Phase ─────────────────────────────
        printPhase("CODE GENERATION");

        CodeGenerator codegen(optimizedAst);
        if(codegen.generate()){
            printOk("Code generation successful");
            codegen.printImage();
        } else {
            for(const std::string& err : codegen.errors){
                printErr(err);
            }
            printErr("Code generation failed with " + std::to_string(codegen.errors.size()) + " error(s)");
        }

        // AST owned by codegen after this point -- no delete needed
        programNum++;
    }
    return 0;
}