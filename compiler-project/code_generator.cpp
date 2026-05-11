#include "code_generator.h"
#include <map>
#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <iomanip>

// Constructor
CodeGenerator::CodeGenerator(CSTNode* ast)
    : ast(ast),
      codePtr(0),
      heapPtr(0xFF),
      tempCounter(0),
      jumpCounter(0)
{
    // initialize to empty image (filled with 0x00)
    std::fill(std::begin(image), std::end(image), 0x00);
}

CodeGenerator::~CodeGenerator() {}

bool CodeGenerator::generate() {
    // Phase 1 - walk AST and emit code
    genBlock(ast);

    // Emit BRK to halt
    emit(0x00);

    // Phase 2 - assign static addresses after code
    int staticAddr = codePtr;
    for(auto& tv : tempVars) {
        tv.address = staticAddr;
        staticAddr += 1; // each variable takes 1 byte (absolute address)
        if(staticAddr >= heapPtr) {
            errors.push_back("Error: codegen - not enough memory to allocate variable '" + tv.varName + "'");
            return false;
        }
    }

    // Phase 3 - backpatch static addresses
    backpatchStatic();

    // Phase 4 - backpatch jumps
    backpatchJumps();

    return errors.empty();
}

// I used claude to print the image
void CodeGenerator::printImage(){
    std::cout << "\n=== Code Generation ===" << std::endl;
    for(int i = 0; i < 256; i++){
        if(i % 8 == 0){
            printf("\n%02X | ", i);
        }
        printf("%02X ", (uint8_t)image[i]);
    }
    printf("\n");
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

void CodeGenerator::emit(uint8_t byte) {
    if(codePtr >= heapPtr) {
        errors.push_back("Error: codegen - code overflow, not enough memory to emit byte");
        return;
    }
    image[codePtr++] = byte;
}

std::string CodeGenerator::newTemp(std::string varName, std::string type) {
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
uint8_t CodeGenerator::writeHeap(std::string str) {
    // we need to make sure we have enough space in heap for string, or else we'll overflow
    int startAddr = heapPtr - str.size(); // calculate start address
    if(startAddr < codePtr) {
        errors.push_back("Error: codegen - heap overflow, not enough space for string '" + str + "'");
        return 0x00; // return null pointer on error
    }
    for(size_t i = 0; i < str.size(); i++) {
        image[startAddr + i] = (uint8_t)str[i];
    }
    image[startAddr + str.size()] = 0x00; // null terminator
    heapPtr = startAddr - 1; // update heap pointer
    return startAddr;
}

void CodeGenerator::backpatchStatic() {
    for(const auto& placeholder : staticPlaceholders) {
        // walk the tempVars to find the address for this tempName
        
        for(const auto& tv : tempVars){
            if(tv.tempName == placeholder.tempName) {
                if(tv.tempName == placeholder.tempName) {
                    image[placeholder.imageIndex] = (uint8_t)tv.address; // low byte
                    image[placeholder.imageIndex + 1] = 0x00; // high byte | always for 256 byte image.
                }
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
}

void CodeGenerator::genBlock(CSTNode* node) {
    enterScope();
    for(const auto& child : node->children) {
        if(child->label == "VarDecl"){
            //handle var decl
            genVarDecl(child);
        } else if(child->label == "AssignmentStatement"){
            //handle assignment
            genAssignment(child);
        } else if (child->label == "PrintStatement"){
            //handle print
            genPrint(child);
        } else if (child->label == "WhileStatement"){
            //handle while
            genWhile(child);
        } else if (child->label == "IfStatement"){
            //handle if
            genIf(child);
        } else if (child->label == "Block") {
            // nested block - recurse
            genBlock(child);
        } else {
            errors.push_back("Error: codegen - unrecognized statement type '" + child->label + "'");
        }
    }
    exitScope();
}

void CodeGenerator::genVarDecl(CSTNode* node) {
    std::string type = node->children[0]->label;
    std::string varName = node->children[1]->label;
    newTemp(varName, type);
}

void CodeGenerator::genAssignment(CSTNode* node) {
    std::string varName = node->children[0]->label;
    std::string tempName = lookupTemp(varName);
    if(tempName.empty()) return; // error already recorded in lookupTemp

    // for now we only support int literals on the right hand side, so we can just emit the LDA immediate instruction with the value
    // in the future, this will need to be more complex to handle different expression types and codegen for them
    genExpr(node->children[1]);

    // STA - store accumulator into variable's address
    emit(0x8D); // STA absolute
    emitStaticRef(tempName);
}

void CodeGenerator::genExpr(CSTNode* node) {
    if(node->label == "IntExpr") {
        genIntExpr(node);
    } else if(node->label == "StringExpr") {
        genStringExpr(node);
    } else if(node->label == "BooleanExpr") {
        genBooleanExpr(node);
    } else if (node->label == "ID") {
        genId(node);
    } else {
        errors.push_back("Error: codegen - unrecognized expression type '" + node->label + "'");
    }
}

void CodeGenerator::genIntExpr(CSTNode* node) {
    // load digit to A
    int digit = node->children[0]->token.value[0] - '0';
    emit(0xA9); // LDA immediate
    emit((uint8_t)digit);

    if(node->children.size() == 3) {
        //store current A into a temp
        std::string temp = newTemp("", "int");
        emit(0x8D); // STA absolute
        emitStaticRef(temp);

        //eval right side
        genExpr(node->children[2]);

        //ADC value from temp
        emit(0x6D); // ADC absolute
        emitStaticRef(temp);
    }
}

void CodeGenerator::genStringExpr(CSTNode* node) {
    std::string str = node->children[0]->label;
    int addr = writeHeap(str);
    emit(0xA9); // LDA immediate
    emit((uint8_t)addr);
}

//load a variables value from static address into A
void CodeGenerator::genId(CSTNode* node) {
    std::string varName = node->children[0]->label;
    std::string tempName = lookupTemp(varName);
    if(tempName.empty()) return; // error already recorded in lookupTemp

    emit(0xAD); // LDA absolute
    emitStaticRef(tempName);
}

void CodeGenerator::genPrint(CSTNode* node) {
    CSTNode* expr = node->children[0];

    // determin syscall
    uint8_t syscallCode = 0x01;
    std::string type = expr->label;
    
    if(type == "IntExpr") {
        syscallCode = 0x01; // OUT instruction for int literals and expressions
    } else if(type == "StringExpr") {
        syscallCode = 0x02; // OUT instruction for string (null terminated at address in A)
    } else if(type == "BooleanExpr") {
        syscallCode = 0x03; // OUT instruction for boolean (0x00 for false, 0x01 for true)
    }else if(type == "ID") {
        // look up type in tempVars
        std::string varName = expr->children[0]->label;
        std::string tempName = lookupTemp(varName);
        for(const auto& tv : tempVars) {
            if(tv.tempName == tempName) {
                if(tv.type == "int") {
                    syscallCode = 0x01;
                } else if(tv.type == "string") {
                    syscallCode = 0x02;
                } else if(tv.type == "boolean") {
                    syscallCode = 0x03;
                } else {
                    errors.push_back("Error: codegen - unrecognized variable type '" + tv.type + "' for print statement");
                    return;
                }
            }
        }
    } else {
        errors.push_back("Error: codegen - unrecognized type in print statement '" + type + "'");
    }
    // eval | leaves the result in A
        genExpr(expr);

        //syscall sequence
        emit(0xA8); // TAY
        emit (0xA2); // LDX immediate
        emit(syscallCode);
        emit(0xFF); // syscall
}

void CodeGenerator::genBooleanExpr(CSTNode* node) {

    if(node->children.size() == 1) {
        // just a boolean literal
        std::string boolVal = node->children[0]->token.value;
        emit(0xA9); // LDA immediate
        emit(boolVal == "true" ? 0x01 : 0x00);
   } else if(node->children.size() == 3){
        std::string op = node->children[1]->label;
        // evaluate left, store in temp
        genExpr(node->children[0]);
        std::string temp = newTemp("", "int");
        emit(0x8D);
        emitStaticRef(temp);

        // evaluate right, store in X via temp
        genExpr(node->children[2]);
        std::string temp2 = newTemp("", "int");
        emit(0x8D);
        emitStaticRef(temp2);

        // LDX absolute - load right side into X
        emit(0xAE);
        emitStaticRef(temp2);

        // CPX absolute - compare X with left side
        emit(0xEC);
        emitStaticRef(temp);

    // branch around the "true" result
    int falseJump = newJumpId();
    if(op == "DOUBLE_EQUALS"){
        emit(0xD0); // BNE - branch if NOT equal
    } else {
        emit(0xF0); // BEQ - branch if equal
    }
    jumpPlaceholders.push_back({codePtr, falseJump});
    emit(0x00); // placeholder

    // true case
    emit(0xA9); emit(0x01); // LDA #01
    int doneJump = newJumpId();
    emit(0xD0); // BNE - unconditional (X never 0 after we set it)
    jumpPlaceholders.push_back({codePtr, doneJump});
    emit(0x00);

    // false case
    placeLabel(falseJump);
    emit(0xA9); emit(0x00); // LDA #00

    // done
    placeLabel(doneJump);
    }
}

//need to record two jump IDs, one for end, one to jump back to start
void CodeGenerator::genWhile(CSTNode* node) {
    int loopStart = newJumpId();
    placeLabel(loopStart);

    // eval and leave result in A
    genExpr(node->children[0]);

    // Sore result, laod into X, compare to 1
    std::string temp = newTemp("", "boolean");
    emit(0x8D); emitStaticRef(temp);
    emit(0xAE); emitStaticRef(temp);
    emit(0xE0); emit(0x01);

    // branch around the body if false
    int loopEnd = newJumpId();
    emit(0xD0); // BNE - branch if NOT equal (i.e. if false)
    jumpPlaceholders.push_back({codePtr, loopEnd});
    emit(0x00); // placeholder

    // emit body block
    genBlock(node->children[1]);

    // jump back to start
    emit(0xD0); // BNE - unconditional (X never 0 after we set it)
    jumpPlaceholders.push_back({codePtr, loopStart});
    emit(0x00);

    // place body label
    placeLabel(loopEnd);
}

void CodeGenerator::genIf(CSTNode* node) {
    // eval condition and leave in A
    genExpr(node->children[0]);

    // store result in temp, load into X, compare to 1
    std::string temp = newTemp("", "boolean");
    emit(0x8D); emitStaticRef(temp);
    emit(0xAE); emitStaticRef(temp);
    emit(0xE0); emit(0x01);

    // branch around body if false
    int elseJump = newJumpId();
    emit(0xD0); // BNE - branch if NOT equal (i.e. if false)
    jumpPlaceholders.push_back({codePtr, elseJump});
    emit(0x00); // placeholder

    // emit body block
    genBlock(node->children[1]);

    // place else label
    placeLabel(elseJump);
}

