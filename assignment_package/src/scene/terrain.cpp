#include "terrain.h"
#include "cube.h"
#include "tree.h"

#include <stdexcept>
#include <iostream>
#include <queue>
#include <stack>

Terrain::Terrain(OpenGLContext *context)
    : m_chunks(), m_generatedTerrain(), mp_context(context), expansionTry(0), need_VBO(), lock_BlockType(QMutex()),
      vbo_data(), vbo_dataT(), lock_VBO(QMutex()), lock_Tree(QMutex()), tree(), flat_data(), lock_Plateau(QMutex()), cactusHeap()
{
    // generates original scene since prev will be empty
}

Terrain::~Terrain() {
    m_chunks.clear();
    m_generatedTerrain.clear();
}

// Combine two 32-bit ints into one 64-bit int
// where the upper 32 bits are X and the lower 32 bits are Z
int64_t toKey(int x, int z) {
    int64_t xz = 0xffffffffffffffff;
    int64_t x64 = x;
    int64_t z64 = z;

    // Set all lower 32 bits to 1 so we can & with Z later
    xz = (xz & (x64 << 32)) | 0x00000000ffffffff;

    // Set all upper 32 bits to 1 so we can & with XZ
    z64 = z64 | 0xffffffff00000000;

    // Combine
    xz = xz & z64;
    return xz;
}

glm::ivec2 toCoords(int64_t k) {
    // Z is lower 32 bits
    int64_t z = k & 0x00000000ffffffff;
    // If the most significant bit of Z is 1, then it's a negative number
    // so we have to set all the upper 32 bits to 1.
    // Note the 8    V
    if(z & 0x0000000080000000) {
        z = z | 0xffffffff00000000;
    }
    int64_t x = (k >> 32);

    return glm::ivec2(x, z);
}

// Surround calls to this with try-catch if you don't know whether
// the coordinates at x, y, z have a corresponding Chunk
BlockType Terrain::getBlockAt(int x, int y, int z) const
{
    if(hasChunkAt(x, z)) {
        // Just disallow action below or above min/max height,
        // but don't crash the game over it.
        if(y < 0 || y >= 256) {
            return EMPTY;
        }
        const uPtr<Chunk> &c = getChunkAt(x, z);
        glm::vec2 chunkOrigin = glm::vec2(floor(x / 16.f) * 16, floor(z / 16.f) * 16);
        return c->getBlockAt(static_cast<unsigned int>(x - chunkOrigin.x),
                             static_cast<unsigned int>(y),
                             static_cast<unsigned int>(z - chunkOrigin.y));
    }
    else {
        throw std::out_of_range("Coordinates " + std::to_string(x) +
                                " " + std::to_string(y) + " " +
                                std::to_string(z) + " have no Chunk!");
    }
}

BlockType Terrain::getBlockAt(glm::vec3 p) const {
    return getBlockAt(p.x, p.y, p.z);
}

bool Terrain::hasChunkAt(int x, int z) const {
    // Map x and z to their nearest Chunk corner
    // By flooring x and z, then multiplying by 16,
    // we clamp (x, z) to its nearest Chunk-space corner,
    // then scale back to a world-space location.
    // Note that floor() lets us handle negative numbers
    // correctly, as floor(-1 / 16.f) gives us -1, as
    // opposed to (int)(-1 / 16.f) giving us 0 (incorrect!).
    int xFloor = static_cast<int>(glm::floor(x / 16.f));
    int zFloor = static_cast<int>(glm::floor(z / 16.f));
    return m_chunks.find(toKey(16 * xFloor, 16 * zFloor)) != m_chunks.end();
}


uPtr<Chunk>& Terrain::getChunkAt(int x, int z) {
    int xFloor = static_cast<int>(glm::floor(x / 16.f));
    int zFloor = static_cast<int>(glm::floor(z / 16.f));
    return m_chunks[toKey(16 * xFloor, 16 * zFloor)];
}


const uPtr<Chunk>& Terrain::getChunkAt(int x, int z) const {
    int xFloor = static_cast<int>(glm::floor(x / 16.f));
    int zFloor = static_cast<int>(glm::floor(z / 16.f));
    return m_chunks.at(toKey(16 * xFloor, 16 * zFloor));
}

void Terrain::setBlockAt(int x, int y, int z, BlockType t)
{
    if(hasChunkAt(x, z)) {
        uPtr<Chunk> &c = getChunkAt(x, z);
        glm::vec2 chunkOrigin = glm::vec2(floor(x / 16.f) * 16, floor(z / 16.f) * 16);
        c->setBlockAt(static_cast<unsigned int>(x - chunkOrigin.x),
                      static_cast<unsigned int>(y),
                      static_cast<unsigned int>(z - chunkOrigin.y),
                      t);
    }
    else {
        throw std::out_of_range("Coordinates " + std::to_string(x) +
                                " " + std::to_string(y) + " " +
                                std::to_string(z) + " have no Chunk!");
    }
}

