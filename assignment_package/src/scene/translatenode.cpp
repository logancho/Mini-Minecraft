#include "translatenode.h"
#include <iostream>
//Constructors

TranslateNode::TranslateNode()
    : Node(), trans(glm::vec3(0.f))
{}

TranslateNode::TranslateNode(glm::vec3 transInput)
    : Node(), trans(transInput)
{}

//Destructor
TranslateNode::~TranslateNode() {};

//PURELY virtual function that outputs 4x4 transMatrix
glm::mat4 TranslateNode::transMatrix() {
//    std::cout << "Returning transMatrix for translate Node\n";
    return glm::translate(glm::mat4(), trans);
}
