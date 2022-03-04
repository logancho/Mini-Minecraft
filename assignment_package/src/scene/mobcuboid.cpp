#include "mobcuboid.h"
#include <glm_includes.h>
#include <iostream>
#include <stdlib.h>


glm::vec4 GetMCubeNormal(const glm::vec4& P)
{
    int idx = 0;
    float val = -1;
    for(int i = 0; i < 3; i++){
        if(glm::abs(P[i]) > val){
            idx = i;
            val = glm::abs(P[i]);
        }
    }
    glm::vec4 N(0,0,0,0);
    N[idx] = glm::sign(P[idx]);
    return N;
}

//These are functions that are only defined in this cpp file. They're used for organizational purposes
//when filling the arrays used to hold the vertex and index data.
void createMCubeVertexPositions(glm::vec4 (&cub_vert_pos)[MobCuboid::CUB_VERT_COUNT])
{
    int idx = 0;
    //Front face

    //UR
    cub_vert_pos[idx++] = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    //LR
    cub_vert_pos[idx++] = glm::vec4(1.0f, 0.0f, 1.0f, 1.0f);
    //LL
    cub_vert_pos[idx++] = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
    //UL
    cub_vert_pos[idx++] = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f);

    //Right face
    //UR
    cub_vert_pos[idx++] = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
    //LR
    cub_vert_pos[idx++] = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
    //LL
    cub_vert_pos[idx++] = glm::vec4(1.0f, 0.0f, 1.0f, 1.0f);
    //UL
    cub_vert_pos[idx++] = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

    //Left face
    //UR
    cub_vert_pos[idx++] = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f);
    //LR
    cub_vert_pos[idx++] = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
    //LL
    cub_vert_pos[idx++] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    //UL
    cub_vert_pos[idx++] = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);

    //Back face
    //UR
    cub_vert_pos[idx++] = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
    //LR
    cub_vert_pos[idx++] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    //LL
    cub_vert_pos[idx++] = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
    //UL
    cub_vert_pos[idx++] = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);

    //Top face
    //UR
    cub_vert_pos[idx++] = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
    //LR
    cub_vert_pos[idx++] = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    //LL
    cub_vert_pos[idx++] = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f);
    //UL
    cub_vert_pos[idx++] = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);

    //Bottom face
    //UR
    cub_vert_pos[idx++] = glm::vec4(1.0f, 0.0f, 1.0f, 1.0f);
    //LR
    cub_vert_pos[idx++] = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
    //LL
    cub_vert_pos[idx++] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    //UL
    cub_vert_pos[idx++] = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);

    glm::vec4 offset = glm::vec4(-0.5f, -0.5f, -0.5f, 0.f);
    for (int i = 0; i < 24; i++) {
        cub_vert_pos[i] += offset;
    }
}


void createMCubeVertexNormals(glm::vec4 (&cub_vert_nor)[MobCuboid::CUB_VERT_COUNT])
{
    int idx = 0;
    //Front
    for(int i = 0; i < 4; i++){
        cub_vert_nor[idx++] = glm::vec4(0,0,1,0);
    }
    //Right
    for(int i = 0; i < 4; i++){
        cub_vert_nor[idx++] = glm::vec4(1,0,0,0);
    }
    //Left
    for(int i = 0; i < 4; i++){
        cub_vert_nor[idx++] = glm::vec4(-1,0,0,0);
    }
    //Back
    for(int i = 0; i < 4; i++){
        cub_vert_nor[idx++] = glm::vec4(0,0,-1,0);
    }
    //Top
    for(int i = 0; i < 4; i++){
        cub_vert_nor[idx++] = glm::vec4(0,1,0,0);
    }
    //Bottom
    for(int i = 0; i < 4; i++){
        cub_vert_nor[idx++] = glm::vec4(0,-1,0,0);
    }
}

