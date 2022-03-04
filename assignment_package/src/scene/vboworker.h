#ifndef VBOWORKER_H
#define VBOWORKER_H

#include <QRunnable>
#include <QMutex>
#include "chunk.h"
#include <unordered_set>


struct VBOData {
    std::vector<Vertex> data;
    std::vector<GLuint> indices;
    Chunk* c;
};

class VBOWorker : public QRunnable
{
private:
    Chunk* to_build;
    std::unordered_set<VBOData*>* updatedVBO; // SHARED DATA
    std::unordered_set<VBOData*>* updatedVBOT; // SHARED DATA
    QMutex* vbo_lock;
public:
    VBOWorker(Chunk* to_build, std::unordered_set<VBOData*>* updatedVBO, std::unordered_set<VBOData*>* updatedVBOT, QMutex* vbo_lock);
    void run() override;

    void generateVBOData(VBOData*, VBOData*);
    bool toDraw(BlockType t, BlockType n_t);
};

#endif // VBOWORKER_H
