#pragma once

#include "player.h"
#include "drawable.h"
#include <glm_includes.h>



class Crosshair : public Drawable {
public:
    Player* m_player;
    Crosshair(OpenGLContext* context);
    ~Crosshair();
    void createVBOdata() override;
    GLenum drawMode() override;

};