Chunk* Terrain::instantiateChunkAt(int x, int z) {
    uPtr<Chunk> chunk = mkU<Chunk>(mp_context, x, z);
    Chunk *cPtr = chunk.get();
    m_chunks[toKey(x, z)] = move(chunk);
    // Set the neighbor pointers of itself and its neighbors
    if(hasChunkAt(x, z + 16)) {
        auto &chunkNorth = m_chunks[toKey(x, z + 16)];
        cPtr->linkNeighbor(chunkNorth, ZPOS);
    }
    if(hasChunkAt(x, z - 16)) {
        auto &chunkSouth = m_chunks[toKey(x, z - 16)];
        cPtr->linkNeighbor(chunkSouth, ZNEG);
    }
    if(hasChunkAt(x + 16, z)) {
        auto &chunkEast = m_chunks[toKey(x + 16, z)];
        cPtr->linkNeighbor(chunkEast, XPOS);
    }
    if(hasChunkAt(x - 16, z)) {
        auto &chunkWest = m_chunks[toKey(x - 16, z)];
        cPtr->linkNeighbor(chunkWest, XNEG);
    }
    return cPtr;
    return cPtr;
}

void Terrain::spawnBlockTypeWorker(int64_t key) {
    m_generatedTerrain.insert(key);

    std::vector<Chunk*> to_generate;
    glm::ivec2 coord = toCoords(key);
    for (int chunk_x = coord.x; chunk_x < coord.x + 64; chunk_x += 16) {
        for (int chunk_z = coord.y; chunk_z < coord.y + 64; chunk_z += 16) {
            Chunk* c = instantiateChunkAt(chunk_x, chunk_z);
            to_generate.push_back(c);
        }
    }

    BlockTypeWorker* worker = new BlockTypeWorker(coord, to_generate, &need_VBO, &tree, &flat_data, &lock_BlockType, &lock_Tree, &lock_Plateau);
    QThreadPool::globalInstance()->start(worker);
}

void Terrain::spawnVBOWorker(Chunk *c) {
    VBOWorker* worker = new VBOWorker(c, &vbo_data, &vbo_dataT, &lock_VBO);
    QThreadPool::globalInstance()->start(worker);
}

