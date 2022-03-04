#ifndef LSYSTEM_H
#define LSYSTEM_H

#include <string>
#include <vector>
#include <unordered_map>
#include "lnode.h"

struct PostCondition {
    float p;
    LNode* node;
};

class LSystem
{
public:
    LSystem();
    LNode* Parse(int iterations, LNode* start);
    LNode* pickRandom(char c);

    std::unordered_map<char, std::vector<PostCondition>*> Ruleset;



};

#endif // LSYSTEM_H
