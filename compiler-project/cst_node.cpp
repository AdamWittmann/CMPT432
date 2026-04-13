#include "cst_node.h"
#include <iostream>


// Internal node constructor
CSTNode::CSTNode(std::string label):
    label(label), token(nullptr), isLeaf(false) {}

// Leaf node constructor
CSTNode::CSTNode(std::string label, Token* token):
    label(label), token(token), isLeaf(true) {}

// Kill off the children
CSTNode::DesCSTNode(){
    for(CSTNode* child : children){
        delete child;
    }
}

void CSTNode::addChild(CSTNode* child){
    // Push child onto children vector
    children.push_back(child);
}

void CSTNode::print(int depth) const {
    // print the tree
    for(int i = 0; i<depth * 2; i++){
        std::cout << " ";
    }
    if(isLeaf){
    std::cout << label << " [ " << token->value << " ]" << std::endl;
    }else {
        std::cout << label << std::endl;
    }
    
    for(int j = 0; j<children.size(); j++){
        children[j]->print(depth+1);
    }
}