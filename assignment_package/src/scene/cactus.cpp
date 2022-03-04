#include "cactus.h"

Cactus::Cactus(int iterations)
{

    LNode* c_node = new LNode('c');
    LNode* C_node = new LNode('C');
    LNode* t_node = new LNode('t');
    LNode* xpos_node = new LNode('1');
    LNode* zpos_node = new LNode('2');
    LNode* xneg_node = new LNode('3');
    LNode* zneg_node = new LNode('4');

    std::vector<PostCondition> c_pc;

    PostCondition pc = {0.6, new LNode(c_node)};
    c_pc.push_back(pc);

    pc.p = 0.625;
    pc.node = new LNode(xpos_node);
    c_pc.push_back(pc);

    pc.p = 0.65;
    pc.node = new LNode(zpos_node);
    c_pc.push_back(pc);

    pc.p = 0.675;
    pc.node = new LNode(xneg_node);
    c_pc.push_back(pc);

    pc.p = 0.7;
    pc.node = new LNode(zneg_node);
    c_pc.push_back(pc);

    pc.p = 0.8;
    pc.node = new LNode(C_node);
    c_pc.push_back(pc);

    Ruleset['c'] = &c_pc;

    std::vector<PostCondition> C_pc;

    pc.p = 0.2;
    pc.node = new LNode(c_node);
    C_pc.push_back(pc);

    pc.p = 0.3;
    pc.node = new LNode(C_node);
    C_pc.push_back(pc);

    pc.p = 0.325;
    pc.node = new LNode(xpos_node);
    C_pc.push_back(pc);

    pc.p = 0.35;
    pc.node = new LNode(zpos_node);
    C_pc.push_back(pc);

    pc.p = 0.375;
    pc.node = new LNode(xneg_node);
    C_pc.push_back(pc);

    pc.p = 0.4;
    pc.node = new LNode(zneg_node);
    C_pc.push_back(pc);

    Ruleset['C'] = &C_pc;

    std::vector<PostCondition> t_pc;

    pc.p = 0.2;
    pc.node = new LNode(t_node);
    t_pc.push_back(pc);

    Ruleset['t'] = &t_pc;

    system = Parse(iterations, c_node);


}

LNode* Cactus::getSystem() {
    return system;
}
