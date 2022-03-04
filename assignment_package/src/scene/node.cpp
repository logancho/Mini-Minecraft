#include "node.h"
#include <iostream>
#include "smartpointerhelp.h"

//Node class implementation

//Default constructor

Node::Node()
    : type(FACE), children{}, mCuboid(nullptr)
{}

//Constructor (Will be called by subclasses)

Node::Node(std::vector<uPtr<Node>> &cList, MobCuboid* p)
    : type(FACE), children(std::move(cList)), mCuboid(p)
{}

//Destructor

Node::~Node() {}

//Methods:

//Adds child, uses std::move
Node& Node::addChild(uPtr<Node> newChild) {

    this->children.push_back(std::move(newChild));
    return *(this->children.back());
}