// TODO: When you make Chunk inherit from Drawable, change this code so
// it draws each Chunk with the given ShaderProgram, remembering to set the
// model matrix to the proper X and Z translation!
void Terrain::expand(glm::vec2 curr_pos, glm::vec2 &prev_pos) {

    // multithreaded draw:
    // contains incremented timer
    // if reaches time, try expansion

    expansionTry++;
    if (expansionTry < 10) {
        return;
    }
    expansionTry = 0;

    // expansion:

    // destroy VBO data if out of bounds

    glm::ivec2 curr_zone = glm::ivec2(64 * glm::floor(curr_pos.x / 64.f), 64 * glm::floor(curr_pos.y / 64.f));
    glm::ivec2 prev_zone = glm::ivec2(64 * glm::floor(prev_pos.x / 64.f), 64 * glm::floor(prev_pos.y / 64.f));

    std::unordered_set<int64_t> curr_border = border(toKey(curr_zone.x, curr_zone.y));
    std::unordered_set<int64_t> prev_border = border(toKey(prev_zone.x, prev_zone.y));
    std::unordered_set<Chunk*> prev_chunk_border = borderingChunks(toKey(prev_zone.x, prev_zone.y));

    for (int64_t prev_borderingZone : prev_border) {
        if (curr_border.count(prev_borderingZone) == 0 && m_generatedTerrain.count(prev_borderingZone) == 1) {
            glm::ivec2 prev_coord = toCoords(prev_borderingZone);
            for (int chunk_x = prev_coord.x; chunk_x < prev_coord.x + 64; chunk_x += 16) {
                for (int chunk_z = prev_coord.y; chunk_z < prev_coord.y + 64; chunk_z += 16) {
                    Chunk* c = getChunkAt(chunk_x, chunk_z).get();
                    c->destroyVBOdata();
                }
            }
        }
    }

    for (int64_t curr_borderingZone : curr_border) {
        if (m_generatedTerrain.count(curr_borderingZone) == 0) {
            spawnBlockTypeWorker(curr_borderingZone);

        } else if (prev_border.count(curr_borderingZone) == 0) {
            glm::ivec2 curr_coord = toCoords(curr_borderingZone);
            for (int chunk_x = curr_coord.x; chunk_x < curr_coord.x + 64; chunk_x += 16) {
                for (int chunk_z = curr_coord.y; chunk_z < curr_coord.y + 64; chunk_z += 16) {
                    Chunk* c = getChunkAt(chunk_x, chunk_z).get();
                    lock_BlockType.lock();
                    need_VBO.insert(c);
                    lock_BlockType.unlock();
                    //spawnVBOWorker(c);
                }
            }
        }
    }
//////////////////////////////3:51pm 6 Jul
//    spreadFlat();
//    buildcactus();

//    lock_Tree.lock();
//    std::unordered_set<glm::ivec2*> notPlaced;
//    std::vector<Chunk*> chunkArea;

//    for (glm::ivec2* coord : tree) {
//        chunkArea.clear();
//        bool build = true;
//        int loc = 0;
//        for (int x = 16 * glm::floor((coord->x - 16) / 16.f); x <= 16 * glm::floor((coord->x + 16) / 16.f); x += 16) {
//            for (int z = 16 * glm::floor((coord->y - 16) / 16.f); z <= 16 * glm::floor((coord->y + 16) / 16.f); z += 16) {
//                build = build && hasChunkAt(x, z);
//                if (hasChunkAt(x, z)) {
//                    chunkArea.push_back(getChunkAt(x, z).get());
//                    loc++;
//                }
//            }
//        }
//        if (build) {
//            Tree t = Tree(glm::ivec2(16 + toLocal(coord->x), 16 + toLocal(coord->y)), chunkArea);
//            t.drawBase1();
//            t.drawTrunk();
//            t.drawTop();
//            // add all the chunks into need_VBO
//            lock_BlockType.lock();
//            for (Chunk* c : chunkArea) {
//                need_VBO.insert(c);
//            }
//            lock_BlockType.unlock();
//        } else {
//            notPlaced.insert(coord);
//        }
//    }

//    tree = notPlaced;
//    lock_Tree.unlock();
//////////////////////////////3:51pm 6 Jul
///
    lock_BlockType.lock();
    if (toKey(curr_zone.x, curr_zone.y) != toKey(prev_zone.x, prev_zone.y)) {
        for (Chunk* c : prev_chunk_border) {
            need_VBO.insert(c);
        }
    }
    for (Chunk* c : need_VBO) {
        //c->createVBOdata();
        spawnVBOWorker(c);
    }
    need_VBO.clear();
    lock_BlockType.unlock();

    lock_VBO.lock();
    for (VBOData* d : vbo_data) {
        // sends vbo data
        d->c->sendVBOdata(d->data, d->indices);
    }
    for (VBOData* d : vbo_dataT) {
        // sends vbo data
        d->c->sendVBOdataT(d->data, d->indices);
    }
    vbo_data.clear();
    vbo_dataT.clear();
    lock_VBO.unlock();

    prev_pos = curr_pos;
}

