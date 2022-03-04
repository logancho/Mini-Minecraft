#include "scalenode.h"

//Constructors
ScaleNode::ScaleNode()
    : Node(), xScale(1.f), yScale(1.f), zScale(1.f)
{}


ScaleNode::ScaleNode(float x, float y, float z)
    : Node(), xScale(x), yScale(y), zScale(z)
{}

//Destructor
ScaleNode::~ScaleNode() {};

//PURELY virtual function that outputs 4x4 transMatrix
glm::mat4 ScaleNode::transMatrix() {
    return glm::scale(glm::mat4(), glm::vec3(this->xScale, this->yScale, this->zScale));
}


