#pragma once
#include "node.h"

class RotateNode : public Node {
public:
    //Fields
    float deg;
    glm::vec3 axisOfRotation;

    //Constructor

    RotateNode();
    RotateNode(float d, glm::vec3 aOfRot);

    //Destructor

    virtual ~RotateNode();


    //PURELY virtual function that outputs 3x3 transMatrix
    glm::mat4 transMatrix() override;
};


//Model = glm::rotate(Model, angle_in_radians, glm::vec3(x, y, z));
