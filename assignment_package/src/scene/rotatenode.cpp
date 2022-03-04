#include "rotatenode.h"
#include <iostream>
//Field

//Constructors
RotateNode::RotateNode()
    : Node(), deg(0), axisOfRotation(glm::vec3(1.f, 0.f, 0.f))
{}

RotateNode::RotateNode(float d, glm::vec3 axis)
    : Node(), deg(d), axisOfRotation(axis)
{}

//Destructor

RotateNode::~RotateNode() {};

//PURELY virtual function that outputs 4x4 transMatrix
glm::mat4 RotateNode::transMatrix() {
    float radDeg = glm::radians(deg);
    return glm::rotate(glm::mat4(), radDeg, axisOfRotation);
}
