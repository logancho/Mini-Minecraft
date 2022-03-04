#ifndef CACTUS_H
#define CACTUS_H

#include "chunk.h"
#include "lsystem.h"
#include <vector>
#include "glm_includes.h"


class Cactus : public LSystem
{
public:
    Cactus(int iterations);
    LNode* system;

    LNode* getSystem();
};

#endif // CACTUS_H
