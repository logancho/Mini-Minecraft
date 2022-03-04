#include "vboworkert.h"

VBOWorkerT::VBOWorkerT(Chunk* to_build, std::set<VBOData*>* updatedVBO, QMutex* vbo_lock)
    : to_build(to_build), updatedVBO(updatedVBO), vbo_lock(vbo_lock)
{}

void VBOWorkerT::run() {
    // call the chunk's generate vbo data

    std::vector<Vertex> vbo_vec;
    std::vector<GLuint> idx_vec;

    VBOData* data = new VBOData{vbo_vec, idx_vec, to_build};
    generateVBOData(data);

    vbo_lock->lock();
    updatedVBO->insert(data);
    vbo_lock->unlock();

    // create struct
    // lock
    // insert in set of data
    // unlock
}

const static Neighbor surrounding[6] = {
    {XPOS, glm::ivec3(1, 0, 0)},
    {XNEG, glm::ivec3(-1, 0, 0)},
    {YPOS, glm::ivec3(0, 1, 0)},
    {YNEG, glm::ivec3(0, -1, 0)},
    {ZPOS, glm::ivec3(0, 0, 1)},
    {ZNEG, glm::ivec3(0, 0, -1)},
};


void VBOWorkerT::generateVBOData(VBOData* data) {
    std::vector<Vertex> VBOdata;
    std::vector<GLuint> idx;

    int count = 0;

    for (int x = 0; x < 16; x++) {
        for (int z = 0; z < 16; z++) {
            for (int y = 0; y < 256; y++) {
                BlockType t = to_build->getBlockAt(x, y, z);
                if (t != EMPTY) {
                    for (Neighbor n : surrounding) {
                        glm::ivec3 neighboring = n.position + glm::ivec3(x, y, z);

                        // check if neighboring block is empty
                        // if neighboring block is out of bounds for current chunk, check neighboring chunk

                        bool sameChunk = to_build->inBound(neighboring);
                        if ((sameChunk && to_build->getBlockAt(neighboring.x, neighboring.y, neighboring.z) == EMPTY)
                                || (!sameChunk && to_build->neighborCheck(neighboring))) {
                            // create vertex
                            std::vector<glm::ivec3> vertexPos = to_build->getFacePos(n.position, x, y, z);
                            glm::vec4 nor = glm::vec4(n.position.x, n.position.y, n.position.z, 1);
                            // change col to uv in milestone 2
                            float a = 1.f / 16.f;
                            float y_0;
                            float x_0;
                            bool opaque = true;

                            switch(t) {
                            case GRASS:
                                if (n.d == Direction::YPOS) {
                                    //Top face:
                                    x_0 = 12.f;
                                    y_0 = 3.f;
                                } else if (n.d == Direction::YNEG){
                                    //Bottom:
                                    x_0 = 2.f;
                                    y_0 = 15.f;
                                } else {
                                    //Side:
                                    x_0 = 3.f;
                                    y_0 = 15.f;
                                }
                                //Other:
                                break;
                            case DIRT:
                                x_0 = 2.f;
                                y_0 = 15.f;
                                break;
                            case STONE:
                                x_0 = 1.f;
                                y_0 = 15.f;
                                break;
                            case WATER:
                                opaque = false;
                                x_0 = 13.f;
                                y_0 = 3.f;
                                break;
                            case SNOW:
                                opaque = false;
                                x_0 = 2.f;
                                y_0 = 11.f;
                                break;
                            case LAVA:
                                x_0 = 13.f;
                                y_0 = 1.f;
                                break;
                            default:
                                // Other block types are not yet handled, so we default to debug purple
                                x_0 = 7.f;
                                y_0 = 1.f;
                                break;
                            }
                            if (!opaque) {
                                Vertex v;
                                VBOdata.push_back(v = {glm::vec4(vertexPos[0].x, vertexPos[0].y, vertexPos[0].z, 1), nor, glm::vec2(x_0 * a, y_0 * a)});
                                VBOdata.push_back(v = {glm::vec4(vertexPos[1].x, vertexPos[1].y, vertexPos[1].z, 1), nor, glm::vec2((x_0 + 1) * a, y_0 * a)});
                                VBOdata.push_back(v = {glm::vec4(vertexPos[2].x, vertexPos[2].y, vertexPos[2].z, 1), nor, glm::vec2((x_0 + 1) * a, (y_0 + 1) * a)});
                                VBOdata.push_back(v = {glm::vec4(vertexPos[3].x, vertexPos[3].y, vertexPos[3].z, 1), nor, glm::vec2(x_0 * a, (y_0 + 1) * a)});

                                idx.push_back(count);
                                idx.push_back(count + 1);
                                idx.push_back(count + 2);
                                idx.push_back(count);
                                idx.push_back(count + 2);
                                idx.push_back(count + 3);
                                count += 4;
                            }
                        }
                    }
                }
            }
        }
    }
    data->data = VBOdata;
    data->indices = idx;
    data->c = to_build;
}

