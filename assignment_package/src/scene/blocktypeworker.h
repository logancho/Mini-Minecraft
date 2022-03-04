#ifndef BLOCKTYPEWORKER_H
#define BLOCKTYPEWORKER_H

#include <QRunnable>
#include "glm_includes.h"
#include "chunk.h"
#include <set>
#include <QMutex>
#include "biome.h"
#include <unordered_set>
#include <queue>


class BlockTypeWorker : public QRunnable
{
private:
    glm::ivec2 bottom_corner;
    std::vector<Chunk*> to_fill;
    std::unordered_set<glm::ivec2*>* treeCoords;
    std::unordered_set<Chunk*>* done;
    std::queue<glm::ivec2*>* flat;
    QMutex* block_lock;
    QMutex* tree_lock;
    QMutex* flat_lock;

public:
    BlockTypeWorker(glm::ivec2 bottom_corner,
                    std::vector<Chunk*> to_fill,
                    std::unordered_set<Chunk*>* done,
                    std::unordered_set<glm::ivec2*>* treeCoords,
                    std::queue<glm::ivec2*>* flatness,
                    QMutex* block_lock, QMutex* tree_lock, QMutex* flat_lock);
    void run() override;
    void CreateTerrain(Chunk* c);
};

#endif // BLOCKTYPEWORKER_H
