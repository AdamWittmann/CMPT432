#ifndef CST_NODE_H
#define CST_NODE_H
#include <string>
#include <vector>
#include "token.h"

class CSTNode {
    public:
        std::string label; // Label for 'Block', 'Program', ect...
        Token* token; // Null pointer
        std::vector<CSTNode*> children; // Stores leaf nodes
        bool isLeaf; // flag

    // Internal Node Constructor
    CSTNode(std::string label);

    // Leaf Node Constructor
    // Token* just points to the memory address of token that already exists. Kinda cool.
    CSTNode(std::string label, Token* token);
    
    // Destructor
    ~CSTNode();

    // Add child node
    void addChild(CSTNode* child);

    // Debug print
    void print(int depth = 0) const;
};

#endif