#include "code_generator.h"
#include <map>
#include <vector>
#include <string>
#include <algorithm>


// Constructor
CodeGenerator::CodeGenerator(CSTNode* ast) 
:   ast(ast) 
    codePtr(0),
    heapPtr(0xFF),
    tempCounter(0),
    jumpCounter(0)
{
    // initialize to empty image (filled with 0x00)
    std::fill(std::begin(image), std::end(img), 0x00)
}

CodeGenerator::~CodeGenerator() {}


// Entry point -- I used claude to lay out the phases
bool CodeGenerator::generate(){
    // Phase 1 - walk AST and emit code
    genBlock(ast->children[0]);

    // Emit BRK to halt
    emit(0x00);

    // Phase 2 - assign static addresses after code

    // Phase 3 - backpatch static addresses

    // Phase 4 - backpatch jumps

    return errors.empty();
}

// I used claude to print the image
void CodeGenerator::printImage() const {
    std::cout << "\n=== Code Generation ===" << std::endl;
    for(int i = 0; i < 256; i++) {
        if(i % 8 == 0) {
            std::cout << std::endl;
            std::cout << "0x" << std::hex << std::uppercase
                      << std::setw(2) << std::setfill('0') << i << " ";
        }
        std::cout << std::hex << std::uppercase
                  << std::setw(2) << std::setfill('0') << (int)image[i] << " ";
    }
    std::cout << std::dec << std::endl;
}

// Emit a static address placeholder and record it
void CodeGenerator::emitStaticRef(std::string tempName) {
    staticPlaceholders.push_back({codePtr, tempName});
    emit(0x00);
    emit(0x00); // absolute addressing so 2 bytes (lil endian)
}

void CodeGenerator::enterScope() {
    scopeStack.push_back(std::map<std::string, std::string>());
}

void CodeGenerator::exitScope() {
    scopeStack.pop_back();

}

std::string CodeGenerator::newTemp(std::String varName, std::string type) {
    std::string tempName = "T" + std::to_string(tempCounter++);

    TempVar tv;
    tv.tempName = tempName;
    tv.varName = varName;
    tv.type = type;
    tv.address =-1;
    tempVars.push_back(tv);

    // register in scope
    scopeStack.back()[varName] = tempName;

    return tempName;
}

std::string CodeGenerator::lookupTemp(std::string varName) {
    if(!tempvars){
        // add warning/debug for now
        std::cout << "Somethings not working" << std::endl;
    }
    for (int i = myVector.size() - 1; i >= 0; --i) {
    if (myVector[i] == targetValue) {
        // Found it!
        break;
    }
}
}