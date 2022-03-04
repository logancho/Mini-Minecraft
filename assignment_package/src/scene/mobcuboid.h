#pragma once

#include "drawable.h"
#include <glm_includes.h>

#include <QOpenGLContext>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>

enum NPCBlockType : unsigned char
{
    FACE, TORSO, LEFTARM, RIGHTARM, LEFTLEG, RIGHTLEG, SNOUT
};

class MobCuboid : public Drawable
{
public:
    static const int CUB_IDX_COUNT = 36;
    static const int CUB_VERT_COUNT = 24;
    NPCBlockType type = FACE;
    MobCuboid(OpenGLContext* context) : Drawable(context){};
    virtual ~MobCuboid(){}
    void createVBOdata() override;
    void createMCubeUVs(glm::vec3 (&cub_vert_UV)[CUB_VERT_COUNT]);
};
