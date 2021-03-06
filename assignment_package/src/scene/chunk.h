#pragma once
#include "smartpointerhelp.h"
#include "glm_includes.h"
#include <array>
#include <unordered_map>
#include <cstddef>
#include <drawable.h>


//using namespace std;

// C++ 11 allows us to define the size of an enum. This lets us use only one byte
// of memory to store our different block types. By default, the size of a C++ enum
// is that of an int (so, usually four bytes). This *does* limit us to only 256 different
// block types, but in the scope of this project we'll never get anywhere near that many.
enum BlockType : unsigned char
{
    EMPTY, GRASS, DIRT, STONE, WATER, SNOW, LAVA, BEDROCK,
    WOOD, LEAF, YELLOW_FLOWER, SAND, SEAGRASS, RED_STONE,
    RED_MUSHROOMS, ICE, DARK_GRASS, DESERT, CACTUS, DEAD_PLANT,
    BROWN_MUSHROOMS, TALL_GRASS, PLANK, COBBLE, BRICK, CORAL_PINK,
    CORAL_PURPLE, CORAL_ORANGE
};

// The six cardinal directions in 3D space
enum Direction : unsigned char
{
    XPOS, XNEG, YPOS, YNEG, ZPOS, ZNEG
};

// Lets us use any enum class as the key of a
// std::unordered_map
struct EnumHash {
    template <typename T>
    size_t operator()(T t) const {
        return static_cast<size_t>(t);
    }
};

// change col to uv in milestone 2
struct Vertex {
    glm::vec4 pos;
    glm::vec4 nor;
    glm::vec3 UV;
};

struct Neighbor {
    Direction d;
    glm::ivec3 position;
};

// One Chunk is a 16 x 256 x 16 section of the world,
// containing all the Minecraft blocks in that area.
// We divide the world into Chunks in order to make
// recomputing its VBO data faster by not having to
// render all the world at once, while also not having
// to render the world block by block.

// TODO have Chunk inherit from Drawable
class Chunk : public Drawable {
private:
    // All of the blocks contained within this Chunk
    std::array<BlockType, 65536> m_blocks;
    // This Chunk's four neighbors to the north, south, east, and west
    // The third input to this map just lets us use a Direction as
    // a key for this map.
    // These allow us to properly determine
    std::unordered_map<Direction, Chunk*, EnumHash> m_neighbors;


public:
    Chunk(OpenGLContext* context, int x, int z);

    void createVBOdata() override;

    void sendVBOdata(std::vector<Vertex> data, std::vector<GLuint> indices);
    void sendVBOdataT(std::vector<Vertex> data, std::vector<GLuint> indices);

    BlockType getBlockAt(unsigned int x, unsigned int y, unsigned int z) const;
    BlockType getBlockAt(int x, int y, int z) const;
    void setBlockAt(unsigned int x, unsigned int y, unsigned int z, BlockType t);
    void linkNeighbor(uPtr<Chunk>& neighbor, Direction dir);

    bool inBound(glm::ivec3 n); // check if neighboring block is in the chunk
    BlockType neighborCheck(glm::ivec3 n); // check if block in neighboring chunk
    std::vector<glm::ivec3> getFacePos(glm::ivec3 d, int x, int y, int z);
    std::vector<Chunk*> checkFromNeighbor(glm::ivec3 n);

    glm::ivec2 starting_coord;
    glm::ivec2 treeAt;
    std::array<int, 256> topTerrainBlock;
    std::array<int, 256> flatness;

    bool hasCactus;
};
