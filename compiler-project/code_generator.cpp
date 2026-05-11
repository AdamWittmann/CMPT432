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
std::string CodeGenerator::lookupTemp(std::string varName){
    // edited with claude to make
    for(int i = scopeStack.size() - 1; i >= 0; i--){
        auto it = scopeStack[i].find(varName);
        if(it != scopeStack[i].end()){
            return it->second;
        }
    }
    errors.push_back("Error: codegen - undeclared variable '" + varName + "'");
    return "";
}

// simple increment for unique jump IDs
int CodeGenerator::newJumpId() {
return jumpCounter++;

}

// Record where a label is defined for backpatching
// labelTargets is jumpId -> codePtr
void CodeGenerator::placeLabel(int jumpId) {
    labelTargets[jumpId] = codePtr;
}

// Write a string to the heap (starts at 0xFF) and decrement heapPtr and return.
int CodeGenerator::writeHeap(std::string str) {
    // we need to make sure we have enough space in heap for string, or else we'll overflow
    int startAddr = heapPtr - str.size(); // calculate start address
    if(startAddr < codePtr) {
        errors.push_back("Error: codegen - heap overflow, not enough space for string '" + str + "'");
        return 0x00; // return null pointer on error
    }
    for(int i = 0; i < str.size(); i++) {
        image[startAddr + i] = (uint8_t)str[i];
    }
    image[startAddr + str.size()] = 0x00; // null terminator
    heapPtr = startAddr - 1; // update heap pointer
    return startAddr;
}

void CodeGenerator::backpatchStatic(uint8_t address) {
    for(const auto& placeholder : staticPlaceholders) {
        // walk the tempVars to find the address for this tempName
        
        for(const auto& tv : tempVars){
            if(tv.tempName == placeholder.tempName) {
                if(tv.tempName == placeholder.tempName) {
                    image[placeholder.imageIndex] = (uint8_t)tv.address; // low byte
                    image[placeholder.imageIndex + 1] = 0x00; // high byte | always for 256 byte image.
                }
                // write the address into the image at the placeholder location (lil endian)
                image[placeholder.imageIndex] = (uint8_t)(tv.address & 0xFF); // low byte
                image[placeholder.imageIndex + 1] = (uint8_t)((tv.address >> 8) & 0xFF); // high byte
            }
        }
    }
}

void CodeGenerator::backpatchJumps() {
    for(const auto& placeholder : jumpPlaceholders) {
        auto it = labelTargets.find(placeholder.jumpId);
        if(it == labelTargets.end()) {
            errors.push_back("Error: codegen - undefined label for jump ID " + std::to_string(placeholder.jumpId));
            continue;
        }
        int targetAddr = it->second;
        // write the offset from the jump instruction to the target address
        int offset = targetAddr - (placeholder.imageIndex + 1); // +1 because offset is calculated from the byte after the jump instruction
        image[placeholder.imageIndex] = (uint8_t)(offset & 0xFF); // low byte    }
}

void CodeGenerator::genBlock(CSTNode* node) {
    enterScope();
    for(const auto& child : node->children) {
        if(child->label == "VarDecl"){
            //handle var decl
        } else if(child->label == "AssignmentStatement"){
            //handle assignment

        } else if (child->label == "PrintStatement"){
            //handle print

        } else if (child->label == "WhileStatement"){
            //handle while

        } else if (child->label == "IfStatement"){
            //handle if

        } else if (child->label == "Block") {
            // nested block - recurse
            genBlock(child);
        } else {
            errors.push_back("Error: codegen - unrecognized statement type '" + child->label + "'");
        }

    }
}