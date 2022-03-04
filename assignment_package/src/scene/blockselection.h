#ifndef BLOCKSELECTION_H
#define BLOCKSELECTION_H

#include "player.h"
#include "drawable.h"
#include "glm_includes.h"

class BlockSelection : public Drawable
{
public:
    Player* m_player;
    BlockSelection(OpenGLContext* context);
    ~BlockSelection();
    void createVBOdata() override;
    GLenum drawMode() override;

};

#endif // BLOCKSELECTION_H
