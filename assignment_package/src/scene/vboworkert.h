#ifndef VBOWORKERT_H
#define VBOWORKERT_H

#include <QRunnable>
#include <QMutex>
#include "chunk.h"
#include <set>
#include "vboworker.h"

class VBOWorkerT : public QRunnable
{
private:
    Chunk* to_build;
    std::set<VBOData*>* updatedVBO; // SHARED DATA
    QMutex* vbo_lock;
public:
    VBOWorkerT(Chunk* to_build, std::set<VBOData*>* updatedVBO, QMutex* vbo_lock);
    void run() override;

    void generateVBOData(VBOData*);
};

#endif // VBOWORKER_H
