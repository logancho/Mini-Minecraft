#include "biome.h"
#include <glm/exponential.hpp>
#include <glm/glm.hpp>

Biome::Biome() {}

Biome::Biome(BlockType topBlock, BlockType underTopBlock) :
    topBlock(topBlock), underTopBlock(underTopBlock)
{}

void Biome::setBlocks(BlockType topBlock, BlockType underTopBlock) {
    this->topBlock = topBlock;
    this->underTopBlock = underTopBlock;
}

// noise function for grass terrain
float Biome::noiseGrass(int x, int z) {
    float worleyNoise = worley(glm::vec2(x, z) / 60.f);
    float warpNoise = warp(glm::vec2(x, z) / 90000.f); // very noisy, needs to be scaled
    return glm::mix((float) pow((1.f - worleyNoise), 2.f), warpNoise, 0.9f); // mostly warp, worley is rippley
}

// noise function for mountain Biome
float Biome::noiseSnow(int x, int z) {
    float fbm = fbm2DSnow(glm::vec2(x, z) / 1000.f);
    float perlin = perlinNoise2D(glm::vec2(x, z) / 70.f);
    return glm::mix(fbm, perlin, 0.35f);
}

// noise function for ocean Biome
float Biome::noiseOcean(int x, int z) {
    float worleyNoise = worley(glm::vec2(x, z) / 100.f);
    float warpNoise = warp(glm::vec2(x, z) / 70000.f);
    float mix = glm::mix((float) pow((1.f - worleyNoise), 2.f), warpNoise, 0.9f);
    return 1.f - glm::smoothstep(0.1f, 0.3f, mix);
}

// noise function for plateau Biome
float Biome::noisePlateau(int x, int z) {
    float perlin = perlinNoise2D(glm::vec2(x, z) / 110.f);
    float warpNoise = warp(glm::vec2(x, z) / 70000.f);
    float mix = glm::mix(perlin, warpNoise, 0.5f);
    return glm::smoothstep(0.08f, 0.f, mix);
}

// noise function for mountain Biome
float Biome::noiseIce(int x, int z) {
    float fbm = fbm2DIce(glm::vec2(x, z) / 800.f);
    float perlin = perlinNoise2D(glm::vec2(x, z) / 35.f); // was 70
    return glm::mix(fbm, perlin, 0.35f);
}

// noise function for mountain Biome
float Biome::noiseDesert(int x, int z) {
    float worleyNoise = worley(glm::vec2(x, z) / 100.f);
    float warpNoise = warp(glm::vec2(x, z) / 90000.f); // very noisy, needs to be scaled
    return glm::mix((float) pow((1.f - worleyNoise), 2.f), warpNoise, 0.9f); // mostly warp, worley is rippley
}

// noise function to interperlate
glm::vec3 Biome::noiseBiome(int x, int z) {
    glm::vec2 xzx = glm::vec2(x, z) / 1600.f;
    glm::vec2 xzy = glm::vec2(x + 400, z) / 1600.f;
    glm::vec2 xzz = glm::vec2(x, z) / 800.f;
    float px = perlinNoise2D(xzx);
    float py = perlinNoise2D(xzy); // offset
    float pz = perlinNoise2D(xzz); // offset
    float tx = glm::smoothstep((double) 0.2, (double) 0.75, (double) abs(px) * 10.f);
    float ty = glm::smoothstep((double) 0.2, (double) 0.75, (double) abs(py) * 10.f);
    float tz = glm::smoothstep((double) 0.2, (double) 0.75, (double) abs(pz) * 10.f);
    return glm::vec3(tx, ty, tz);
}

glm::vec2 Biome::random2(glm::vec2 p) {
    return glm::fract(glm::sin(glm::vec2(glm::dot(p, glm::vec2(127.1f, 311.7f)), glm::dot(p, glm::vec2(269.5f, 183.3f)))) * 43758.5453f);
}