void Terrain::create(glm::ivec2 curr_zone)
{
    for (int zone_x = curr_zone.x - 128; zone_x <= curr_zone.x + 128; zone_x += 64) {
        for (int zone_z = curr_zone.y - 128; zone_z <= curr_zone.y + 128; zone_z += 64) {
            int64_t zone_key = toKey(zone_x, zone_z);
            spawnBlockTypeWorker(zone_key);
            // check if VBO data is needed
        }
    }
    spreadFlat();

    lock_BlockType.lock();
    buildcactus();
    for (Chunk* c : need_VBO) {
        //c->createVBOdata();
        spawnVBOWorker(c);
    }
    need_VBO.clear();
    lock_BlockType.unlock();

    lock_VBO.lock();
    for (VBOData* d : vbo_data) {
        // sends vbo data
        d->c->sendVBOdata(d->data, d->indices);
    }
    for (VBOData* d : vbo_dataT) {
        // sends vbo data
        d->c->sendVBOdataT(d->data, d->indices);
    }
    vbo_data.clear();
    vbo_dataT.clear();
    lock_VBO.unlock();

    //

    ////CUSTOM MAP//// 1 141 11
    glm::ivec3 ref = glm::ivec3(2.f, 140.f, 13.f);
    placeBlock(ref, 0, 0, 0, BEDROCK);
    placeBlock(ref, -1, 0, 0, BEDROCK);
    placeBlock(ref, -2, 0, 0, BEDROCK);
    placeBlock(ref, 0, 0, -1, BEDROCK);
    placeBlock(ref, -1, 0, -1, BEDROCK);
    placeBlock(ref, -2, 0, -1, BEDROCK);
    placeBlock(ref, 0, 0, -2, BEDROCK);
    placeBlock(ref, -1, 0, -2, BEDROCK);
    placeBlock(ref, -2, 0, -2, BEDROCK);

    placeBlock(ref, -1, 1, -3, BEDROCK);
    placeBlock(ref, -1, 2, -4, BEDROCK);
    placeBlock(ref, 0, 3, -4, BEDROCK);
    placeBlock(ref, 1, 4, -4, BEDROCK);
    placeBlock(ref, 2, 5, -4, BEDROCK);
    placeBlock(ref, 2, 6, -3, BEDROCK);
    placeBlock(ref, 2, 7, -2, BEDROCK);
    placeBlock(ref, 2, 7, -1, BEDROCK);
    placeBlock(ref, 2, 7, 0, BEDROCK);
    placeBlock(ref, 2, 7, 1, BEDROCK);
    placeBlock(ref, 2, 7, 2, BEDROCK);
    placeBlock(ref, 1, 7, 2, BEDROCK);

    placeBlock(ref, 0, 7, 2, BEDROCK);
    placeBlock(ref, -1, 7, 2, BEDROCK);
    placeBlock(ref, -1, 7, 1, BEDROCK);
    placeBlock(ref, -1, 7, 0, BEDROCK);

    placeBlock(ref, -1, 7, -1, BEDROCK);
    placeBlock(ref, -2, 7, -1, BEDROCK);
    placeBlock(ref, -3, 7, -1, BEDROCK);
    placeBlock(ref, -4, 7, -1, BEDROCK);
    placeBlock(ref, -5, 7, -1, BEDROCK);
    placeBlock(ref, -6, 7, -1, BEDROCK);
    placeBlock(ref, -7, 7, -1, BEDROCK);
    placeBlock(ref, -8, 7, -1, BEDROCK);
    placeBlock(ref, -9, 7, -1, BEDROCK);
    placeBlock(ref, -10, 7, -1, BEDROCK);

    placeBlock(ref, -1, 7, -2, BEDROCK);
    placeBlock(ref, -2, 7, -2, BEDROCK);
    placeBlock(ref, -3, 7, -2, BEDROCK);
    placeBlock(ref, -4, 7, -2, BEDROCK);
    placeBlock(ref, -5, 7, -2, BEDROCK);
    placeBlock(ref, -6, 7, -2, BEDROCK);
    placeBlock(ref, -7, 7, -2, BEDROCK);
    placeBlock(ref, -8, 7, -2, BEDROCK);
    placeBlock(ref, -9, 7, -2, BEDROCK);
    placeBlock(ref, -10, 7, -2, BEDROCK);

    placeBlock(ref, -1, 8, -3, BEDROCK);
    placeBlock(ref, -2, 8, -3, BEDROCK);
    placeBlock(ref, -3, 8, -3, BEDROCK);
    placeBlock(ref, -4, 8, -3, BEDROCK);
    placeBlock(ref, -5, 8, -3, BEDROCK);
    placeBlock(ref, -6, 8, -3, BEDROCK);
    placeBlock(ref, -7, 8, -3, BEDROCK);
    placeBlock(ref, -8, 8, -3, BEDROCK);
    placeBlock(ref, -9, 8, -3, BEDROCK);
    placeBlock(ref, -10, 8, -3, BEDROCK);

    placeBlock(ref, -1, 8, -4, BEDROCK);
    placeBlock(ref, -2, 8, -4, BEDROCK);
    placeBlock(ref, -3, 8, -4, BEDROCK);
    placeBlock(ref, -4, 8, -4, BEDROCK);
    placeBlock(ref, -5, 8, -4, BEDROCK);
    placeBlock(ref, -6, 8, -4, BEDROCK);
    placeBlock(ref, -7, 8, -4, BEDROCK);
    placeBlock(ref, -8, 8, -4, BEDROCK);
    placeBlock(ref, -9, 8, -4, BEDROCK);
    placeBlock(ref, -10, 8, -4, BEDROCK);

    placeBlock(ref, -1, 8, -5, BEDROCK);
    placeBlock(ref, -2, 8, -5, BEDROCK);
    placeBlock(ref, -3, 8, -5, BEDROCK);
    placeBlock(ref, -4, 8, -5, BEDROCK);
    placeBlock(ref, -5, 8, -5, BEDROCK);
    placeBlock(ref, -6, 8, -5, BEDROCK);
    placeBlock(ref, -7, 8, -5, BEDROCK);
    placeBlock(ref, -8, 8, -5, BEDROCK);
    placeBlock(ref, -9, 8, -5, BEDROCK);
    placeBlock(ref, -10, 8, -5, BEDROCK);

    ////CUSTOM MAP////
}
/*
 * enum BlockType : unsigned char
{
    EMPTY, GRASS, DIRT, STONE, WATER, SNOW, LAVA, BEDROCK,
    WOOD, LEAF, YELLOW_FLOWER, SAND, SEAGRASS, RED_STONE,
    RED_MUSHROOMS, ICE, DARK_GRASS, DESERT, CACTUS, DEAD_PLANT,
    BROWN_MUSHROOMS, TALL_GRASS, PLANK, COBBLE, BRICK, CORAL_PINK,
    CORAL_PURPLE, CORAL_ORANGE
};

 */

