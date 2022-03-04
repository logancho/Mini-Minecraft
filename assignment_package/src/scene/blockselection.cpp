#include "blockselection.h"

BlockSelection::BlockSelection(OpenGLContext* context)
    :Drawable(context), m_player(nullptr)
{

}

BlockSelection::~BlockSelection(){}

void BlockSelection::createVBOdata() {

    glm::vec4 centerPos = glm::vec4(m_player->mcr_camera->mcr_position + m_player->getLocalForward() * 3.f, 1.f);

    glm::vec4 crossRight = glm::vec4(m_player->getLocalRight(), 0.f);
    glm::vec4 crossUp = glm::vec4(m_player->getLocalUp(), 0.f);

    float w = m_player->mcr_camera->getWidth() / 400.f;
    float h = m_player->mcr_camera->getHeight() / 500.f;

    std::vector<glm::vec4> pos;
    std::vector<glm::vec4> nor;
    std::vector<glm::vec3> uv;
    std::vector<GLuint> idx;

    std::array<glm::vec2, 10> uvSelection = {
        glm::vec2(1.f / 16.f, 15.f / 16.f),
        glm::vec2(3.f / 16.f, 15.f / 16.f),
        glm::vec2(4.f / 16.f, 15.f / 16.f),
        glm::vec2(0.f / 16.f, 14.f / 16.f),
        glm::vec2(2.f / 16.f, 14.f / 16.f),
        glm::vec2(6.f / 16.f, 11.f / 16.f),
        glm::vec2(13.f / 16.f, 15.f / 16.f),
        glm::vec2(3.f / 16.f, 11.f / 16.f),
        glm::vec2(7.f / 16.f, 15.f / 16.f),
        glm::vec2(2.f / 16.f, 11.f / 16.f)
    };

    float width = 0.180f * w;
    float border = 0.019f * w;

    int v_count = 0;

    for (int i = 0; i < 10; i++) {
        pos.push_back(centerPos - (h + border) * crossUp + (-1.f * w + (i + 1) * border + i * width) * crossRight);
        pos.push_back(centerPos - (h + border + width) * crossUp + (-1.f * w + (i + 1) * border + i * width) * crossRight);
        pos.push_back(centerPos - (h + border + width) * crossUp + (-1.f * w + (i + 1) * border + (i + 1) * width) * crossRight);
        pos.push_back(centerPos - (h + border) * crossUp + (-1.f * w + (i + 1) * border + (i + 1) * width) * crossRight);

        uv.push_back(glm::vec3(uvSelection[i], 0.f) + glm::vec3(0.f / 16.f, 1.f / 16.f, 0.f));
        uv.push_back(glm::vec3(uvSelection[i], 0.f) + glm::vec3(0.f / 16.f, 0.f / 16.f, 0.f));
        uv.push_back(glm::vec3(uvSelection[i], 0.f) + glm::vec3(1.f / 16.f, 0.f / 16.f, 0.f));
        uv.push_back(glm::vec3(uvSelection[i], 0.f) + glm::vec3(1.f / 16.f, 1.f / 16.f, 0.f));

        nor.push_back(glm::vec4(1.f));
        nor.push_back(glm::vec4(1.f));
        nor.push_back(glm::vec4(1.f));
        nor.push_back(glm::vec4(1.f));

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
    generateUV();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufUV);
    mp_context->glBufferData(GL_ARRAY_BUFFER, uv.size() * sizeof(glm::vec3), uv.data(), GL_STATIC_DRAW);
    generateNor();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufNor);
    mp_context->glBufferData(GL_ARRAY_BUFFER, nor.size() * sizeof(glm::vec4), nor.data(), GL_STATIC_DRAW);

}

GLenum BlockSelection::drawMode() {
    return GL_TRIANGLES;
}