glm::vec3 Biome::random3(glm::vec2 p) {
    return glm::fract(glm::sin(glm::vec3(glm::dot(p, glm::vec2(127.1, 311.7)),
                                         glm::dot(p, glm::vec2(269.5, 183.3)),
                                         glm::dot(p, glm::vec2(420.6, 631.2))
                                   )) * 43758.5453f);
}

float Biome::surflet2D(glm::vec2 P, glm::vec2 gridPoint) {
    // Compute falloff function by converting linear distance to a polynomial (quintic smootherstep function)
    float distX = abs(P.x - gridPoint.x);
    float distY = abs(P.y - gridPoint.y);
    float tX = 1 - 6 * pow(distX, 5.f) + 15 * pow(distX, 4.f) - 10 * pow(distX, 3.f);
    float tY = 1 - 6 * pow(distY, 5.f) + 15 * pow(distY, 4.f) - 10 * pow(distY, 3.f);
    // Get the random vector for the grid point
    glm::vec2 gradient = random2(gridPoint);
    // Get the vector from the grid point to P
    glm::vec2 diff = P - gridPoint;
    // Get the value of our height field by dotting grid->P with our gradient
    float height = glm::dot(diff, gradient);
    // Scale our height field (i.e. reduce it) by our polynomial falloff function
    return height * tX * tY;
}

float Biome::surflet3D(glm::vec3 p, glm::vec3 gridPoint) {
    // Compute the distance between p and the grid point along each axis, and warp it with a
    // quintic function so we can smooth our cells
    glm::vec3 t2 = glm::abs(p - gridPoint);
    glm::vec3 t = glm::vec3(1.f) - 6.f * glm::vec3(pow(t2.x, 5.f), pow(t2.y, 5.f), pow(t2.z, 5.f))
            + 15.f * glm::vec3(pow(t2.x, 4.f), pow(t2.y, 4.f), pow(t2.z, 4.f))
            - 10.f * glm::vec3(pow(t2.x, 3.f), pow(t2.y, 3.f), pow(t2.z, 3.f));
    // Get the random vector for the grid point (assume we wrote a function random2
    // that returns a vec2 in the range [0, 1])
    glm::vec3 gradient = random3(glm::vec2(gridPoint)) * 2.f - glm::vec3(1.f, 1.f, 1.f);
    // Get the vector from the grid point to P
    glm::vec3 diff = p - gridPoint;
    // Get the value of our height field by dotting grid->P with our gradient
    float height = glm::dot(diff, gradient);
    // Scale our height field (i.e. reduce it) by our polynomial falloff function
    return height * t.x * t.y * t.z;
}

float Biome::perlinNoise2D(glm::vec2 uv) {
    // Tile the space
    glm::vec2 uvXLYL = glm::floor(uv);
    glm::vec2 uvXHYL = uvXLYL + glm::vec2(1,0);
    glm::vec2 uvXHYH = uvXLYL + glm::vec2(1,1);
    glm::vec2 uvXLYH = uvXLYL + glm::vec2(0,1);

    return surflet2D(uv, uvXLYL) + surflet2D(uv, uvXHYL) + surflet2D(uv, uvXHYH) + surflet2D(uv, uvXLYH);
}

float Biome::perlinNoise3D(glm::vec3 p) {
    float surfletSum = 0.f;
    // Iterate over the four integer corners surrounding uv
    for(int dx = 0; dx <= 1; ++dx) {
        for(int dy = 0; dy <= 1; ++dy) {
            for(int dz = 0; dz <= 1; ++dz) {
                surfletSum += surflet3D(p, glm::floor(p) + glm::vec3(dx, dy, dz));
            }
        }
    }
    return surfletSum;
}


float Biome::noise1D(int x) {
    x = (x << 13) ^ x;
    return (1.0 - (x * (x * x * 15731 + 789221)
            + 1376312589) && 0x7fffffff)
            / 10737741824.0;
}


float Biome::noise2D(glm::vec2 p) {
    return glm::fract(glm::sin(glm::dot(p, glm::vec2(127.1, 311.7))) *
                 43758.5453);
}