void Terrain::draw(glm::vec2 curr_pos, ShaderProgram *shaderProgram) {
    glm::ivec2 curr_zone = glm::ivec2(64 * glm::floor(curr_pos.x / 64.f), 64 * glm::floor(curr_pos.y / 64.f));
    for (int zone_x = curr_zone.x - 64; zone_x <= curr_zone.x + 64; zone_x += 64) {
        for (int zone_z = curr_zone.y - 64; zone_z <= curr_zone.y + 64; zone_z += 64) {
            for (int chunk_x = zone_x; chunk_x < zone_x + 64; chunk_x += 16) {
                for (int chunk_z = zone_z; chunk_z < zone_z + 64; chunk_z += 16) {
                    Chunk* c = getChunkAt(chunk_x, chunk_z).get();
                    if(c->elemCount() > 0) {
                        shaderProgram->draw(*c);
                    }
                }
            }
        }
    }
//    for (int zone_x = curr_zone.x - 64; zone_x <= curr_zone.x + 64; zone_x += 64) {
//        for (int zone_z = curr_zone.y - 64; zone_z <= curr_zone.y + 64; zone_z += 64) {
//            for (int chunk_x = zone_x; chunk_x < zone_x + 64; chunk_x += 16) {
//                for (int chunk_z = zone_z; chunk_z < zone_z + 64; chunk_z += 16) {
//                    Chunk* c = getChunkAt(chunk_x, chunk_z).get();
//                    if(c->elemCountT() > 0) {
//                        shaderProgram->drawT(*c);
//                    }
//                }
//            }
//        }
//    }
}

std::unordered_set<int64_t> Terrain::border(int64_t center) {
    std::unordered_set<int64_t> surrounding;
    glm::ivec2 coord = toCoords(center);
    for (int z = coord.y - 128; z <= coord.y + 128; z += 64) {
        for (int x = coord.x - 128; x <= coord.x + 128; x += 64) {
            surrounding.insert(toKey(x, z));
        }
    }

    return surrounding;
}

std::unordered_set<Chunk*> Terrain::borderingChunks(int64_t center) {
    std::unordered_set<Chunk*> border_c;
    glm::ivec2 coord = toCoords(center);
    for (int x = -64; x <= 112; x += 16) {
        border_c.insert(getChunkAt(coord.x + x, coord.y - 64).get());
        border_c.insert(getChunkAt(coord.x + x, coord.y + 112).get());
    }
    for (int z = -48; z < 112; z += 16) {
        border_c.insert(getChunkAt(coord.x - 64, coord.y + z).get());
        border_c.insert(getChunkAt(coord.x + 112, coord.y + z).get());
    }
    return border_c;
}

void Terrain::appendEditedChunk(Chunk *c) {
    lock_BlockType.lock();
    need_VBO.insert(c);
    lock_BlockType.unlock();

    expansionTry += 10;

}

void Terrain::spreadFlat() {
    // goal: use the coords in flat and calculate other flatness
    // any block with a nullptr neighboring block will not get removed

    std::queue<glm::ivec2*> keep; // keeping the old stuff
    std::array<glm::ivec2, 4> dir = {glm::ivec2(-1, 0), glm::ivec2(1, 0), glm::ivec2(0, -1), glm::ivec2(0, 1)};
    lock_Plateau.lock();
    while (!flat_data.empty()) {
        glm::ivec2 coord = *flat_data.front();
        Chunk* original_c = getChunkAt(coord.x, coord.y).get();
        int original_local_x = toLocal(coord.x);
        int original_local_z = toLocal(coord.y);
        int data = original_c->flatness[original_local_x * 16 + original_local_z];
        bool toKeep = false;
        for (glm::ivec2 &d : dir){
            glm::ivec2 n_coord = d + coord;
            if (hasChunkAt(n_coord.x, n_coord.y)) {
                Chunk* c = getChunkAt(n_coord.x, n_coord.y).get();
                int local_x = toLocal(n_coord.x);
                int local_z = toLocal(n_coord.y);;
                if (c->flatness[local_x * 16 + local_z] > data + 1) {
                    c->flatness[local_x * 16 + local_z] = 1 + data;
                    flat_data.push(new glm::ivec2(n_coord));
                    if (data + 1 > 5) {
                        cactusHeap.push(std::make_pair(data + 1, new glm::ivec2(n_coord)));
                    }
                }
            } else {
                toKeep = true;
            }
        }
        if (toKeep) {
            keep.push(flat_data.front());
        }
        flat_data.pop();
    }
    flat_data = keep;
    lock_Plateau.unlock();
}

