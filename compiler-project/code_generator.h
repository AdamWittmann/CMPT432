#ifndef CODE_GENERATOR_H
#define CODE_GENERATOR_H

#include "cst_node.h"
#include <vector>
#include <string>
#include <map>
#include <cstdint>

struct TempVar  {
    std::string tempName;
    std::string varName;
    std::string type;
    int address;   
};

struct JumpPlaceholder  {
    int imageIndex; // where in image[] to write offset
    int jumpId; // unique ID matching a label target
};

struct StaticPlaceholder    {
    int imageIndex; // where to write the address
    std::string tempName; // which TempVar's address goes here
};

class CodeGenerator {
    private:
        CSTNode* ast;
        uint8_t image[256];
        int codePtr;
        int heapPtr;

        std::vector<TempVar> tempVars;
        std::vector<JumpPlaceholder> jumpPlaceholders;
        std::vector<StaticPlaceholder> staticPlaceholders;
        std::map<int, int> labelTargets;

        int tempCounter;
        int jumpCounter;

        public:
            CodeGenerator(CSTNode* ast);
            ~CodeGenerator();

            bool generate();
            void printImage();

            std::vector<std::string> errors;
            std::vector<std::string> warnings;

            // Scope management
            std::vector<std::map<std::string, std::string>> scopeStack; // varName -> tempName map
            void enterScope();
            void exitScope();
            std::string newTemp(std::string varName, std::string type);
            std::String lookupTemp(std::string varName);

            // Emit
            void emit(uint8_t byte);
            void emitStaticRef(std::string tempName); // emit placeholder (0x00) & record that thang

            // Backpatching
            void backpatchStatic();
            void backpatchJumps();

            // Heap
            uint8_t writeHeap(std::string str); // writes null terminated string and returns a start address, like hello world would have ascii and then 0x00 at the end

            // Jump helpers
            int newJumpId();
            void placeLabel(int jumpId);

            // AST visitors-- one for each
            void genBlock(CSTNode* node);
            void genVarDecl(CSTNode* node);
            void genAssignment(CSTNode* node);
            void genPrint(CSTNode* node);
            void genWhile(CSTNode* node);
            void genIf(CSTNode* node);
            std::string genExpr(CSTNode* node);
            std::string genIntExpr(CSTNode* node);
            std::string genStringExpr(CSTNode* node);
            std::string genBooleanExpr(CSTNode* node);
            std::string genId(CSTNode* node);
};

#endif
