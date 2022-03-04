#ifndef LNODE_H
#define LNODE_H


class LNode
{
public:
    LNode(LNode*);
    LNode(char c);

    char c;
    int layer;
    LNode* next;
};

#endif // LNODE_H
