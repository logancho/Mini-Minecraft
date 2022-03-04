#ifndef TREEBUILDWORKER_H
#define TREEBUILDWORKER_H

#include <QRunnable>


class TreeBuildWorker : public QRunnable
{
private:

public:
    TreeBuildWorker();
    void run() override;
};

#endif // TREEBUILDWORKER_H