void Terrain::buildcactus() {
    while (!cactusHeap.empty()) {
        std::pair<int, glm::ivec2*> cactus = cactusHeap.top();
        glm::ivec2 topLeft = *cactus.second + glm::ivec2(-1 * floor(cactus.first), -1 * floor(cactus.first));
        glm::ivec2 topRight = *cactus.second + glm::ivec2(floor(cactus.first), -1 * floor(cactus.first));
        glm::ivec2 bottomLeft = *cactus.second + glm::ivec2(-1 * floor(cactus.first), floor(cactus.first));
        glm::ivec2 bottomRight = *cactus.second + glm::ivec2(floor(cactus.first), floor(cactus.first));

        if (cactus.first > 10 && hasNoCactus(topLeft) && hasNoCactus(topRight) && hasNoCactus(bottomLeft) && hasNoCactus(bottomRight)) {
            for (int x = topLeft.x; x <= bottomRight.x; x++) {
                for (int z = topLeft.y; z <= bottomRight.y; z++) {
                    getChunkAt(x, z).get()->hasCactus = true;
                    need_VBO.insert(getChunkAt(x, z).get());
                }
            }
            Cactus c = Cactus(std::min(10, int(floor(cactus.first / 3))));
            LNode* system = c.getSystem();

            Chunk* chunk = getChunkAt(cactus.second->x, cactus.second->y).get();

            int x = cactus.second->x;
            int z = cactus.second->y;
            int y = chunk->topTerrainBlock[toLocal(x) * 16 + toLocal(z)];

            placeCactus(x, y, z, system);
        }
        cactusHeap.pop();
    }
}

int Terrain::toLocal(int i) {
    return (i % 16 + 16) % 16;
}

bool Terrain::hasNoCactus(glm::ivec2 coord) {
    return hasChunkAt(coord.x, coord.y) && !(getChunkAt(coord.x, coord.y).get()->hasCactus);
}

