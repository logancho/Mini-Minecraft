#ifndef INVENTORYBAR_H
#define INVENTORYBAR_H

#include "player.h"
#include "drawable.h"
#include "glm_includes.h"


class InventoryBar : public Drawable
{
public:
    Player* m_player;
    InventoryBar(OpenGLContext* context);
    ~InventoryBar();
    void createVBOdata() override;
    GLenum drawMode() override;
};

#endif // INVENTORYBAR_H
