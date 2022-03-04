#include "crosshair.h"
#include <iostream>
Crosshair::Crosshair(OpenGLContext* context)
    : Drawable(context), m_player(nullptr)
{}

Crosshair::~Crosshair(){};

//We need access to our player, from the player we can then use forward vector and camera position

void Crosshair::createVBOdata() {

    GLuint idx[4] = {0, 1, 2, 3};

    float w = 0.05f;
    glm::vec4 centerPos = glm::vec4(m_player->mcr_camera->mcr_position + m_player->getLocalForward() * 3.f, 1.f);

    glm::vec4 crossRight = glm::vec4(m_player->getLocalRight(), 0.f);
    glm::vec4 crossUp = glm::vec4(m_player->getLocalUp(), 0.f);

    glm::vec4 pos[4] = {centerPos - w * crossUp,
                        centerPos + w * crossUp,
                        centerPos - w * crossRight,
                        centerPos + w * crossRight};

    glm::vec4 col[4] = {glm::vec4(1,1,1,1), glm::vec4(1,1,1,1),
                        glm::vec4(1,1,1,1), glm::vec4(1,1,1,1)};

    m_count = 4;

    generateIdx();
    mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdx);
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_count * sizeof(GLuint), idx, GL_STATIC_DRAW);
    generatePos();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufPos);
    mp_context->glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(glm::vec4), pos, GL_STATIC_DRAW);
    generateCol();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufCol);
    mp_context->glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(glm::vec4), col, GL_STATIC_DRAW);
}


GLenum Crosshair::drawMode() {
    return GL_LINES;
}
