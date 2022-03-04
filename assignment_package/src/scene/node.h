#pragma once
#include "smartpointerhelp.h"
#include "mobcuboid.h"

class Node {
    // TODO
public:

    ////Fields:
    NPCBlockType type = NPCBlockType::FACE;
    //Children vector
    std::vector<uPtr<Node>> children;
    //Raw pointer to MobCuboid
    MobCuboid* mCuboid;
    //Visible flag
    bool visible = true;

    ////Constructors
    //Default constructor
    Node();

    //Instantiate node with children vector, polygon
    Node(std::vector<uPtr<Node>> &cList, MobCuboid* p);

    ////Destructor

    //Operator

    virtual ~Node();

    ////Methods
    //PURELY virtual function that outputs 3x3 transMatrix
    virtual glm::mat4 transMatrix() = 0;

    //Adds a child
    Node& addChild(uPtr<Node> newChild);
};