float Biome::fbm1D(float x) {
    float total = 0;
    float persistence = 0.5f;
    int octaves = 8;
    float freq = 2.f;
    float amp = 0.5f;
    for(int i = 1; i <= octaves; i++) {
        freq *= 2.f;
        amp *= persistence;

        total += interpNoise1D(x * freq) * amp;
    }
    return total;
}

float Biome::interpNoise1D(float x) {
    int intX = int(floor(x));
    float fractX = glm::fract(x);

    float v1 = noise1D(intX);
    float v2 = noise1D(intX + 1);
    return glm::mix(v1, v2, fractX);
}


float Biome::fbm2DSnow(glm::vec2 uv) {
    float total = 0;
    float persistence = 0.15f; // changes frequency of ripples?
    int octaves = 10.f; // changes frequency of ripples
    float freq = 18.f; // changes spacing of noise
    float amp = 3.5f; // channges height of spikes
    for(int i = 1; i <= octaves; i++) {
        freq *= 1.8f; // was 2.f
        amp *= persistence;
        total += interpNoise2D(uv.x * freq, uv.y * freq) * amp;
    }
    return pow(total, 1.4f);

}

float Biome::fbm2DIce(glm::vec2 uv) {
    float total = 0;
    float persistence = 0.15f; // changes frequency of ripples?
    int octaves = 10.f; // changes frequency of ripples
    float freq = 12.f; // changes spacing of noise was 18
    float amp = 5.5f; // channges height of spikes was 5
    for(int i = 1; i <= octaves; i++) {
        freq *= 1.8f; // was 2.f
        amp *= persistence;
        total += interpNoise2D(uv.x * freq, uv.y * freq) * amp;
    }
    return pow(total, 1.4f);

}

float Biome::fbm2DPerlin(glm::vec2 uv) {
    float total = 0;
    float persistence = 0.5f;
    int octaves = 8.f; // was 8
    float freq = 15.f; // was 2.f
    float amp = 0.6f;
    for(int i = 1; i <= octaves; i++) {
        freq *= 2.f;
        amp *= persistence;

        total += (1.f - perlinNoise2D(uv * freq)) * amp;
    }
    return total;

}

float Biome::interpNoise2D(float x, float y) {
    int intX = int(floor(x));
    float fractX = glm::fract(x);
    int intY = int(floor(y));
    float fractY = glm::fract(y);

    float v1 = noise2D(glm::vec2(intX, intY));
    float v2 = noise2D(glm::vec2(intX + 1, intY));
    float v3 = noise2D(glm::vec2(intX, intY + 1));
    float v4 = noise2D(glm::vec2(intX + 1, intY + 1));

    // check mix function
    float i1 = glm::mix(v1, v2, fractX);
    float i2 = glm::mix(v3, v4, fractX);
    return glm::mix(i1, i2, fractY);
}

float Biome::worley(glm::vec2 uv) {
    uv *= 1.f; // Now the space is 10x10 instead of 1x1. Change this to any number you want.
    glm::vec2 uvInt = glm::floor(uv);
    glm::vec2 uvFract = glm::fract(uv);
    float minDist = 0.7f; // Minimum distance initialized to max. // was 1.f ((rand() % 4) + 6) / 10.f
    for(int y = -1; y <= 1; ++y) {
        for(int x = -1; x <= 1; ++x) {
            glm::vec2 neighbor = glm::vec2(float(x), float(y)); // Direction in which neighbor cell lies
            glm::vec2 point = random2(uvInt + neighbor); // Get the Voronoi centerpoint for the neighboring cell
            glm::vec2 diff = neighbor + point - uvFract; // Distance between fragment coord and neighborâ€™s Voronoi point
            float dist = glm::length(diff);
            minDist = glm::min(minDist, dist);
        }
    }
    return minDist;
}

float Biome::warp(glm::vec2 uv) {
    glm::vec2 q = glm::vec2(fbm2DPerlin(uv + glm::vec2(0.0,0.0) ),
                   fbm2DPerlin( uv + glm::vec2(5.2,1.3) ) );

    return fbm2DSnow(uv + q * 4.f);
}
