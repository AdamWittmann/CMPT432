#include "java_generator.h"

// Constructor
JavaGenerator::JavaGenerator(CSTNode* ast, int programNum)
    : ast(ast), programNum(programNum), indentLevel(0) {}

JavaGenerator::~JavaGenerator() {}

// Emit a line with current indentation
void JavaGenerator::emit(const std::string& line){
    for(int i = 0; i < indentLevel; i++) output += "    ";
    output += line + "\n";
}

// Increase indentation
void JavaGenerator::indent(){ indentLevel++; }

// Decrease indentation
void JavaGenerator::dedent(){ indentLevel--; }

// Entry point -- wraps generated code in a Java class
std::string JavaGenerator::generate(){
    std::string className = "AlanProgram" + std::to_string(programNum);

    emit("public class " + className + " {");
    indent();
    emit("public static void main(String[] args) {");
    indent();

    // Generate code for the program block
    genBlock(ast);

    dedent();
    emit("}");
    dedent();
    emit("}");

    traces.push_back("Generated class " + className);
    return output;
}

void JavaGenerator::genBlock(CSTNode* node){
    // Block -> { StatementList }
    if(node->label != "Block"){
        errors.push_back("Error: expected Block node but found " + node->label);
        return;
    }
    for(CSTNode* stmt : node->children){
        if(stmt->label == "VarDecl"){
            genVarDecl(stmt);
        } else if(stmt->label == "AssignmentStatement"){
            genAssignment(stmt);
        } else if(stmt->label == "PrintStatement"){
            genPrint(stmt);
        } else if(stmt->label == "WhileStatement"){
            genWhile(stmt);
        } else if(stmt->label == "IfStatement"){
            genIf(stmt);
        } else {
            errors.push_back("Error: unrecognized statement type '" + stmt->label + "'");
        }
    }
}

void JavaGenerator::genVarDecl(CSTNode* node){
    std::string alanType = node->children[0]->label;
    std::string name = node->children[1]->label;

    // Map Alan types to Java types
    std::string javaType;
    if(alanType == "int") javaType = "int";
    else if(alanType == "string") javaType = "String";
    else if(alanType == "boolean") javaType = "boolean";
    else javaType = alanType;

    emit(javaType + " " + name + ";");
    traces.push_back("Declared " + javaType + " " + name);
}

void JavaGenerator::genAssignment(CSTNode* node){
    std::string name = node->children[0]->label;
    std::string expr = genExpr(node->children[1]);
    emit(name + " = " + expr + ";");
    traces.push_back("Assigned " + name + " = " + expr);
}

void JavaGenerator::genPrint(CSTNode* node){
    std::string expr = genExpr(node->children[0]);
    emit("System.out.println(" + expr + ");");
    traces.push_back("Generated print statement for " + expr);
}

void JavaGenerator::genWhile(CSTNode* node){
    std::string condition = genExpr(node->children[0]);
    emit("while (" + condition + ") {");
    indent();
    genBlock(node->children[1]);
    dedent();
    emit("}");
    traces.push_back("Generated while loop with condition " + condition);
}

void JavaGenerator::genIf(CSTNode* node){
    std::string condition = genExpr(node->children[0]);
    emit("if (" + condition + ") {");
    indent();
    genBlock(node->children[1]);
    dedent();
    emit("}");
    traces.push_back("Generated if statement with condition " + condition);
}

std::string JavaGenerator::genExpr(CSTNode* node){
    if(node->label == "IntExpr"){
        return genIntExpr(node);
    } else if(node->label == "StringExpr"){
        return genStringExpr(node);
    } else if(node->label == "BooleanExpr"){
        return genBooleanExpr(node);
    } else if(node->label == "ID"){
        return genId(node);
    } else {
        errors.push_back("Error: unrecognized expression type '" + node->label + "'");
        return "/* error */";
    }
}

std::string JavaGenerator::genIntExpr(CSTNode* node){
    std::string result = node->children[0]->token.value; // DIGIT
    if(node->children.size() == 3){
        result += " + " + genExpr(node->children[2]);
    }
    return result;
}

std::string JavaGenerator::genStringExpr(CSTNode* node){
    return "\"" + node->children[0]->label + "\""; // CharList
}

std::string JavaGenerator::genBooleanExpr(CSTNode* node){
    if(node->children.size() == 1){
        return node->children[0]->token.value; // returns true or false
    } else {
        std::string left = genExpr(node->children[0]);
        std::string op = (node->children[1]->label == "DOUBLE_EQUALS") ? "==" : "!=";
        std::string right = genExpr(node->children[2]);
        return "(" + left + " " + op + " " + right + ")";
    }
}

std::string JavaGenerator::genId(CSTNode* node){
    return node->children[0]->label; // ID
}