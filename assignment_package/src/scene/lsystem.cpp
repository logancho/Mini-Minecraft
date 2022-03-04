#include "lsystem.h"

LSystem::LSystem()
{

}

LNode* LSystem::Parse(int iterations, LNode* start) {
    LNode* copy = new LNode(start);
    for (int i = 0; i < iterations; i++) {
        LNode* n = copy;
        do {
            // expand
            LNode* replace = pickRandom(n->c);
            if (replace != nullptr) {
                // list is expanded
                LNode* run = replace;
                while (run->next != nullptr) {
                    run = run->next;
                }
                // run now points to the last node of replace);
                run->next = n->next;
                n->next = replace;
                n = run->next;
            } else {
                n = n->next;
            }
        } while (n != nullptr);
    }
    return copy;
}

LNode* LSystem::pickRandom(char c) {
    std::vector<PostCondition>* distribution = Ruleset[c];
    float r = ((double) rand() / (RAND_MAX));

    LNode* toUse = nullptr;
    float min_p = 1.f;
    if (c == 'c' || c == 'C' || c == 't') {
        for (PostCondition pc : *distribution) {
            if (pc.p >= r && pc.p <= min_p) {
                toUse = new LNode(pc.node->c);
                min_p = pc.p;
            }
        }
    }

    return toUse;
}