void Terrain::placeCactus(int x, int y, int z, LNode* system) {

    std::stack<glm::ivec3> savedPos;
    glm::ivec3 curr = glm::ivec3(x - 2, y - 3, z - 2);

    do {
        char c = system->c;
        if (curr.y <= 200) {
            if (c == 'c') {
                for (int i = 0; i < 3; i++) {
                    placeBlock(curr, 0, i, 1, CACTUS);
                    placeBlock(curr, 0, i, 2, CACTUS);
                    placeBlock(curr, 0, i, 3, CACTUS);
                    placeBlock(curr, 4, i, 1, CACTUS);
                    placeBlock(curr, 4, i, 2, CACTUS);
                    placeBlock(curr, 4, i, 3, CACTUS);
                    placeBlock(curr, 1, i, 0, CACTUS);
                    placeBlock(curr, 2, i, 0, CACTUS);
                    placeBlock(curr, 3, i, 0, CACTUS);
                    placeBlock(curr, 1, i, 4, CACTUS);
                    placeBlock(curr, 2, i, 4, CACTUS);
                    placeBlock(curr, 3, i, 4, CACTUS);
                }
                curr.y += 3;
            } else if (c == 'C') {
                curr.x -= 1;
                curr.z -= 1;
                for (int i = 0; i < 5; i++) {

                    placeBlock(curr, 0, i, 1, CACTUS);
                    placeBlock(curr, 0, i, 2, CACTUS);
                    placeBlock(curr, 0, i, 3, CACTUS);
                    placeBlock(curr, 0, i, 4, CACTUS);
                    placeBlock(curr, 0, i, 5, CACTUS);
                    placeBlock(curr, 6, i, 1, CACTUS);
                    placeBlock(curr, 6, i, 2, CACTUS);
                    placeBlock(curr, 6, i, 3, CACTUS);
                    placeBlock(curr, 6, i, 4, CACTUS);
                    placeBlock(curr, 6, i, 5, CACTUS);

                    placeBlock(curr, 1, i, 0, CACTUS);
                    placeBlock(curr, 2, i, 0, CACTUS);
                    placeBlock(curr, 3, i, 0, CACTUS);
                    placeBlock(curr, 4, i, 0, CACTUS);
                    placeBlock(curr, 5, i, 0, CACTUS);
                    placeBlock(curr, 1, i, 6, CACTUS);
                    placeBlock(curr, 2, i, 6, CACTUS);
                    placeBlock(curr, 3, i, 6, CACTUS);
                    placeBlock(curr, 4, i, 6, CACTUS);
                    placeBlock(curr, 5, i, 6, CACTUS);

                    placeBlock(curr, 1, i, 1, CACTUS);
                    placeBlock(curr, 1, i, 5, CACTUS);
                    placeBlock(curr, 5, i, 5, CACTUS);
                    placeBlock(curr, 5, i, 1, CACTUS);

                }
                curr.x += 1;
                curr.y += 5;
                curr.z += 1;
            } else if (c == '1') {
                for (int i = 0; i < 3; i++) {
                    placeBlock(curr, 0, i, 1, CACTUS);
                    placeBlock(curr, 0, i, 2, CACTUS);
                    placeBlock(curr, 0, i, 3, CACTUS);
                    placeBlock(curr, 4, i, 1, CACTUS);
                    placeBlock(curr, 4, i, 2, CACTUS);
                    placeBlock(curr, 4, i, 3, CACTUS);
                    placeBlock(curr, 1, i, 0, CACTUS);
                    placeBlock(curr, 2, i, 0, CACTUS);
                    placeBlock(curr, 3, i, 0, CACTUS);
                    placeBlock(curr, 1, i, 4, CACTUS);
                    placeBlock(curr, 2, i, 4, CACTUS);
                    placeBlock(curr, 3, i, 4, CACTUS);
                }
                savedPos.push(curr);
                curr.x += 5;
                system = system->next;
                while (system->next != nullptr && system->next->c == 't') {
                    system = system->next;
                    for (int i = 0; i < 3; i++) {
                        placeBlock(curr, i, 0, 1, CACTUS);
                        placeBlock(curr, i, 0, 2, CACTUS);
                        placeBlock(curr, i, 0, 3, CACTUS);
                        placeBlock(curr, i, 3, 1, CACTUS);
                        placeBlock(curr, i, 3, 2, CACTUS);
                        placeBlock(curr, i, 3, 3, CACTUS);
                        placeBlock(curr, i, 1, 1, CACTUS);
                        placeBlock(curr, i, 1, 3, CACTUS);
                        placeBlock(curr, i, 2, 1, CACTUS);
                        placeBlock(curr, i, 2, 3, CACTUS);
                    }
                    curr.x += 3;
                }
                drawFlatCactus(curr);
            } else if (c == '2') {
                for (int i = 0; i < 3; i++) {
                    placeBlock(curr, 0, i, 1, CACTUS);
                    placeBlock(curr, 0, i, 2, CACTUS);
                    placeBlock(curr, 0, i, 3, CACTUS);
                    placeBlock(curr, 4, i, 1, CACTUS);
                    placeBlock(curr, 4, i, 2, CACTUS);
                    placeBlock(curr, 4, i, 3, CACTUS);
                    placeBlock(curr, 1, i, 0, CACTUS);
                    placeBlock(curr, 2, i, 0, CACTUS);
                    placeBlock(curr, 3, i, 0, CACTUS);
                    placeBlock(curr, 1, i, 4, CACTUS);
                    placeBlock(curr, 2, i, 4, CACTUS);
                    placeBlock(curr, 3, i, 4, CACTUS);
                }
                savedPos.push(curr);
                curr.z += 5;
                system = system->next;
                while (system->next != nullptr && system->next->c == 't') {
                    system = system->next;
                    for (int i = 0; i < 3; i++) {
                        placeBlock(curr, 1, 0, i, CACTUS);
                        placeBlock(curr, 2, 0, i, CACTUS);
                        placeBlock(curr, 3, 0, i, CACTUS);
                        placeBlock(curr, 1, 3, i, CACTUS);
                        placeBlock(curr, 2, 3, i, CACTUS);
                        placeBlock(curr, 3, 3, i, CACTUS);
                        placeBlock(curr, 1, 1, i, CACTUS);
                        placeBlock(curr, 3, 1, i, CACTUS);
                        placeBlock(curr, 1, 2, i, CACTUS);
                        placeBlock(curr, 3, 2, i, CACTUS);
                    }
                    curr.z += 3;
                }
                drawFlatCactus(curr);
            } else if (c == '3') {
                for (int i = 0; i < 3; i++) {
                    placeBlock(curr, 0, i, 1, CACTUS);
                    placeBlock(curr, 0, i, 2, CACTUS);
                    placeBlock(curr, 0, i, 3, CACTUS);
                    placeBlock(curr, 4, i, 1, CACTUS);
                    placeBlock(curr, 4, i, 2, CACTUS);
                    placeBlock(curr, 4, i, 3, CACTUS);
                    placeBlock(curr, 1, i, 0, CACTUS);
                    placeBlock(curr, 2, i, 0, CACTUS);
                    placeBlock(curr, 3, i, 0, CACTUS);
                    placeBlock(curr, 1, i, 4, CACTUS);
                    placeBlock(curr, 2, i, 4, CACTUS);
                    placeBlock(curr, 3, i, 4, CACTUS);
                }
                savedPos.push(curr);
                system = system->next;
                while (system->next != nullptr && system->next->c == 't') {
                    system = system->next;
                    curr.x -= 3;
                    for (int i = 0; i < 3; i++) {
                        placeBlock(curr, i, 0, 1, CACTUS);
                        placeBlock(curr, i, 0, 2, CACTUS);
                        placeBlock(curr, i, 0, 3, CACTUS);
                        placeBlock(curr, i, 3, 1, CACTUS);
                        placeBlock(curr, i, 3, 2, CACTUS);
                        placeBlock(curr, i, 3, 3, CACTUS);
                        placeBlock(curr, i, 1, 1, CACTUS);
                        placeBlock(curr, i, 1, 3, CACTUS);
                        placeBlock(curr, i, 2, 1, CACTUS);
                        placeBlock(curr, i, 2, 3, CACTUS);
                    }
                }
                curr.x -= 5;
                drawFlatCactus(curr);
            } else if (c == '4') {
                for (int i = 0; i < 3; i++) {
                    placeBlock(curr, 0, i, 1, CACTUS);
                    placeBlock(curr, 0, i, 2, CACTUS);
                    placeBlock(curr, 0, i, 3, CACTUS);
                    placeBlock(curr, 4, i, 1, CACTUS);
                    placeBlock(curr, 4, i, 2, CACTUS);
                    placeBlock(curr, 4, i, 3, CACTUS);
                    placeBlock(curr, 1, i, 0, CACTUS);
                    placeBlock(curr, 2, i, 0, CACTUS);
                    placeBlock(curr, 3, i, 0, CACTUS);
                    placeBlock(curr, 1, i, 4, CACTUS);
                    placeBlock(curr, 2, i, 4, CACTUS);
                    placeBlock(curr, 3, i, 4, CACTUS);
                }
                savedPos.push(curr);
                system = system->next;
                while (system->next != nullptr && system->next->c == 't') {
                    system = system->next;
                    curr.z -= 3;
                    for (int i = 0; i < 3; i++) {
                        placeBlock(curr, 1, 0, i, CACTUS);
                        placeBlock(curr, 2, 0, i, CACTUS);
                        placeBlock(curr, 3, 0, i, CACTUS);
                        placeBlock(curr, 1, 3, i, CACTUS);
                        placeBlock(curr, 2, 3, i, CACTUS);
                        placeBlock(curr, 3, 3, i, CACTUS);
                        placeBlock(curr, 1, 1, i, CACTUS);
                        placeBlock(curr, 3, 1, i, CACTUS);
                        placeBlock(curr, 1, 2, i, CACTUS);
                        placeBlock(curr, 3, 2, i, CACTUS);
                    }

                }
                curr.z -= 5;
                drawFlatCactus(curr);
            }
        }
        if (c == ']' && !savedPos.empty()) {
            drawFlatCactus(curr);
            curr = savedPos.top();
            savedPos.pop();
        }
        system = system->next;
    } while (system != nullptr);
    drawFlatCactus(curr);
}

