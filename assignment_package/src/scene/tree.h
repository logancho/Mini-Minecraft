#ifndef TREE_H
#define TREE_H

#include "chunk.h"
#include "glm_includes.h"


class Tree
{
public:
    Tree(glm::ivec2 coord, std::vector<Chunk*> chunkArea);
    std::vector<Chunk*> chunkArea;
    glm::ivec3 treeBase;

    void drawBase1();
    void drawTrunk();
    void drawTop();

    Chunk* getChunk(int x, int z);
    void place(int x, int y, int z, BlockType);

};

#endif // TREE_H
