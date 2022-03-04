#include "drawable.h"
#include <glm_includes.h>

Drawable::Drawable(OpenGLContext* context)
    : m_count(0), m_countT(0), m_bufIdx(), m_bufIdxT(), m_bufPos(), m_bufNor(), m_bufCol(), m_bufVerO(), m_bufVerT(), m_bufUV(),
      m_idxGenerated(false), m_posGenerated(false), m_norGenerated(false), m_colGenerated(false), m_verGeneratedO(false),

      mp_context(context)
{}

Drawable::~Drawable()
{
    destroyVBOdata();
}


void Drawable::destroyVBOdata()
{
    mp_context->glDeleteBuffers(1, &m_bufIdx);
    mp_context->glDeleteBuffers(1, &m_bufIdxT);
    mp_context->glDeleteBuffers(1, &m_bufPos);
    mp_context->glDeleteBuffers(1, &m_bufNor);
    mp_context->glDeleteBuffers(1, &m_bufCol);

    mp_context->glDeleteBuffers(1, &m_bufVerO);
    mp_context->glDeleteBuffers(1, &m_bufVerT);
    mp_context->glDeleteBuffers(1, &m_bufUV);
    m_idxGenerated = m_idxGeneratedT = m_posGenerated = m_norGenerated = m_colGenerated = m_verGeneratedO = m_verGeneratedT = m_UVGenerated = false;
    m_count = 0;

    mp_context->printGLErrorLog();
}

GLenum Drawable::drawMode()
{
    // Since we want every three indices in bufIdx to be
    // read to draw our Drawable, we tell that the draw mode
    // of this Drawable is GL_TRIANGLES

    // If we wanted to draw a wireframe, we would return GL_LINES

    return GL_TRIANGLES;
}

int Drawable::elemCount()
{
    return m_count;
}

int Drawable::elemCountT()
{
    return m_countT;
}

void Drawable::generateIdx()
{
    m_idxGenerated = true;
    // Create a VBO on our GPU and store its handle in bufIdx
    mp_context->glGenBuffers(1, &m_bufIdx);
}

void Drawable::generateIdxT()
{
    m_idxGeneratedT = true;
    // Create a VBO on our GPU and store its handle in bufIdx
    mp_context->glGenBuffers(1, &m_bufIdxT);
}

void Drawable::generatePos()
{
    m_posGenerated = true;
    // Create a VBO on our GPU and store its handle in bufPos
    mp_context->glGenBuffers(1, &m_bufPos);
}

void Drawable::generateNor()
{
    m_norGenerated = true;
    // Create a VBO on our GPU and store its handle in bufNor
    mp_context->glGenBuffers(1, &m_bufNor);
}

void Drawable::generateCol()
{
    m_colGenerated = true;
    // Create a VBO on our GPU and store its handle in bufCol
    mp_context->glGenBuffers(1, &m_bufCol);
}

void Drawable::generateVerO()
{
    m_verGeneratedO = true;
    mp_context->glGenBuffers(1, &m_bufVerO);
}

void Drawable::generateVerT()
{
    m_verGeneratedT = true;
    mp_context->glGenBuffers(1, &m_bufVerT);
}


void Drawable::generateUV()
{
    m_UVGenerated = true;
    mp_context->glGenBuffers(1, &m_bufUV);
}

bool Drawable::bindIdx()
{
    if(m_idxGenerated) {
        mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdx);
    }
    return m_idxGenerated;
}

bool Drawable::bindIdxT()
{
    if(m_idxGeneratedT) {
        mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdxT);
    }
    return m_idxGeneratedT;
}

bool Drawable::bindPos()
{
    if(m_posGenerated){
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufPos);
    }
    return m_posGenerated;
}

bool Drawable::bindNor()
{
    if(m_norGenerated){
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufNor);
    }
    return m_norGenerated;
}

bool Drawable::bindCol()
{
    if(m_colGenerated){
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufCol);
    }
    return m_colGenerated;
}

bool Drawable::bindVerO()
{
    if (m_verGeneratedO) {
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufVerO);
    }
    return m_verGeneratedO;
}

bool Drawable::bindVerT()
{
    if (m_verGeneratedT) {
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufVerT);
    }
    return m_verGeneratedT;
}


bool Drawable::bindUV()
{
    if (m_UVGenerated) {
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufUV);
    }
    return m_UVGenerated;
}







InstancedDrawable::InstancedDrawable(OpenGLContext *context)
    : Drawable(context), m_numInstances(0), m_bufPosOffset(-1), m_offsetGenerated(false)
{}

InstancedDrawable::~InstancedDrawable(){}

int InstancedDrawable::instanceCount() const {
    return m_numInstances;
}

void InstancedDrawable::generateOffsetBuf() {
    m_offsetGenerated = true;
    mp_context->glGenBuffers(1, &m_bufPosOffset);
}

bool InstancedDrawable::bindOffsetBuf() {
    if(m_offsetGenerated){
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufPosOffset);
    }
    return m_offsetGenerated;
}


void InstancedDrawable::clearOffsetBuf() {
    if(m_offsetGenerated) {
        mp_context->glDeleteBuffers(1, &m_bufPosOffset);
        m_offsetGenerated = false;
    }
}

void InstancedDrawable::clearColorBuf() {
    if(m_colGenerated) {
        mp_context->glDeleteBuffers(1, &m_bufCol);
        m_colGenerated = false;
    }
}
