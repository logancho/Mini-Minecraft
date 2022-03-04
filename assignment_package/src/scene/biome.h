#pragma once
#include "chunk.h"
#include <glm/gtx/vector_angle.hpp>

class Biome {

public:

    BlockType topBlock;
    BlockType underTopBlock;

    Biome();
    Biome(BlockType topBlock, BlockType underTopBlock);

    void setBlocks(BlockType topBlock, BlockType underTopBlock);

    static float noiseGrass(int x, int z);
    static float noiseSnow(int x, int z);
    static float noiseOcean(int x, int z);
    static float noisePlateau(int x, int z);
    static float noiseIce(int x, int z);
    static float noiseDesert(int x, int z);

    static glm::vec3 noiseBiome(int x, int z);

    static glm::vec2 random2(glm::vec2 p);
    static glm::vec3 random3(glm::vec2 p);

    static float surflet2D(glm::vec2 p, glm::vec2 gridPoint);
    static float perlinNoise2D(glm::vec2 uv);
    static float surflet3D(glm::vec3 p, glm::vec3 gridPoint);
    static float perlinNoise3D(glm::vec3 p);

    static float noise1D(int x);
    static float noise2D(glm::vec2 p);

    static float fbm1D(float x);
    static float interpNoise1D(float x);

    static float fbm2DSnow(glm::vec2 uv);
    static float fbm2DIce(glm::vec2 uv);
    static float fbm2DPerlin(glm::vec2 uv);
    static float interpNoise2D(float x, float y);

    static float worley(glm::vec2 uv);

    static float warp(glm::vec2 uv);
};