void Terrain::placeBlock(glm::ivec3 start, int x, int y, int z, BlockType t) {
    x += start.x;
    y += start.y;
    z += start.z;
    Chunk* c = getChunkAt(x, z).get();
    c->setBlockAt(toLocal(x), y, toLocal(z), t);
}

void Terrain::drawFlatCactus(glm::ivec3 curr) {
    placeBlock(curr, 1, 0, 0, CACTUS);
    placeBlock(curr, 2, 0, 0, CACTUS);
    placeBlock(curr, 3, 0, 0, CACTUS);
    placeBlock(curr, 0, 0, 1, CACTUS);
    placeBlock(curr, 1, 0, 1, CACTUS);
    placeBlock(curr, 2, 0, 1, CACTUS);
    placeBlock(curr, 3, 0, 1, CACTUS);
    placeBlock(curr, 4, 0, 1, CACTUS);
    placeBlock(curr, 0, 0, 2, CACTUS);
    placeBlock(curr, 1, 0, 2, CACTUS);
    placeBlock(curr, 2, 0, 2, CACTUS);
    placeBlock(curr, 3, 0, 2, CACTUS);
    placeBlock(curr, 4, 0, 2, CACTUS);
    placeBlock(curr, 0, 0, 3, CACTUS);
    placeBlock(curr, 1, 0, 3, CACTUS);
    placeBlock(curr, 2, 0, 3, CACTUS);
    placeBlock(curr, 3, 0, 3, CACTUS);
    placeBlock(curr, 4, 0, 3, CACTUS);
    placeBlock(curr, 1, 0, 4, CACTUS);
    placeBlock(curr, 2, 0, 4, CACTUS);
    placeBlock(curr, 3, 0, 4, CACTUS);
}
