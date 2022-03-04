#include "blocktypeworker.h"
#include <iostream>

#include "blocktypeworker.h"
#include <iostream>

BlockTypeWorker::BlockTypeWorker(glm::ivec2 bottom_corner, std::vector<Chunk*> to_fill, std::unordered_set<Chunk*>* done, std::unordered_set<glm::ivec2*>* treeCoords, std::queue<glm::ivec2*>* flatness, QMutex* block_lock, QMutex* tree_lock, QMutex* flat_lock)
    : bottom_corner(bottom_corner), to_fill(to_fill), done(done), treeCoords(treeCoords), block_lock(block_lock), tree_lock(tree_lock), flat(flatness), flat_lock(flat_lock)
{}


void BlockTypeWorker::run() {
    // fill all the chunks in to_fill

    for (Chunk* c : to_fill) {
        CreateTerrain(c);
        block_lock->lock();
        done->insert(c);
        block_lock->unlock();
    }

}

void BlockTypeWorker::CreateTerrain(Chunk* c) {

    glm::ivec3 mostTree = glm::ivec3();
    float treeVal = 1.f;
    Biome biome;

    for (int x = 0; x < 16; x++) {
        for (int z = 0; z < 16; z++) {
            int currX = c->starting_coord.x + x;
            int currZ = c->starting_coord.y + z;


            // biome interpolation
            glm::vec3 t = glm::smoothstep(0.4f, 1.1f, Biome::noiseBiome(currX, currZ));

            float oceanY = fabs(Biome::noiseOcean(currX, currZ)) * 20 + 60;
            float iceY = glm::min(255.f, abs(Biome::noiseIce(currX, currZ)) * 170 + 60);
            int oceanIceY = t.x * oceanY + (1.f - t.x) * iceY;

            float swampY = fabs(Biome::noiseGrass(currX, currZ)) * 40 + 92;
            float jungleY = fabs(Biome::noiseGrass(currX, currZ)) * 50 + 105;
            int swampJungleY = t.x * swampY + (1.f - t.x) * jungleY;

            float desertY = fabs(Biome::noiseDesert(currX, currZ)) * 2 + 101;
            float plateauY = fabs(Biome::noisePlateau(currX, currZ)) * 30 + 110;
            int desertPlateauY = t.x * desertY + (1.f - t.x) * plateauY;

            float caveY = fabs(Biome::noiseGrass(currX, currZ)) * 20 + 100;
            float snowY = glm::min(255.f, abs(Biome::noiseSnow(currX, currZ)) * 170 + 110);
            int caveSnowY = t.x * caveY + (1.f - t.x) * snowY;

            int oceanIceSwampJungleY = t.y * oceanIceY + (1.f - t.y) * swampJungleY;
            int desertPlateauCaveSnowY = t.y * desertPlateauY + (1.f - t.y) * caveSnowY;


            int y = t.z * oceanIceSwampJungleY + (1.f - t.z) * desertPlateauCaveSnowY;

            c->topTerrainBlock[x * 16 + z] = y;
            c->flatness[x * 16 + z] = -1;

            // set biomes depending on t value
            if (t.z > 0.5f) {
                if (t.y > 0.5f) {
                    if (t.x > 0.5f) {
                        // ocean
                        biome.setBlocks(SAND, SAND);

                        // placing corals
                        if (y < 97 && Biome::fbm2DSnow(glm::vec2(currX, currZ) / 500.f) < 0.2f) {
                            int random = rand() % 3;
                            if (random == 0) {
                                c->setBlockAt(x, y + 1, z, CORAL_PINK);
                            } else if (random == 1) {
                                c->setBlockAt(x, y + 1, z, CORAL_PURPLE);
                            } else {
                                c->setBlockAt(x, y + 1, z, CORAL_ORANGE);
                            }
                        }
                    } else {
                        // ice
                        biome.setBlocks(SNOW, ICE);
                    }
                } else {
                    if (t.x > 0.5f) {
                        // swamp
                        biome.setBlocks(DARK_GRASS, DIRT);

                        float blockSwampVal = 10 * biome.noise2D(glm::vec2(swampY * currX, (1 - jungleY) * currZ));
                        if (blockSwampVal < 0.1) {
                            c->setBlockAt(x, y + 1, z, BROWN_MUSHROOMS);
                        } else if (blockSwampVal < 0.85) {
                            c->setBlockAt(x, y + 1, z, TALL_GRASS);
                        }

                        // patchy grass
                        if (y >= 100 && Biome::fbm2DSnow(glm::vec2(currX, currZ) / 150.f) < 0.2f) {
                            c->setBlockAt(x, y + 1, z, SEAGRASS);
                        }
                    } else {
                        // jungle
                        biome.setBlocks(GRASS, DIRT);

                        // test: placing trees:
                        float blockTreeVal = 3 * biome.noise2D(biome.random2((glm::vec2(currX * 16, y * currZ))));
                        if ((currX % 32 + 32) % 32 < 16 && (currZ % 16 + 16) % 16 > 7) {
                            if (blockTreeVal < treeVal && y > 100 && y < 120) {
                                mostTree = glm::ivec3(x, y, z);
                                treeVal = blockTreeVal;
                            }
                        }
                        if (blockTreeVal < 0.07) {
                            c->setBlockAt(x, y + 1, z, YELLOW_FLOWER);
                        } else if (blockTreeVal < 0.12) {
                            c->setBlockAt(x, y + 1, z, RED_MUSHROOMS);
                        }
                    }
                }
            } else {
                if (t.y > 0.5f) {
                    if (t.x > 0.5f) {
                        // desert
                        biome.setBlocks(DESERT, DESERT);
                        float blockDesertVal = 45 * biome.noise2D(glm::ivec2(desertY * currX, desertY * currZ + y));
                        if (blockDesertVal < 0.08) {
                            int blockCactusVal = floor(4 - 3 * glm::fract(15 * desertY));
                            for (int i = 0; i < blockCactusVal; i++) {
                                c->setBlockAt(x, y + i + 1, z, CACTUS);
                            }
                        } else if (blockDesertVal < 0.12) {
                            c->setBlockAt(x, y + 1, z, DEAD_PLANT);
                        }

                    } else {
                        // plateau
                        biome.setBlocks(RED_STONE, RED_STONE);
                        if (y >= 110 && y <= 112) {
                            c->flatness[x * 16 + z] = INT_MAX - 1;
                        } else {
                            flat_lock->lock();
                            if (y == 113) {
                                c->flatness[x * 16 + z] = 0;
                                flat->push(new glm::ivec2(currX, currZ));
                            }
                            flat_lock->unlock();
                        }
                    }
                } else {
                    if (t.x > 0.5f) {
                        // cave
                        biome.setBlocks(BEDROCK, BEDROCK);
                    } else {
                        // snow
                        biome.setBlocks(SNOW, STONE);
                    }
                }
            }

            // stack the blocks
            int underTopBlockStart = 60;
            if (biome.underTopBlock == BEDROCK) {
                underTopBlockStart = 115;
            } else if (biome.underTopBlock == ICE) {
                underTopBlockStart = 100;
            }

            for (int i = underTopBlockStart; i < y; i++) {
                c->setBlockAt(x, i, z, biome.underTopBlock);
            }

            // add snow
            if ((biome.underTopBlock == ICE && y > 130) ||
                    (biome.underTopBlock == STONE && y > 120) ||
                    biome.topBlock == GRASS ||
                    biome.topBlock == DARK_GRASS) {
                c->setBlockAt(x, y, z, biome.topBlock);
            } else {
                c->setBlockAt(x, y, z, biome.underTopBlock);
            }

            // blocks under ground level
            if (biome.underTopBlock == BEDROCK) {
                c->setBlockAt(x, 50, z, BEDROCK);
                for (int i = 51; i < 103; i++) {
                    float cavePerlin = Biome::perlinNoise3D(glm::vec3(currX, i, currZ) / 50.f);
                    if (cavePerlin < 0.f) {
                        if (i < 70) {
                            c->setBlockAt(x, i, z, LAVA);
                        } else {
                            c->setBlockAt(x, i, z, EMPTY);
                        }
                    } else {
                        c->setBlockAt(x, i, z, BEDROCK);
                    }
                }
            } else if (biome.underTopBlock == ICE) {
                c->setBlockAt(x, 50, z, ICE);
                int underwaterY = 100 - ((y - 100) * 2);
                if (underwaterY < 100) {
                    for (int i = 99; i > glm::max(50, underwaterY); i--) {
                        c->setBlockAt(x, i, z, ICE);
                    }
                }

                for (int i = glm::min(99, underwaterY); i > 50; i--) {
                    c->setBlockAt(x, i, z, WATER);
                }
            } else {
                if (y < 100) {
                    int waterStart = y + 1;
                    // for the seagrass
                    if (biome.topBlock == SAND) {
                        waterStart ++;
                    }
                    for (int k = waterStart; k < 100; k++) {
                        c->setBlockAt(x, k, z, WATER);
                    }
                }
            }
        }
    }

    if (treeVal < 0.01) {
        tree_lock->lock();

        // comment out this line for no trees
        treeCoords->insert(new glm::ivec2(c->starting_coord.x + mostTree.x, c->starting_coord.y + mostTree.z));
        c->treeAt = glm::ivec2(mostTree.x, mostTree.z);

        tree_lock->unlock();
    }


}