void createMCubeIndices(GLuint (&cub_idx)[MobCuboid::CUB_IDX_COUNT])
{
    int idx = 0;
    for(int i = 0; i < 6; i++){
        cub_idx[idx++] = i*4;
        cub_idx[idx++] = i*4+1;
        cub_idx[idx++] = i*4+2;
        cub_idx[idx++] = i*4;
        cub_idx[idx++] = i*4+2;
        cub_idx[idx++] = i*4+3;
    }
}
void createMCubeColors(glm::vec4 (&cub_vert_col)[MobCuboid::CUB_VERT_COUNT]) {
    int idx = 0;
    float r = 0.f;
    float g = 1.f;
    float b = 1.f;
    //Front
    for(int i = 0; i < 4; i++){
        cub_vert_col[idx++] = glm::vec4(r,g,b,1);
    }
    //Right
    for(int i = 0; i < 4; i++){
        cub_vert_col[idx++] = glm::vec4(r,g,b,1);
    }
    //Left
    for(int i = 0; i < 4; i++){
        cub_vert_col[idx++] = glm::vec4(r,g,b,1);
    }
    //Back
    for(int i = 0; i < 4; i++){
        cub_vert_col[idx++] = glm::vec4(r,g,b,1);
    }
    //Top
    for(int i = 0; i < 4; i++){
        cub_vert_col[idx++] = glm::vec4(r,g,b,1);
    }
    //Bottom
    for(int i = 0; i < 4; i++){
        cub_vert_col[idx++] = glm::vec4(r,g,b,1);
    }
}

void createMCubeUVsFACE(glm::vec3 (&cub_vert_UV)[MobCuboid::CUB_VERT_COUNT]) {
    float a = 1.f / 8.f;
    float y_0;
    float x_0;
    float animatable = 0;
    int idx = 0;
    //Front, right, left, back, top, bottom
    for (int i = 0; i < 6; i++) {
        switch (i) {
        case 0: //Back
            x_0 = 3.f;
            y_0 = 6.f;
            break;
        case 1: //Left
            x_0 = 0.f;
            y_0 = 6.f;
            break;
        case 2: //Right
            x_0 = 2.f;
            y_0 = 6.f;
            break;
        case 3: //Front
            x_0 = 1.f;
            y_0 = 6.f;
            break;
        case 4: //Top
            x_0 = 1.f;
            y_0 = 7.f;
            break;
        case 5: //Bottom
            x_0 = 2.f;
            y_0 = 7.f;
            break;
        default: //Default front
            x_0 = 1.f;
            y_0 = 6.f;
            break;
        }
        //UR, LR, LL, UL
        cub_vert_UV[idx++] = glm::vec3((x_0 + 1) * a, (y_0 + 1) * a, animatable);
        cub_vert_UV[idx++] = glm::vec3((x_0 + 1) * a, y_0 * a, animatable);
        cub_vert_UV[idx++] = glm::vec3(x_0 * a, y_0 * a, animatable);
        cub_vert_UV[idx++] = glm::vec3(x_0 * a, (y_0 + 1) * a, animatable);
    }
}

