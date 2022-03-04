#include "chunk.h"


Chunk::Chunk(OpenGLContext* context, int x, int z) : Drawable(context), starting_coord(glm::ivec2(x, z)), m_blocks(),
    m_neighbors{{XPOS, nullptr}, {XNEG, nullptr}, {ZPOS, nullptr}, {ZNEG, nullptr}}, topTerrainBlock(), flatness(), treeAt(glm::ivec2(-1, -1)), hasCactus(false)
{
    std::fill_n(m_blocks.begin(), 65536, EMPTY);
}

// Does bounds checking with at()
BlockType Chunk::getBlockAt(unsigned int x, unsigned int y, unsigned int z) const {
    return m_blocks.at(x + 16 * y + 16 * 256 * z);
}

// Exists to get rid of compiler warnings about int -> unsigned int implicit conversion
BlockType Chunk::getBlockAt(int x, int y, int z) const {
    return getBlockAt(static_cast<unsigned int>(x), static_cast<unsigned int>(y), static_cast<unsigned int>(z));
}

// Does bounds checking with at()
void Chunk::setBlockAt(unsigned int x, unsigned int y, unsigned int z, BlockType t) {
    m_blocks.at(x + 16 * y + 16 * 256 * z) = t;
}


const static std::unordered_map<Direction, Direction, EnumHash> oppositeDirection {
    {XPOS, XNEG},
    {XNEG, XPOS},
    {YPOS, YNEG},
    {YNEG, YPOS},
    {ZPOS, ZNEG},
    {ZNEG, ZPOS}
};

const static Neighbor surrounding[6] = {
    {XPOS, glm::ivec3(1, 0, 0)},
    {XNEG, glm::ivec3(-1, 0, 0)},
    {YPOS, glm::ivec3(0, 1, 0)},
    {YNEG, glm::ivec3(0, -1, 0)},
    {ZPOS, glm::ivec3(0, 0, 1)},
    {ZNEG, glm::ivec3(0, 0, -1)},
};

void Chunk::linkNeighbor(uPtr<Chunk> &neighbor, Direction dir) {
    if(neighbor != nullptr) {
        this->m_neighbors[dir] = neighbor.get();
        neighbor->m_neighbors[oppositeDirection.at(dir)] = this;
    }
}

void Chunk::createVBOdata() {

}

void Chunk::sendVBOdata(std::vector<Vertex> data, std::vector<GLuint> indices) {
    m_count = indices.size();

    if (m_count > 0) {
        generateIdx();
        mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdx);
        mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

        generateVerO();
        mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufVerO);
        mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, data.size() * sizeof (Vertex), data.data(), GL_STATIC_DRAW);
    }
}

void Chunk::sendVBOdataT(std::vector<Vertex> data, std::vector<GLuint> indices) {
    m_countT = indices.size();

    if (m_countT > 0) {
        generateIdxT();
        mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdxT);
        mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

        generateVerT();
        mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufVerT);
        mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, data.size() * sizeof (Vertex), data.data(), GL_STATIC_DRAW);
    }
}


bool Chunk::inBound(glm::ivec3 n) {

    return (n.x >= 0 && n.x < 16 && n.y >= 0 && n.y < 256 && n.z >= 0 && n.z < 16);
}

BlockType Chunk::neighborCheck(glm::ivec3 n) {
    // if not valid; check neighboring chunk

    Chunk* neighbor;

    if (n.x < 0) {
        // check chunk in -x direction

        neighbor = m_neighbors[XNEG];
        if (neighbor != nullptr) {
            return neighbor->getBlockAt(15, n.y, n.z);
        }

    } else if (n.x > 15) {
        // check chunk in +x direction

        neighbor = m_neighbors[XPOS];
        if (neighbor != nullptr) {
            return neighbor->getBlockAt(0, n.y, n.z);
        }

    } else if (n.z < 0) {
        // check chunk in -z direction

        neighbor = m_neighbors[ZNEG];
        if (neighbor != nullptr) {
            return neighbor->getBlockAt(n.x, n.y, 15);
        }

    } else if (n.z > 15) {
        //check chunk in +z direction

        neighbor = m_neighbors[ZPOS];
        if (neighbor != nullptr) {
            return neighbor->getBlockAt(n.x, n.y, 0);
        }
    }

    return EMPTY;
}

std::vector<glm::ivec3> Chunk::getFacePos(glm::ivec3 d, int x, int y, int z) {
    x += starting_coord.x;
    z += starting_coord.y;
    std::vector<glm::ivec3> vertexPos;
    if (d.x != 0) {
        if (d.x == 1) {
            x++;
        }
        vertexPos = {glm::ivec3(x, y, z), glm::ivec3(x, y, z+1), glm::ivec3(x, y+1, z+1), glm::ivec3(x, y+1, z)};
    } else if (d.y != 0) {
        if (d.y == 1) {
            y++;
        }
        vertexPos = {glm::ivec3(x, y, z), glm::ivec3(x+1, y, z), glm::ivec3(x+1, y, z+1), glm::ivec3(x, y, z+1)};
    } else if (d.z != 0) {
        if (d.z == 1) {
            z++;
        }
        vertexPos = {glm::ivec3(x, y, z), glm::ivec3(x+1, y, z), glm::ivec3(x+1, y+1, z), glm::ivec3(x, y+1, z)};
    }
    return vertexPos;
}

std::vector<Chunk*> Chunk::checkFromNeighbor(glm::ivec3 n) {

    Chunk* neighbor;
    std::vector<Chunk*> toAdd;

    n.x = (n.x % 16 + 16) % 16;
    n.y = (n.y % 16 + 16) % 16;
    n.z = (n.z % 16 + 16) % 16;

    if (n.x - 1 < 0) {
        // check chunk in -x direction

        neighbor = m_neighbors[XNEG];
        if (neighbor != nullptr) {
            toAdd.push_back(neighbor);
        }
    }

    if (n.x + 1 > 15) {
        // check chunk in +x direction

        neighbor = m_neighbors[XPOS];
        if (neighbor != nullptr) {
            toAdd.push_back(neighbor);
        }

    }

    if (n.z - 1 < 0) {
        // check chunk in -z direction

        neighbor = m_neighbors[ZNEG];
        if (neighbor != nullptr) {
            toAdd.push_back(neighbor);
        }

    }

    if (n.z + 1 > 15) {
        //check chunk in +z direction

        neighbor = m_neighbors[ZPOS];
        if (neighbor != nullptr) {
            toAdd.push_back(neighbor);
        }

    }

    return toAdd;

}
