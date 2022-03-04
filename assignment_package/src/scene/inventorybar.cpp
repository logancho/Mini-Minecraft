#include "inventorybar.h"

InventoryBar::InventoryBar(OpenGLContext* context)
    : Drawable(context), m_player(nullptr)
{

}

InventoryBar::~InventoryBar(){}

void InventoryBar::createVBOdata() {

    glm::vec4 centerPos = glm::vec4(m_player->mcr_camera->mcr_position + m_player->getLocalForward() * 3.f, 1.f);

    glm::vec4 crossRight = glm::vec4(m_player->getLocalRight(), 0.f);
    glm::vec4 crossUp = glm::vec4(m_player->getLocalUp(), 0.f);

    float w = m_player->mcr_camera->getWidth() / 400.f;
    float h = m_player->mcr_camera->getHeight() / 500.f;

    std::vector<glm::vec4> pos;
    std::vector<glm::vec4> col;
    std::vector<GLuint> idx;

    pos.push_back(centerPos - h * crossUp - w * crossRight);
    pos.push_back(centerPos - 1.42f * h * crossUp - w * crossRight);
    pos.push_back(centerPos - 1.42f * h * crossUp + w * crossRight);
    pos.push_back(centerPos - h * crossUp + w * crossRight);

    col.push_back(glm::vec4(0.3f, 0.3f, 0.3f, 1.f));
    col.push_back(glm::vec4(0.3f, 0.3f, 0.3f, 1.f));
    col.push_back(glm::vec4(0.3f, 0.3f, 0.3f, 1.f));
    col.push_back(glm::vec4(0.3f, 0.3f, 0.3f, 1.f));

    idx.push_back(0);
    idx.push_back(1);
    idx.push_back(2);
    idx.push_back(0);
    idx.push_back(2);
    idx.push_back(3);

    int v_count = 4;

    float width = 0.189f * w;
    float border = 0.01f * w;

    for (int i = 0; i < 10; i++) {
        pos.push_back(centerPos - (h + border) * crossUp + (-1.f * w + (i + 1) * border + i * width) * crossRight);
        pos.push_back(centerPos - (h + border + width) * crossUp + (-1.f * w + (i + 1) * border + i * width) * crossRight);
        pos.push_back(centerPos - (h + border + width) * crossUp + (-1.f * w + (i + 1) * border + (i + 1) * width) * crossRight);
        pos.push_back(centerPos - (h + border) * crossUp + (-1.f * w + (i + 1) * border + (i + 1) * width) * crossRight);

        col.push_back(glm::vec4(0.2f, 0.1f, 0.f, 1.f));
        col.push_back(glm::vec4(0.2f, 0.1f, 0.f, 1.f));
        col.push_back(glm::vec4(0.2f, 0.1f, 0.f, 1.f));
        col.push_back(glm::vec4(0.2f, 0.1f, 0.f, 1.f));

        idx.push_back(v_count);
        idx.push_back(v_count + 1);
        idx.push_back(v_count + 2);
        idx.push_back(v_count);
        idx.push_back(v_count + 2);
        idx.push_back(v_count + 3);

        v_count += 4;
    }

    m_count = idx.size();

    generateIdx();
    mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdx);
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_count * sizeof(GLuint), idx.data(), GL_STATIC_DRAW);
    generatePos();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufPos);
    mp_context->glBufferData(GL_ARRAY_BUFFER, pos.size() * sizeof(glm::vec4), pos.data(), GL_STATIC_DRAW);
    generateCol();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufCol);
    mp_context->glBufferData(GL_ARRAY_BUFFER, col.size() * sizeof(glm::vec4), col.data(), GL_STATIC_DRAW);
}

GLenum InventoryBar::drawMode() {
    return GL_TRIANGLES;
}