void createMCubeUVsTORSO(glm::vec3 (&cub_vert_UV)[MobCuboid::CUB_VERT_COUNT]) {
    float a = 1.f / 8.f;
    float y_0;
    float x_0;
    float animatable = 0;
    int idx = 0;
    float h;
    float w;
    //Front, right, left, back, top, bottom
    for (int i = 0; i < 6; i++) {
        switch (i) {
        case 0: //Back
            x_0 = 4.f;
            y_0 = 4.f;
            h = 1.5f;
            w = 1.f;
            break;
        case 1: //Left
            x_0 = 2.f;
            y_0 = 4.f;
            h = 1.5f;
            w = 0.5f;
            break;
        case 2: //Right
            x_0 = 3.5f;
            y_0 = 4.f;
            h = 1.5f;
            w = 0.5f;
            break;
        case 3: //Front
            x_0 = 2.5f;
            y_0 = 4.f;
            h = 1.5f;
            w = 1.f;
            break;
        case 4: //Top
            x_0 = 2.5f;
            y_0 = 5.5f;
            h = 0.5f;
            w = 1.f;
            break;
        case 5: //Bottom
            x_0 = 3.5f;
            y_0 = 5.5f;
            h = 0.5f;
            w = 1.f;
            break;
        default: //Default front
            x_0 = 2.5f;
            y_0 = 4.f;
            h = 1.5f;
            w = 1.f;
            break;
        }
        //UR, LR, LL, UL
        cub_vert_UV[idx++] = glm::vec3((x_0 + w) * a, (y_0 + h) * a, animatable);
        cub_vert_UV[idx++] = glm::vec3((x_0 + w) * a, y_0 * a, animatable);
        cub_vert_UV[idx++] = glm::vec3(x_0 * a, y_0 * a, animatable);
        cub_vert_UV[idx++] = glm::vec3(x_0 * a, (y_0 + h) * a, animatable);
    }
}
void createMCubeUVsRIGHTARM(glm::vec3 (&cub_vert_UV)[MobCuboid::CUB_VERT_COUNT]) {
    float a = 1.f / 8.f;
    float y_0;
    float x_0;
    float animatable = 0;
    int idx = 0;
    float h;
    float w;
    //Front, right, left, back, top, bottom
    for (int i = 0; i < 6; i++) {
        switch (i) {
        case 0: //Back
            x_0 = 5.5f;
            y_0 = 0.f;
            h = 1.5f;
            w = 0.5f;
            break;
        case 1: //Left
            x_0 = 4.f;
            y_0 = 0.f;
            h = 1.5f;
            w = 0.5f;
            break;
        case 2: //Right
            x_0 = 5.0f;
            y_0 = 0.f;
            h = 1.5f;
            w = 0.5f;
            break;
        case 3: //Front
            x_0 = 4.5f;
            y_0 = 0.f;
            h = 1.5f;
            w = 0.5f;
            break;
        case 4: //Top
            x_0 = 4.5f;
            y_0 = 1.5f;
            h = 0.5f;
            w = 0.5f;
            break;
        case 5: //Bottom
            x_0 = 5.f;
            y_0 = 1.5f;
            h = 0.5f;
            w = 0.5f;
            break;
        default: //Default front
            x_0 = 4.5f;
            y_0 = 0.f;
            h = 1.5f;
            w = 0.5f;
            break;
        }
        //UR, LR, LL, UL
        cub_vert_UV[idx++] = glm::vec3((x_0 + w) * a, (y_0 + h) * a, animatable);
        cub_vert_UV[idx++] = glm::vec3((x_0 + w) * a, y_0 * a, animatable);
        cub_vert_UV[idx++] = glm::vec3(x_0 * a, y_0 * a, animatable);
        cub_vert_UV[idx++] = glm::vec3(x_0 * a, (y_0 + h) * a, animatable);
    }
}
void createMCubeUVsLEFTARM(glm::vec3 (&cub_vert_UV)[MobCuboid::CUB_VERT_COUNT]) {
    float a = 1.f / 8.f;
    float y_0;
    float x_0;
    float animatable = 0;
    int idx = 0;
    float h;
    float w;
    //Front, right, left, back, top, bottom
    for (int i = 0; i < 6; i++) {
        switch (i) {
        case 0: //Back
            x_0 = 6.5f;
            y_0 = 4.f;
            h = 1.5f;
            w = 0.5f;
            break;
        case 1: //Left
            x_0 = 5.f;
            y_0 = 4.f;
            h = 1.5f;
            w = 0.5f;
            break;
        case 2: //Right
            x_0 = 6.0f;
            y_0 = 4.f;
            h = 1.5f;
            w = 0.5f;
            break;
        case 3: //Front
            x_0 = 5.5f;
            y_0 = 4.f;
            h = 1.5f;
            w = 0.5f;
            break;
        case 4: //Top
            x_0 = 5.5f;
            y_0 = 5.5f;
            h = 0.5f;
            w = 0.5f;
            break;
        case 5: //Bottom
            x_0 = 6.f;
            y_0 = 5.5f;
            h = 0.5f;
            w = 0.5f;
            break;
        default: //Default front
            x_0 = 5.5f;
            y_0 = 4.f;
            h = 1.5f;
            w = 0.5f;
            break;
        }
        //UR, LR, LL, UL
        cub_vert_UV[idx++] = glm::vec3((x_0 + w) * a, (y_0 + h) * a, animatable);
        cub_vert_UV[idx++] = glm::vec3((x_0 + w) * a, y_0 * a, animatable);
        cub_vert_UV[idx++] = glm::vec3(x_0 * a, y_0 * a, animatable);
        cub_vert_UV[idx++] = glm::vec3(x_0 * a, (y_0 + h) * a, animatable);
    }
}
void createMCubeUVsRIGHTLEG(glm::vec3 (&cub_vert_UV)[MobCuboid::CUB_VERT_COUNT]) {
    float a = 1.f / 8.f;
    float y_0;
    float x_0;
    float animatable = 0;
    int idx = 0;
    float h;
    float w;
    //Front, right, left, back, top, bottom
    for (int i = 0; i < 6; i++) {
        switch (i) {
        case 0: //Back
            x_0 = 3.5f;
            y_0 = 0.f;
            h = 1.5f;
            w = 0.5f;
            break;
        case 1: //Left
            x_0 = 2.f;
            y_0 = 0.f;
            h = 1.5f;
            w = 0.5f;
            break;
        case 2: //Right
            x_0 = 3.0f;
            y_0 = 0.f;
            h = 1.5f;
            w = 0.5f;
            break;
        case 3: //Front
            x_0 = 2.5f;
            y_0 = 0.f;
            h = 1.5f;
            w = 0.5f;
            break;
        case 4: //Top
            x_0 = 2.5f;
            y_0 = 1.5f;
            h = 0.5f;
            w = 0.5f;
            break;
        case 5: //Bottom
            x_0 = 3.f;
            y_0 = 1.5f;
            h = 0.5f;
            w = 0.5f;
            break;
        default: //Default front
            x_0 = 2.5f;
            y_0 = 0.f;
            h = 1.5f;
            w = 0.5f;
            break;
        }
        //UR, LR, LL, UL
        cub_vert_UV[idx++] = glm::vec3((x_0 + w) * a, (y_0 + h) * a, animatable);
        cub_vert_UV[idx++] = glm::vec3((x_0 + w) * a, y_0 * a, animatable);
        cub_vert_UV[idx++] = glm::vec3(x_0 * a, y_0 * a, animatable);
        cub_vert_UV[idx++] = glm::vec3(x_0 * a, (y_0 + h) * a, animatable);
    }
}
void createMCubeUVsLEFTLEG(glm::vec3 (&cub_vert_UV)[MobCuboid::CUB_VERT_COUNT]) {
    float a = 1.f / 8.f;
    float y_0;
    float x_0;
    float animatable = 0;
    int idx = 0;
    float h;
    float w;
    //Front, right, left, back, top, bottom
    for (int i = 0; i < 6; i++) {
        switch (i) {
        case 0: //Back
            x_0 = 1.5f;
            y_0 = 4.f;
            h = 1.5f;
            w = 0.5f;
            break;
        case 1: //Left
            x_0 = 0.f;
            y_0 = 4.f;
            h = 1.5f;
            w = 0.5f;
            break;
        case 2: //Right
            x_0 = 1.0f;
            y_0 = 4.f;
            h = 1.5f;
            w = 0.5f;
            break;
        case 3: //Front
            x_0 = 0.5f;
            y_0 = 4.f;
            h = 1.5f;
            w = 0.5f;
            break;
        case 4: //Top
            x_0 = 0.5f;
            y_0 = 5.5f;
            h = 0.5f;
            w = 0.5f;
            break;
        case 5: //Bottom
            x_0 = 1.f;
            y_0 = 5.5f;
            h = 0.5f;
            w = 0.5f;
            break;
        default: //Default front
            x_0 = 0.5f;
            y_0 = 4.f;
            h = 1.5f;
            w = 0.5f;
            break;
        }
        //UR, LR, LL, UL
        cub_vert_UV[idx++] = glm::vec3((x_0 + w) * a, (y_0 + h) * a, animatable);
        cub_vert_UV[idx++] = glm::vec3((x_0 + w) * a, y_0 * a, animatable);
        cub_vert_UV[idx++] = glm::vec3(x_0 * a, y_0 * a, animatable);
        cub_vert_UV[idx++] = glm::vec3(x_0 * a, (y_0 + h) * a, animatable);
    }
}
void createMCubeUVsSNOUT(glm::vec3 (&cub_vert_UV)[MobCuboid::CUB_VERT_COUNT]) {
    float a = 1.f / 8.f;
    float y_0;
    float x_0;
    float animatable = 0;
    int idx = 0;
    float h;
    float w;
    //Front, right, left, back, top, bottom
    for (int i = 0; i < 6; i++) {
        x_0 = 0.f;
        y_0 = 7.f;
        h = 1.f;
        w = 1.f;
        //UR, LR, LL, UL
        cub_vert_UV[idx++] = glm::vec3((x_0 + w) * a, (y_0 + h) * a, animatable);
        cub_vert_UV[idx++] = glm::vec3((x_0 + w) * a, y_0 * a, animatable);
        cub_vert_UV[idx++] = glm::vec3(x_0 * a, y_0 * a, animatable);
        cub_vert_UV[idx++] = glm::vec3(x_0 * a, (y_0 + h) * a, animatable);
    }
}
void MobCuboid::createMCubeUVs(glm::vec3 (&cub_vert_UV)[MobCuboid::CUB_VERT_COUNT]) {
    switch (type) {
    case FACE:
        createMCubeUVsFACE(cub_vert_UV);
        break;
    case TORSO:
        createMCubeUVsTORSO(cub_vert_UV);
        break;
    case RIGHTARM:
        createMCubeUVsRIGHTARM(cub_vert_UV);
        break;
    case LEFTARM:
        createMCubeUVsLEFTARM(cub_vert_UV);
        break;
    case RIGHTLEG:
        createMCubeUVsRIGHTLEG(cub_vert_UV);
        break;
    case LEFTLEG:
        createMCubeUVsLEFTLEG(cub_vert_UV);
        break;
    case SNOUT:
        createMCubeUVsSNOUT(cub_vert_UV);
        break;
    default: //Default FACE
        createMCubeUVsFACE(cub_vert_UV);
        break;
    }
}

