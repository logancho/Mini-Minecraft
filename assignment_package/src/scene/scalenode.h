#pragma once
#include "node.h"

class ScaleNode : public Node {
public:
    //Fields
    float xScale;
    float yScale;
    float zScale;


    //Constructor

    ScaleNode();
    ScaleNode(float x, float y, float z);

    //Destructor

    virtual ~ScaleNode();


    //PURELY virtual function that outputs 3x3 transMatrix
    glm::mat4 transMatrix() override;
};


