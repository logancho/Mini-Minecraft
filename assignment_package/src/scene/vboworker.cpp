#include "vboworker.h"
#include <iostream>

VBOWorker::VBOWorker(Chunk* to_build, std::unordered_set<VBOData*>* updatedVBO, std::unordered_set<VBOData*>* updatedVBOT, QMutex* vbo_lock)
    : to_build(to_build), updatedVBO(updatedVBO), updatedVBOT(updatedVBOT), vbo_lock(vbo_lock)
{}

void VBOWorker::run() {
    // call the chunk's generate vbo data

    std::vector<Vertex> vbo_vec;
    std::vector<GLuint> idx_vec;

    std::vector<Vertex> vbo_vecT;
    std::vector<GLuint> idx_vecT;

    VBOData* data = new VBOData{vbo_vec, idx_vec, to_build};
    VBOData* dataT = new VBOData{vbo_vecT, idx_vecT, to_build};

    generateVBOData(data, dataT);

    vbo_lock->lock();
    updatedVBO->insert(data);
    updatedVBOT->insert(dataT);
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

bool VBOWorker::toDraw(BlockType t, BlockType n_t) {
    if (n_t == EMPTY ||
            n_t == YELLOW_FLOWER ||
            n_t == SEAGRASS ||
            n_t == RED_MUSHROOMS ||
            n_t == DEAD_PLANT ||
            n_t == TALL_GRASS ||
            n_t == BROWN_MUSHROOMS ||
            n_t == CORAL_PINK ||
            n_t == CORAL_PURPLE ||
            n_t == CORAL_ORANGE) {
        return true;
    } else if (n_t == WATER || n_t == LEAF || n_t == LAVA) {
        if (t == WATER || t == LAVA) {
            return false;
        }
        return true;
    } else {
        if (t == WATER || n_t == LEAF) {
            return true;
        }
    }
    return false;
}

void VBOWorker::generateVBOData(VBOData* data, VBOData* dataT) {
    std::vector<Vertex> VBOdata;
    std::vector<GLuint> idx;

    std::vector<Vertex> VBOdataT;
    std::vector<GLuint> idxT;

    int count = 0;
    int countT = 0;

    for (int x = 0; x < 16; x++) {
        for (int z = 0; z < 16; z++) {
            for (int y = 0; y < 256; y++) {
                BlockType t = to_build->getBlockAt(x, y, z);
                if (t != EMPTY) {
                    if (t == YELLOW_FLOWER
                            || t == SEAGRASS
                            || t == RED_MUSHROOMS
                            || t == DEAD_PLANT
                            || t == TALL_GRASS
                            || t == BROWN_MUSHROOMS
                            || t == CORAL_PINK
                            || t == CORAL_PURPLE
                            || t == CORAL_ORANGE) { // add switch cases for more "decor" type blocks
                        std::vector<glm::ivec2> xzCoords;
                        std::vector<glm::vec4> nor;

                        xzCoords.push_back(glm::ivec2(x, z));
                        xzCoords.push_back(glm::ivec2(x + 1, z));
                        xzCoords.push_back(glm::ivec2(x + 1, z + 1));
                        xzCoords.push_back(glm::ivec2(x, z + 1));

                        float sqrt2 = pow(2, 0.5);

                        nor.push_back(glm::vec4(sqrt2, 0, sqrt2, 1));
                        nor.push_back(glm::vec4(-1.f * sqrt2, 0, sqrt2, 1));

                        float a = 1.f / 16.f;

                        float y_0;
                        float x_0;

                        switch (t) {
                        case YELLOW_FLOWER:
                            y_0 = 15.f;
                            x_0 = 13.f;
                            break;
                        case RED_MUSHROOMS:
                            y_0 = 14.f;
                            x_0 = 12.f;
                            break;
                        case SEAGRASS:
                            y_0 = 10.f;
                            x_0 = 12.f;
                            break;
                        case BROWN_MUSHROOMS:
                            y_0 = 14.f;
                            x_0 = 13.f;
                            break;
                        case DEAD_PLANT:
                            y_0 = 12.f;
                            x_0 = 7.f;
                            break;
                        case TALL_GRASS:
                            y_0 = 10.f;
                            x_0 = 13.f;
                            break;
                        case CORAL_PINK:
                            y_0 = 2.f;
                            x_0 = 8.f;
                            break;
                        case CORAL_PURPLE:
                            y_0 = 3.f;
                            x_0 = 8.f;
                            break;
                        case CORAL_ORANGE:
                            y_0 = 4.f;
                            x_0 = 8.f;
                            break;
                        default:
                            y_0 = 1.f;
                            x_0 = 7.f;
                            break;
                        }

                        Vertex v;
                        int startX = to_build->starting_coord.x;
                        int startZ = to_build->starting_coord.y;

                        VBOdataT.push_back(v = {glm::vec4(startX + xzCoords[2].x, y, startZ + xzCoords[2].y, 1), nor[1], glm::vec3((x_0 + 1) * a, y_0 * a, 0)});
                        VBOdataT.push_back(v = {glm::vec4(startX + xzCoords[0].x, y, startZ + xzCoords[0].y, 1), nor[1], glm::vec3(x_0 * a, y_0 * a, 0)});
                        VBOdataT.push_back(v = {glm::vec4(startX + xzCoords[0].x, y + 1, startZ + xzCoords[0].y, 1), nor[1], glm::vec3(x_0 * a, (y_0 + 1) * a, 0)});
                        VBOdataT.push_back(v = {glm::vec4(startX + xzCoords[2].x, y + 1, startZ + xzCoords[2].y, 1), nor[1], glm::vec3((x_0 + 1) * a, (y_0 + 1) * a, 0)});

                        VBOdataT.push_back(v = {glm::vec4(startX + xzCoords[1].x, y, startZ + xzCoords[1].y, 1), nor[0], glm::vec3((x_0 + 1) * a, y_0 * a, 0)});
                        VBOdataT.push_back(v = {glm::vec4(startX + xzCoords[3].x, y, startZ + xzCoords[3].y, 1), nor[0], glm::vec3(x_0 * a, y_0 * a, 0)});
                        VBOdataT.push_back(v = {glm::vec4(startX + xzCoords[3].x, y + 1, startZ + xzCoords[3].y, 1), nor[0], glm::vec3(x_0 * a, (y_0 + 1) * a, 0)});
                        VBOdataT.push_back(v = {glm::vec4(startX + xzCoords[1].x, y + 1, startZ + xzCoords[1].y, 1), nor[0], glm::vec3((x_0 + 1) * a, (y_0 + 1) * a, 0)});

                        for (int i = 0; i < 2; i++) {
                            idxT.push_back(countT);
                            idxT.push_back(countT + 1);
                            idxT.push_back(countT + 2);
                            idxT.push_back(countT);
                            idxT.push_back(countT + 2);
                            idxT.push_back(countT + 3);
                            countT += 4;
                        }

                    } else {
                        for (Neighbor n : surrounding) {
                            glm::ivec3 neighboring = n.position + glm::ivec3(x, y, z);

                            // check if neighboring block is empty
                            // if neighboring block is out of bounds for current chunk, check neighboring chunk

                            BlockType n_t;
                            bool sameChunk = to_build->inBound(neighboring);

                            if (sameChunk) {
                                n_t = to_build->getBlockAt(neighboring.x, neighboring.y, neighboring.z);
                            } else {
                                n_t = to_build->neighborCheck(neighboring);
                            }

                            if (((t == WATER || t == LAVA) && n.d == YPOS && toDraw(t, n_t)) || (t != WATER && t != LAVA && toDraw(t, n_t))) {
                                // create vertex
                                std::vector<glm::ivec3> vertexPos = to_build->getFacePos(n.position, x, y, z);
                                glm::vec4 nor = glm::vec4(n.position.x, n.position.y, n.position.z, 1);
                                // change col to uv in milestone 2
                                float a = 1.f / 16.f;
                                float y_0;
                                float x_0;
                                bool opaque = true;
                                float animatable = 0;

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
                                    animatable = 1.0f;
                                    x_0 = 13.f;
                                    y_0 = 3.f;
                                    break;
                                case SNOW:
                                    opaque = false;
                                    x_0 = 2.f;
                                    y_0 = 11.f;
                                    break;
                                case LAVA:
                                    animatable = 1.0f;
                                    x_0 = 13.f;
                                    y_0 = 1.f;
                                    break;
                                case WOOD:
                                    x_0 = 4.f;
                                    y_0 = 14.f;
                                    if (n.d == Direction::YNEG || n.d == Direction::YPOS) {
                                        x_0 += 1.f;
                                    }
                                    break;
                                case LEAF:
                                    opaque = false;
                                    x_0 = 4.f;
                                    y_0 = 12.f;
                                    break;
                                case BEDROCK:
                                    x_0 = 1.f;
                                    y_0 = 14.f;
                                    break;
                                case SAND:
                                    x_0 = 2.f;
                                    y_0 = 14.f;
                                    break;
                                case RED_STONE:
                                    x_0 = 2.f;
                                    y_0 = 2.f;
                                    break;
                                case CACTUS:
                                    x_0 = 6.f;
                                    y_0 = 11.f;
                                    if (n.d == Direction::YNEG || n.d == Direction::YPOS) {
                                        x_0 += 1.f;
                                    }
                                    break;
                                case ICE:
                                    x_0 = 3.f;
                                    y_0 = 11.f;
                                    break;
                                case DARK_GRASS:
                                    if (n.d == Direction::YPOS) {
                                        //Top face:
                                        x_0 = 7.f;
                                        y_0 = 2.f;
                                    } else if (n.d == Direction::YNEG){
                                        //Bottom:
                                        x_0 = 2.f;
                                        y_0 = 15.f;
                                    } else {
                                        //Side:
                                        x_0 = 7.f;
                                        y_0 = 3.f;
                                    }
                                    break;
                                case DESERT:
                                    x_0 = 9.f;
                                    y_0 = 6.f;
                                    break;
                                case PLANK:
                                    x_0 = 4.f;
                                    y_0 = 15.f;
                                    break;
                                case COBBLE:
                                    x_0 = 0.f;
                                    y_0 = 14.f;
                                    break;
                                case BRICK:
                                    x_0 = 7.f;
                                    y_0 = 15.f;
                                    break;
                                default:
                                    // Other block types are not yet handled, so we default to debug purple
                                    x_0 = 7.f;
                                    y_0 = 1.f;
                                    break;
                                }

                                if (opaque) {
                                    Vertex v;
                                    VBOdata.push_back(v = {glm::vec4(vertexPos[0].x, vertexPos[0].y, vertexPos[0].z, 1), nor, glm::vec3(x_0 * a, y_0 * a, animatable)});
                                    VBOdata.push_back(v = {glm::vec4(vertexPos[1].x, vertexPos[1].y, vertexPos[1].z, 1), nor, glm::vec3((x_0 + 1) * a, y_0 * a, animatable)});
                                    VBOdata.push_back(v = {glm::vec4(vertexPos[2].x, vertexPos[2].y, vertexPos[2].z, 1), nor, glm::vec3((x_0 + 1) * a, (y_0 + 1) * a, animatable)});
                                    VBOdata.push_back(v = {glm::vec4(vertexPos[3].x, vertexPos[3].y, vertexPos[3].z, 1), nor, glm::vec3(x_0 * a, (y_0 + 1) * a, animatable)});

                                    idx.push_back(count);
                                    idx.push_back(count + 1);
                                    idx.push_back(count + 2);
                                    idx.push_back(count);
                                    idx.push_back(count + 2);
                                    idx.push_back(count + 3);
                                    count += 4;
                                } else {
                                    Vertex v;
                                    VBOdataT.push_back(v = {glm::vec4(vertexPos[0].x, vertexPos[0].y, vertexPos[0].z, 1), nor, glm::vec3(x_0 * a, y_0 * a, animatable)});
                                    VBOdataT.push_back(v = {glm::vec4(vertexPos[1].x, vertexPos[1].y, vertexPos[1].z, 1), nor, glm::vec3((x_0 + 1) * a, y_0 * a, animatable)});
                                    VBOdataT.push_back(v = {glm::vec4(vertexPos[2].x, vertexPos[2].y, vertexPos[2].z, 1), nor, glm::vec3((x_0 + 1) * a, (y_0 + 1) * a, animatable)});
                                    VBOdataT.push_back(v = {glm::vec4(vertexPos[3].x, vertexPos[3].y, vertexPos[3].z, 1), nor, glm::vec3(x_0 * a, (y_0 + 1) * a, animatable)});

                                    idxT.push_back(countT);
                                    idxT.push_back(countT + 1);
                                    idxT.push_back(countT + 2);
                                    idxT.push_back(countT);
                                    idxT.push_back(countT + 2);
                                    idxT.push_back(countT + 3);
                                    countT += 4;
                                }
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

    dataT->data = VBOdataT;
    dataT->indices = idxT;
    dataT->c = to_build;
}