void MobCuboid::createVBOdata()
{
    GLuint sph_idx[CUB_IDX_COUNT];
    glm::vec4 sph_vert_pos[CUB_VERT_COUNT];
    glm::vec4 sph_vert_nor[CUB_VERT_COUNT];
    glm::vec4 sph_vert_col[CUB_VERT_COUNT];
    glm::vec3 sph_vert_UV[CUB_VERT_COUNT];

    createMCubeVertexPositions(sph_vert_pos);
    createMCubeVertexNormals(sph_vert_nor);
    createMCubeIndices(sph_idx);
    createMCubeColors(sph_vert_col);
    createMCubeUVs(sph_vert_UV);

    m_count = CUB_IDX_COUNT;

    // Create a VBO on our GPU and store its handle in bufIdx
    generateIdx();
    // Tell OpenGL that we want to perform subsequent operations on the VBO referred to by bufIdx
    // and that it will be treated as an element array buffer (since it will contain triangle indices)
    mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdx);
    // Pass the data stored in cyl_idx into the bound buffer, reading a number of bytes equal to
    // SPH_IDX_COUNT multiplied by the size of a GLuint. This data is sent to the GPU to be read by shader programs.
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, CUB_IDX_COUNT * sizeof(GLuint), sph_idx, GL_STATIC_DRAW);

    // The next few sets of function calls are basically the same as above, except bufPos and bufNor are
    // array buffers rather than element array buffers, as they store vertex attributes like position.
    generatePos();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufPos);
    mp_context->glBufferData(GL_ARRAY_BUFFER, CUB_VERT_COUNT * sizeof(glm::vec4), sph_vert_pos, GL_STATIC_DRAW);

    generateNor();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufNor);
    mp_context->glBufferData(GL_ARRAY_BUFFER, CUB_VERT_COUNT * sizeof(glm::vec4), sph_vert_nor, GL_STATIC_DRAW);

    //UV
    generateUV();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufUV);
    mp_context->glBufferData(GL_ARRAY_BUFFER, CUB_VERT_COUNT * sizeof(glm::vec3), sph_vert_UV, GL_STATIC_DRAW);
}

