#pragma once
#include "node.h"

class TranslateNode : public Node {
public:
    //Fields
    glm::vec3 trans;
    //Constructor

    TranslateNode();

    TranslateNode(glm::vec3 transInput);

    //Destructor

    virtual ~TranslateNode();

    //PURELY virtual function that outputs 4x4 transMatrix
    glm::mat4 transMatrix() override;
};
