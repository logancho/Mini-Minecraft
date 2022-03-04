#include "tree.h"
#include <iostream>

Tree::Tree(glm::ivec2 coord, std::vector<Chunk*> chunkArea)
    :chunkArea(chunkArea)
{
    int minY = 256;

    for (int x = coord.x - 3; x <= coord.x + 3; x++) {
        for (int z = coord.y - 3; z <= coord.y + 3; z++) {
                minY = std::min(minY, getChunk(x, z)->topTerrainBlock[((x % 16 + 16) % 16) * 16 + ((z % 16 + 16) % 16)]);
        }
    }

    treeBase = glm::ivec3(coord.x, minY, coord.y);

}

void Tree::drawBase1() {
    // layer 1:

    for (int i = 0; i < 2; i++) {
        place(0, i, -3, WOOD);
        place(1, i, -3, WOOD);
        place(-1, i, -2, WOOD);
        place(0, i, -2, WOOD);
        place(1, i, -2, WOOD);
        place(2, i, -2, WOOD);
        place(-2, i, -1, WOOD);
        place(-1, i, -1, WOOD);
        place(0, i, -1, WOOD);
        place(1, i, -1, WOOD);
        place(2, i, -1, WOOD);
        place(3, i, -1, WOOD);
        place(-3, i, 0, WOOD);
        place(-2, i, 0, WOOD);
        place(-1, i, 0, WOOD);
        place(0, i, 0, WOOD);
        place(1, i, 0, WOOD);
        place(2, i, 0, WOOD);
        place(3, i, 0, WOOD);
        place(-2, i, 1, WOOD);
        place(-1, i, 1, WOOD);
        place(0, i, 1, WOOD);
        place(1, i, 1, WOOD);
        place(2, i, 1, WOOD);
        place(3, i, 1, WOOD);
        place(-1, i, 2, WOOD);
        place(0, i, 2, WOOD);
        place(1, i, 2, WOOD);
        place(0, i, 3, WOOD);
    }

    place(0, 2, -2, WOOD);
    place(1, 2, -2, WOOD);
    place(-1, 2, -1, WOOD);
    place(0, 2, -1, WOOD);
    place(1, 2, -1, WOOD);
    place(-2, 2, 0, WOOD);
    place(-1, 2, 0, WOOD);
    place(0, 2, 0, WOOD);
    place(1, 2, 0, WOOD);
    place(2, 2, 0, WOOD);
    place(-2, 2, 1, WOOD);
    place(-1, 2, 1, WOOD);
    place(0, 2, 1, WOOD);
    place(1, 2, 1, WOOD);
    place(2, 2, 1, WOOD);
    place(0, 2, 2, WOOD);
    place(1, 2, 2, WOOD);

    place(0, 3, -2, WOOD);
    place(-1, 3, -1, WOOD);
    place(0, 3, -1, WOOD);
    place(1, 3, -1, WOOD);
    place(-2, 3, 0, WOOD);
    place(-1, 3, 0, WOOD);
    place(0, 3, 0, WOOD);
    place(1, 3, 0, WOOD);
    place(-1, 3, 1, WOOD);
    place(0, 3, 1, WOOD);
    place(1, 3, 1, WOOD);
    place(2, 3, 1, WOOD);
    place(0, 3, 2, WOOD);

}

void Tree::drawTop() {

    std::vector<glm::vec2> Trees;

    for (int i = 0; i < 9; i++) {
        if (i != 4 && chunkArea[i]->treeAt.x != -1) {
            Trees.push_back(glm::ivec2(glm::floor(i / 3.f) * 16, (i % 3) * 16) + chunkArea[i]->treeAt);
        }
    }

    for (int x = -15; x <= 15; x++) {
        for (int z = -15; z <= 15; z++) {
            float dist = std::pow(std::pow(x, 2.f) + std::pow(z, 2.f), 0.5f);
            float minAdjDist = 15.5;

            for (glm::vec2 loc : Trees) {
                minAdjDist = glm::min(minAdjDist, glm::distance(loc, glm::vec2(treeBase.x + x, treeBase.z + z)));
            }

            float drawValue = 1 - (dist / minAdjDist);
            int upper = glm::floor(pow(drawValue, 0.4) * 10);
            int lower = glm::floor(drawValue * 5);

            for (int i = 0; i <= upper; i++) {
                place(x, 150 - treeBase.y + i, z, LEAF);
            }
            for (int i = 0; i <= lower; i++) {
                place(x, 150 - treeBase.y - i, z, LEAF);
            }
        }
    }
}

void Tree::drawTrunk() {
    for (int y = 0; y < 150 - treeBase.y; y++) {
        for (int x = -1; x <= 1; x++) {
            for (int z = -1; z <= 1; z++) {
                place(x, y + 3, z, WOOD);
            }
        }
    }
}


Chunk* Tree::getChunk(int x, int z) {
    return chunkArea[glm::floor(x / 16.f) * 3 + glm::floor(z / 16.f)];
}

void Tree::place(int x, int y, int z, BlockType t) {
    Chunk* c = getChunk(treeBase.x + x, treeBase.z + z);

    if (c->getBlockAt((treeBase.x + x) % 16, treeBase.y + y, (treeBase.z + z) % 16) == BlockType::EMPTY ||
            c->getBlockAt((treeBase.x + x) % 16, treeBase.y + y, (treeBase.z + z) % 16) == BlockType::YELLOW_FLOWER) {
        c->setBlockAt((treeBase.x + x) % 16, treeBase.y + y, (treeBase.z + z) % 16, t);
    }
}
