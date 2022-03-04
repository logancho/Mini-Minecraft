#include "lnode.h"

LNode::LNode(LNode* n)
    :c(n->c), layer(n->layer), next(n->next)
{

}

LNode::LNode(char c)
    :c(c), layer(0), next(nullptr)
{
    if (c == '1' || c == '2' || c == '3' || c == '4') {
        next = new LNode('[');
    } else if (c == '[') {
        next = new LNode('t');
        next->next = new LNode('c');
        next->next->next = new LNode(']');
    }
}
