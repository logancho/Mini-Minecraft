#include "bear.h"
#include <QString>
#include <iostream>

Bear::Bear(glm::vec3 pos, Terrain &terrain, OpenGLContext* context, const Player &p)
    : Entity(pos), contextSave(context), m_player(p), m_velocity(0,0,0), m_acceleration(0,0,0),
      playerDir(0, 0, 0),
      mcr_terrain(terrain),
      jumpReady(false),
      angleX(0.f), angleY(0.f),
      epsilon(0.005f),
      legsInLiquid(false), legsLiquid(EMPTY),
      bearTranslate(nullptr), bearRotate(nullptr), torsoScale(nullptr),
      leftArmRotate(nullptr), leftArmScale(nullptr),
      rightArmRotate(nullptr), rightArmRotate2(nullptr), rightArmRotate0(nullptr), rightArmScale(nullptr),
      leftLegRotate(nullptr), leftLegScale(nullptr),
      rightLegRotate(nullptr), rightLegScale(nullptr),
      headRotate(nullptr), headScale(nullptr),
      m_geomCube(context), m_geomCube2(context),
      defaultArmAngle(0), running(false),
      animationTimer(0.f),
      armAnimation(false),
      pathFinder(false),
      path(),
      prevPos(0, 0, 0),
      prevAutoJumpBlock(0, 0, 0)
{}

Bear::~Bear()
{}

void Bear::tickNPC(float dT) {
    processInputsNPC();
    runAnimation(dT);
    computePhysics(dT, mcr_terrain);
}
void Bear::activatePathFinder() {
    pathFinder = true;
    prevPos = glm::ivec3(glm::floor(m_position));
}

const static glm::ivec3 surroundingGrid[4] = {
    glm::ivec3(0, 0, 1),
    glm::ivec3(1, 0, 0),
    glm::ivec3(0, 0, -1),
    glm::ivec3(-1, 0, 0),
};

//maxDist: Visible range for Bear NPC to detect Player
const int maxDist = 50;

//inline double heuristic(GridLocation a, GridLocation b) {
//  return std::abs(a.x - b.x) + std::abs(a.y - b.y);
//}

inline int heuristic(glm::ivec3 a, glm::ivec3 b) {
    return std::abs(a.x - b.x) + std::abs(a.y - b.y) + std::abs(a.z - b.z);
}

inline std::pair<std::pair<int,int>, int> convertToKey(glm::ivec3 input) {
    return std::make_pair(std::make_pair(input.x, input.y), input.z);
}

inline glm::ivec3 convertToVec(std::pair<std::pair<int,int>, int> input) {
//    return std::make_pair(std::make_pair(input.x, input.y), input.z);
    return glm::ivec3(input.first.first, input.first.second, input.second);
}

inline bool Compare(std::pair<int, std::pair<std::pair<int,int>, int>> a, std::pair<int, std::pair<std::pair<int,int>, int>> b) {
    return a.first > b.first;
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

WOOD for discovering
SNOW for processing
GRASS for path
LEAF for goal
*/
void Bear::processInputsNPC() {
    playerDir = m_player.m_position - m_position;
    if (!pathFinder && glm::length(playerDir) <= 6.f) {
        float yAngle = glm::orientedAngle(m_forward, glm::normalize(playerDir),  m_right);
        rotateOnRightLocal(yAngle * 180.f / M_PI);

        float xzAngle = glm::orientedAngle(glm::vec3(m_forward.x, 0.f, m_forward.z), glm::normalize(glm::vec3(playerDir.x, 0.f, playerDir.z)),
                                           glm::vec3(0.f, 1.f, 0.f));
        rotateOnUpGlobal(xzAngle * 180.f / M_PI);
    }

    //Check to see if Player is within visible range of NPC
    if (glm::length(playerDir) >= 1.f && glm::length(playerDir) <= maxDist) {
        //BFS NPC AI Player Pathfinder:
        if (pathFinder && path.empty()) {
            glm::ivec3 currCell = glm::ivec3(glm::floor(m_position));
            glm::ivec3 currCellPlayer = glm::ivec3(glm::floor(m_player.m_position));

            ////Visualize
            mcr_terrain.setBlockAt(currCellPlayer.x, currCellPlayer.y - 1, currCellPlayer.z, BlockType::ICE);
            Chunk* c1 = mcr_terrain.getChunkAt(currCellPlayer.x, currCellPlayer.z).get();

            mcr_terrain.appendEditedChunk(c1);
            ////

            std::map<std::pair<std::pair<int,int>, int>, std::pair<std::pair<int,int>, int>> came_from;
            std::map<std::pair<std::pair<int,int>, int>, int> cost_so_far;

            //2. Set the center of our grid to be 0
            came_from[convertToKey(currCell)] = convertToKey(currCell);
            cost_so_far[convertToKey(currCell)] = 0;

            std::priority_queue<std::pair<int, std::pair<std::pair<int,int>, int>>,
                    std::vector<std::pair<int, std::pair<std::pair<int,int>, int>>>,
                    std::function<bool(std::pair<int, std::pair<std::pair<int,int>, int>>,
                                       std::pair<int, std::pair<std::pair<int,int>, int>>)>> pq(Compare);
            pq.push(std::make_pair(0, convertToKey(currCell)));

            while (!pq.empty()) {
                std::pair<std::pair<int,int>, int> cur = pq.top().second;
                pq.pop();

                if (cur == convertToKey(currCellPlayer)) {
                    break;
                }

                ////Visualize
                mcr_terrain.setBlockAt(cur.first.first, cur.first.second - 1, cur.second, BlockType::SNOW);
                Chunk* c2 = mcr_terrain.getChunkAt(cur.first.first, cur.second).get();

                mcr_terrain.appendEditedChunk(c2);
                ////

                //Check direct neighbors in 4 directions of current block, North, East, South, West:
                for (int i = 0; i < 4; i++) {
                    glm::ivec3 neighbor = convertToVec(cur) + surroundingGrid[i]; //neighbor in 3D space (x, z relative to grid, y in world space)
                    //Check if the neighbor is in bounds of our 2d array
                    if (std::abs(neighbor.x - currCell.x) <= maxDist * 2 &&
                            std::abs(neighbor.y - currCell.y) <= maxDist * 2 &&
                            std::abs(neighbor.z - currCell.z) <= maxDist * 2) {
                        //Casework:
                        //CASE 1: FLAT - y is unchanged
                        if (mcr_terrain.getBlockAt(neighbor.x, neighbor.y, neighbor.z) == EMPTY
                                && mcr_terrain.getBlockAt(neighbor.x, neighbor.y + 1, neighbor.z) == EMPTY
                                && mcr_terrain.getBlockAt(neighbor.x, neighbor.y - 1, neighbor.z) != EMPTY) {
                            glm::ivec3 next = neighbor;
                            int new_cost = cost_so_far[cur] + 1;
                            if (cost_so_far.find(convertToKey(next)) == cost_so_far.end() || new_cost < cost_so_far[convertToKey(next)]) {
                                cost_so_far[convertToKey(next)] = new_cost;
                                int priority = new_cost + heuristic(next, currCellPlayer);

                                pq.push(std::make_pair(priority, convertToKey(next)));
                                came_from[convertToKey(next)] = cur;
                                ////Visualize
                                mcr_terrain.setBlockAt(next.x, next.y - 1, next.z, BlockType::WOOD);
                                Chunk* c3 = mcr_terrain.getChunkAt(cur.first.first, cur.second).get();

                                mcr_terrain.appendEditedChunk(c3);
                                ////
                            }
                        }
                        //Case 2: UP HILL - change in y is +1
                        else if (mcr_terrain.getBlockAt(neighbor.x, neighbor.y, neighbor.z) != EMPTY
                                && mcr_terrain.getBlockAt(neighbor.x, neighbor.y + 1, neighbor.z) == EMPTY
                                && mcr_terrain.getBlockAt(neighbor.x, neighbor.y + 2, neighbor.z) == EMPTY) {

                            glm::ivec3 next = glm::ivec3(neighbor.x, neighbor.y + 1, neighbor.z);
                            int new_cost = cost_so_far[cur] + 1;
                            if (cost_so_far.find(convertToKey(next)) == cost_so_far.end() || new_cost < cost_so_far[convertToKey(next)]) {
                                cost_so_far[convertToKey(next)] = new_cost;
                                int priority = new_cost + heuristic(next, currCellPlayer);
                                pq.push(std::make_pair(priority, convertToKey(next)));
                                came_from[convertToKey(next)] = cur;
                                ////Visualize
                                mcr_terrain.setBlockAt(next.x, next.y - 1, next.z, BlockType::WOOD);
                                Chunk* c3 = mcr_terrain.getChunkAt(cur.first.first, cur.second).get();

                                mcr_terrain.appendEditedChunk(c3);
                                ////
                            }
                        }
                        //Case 3: DOWN HILL - change in y is -1
                        else if (mcr_terrain.getBlockAt(neighbor.x, neighbor.y, neighbor.z) == EMPTY
                                && mcr_terrain.getBlockAt(neighbor.x, neighbor.y - 1, neighbor.z) == EMPTY
                                && mcr_terrain.getBlockAt(neighbor.x, neighbor.y - 2, neighbor.z) != EMPTY) {
                            glm::ivec3 next = glm::ivec3(neighbor.x, neighbor.y - 1, neighbor.z);
                            int new_cost = cost_so_far[cur] + 1;
                            if (cost_so_far.find(convertToKey(next)) == cost_so_far.end() || new_cost < cost_so_far[convertToKey(next)]) {
                                cost_so_far[convertToKey(next)] = new_cost;
                                int priority = new_cost + heuristic(next, currCellPlayer);
                                pq.push(std::make_pair(priority, convertToKey(next)));
                                came_from[convertToKey(next)] = cur;
                                ////Visualize
                                mcr_terrain.setBlockAt(next.x, next.y - 1, next.z, BlockType::WOOD);
                                Chunk* c3 = mcr_terrain.getChunkAt(cur.first.first, cur.second).get();

                                mcr_terrain.appendEditedChunk(c3);
                                ////
                            }
                        }
                    }
                }
            }

            if (came_from.find(convertToKey(currCellPlayer)) != came_from.end()) {
                //Replace old path with new path:
                path.clear();
                std::pair<std::pair<int,int>, int> cur = came_from[convertToKey(currCellPlayer)];
                ////Visualize
                mcr_terrain.setBlockAt(currCellPlayer.x, currCellPlayer.y - 1, currCellPlayer.z, BlockType::ICE);
                Chunk* c1 = mcr_terrain.getChunkAt(currCellPlayer.x, currCellPlayer.z).get();

                mcr_terrain.appendEditedChunk(c1);
                ////

                while (came_from[cur] != cur) {
                    ////Visualize
                    mcr_terrain.setBlockAt(cur.first.first, cur.first.second - 1, cur.second, BlockType::GRASS);
                    Chunk* c4 = mcr_terrain.getChunkAt(cur.first.first, cur.second).get();

                    mcr_terrain.appendEditedChunk(c4);
                    ////
                    glm::ivec2 diff = glm::ivec2(cur.first.first, cur.second) - glm::ivec2(came_from[cur].first.first, came_from[cur].second);
                    if (diff == glm::ivec2(0, 1)) {
                        path.push_front(FORWARDS);
                    }
                    else if (diff == glm::ivec2(1, 0)) {
                        path.push_front(LEFT);
                    }
                    else if (diff == glm::ivec2(0, -1)) {
                        path.push_front(BACKWARDS);
                    }
                    else if (diff == glm::ivec2(-1, 0)) {
                        path.push_front(RIGHT);
                    }
                    cur = came_from[cur];
                }
            } else {
                pathFinder = false;
            }

        }
    }

    //Bear's Acceleration
    float scalarMultiplier = 2.f;
    m_acceleration = glm::vec3(0, 0, 0);
    float threshold = 0.05f;
    float bearAcceleration = 7.f;
    float turnDrag = 0.1f;

    //If bear is in player tracking mode, and there exists a valid path to the player:
    if (pathFinder && !path.empty()) {
        MobDirection cur = path.front();
        float potentialAngle = 0.f;

        switch(cur) {
        case FORWARDS:
            potentialAngle = glm::orientedAngle(glm::vec3(m_forward.x, 0.f, m_forward.z),
                                                glm::normalize(glm::vec3(0.f, 0.f, 1.f)),
                                                glm::vec3(0.f, 1.f, 0.f));
            if (potentialAngle != 0) {
                rotateOnUpGlobal((potentialAngle * 180.f / M_PI) * turnDrag);
            }
            m_acceleration.z = bearAcceleration;
            if (std::abs(prevPos.z + 1.5f - m_position.z) <= threshold) {
//                prevPos = glm::ivec3(glm::floor(m_position));
                prevPos.z++;
                prevPos.y = glm::floor(m_position).y;
                path.pop_front();
            }
            break;
        case BACKWARDS:
            //Rotation:
            potentialAngle = glm::orientedAngle(glm::vec3(m_forward.x, 0.f, m_forward.z),
                                                glm::normalize(glm::vec3(0.f, 0.f, -1.f)),
                                                glm::vec3(0.f, 1.f, 0.f));
            if (potentialAngle != 0) {
                rotateOnUpGlobal((potentialAngle * 180.f / M_PI) * turnDrag);
            }
            m_acceleration.z = -bearAcceleration;
            if (std::abs(m_position.z - (prevPos.z - 0.5f)) <= threshold) {
//                prevPos = glm::ivec3(glm::floor(m_position));
                prevPos.z--;
                prevPos.y = glm::floor(m_position).y;
                path.pop_front();
            }
            break;
        case LEFT:
            //Rotation:
            potentialAngle = glm::orientedAngle(glm::vec3(m_forward.x, 0.f, m_forward.z),
                                                glm::normalize(glm::vec3(1.f, 0.f, 0.f)),
                                                glm::vec3(0.f, 1.f, 0.f));
            if (potentialAngle != 0) {
                rotateOnUpGlobal((potentialAngle * 180.f / M_PI) * turnDrag);
            }
            m_acceleration.x = bearAcceleration;
            if (std::abs(prevPos.x + 1.5f - m_position.x) <= threshold) {
//                prevPos = glm::ivec3(glm::floor(m_position));
                prevPos.x++;
                prevPos.y = glm::floor(m_position).y;
                path.pop_front();
            }
            break;
        case RIGHT:
            //Rotation:
            potentialAngle = glm::orientedAngle(glm::vec3(m_forward.x, 0.f, m_forward.z),
                                                glm::normalize(glm::vec3(-1.f, 0.f, 0.f)),
                                                glm::vec3(0.f, 1.f, 0.f));
            if (potentialAngle != 0) {
                rotateOnUpGlobal((potentialAngle * 180.f / M_PI) * turnDrag);
            }
            m_acceleration.x = -bearAcceleration;
            if (std::abs(m_position.x - (prevPos.x - 0.5f)) <= threshold) {
//                prevPos = glm::ivec3(glm::floor(m_position));
                prevPos.x--;
                prevPos.y = glm::floor(m_position).y;
                path.pop_front();
            }
            break;
        default:
            break;
        }
        if (path.empty()) {
            pathFinder = false;
        }
    }
    //Final step: Scalar multiplier + Gravity

    m_acceleration *= scalarMultiplier;
    float gravity = -14.0f;
    m_acceleration.y = gravity;
}

void Bear::runAnimation(float dT) {
    //Animations
    animationTimer += (dT / 1000.f);
    if (animationTimer > M_PI * 100.f) {
        animationTimer = 0.f;
    }
    float angle = 60.f;
    float speed = 5.f;
    if (running) {
        if (leftArmRotate != nullptr) {
            (static_cast<RotateNode*>(leftArmRotate))->deg = angle * glm::sin(animationTimer * speed + M_PI);
        }
        if (rightArmRotate != nullptr && !armAnimation) {
            (static_cast<RotateNode*>(rightArmRotate))->deg = angle * glm::sin(animationTimer * speed);
        }
        if (leftLegRotate != nullptr) {
            (static_cast<RotateNode*>(leftLegRotate))->deg = angle * glm::sin(animationTimer * speed);
        }
        if (rightLegRotate != nullptr) {
            (static_cast<RotateNode*>(rightLegRotate))->deg = angle * glm::sin(animationTimer * speed + M_PI);
        }
    } else {
        if (leftArmRotate != nullptr && glm::abs((static_cast<RotateNode*>(leftArmRotate))->deg) >= 5.0f) {
            (static_cast<RotateNode*>(leftArmRotate))->deg = angle * glm::sin(animationTimer * speed + M_PI);
        } else {
            (static_cast<RotateNode*>(leftArmRotate))->deg = 0.f;
        }
        if (rightArmRotate != nullptr && glm::abs((static_cast<RotateNode*>(rightArmRotate))->deg) >= 5.0f) {
            (static_cast<RotateNode*>(rightArmRotate))->deg = angle * glm::sin(animationTimer * speed);
        } else {
            (static_cast<RotateNode*>(rightArmRotate))->deg = 0.f;
        }
        if (leftLegRotate != nullptr && glm::abs((static_cast<RotateNode*>(leftLegRotate))->deg) >= 5.0f) {
            (static_cast<RotateNode*>(leftLegRotate))->deg = angle * glm::sin(animationTimer * speed);
        } else {
            (static_cast<RotateNode*>(leftLegRotate))->deg = 0.f;
        }
        if (rightLegRotate != nullptr && glm::abs((static_cast<RotateNode*>(rightLegRotate))->deg) >= 5.0f) {
            (static_cast<RotateNode*>(rightLegRotate))->deg = angle * glm::sin(animationTimer * speed + M_PI);
        } else {
            (static_cast<RotateNode*>(rightLegRotate))->deg = 0.f;
        }
    }
}

void Bear::computePhysics(float dT, Terrain &terrain) {
    // TODO: Update the Bear's position based on its acceleration
    // and velocity, and also perform collision detection.

    checkLegsInLiquid(terrain);

    //Friction:
    float friction;

    friction = 0.86f;
    m_velocity.x *= friction;
    m_velocity.z *= friction;

    m_velocity += m_acceleration * (dT / 1000.f);

    glm::vec3 rayDir = m_velocity * (dT / 1000.f);

    physicsCollisions(rayDir, terrain);

    if (legsInLiquid) {
        // slow down movement by a lot more than 2/3 as it is not noticeable
        rayDir *= 0.2;
    }

    running = (glm::abs(rayDir.x / (dT / 1000.f)) >= 0.1f) || (glm::abs(rayDir.z / (dT / 1000.f)) >= 0.1f);
    moveAlongVector(rayDir);
}

void Bear::moveAlongVector(glm::vec3 dir) {
    Entity::moveAlongVector(dir);

    //Update translate
    if (bearTranslate != nullptr) {
        (static_cast<TranslateNode*>(bearTranslate.get()))->trans = glm::vec3(m_position.x, m_position.y + 1.8f, m_position.z);
    }
}

void Bear::moveForwardLocal(float amount) {
    Entity::moveForwardLocal(amount);
}
void Bear::moveRightLocal(float amount) {
    Entity::moveRightLocal(amount);
}
void Bear::moveUpLocal(float amount) {
    Entity::moveUpLocal(amount);
}
void Bear::moveForwardGlobal(float amount) {
    Entity::moveForwardGlobal(amount);
}
void Bear::moveRightGlobal(float amount) {
    Entity::moveRightGlobal(amount);
}
void Bear::moveUpGlobal(float amount) {
    Entity::moveUpGlobal(amount);
}

void Bear::rotateOnForwardLocal(float degrees) {
    Entity::rotateOnForwardLocal(degrees);
}
void Bear::rotateOnRightLocal(float degrees) {
    //Head animation
    if (headRotate != nullptr) {
        (static_cast<RotateNode*>(headRotate))->deg = degrees;
    }
}

void Bear::rotateOnUpLocal(float degrees) {
    Entity::rotateOnUpLocal(degrees);
}
void Bear::rotateOnForwardGlobal(float degrees) {
    Entity::rotateOnForwardGlobal(degrees);
}
void Bear::rotateOnRightGlobal(float degrees) {
    Entity::rotateOnRightGlobal(degrees);

}
void Bear::rotateOnUpGlobal(float degrees) {
    Entity::rotateOnUpGlobal(degrees);

    //Rotate our Bearmodel
    if (bearRotate != nullptr) {
        (static_cast<RotateNode*>(bearRotate))->deg += degrees;
    }
}

QString Bear::posAsQString() const {
    std::string str("( " + std::to_string(m_position.x) + ", " + std::to_string(m_position.y) + ", " + std::to_string(m_position.z) + ")");
    return QString::fromStdString(str);
}
QString Bear::velAsQString() const {
    std::string str("( " + std::to_string(m_velocity.x) + ", " + std::to_string(m_velocity.y) + ", " + std::to_string(m_velocity.z) + ")");
    return QString::fromStdString(str);
}
QString Bear::accAsQString() const {
    std::string str("( " + std::to_string(m_acceleration.x) + ", " + std::to_string(m_acceleration.y) + ", " + std::to_string(m_acceleration.z) + ")");
    return QString::fromStdString(str);
}
QString Bear::lookAsQString() const {
    std::string str("( " + std::to_string(m_forward.x) + ", " + std::to_string(m_forward.y) + ", " + std::to_string(m_forward.z) + ")");
    return QString::fromStdString(str);
}

std::pair<bool, BlockType> Bear::gridMarch(glm::vec3 rayOrigin, glm::vec3 rayDirection, const Terrain &terrain, float *out_dist, glm::ivec3 *out_blockHit) {

    float maxLen = glm::length(rayDirection); // Farthest we search

    glm::ivec3 currCell = glm::ivec3(glm::floor(rayOrigin));

    rayDirection = glm::normalize(rayDirection); // Now all t values represent world dist.

    float curr_t = 0.f;


    while(curr_t < maxLen) {

        float min_t = glm::sqrt(3.f);

        float interfaceAxis = -1; // Track axis for which t is smallest

        for(int i = 0; i < 3; ++i) { // Iterate over the three axes

            if(rayDirection[i] != 0) { // Is ray parallel to axis i?

                float offset = glm::max(0.f, glm::sign(rayDirection[i])); // See slide 5


                // If the Bear is *exactly* on an interface then
                // they'll never move if they're looking in a negative direction
                if(currCell[i] == rayOrigin[i] && offset == 0.f) {

                    offset = -1.f;
                }

                int nextIntercept = currCell[i] + offset;

                float axis_t = (nextIntercept - rayOrigin[i]) / rayDirection[i];

                axis_t = glm::min(axis_t, maxLen); // Clamp to max len to avoid super out of bounds errors

                if(axis_t < min_t) {
                    min_t = axis_t;
                    interfaceAxis = i;

                }
            }
        }
        if(interfaceAxis == -1) {
            throw std::out_of_range("interfaceAxis was -1 after the for loop in gridMarch!");
        }

        curr_t += min_t; // min_t is declared in slide 7 algorithm

        rayOrigin += rayDirection * min_t;

        glm::ivec3 offset = glm::ivec3(0,0,0);
        // Sets it to 0 if sign is +, -1 if sign is -
        offset[interfaceAxis] = glm::min(0.f, glm::sign(rayDirection[interfaceAxis]));

        currCell = glm::ivec3(glm::floor(rayOrigin + glm::vec3(offset)));

        //IF STATEMENT I ADDED!!! Fixes all issues
        if (curr_t < maxLen) {
            BlockType cellType = terrain.getBlockAt(currCell.x, currCell.y, currCell.z);
            if(cellType != EMPTY) {
                *out_blockHit = currCell;
                *out_dist = glm::min(maxLen, curr_t);
                return std::make_pair(true, cellType);
            }
        }
    }
    *out_dist = glm::min(maxLen, curr_t);
    return std::make_pair(false, EMPTY);
}

void Bear::constructSceneGraph() {
    ////// 1. Create Nodes

    //Torso
    uPtr<Node> torsoT = mkU<TranslateNode> (glm::vec3(m_position.x, m_position.y + 1.8f, m_position.z));
    uPtr<Node> torsoR = mkU<RotateNode> ();
    uPtr<Node> torsoS = mkU<ScaleNode> (2.0f, 2.0f, 2.0f);
    //Add geom to torsoS
    torsoS->mCuboid = (&m_geomCube);
    torsoS->type = TORSO;
    //Legs
    //Left Leg
    uPtr<Node> leftLegT = mkU<TranslateNode> (glm::vec3(-0.375f, -0.875f, 0.25f));
    uPtr<Node> leftLegR = mkU<RotateNode> ();
    uPtr<Node> leftLegT2 = mkU<TranslateNode> (glm::vec3(0.f, -0.5f, 0.f));
    uPtr<Node> leftLegS = mkU<ScaleNode> (0.5f, 0.75f, 0.5f);
    leftLegS->mCuboid = (&m_geomCube);
    leftLegS->type = LEFTLEG;
    //Right Leg
    uPtr<Node> rightLegT = mkU<TranslateNode> (glm::vec3(0.375f, -0.875f, 0.25f));
    uPtr<Node> rightLegR = mkU<RotateNode> ();
    uPtr<Node> rightLegT2 = mkU<TranslateNode> (glm::vec3(0.f, -0.5f, 0.f));
    uPtr<Node> rightLegS = mkU<ScaleNode> (0.5f, 0.75f, 0.5f);
    rightLegS->mCuboid = (&m_geomCube);
    rightLegS->type = RIGHTLEG;
    //Arms
    //Left Arm
    uPtr<Node> leftArmT = mkU<TranslateNode> (glm::vec3(-1.375f, 0.25f, 0.375f));
    uPtr<Node> leftArmR = mkU<RotateNode> ();
    uPtr<Node> leftArmT2 = mkU<TranslateNode> (glm::vec3(0.f, -0.25f, 0.f));
    uPtr<Node> leftArmS = mkU<ScaleNode> (0.75f, 1.5f, 0.75f);
    leftArmS->mCuboid = (&m_geomCube);
    leftArmS->type = LEFTARM;
    //Right Arm
    uPtr<Node> rightArmT = mkU<TranslateNode> (glm::vec3(1.375f, 0.25f, 0.375f));
    uPtr<Node> rightArmR2 = mkU<RotateNode> (0.f, glm::vec3(0.f, 1.f, 0.f));
    uPtr<Node> rightArmR = mkU<RotateNode> (0.f, glm::vec3(1.f, 0.f, 0.f));
    uPtr<Node> rightArmT2 = mkU<TranslateNode> (glm::vec3(0.f, -0.25f, 0.f));
    uPtr<Node> rightArmS = mkU<ScaleNode> (0.75f, 1.5f, 0.75f);
    rightArmS->mCuboid = (&m_geomCube);
    rightArmS->type = RIGHTARM;

    //Head
    uPtr<Node> headT = mkU<TranslateNode> (glm::vec3(0.f, 1.5f, 0.5f));
    uPtr<Node> headR = mkU<RotateNode> (0.f, glm::vec3(1.f, 0.f, 0.f));
    uPtr<Node> headS = mkU<ScaleNode> (1.0f, 1.0f, 1.0f);
    headS->mCuboid = (&m_geomCube);
    headS->type = FACE;

    uPtr<Node> snoutT = mkU<TranslateNode> (glm::vec3(0.f, -0.2f, -0.7f));
    uPtr<Node> snoutR = mkU<RotateNode> (0.f, glm::vec3(1.f, 0.f, 0.f));
    uPtr<Node> snoutS = mkU<ScaleNode> (0.8f, 0.4f, 0.2f);
    snoutS->mCuboid = (&m_geomCube);
    snoutS->visible = false;

    ////// 1.5 Assign pointers

    //BearRotate
    bearRotate = torsoR.get();
    (static_cast<RotateNode*>(bearRotate))->axisOfRotation = glm::vec3(0.f, 1.f, 0.f);

    //torsoScale
    torsoScale = torsoS.get();

    //leftArmRotate
    leftArmRotate = leftArmR.get();
    //leftArmScale
    leftArmScale = leftArmS.get();


    //rightArmRotate
    rightArmRotate = rightArmR.get();
    //rightArmRotate2
    rightArmRotate2 = rightArmR2.get();
    //rightArmScale
    rightArmScale = rightArmS.get();


    //leftLegRotate
    leftLegRotate = leftLegR.get();
    //leftLegScale
    leftLegScale = leftLegS.get();


    //rightLegRotate
    rightLegRotate = rightLegR.get();
    //rightLegScale
    rightLegScale = rightLegS.get();


    //headRotate
    headRotate = headR.get();

    //headScale
    headScale = headS.get();


    ////// 2. Assign Hierarchy

    Node& torsoTemp = torsoT->addChild(std::move(torsoR));
    torsoTemp.addChild(std::move(torsoS));

    Node& leftLegTemp = torsoTemp.addChild(std::move(leftLegT))
            .addChild(std::move(leftLegR))
            .addChild(std::move(leftLegT2));
    leftLegTemp.addChild(std::move(leftLegS));

    Node& rightLegTemp = torsoTemp.addChild(std::move(rightLegT))
            .addChild(std::move(rightLegR))
            .addChild(std::move(rightLegT2));
    rightLegTemp.addChild(std::move(rightLegS));

    Node& leftArmTemp = torsoTemp.addChild(std::move(leftArmT))
            .addChild(std::move(leftArmR))
            .addChild(std::move(leftArmT2));
    leftArmTemp.addChild(std::move(leftArmS));

    Node& rightArmTemp = torsoTemp.addChild(std::move(rightArmT))
            .addChild(std::move(rightArmR2))
            .addChild(std::move(rightArmR))
            .addChild(std::move(rightArmT2));
    rightArmTemp.addChild(std::move(rightArmS));

    Node& headTemp = torsoTemp.addChild(std::move(headT))
            .addChild(std::move(headR));
    headTemp.addChild(std::move(headS));

    headTemp.addChild(std::move(snoutT))
            .addChild(std::move(snoutR))
            .addChild(std::move(snoutS));

    ////// 3. Assign Main to Field
    bearTranslate = std::move(torsoT);
}

void Bear::toggleNodeVisibility() {
    std::vector<Node*> geomNodes = {headScale, torsoScale, leftArmScale, leftLegScale, rightLegScale};
    for (Node* n : geomNodes) {
        n->visible = !(n->visible);
    }
}

void Bear::checkLegsInLiquid(const Terrain &terrain) {
    legsInLiquid = false;
    // check if legs and camera are in liquid
    glm::ivec3 currCellLegs = glm::ivec3(glm::floor(m_position));glm::ivec3 currCellCamera = glm::ivec3(glm::floor(m_position + glm::vec3(0.f, 1.5f, 0.f)));
    BlockType cellTypeLegs = terrain.getBlockAt(currCellLegs.x, currCellLegs.y, currCellLegs.z);
    if (cellTypeLegs == BlockType::LAVA || cellTypeLegs == BlockType::WATER) {
        legsInLiquid = true;
    }
}

void Bear::physicsCollisions(glm::vec3 &rayDir, const Terrain &terrain) {
    std::vector<glm::vec3> rayOrigins;

    float w = 0.2f;
    float h = 1.9f;
    glm::vec3 b1 = m_position + glm::vec3(w, 0, w);
    glm::vec3 b2 = m_position + glm::vec3(-w, 0, -w);
    glm::vec3 b3 = m_position + glm::vec3(w, 0, -w);
    glm::vec3 b4 = m_position + glm::vec3(-w, 0, w);

    glm::vec3 m1 = m_position + glm::vec3(w, h, w);
    glm::vec3 m2 = m_position + glm::vec3(-w, h, -w);
    glm::vec3 m3 = m_position + glm::vec3(w, h, -w);
    glm::vec3 m4 = m_position + glm::vec3(-w, h, w);

    glm::vec3 t1 = m_position + glm::vec3(w, 2 * h, w);
    glm::vec3 t2 = m_position + glm::vec3(-w, 2 * h, -w);
    glm::vec3 t3 = m_position + glm::vec3(w, 2 * h, -w);
    glm::vec3 t4 = m_position + glm::vec3(-w, 2 * h, w);

    rayOrigins.push_back(b1);
    rayOrigins.push_back(b2);
    rayOrigins.push_back(b3);
    rayOrigins.push_back(b4);
    rayOrigins.push_back(m1);
    rayOrigins.push_back(m2);
    rayOrigins.push_back(m3);
    rayOrigins.push_back(m4);
    rayOrigins.push_back(t1);
    rayOrigins.push_back(t2);
    rayOrigins.push_back(t3);
    rayOrigins.push_back(t4);

    glm::vec3 rayDirX = glm::vec3(rayDir.x, 0, 0);
    glm::vec3 rayDirY = glm::vec3(0, rayDir.y, 0);
    glm::vec3 rayDirZ = glm::vec3(0, 0, rayDir.z);

    //For every single vertex that ray casts, I need to find the onetime where out_dist is the SMALLEST!!!
    //This is the only one that matters
    float minOutDistY = std::numeric_limits<float>::infinity();
    float minOutDistX = std::numeric_limits<float>::infinity();
    float minOutDistZ = std::numeric_limits<float>::infinity();

    glm::ivec3 closestOut_BlockHitX = glm::ivec3();
    glm::ivec3 closestOut_BlockHitZ= glm::ivec3();

    for (int i = 0; i < rayOrigins.size(); i++) {

        glm::vec3 v = rayOrigins[i];

        glm::ivec3 out_blockHitX = glm::ivec3();
        float out_distX = 0.f;
        std::pair<bool, BlockType> gridMarchX = gridMarch(v, rayDirX, mcr_terrain, &out_distX, &out_blockHitX);
        if (gridMarchX.first) {
            if (gridMarchX.second != LAVA && gridMarchX.second != WATER && out_distX < minOutDistX && out_distX < minOutDistX) {
                minOutDistX = out_distX;
            }
        }

        glm::ivec3 out_blockHitY = glm::ivec3();
        float out_distY = 0.f;
        std::pair<bool, BlockType> gridMarchY = gridMarch(v, rayDirY, mcr_terrain, &out_distY, &out_blockHitY);
        if (gridMarchY.first) {
            if (gridMarchY.second != LAVA && gridMarchY.second != WATER && out_distY < minOutDistY && out_distY < minOutDistY) {
                jumpReady = true;
                minOutDistY = out_distY;
            }
        }
        glm::ivec3 out_blockHitZ = glm::ivec3();
        float out_distZ = 0.f;
        std::pair<bool, BlockType> gridMarchZ = gridMarch(v, rayDirZ, mcr_terrain, &out_distZ, &out_blockHitZ);
        if (gridMarchZ.first) {
            if (gridMarchZ.second != LAVA && gridMarchZ.second != WATER && out_distZ < minOutDistZ && out_distZ < minOutDistZ) {
                minOutDistZ = out_distZ;
            }
        }
    }

    float autoJumpThreshold = 0.1f;
    if (minOutDistY < std::numeric_limits<float>::infinity()) {
        if (minOutDistY < epsilon) {
            rayDir.y = 0.f;
            //Reduce y velocity which builds up to max value due to constant gravity
            m_velocity.y *= 0.5f; // was 0.5f
        } else {
            rayDir.y = (minOutDistY * 0.8) * glm::sign(rayDir.y);
        }
    }

    if (minOutDistX < std::numeric_limits<float>::infinity()) {
        if (minOutDistX < epsilon) {
            rayDir.x = 0.f;

            if (terrain.getBlockAt(closestOut_BlockHitX.x, closestOut_BlockHitX.y + 1, closestOut_BlockHitX.z) == BlockType::EMPTY
                    && jumpReady && glm::abs(m_velocity.x) > autoJumpThreshold) {
                if (prevAutoJumpBlock == glm::ivec3(closestOut_BlockHitX.x, closestOut_BlockHitX.y + 1, closestOut_BlockHitX.z)) {
                    if (closestOut_BlockHitX.z + 0.5 - m_position.z > 0) {
                        m_velocity.z = -0.2f;
                    } else {
                        m_velocity.z = 0.2f;
                    }
                }
                prevAutoJumpBlock = glm::ivec3(closestOut_BlockHitX.x, closestOut_BlockHitX.y + 1, closestOut_BlockHitX.z);
                m_velocity.y = 6.0f;
                jumpReady = false;
            }
        } else {
            rayDir.x = (minOutDistX * 0.8) * glm::sign(rayDir.x);
        }
    }

    if (minOutDistZ < std::numeric_limits<float>::infinity()) {
        if (minOutDistZ < epsilon) {
            rayDir.z = 0.f;
//                Allow to jump over block of height unit 1
            if (terrain.getBlockAt(closestOut_BlockHitZ.x, closestOut_BlockHitZ.y + 1, closestOut_BlockHitZ.z) == BlockType::EMPTY
                    && jumpReady && glm::abs(m_velocity.z) > autoJumpThreshold) {
                //
                if (prevAutoJumpBlock == glm::ivec3(closestOut_BlockHitZ.x, closestOut_BlockHitZ.y + 1, closestOut_BlockHitZ.z)) {
//                    std::cout << "Yo! z repeat\n";
                    //compare center of block to bear's position, then determine whether we need to move left or right
                    //clearly, we are jumping in the +z direction in this case, therefore, we will adjust either +- x
//                    float diff = closestOut_BlockHitZ.x + 0.5 - m_position.x;
                    if (closestOut_BlockHitZ.x + 0.5 - m_position.x > 0) {
                        m_velocity.x = -0.2f;
                    } else {
                        m_velocity.x = 0.2f;
                    }
                }
                prevAutoJumpBlock = glm::ivec3(closestOut_BlockHitZ.x, closestOut_BlockHitZ.y + 1, closestOut_BlockHitZ.z);
                m_velocity.y = 6.0f;
                jumpReady = false;
            }
        } else {
            rayDir.z = (minOutDistZ * 0.8) * glm::sign(rayDir.z);
        }
    }
}

void Bear::defaultBearRotation() {
    if (leftArmRotate != nullptr && rightArmRotate != nullptr && leftLegRotate != nullptr && rightLegRotate != nullptr) {
        (static_cast<RotateNode*>(leftArmRotate))->deg = 0.f;

        (static_cast<RotateNode*>(rightArmRotate))->deg = 0.f;

        (static_cast<RotateNode*>(leftLegRotate))->deg = 0.f;

        (static_cast<RotateNode*>(rightLegRotate))->deg = 0.f;
    }
}

void Bear::tick(float dT, InputBundle &input) {
    processInputs(input);
    computePhysics(dT, mcr_terrain);
}


void Bear::processInputs(InputBundle &inputs) {

    //Acceleration
//    thirdPerson = inputs.thirdPerson;

    float scalarMultiplier = 25.f;
    m_acceleration = glm::vec3(0, 0, 0);

    if (inputs.wPressed) {
        m_acceleration += glm::normalize(glm::vec3(m_forward.x, 0, m_forward.z));
    }
    if (inputs.aPressed) {
        m_acceleration -= glm::normalize(glm::vec3(m_right.x, 0, m_right.z));
    }
    if (inputs.sPressed) {
        m_acceleration -= glm::normalize(glm::vec3(m_forward.x, 0, m_forward.z));
    }
    if (inputs.dPressed) {
        m_acceleration += glm::normalize(glm::vec3(m_right.x, 0, m_right.z));
    }
    if (inputs.spacePressed && !legsInLiquid && jumpReady) {
        m_velocity += glm::vec3(0, 6.0f, 0);
        jumpReady = false;
    } else if (inputs.spacePressed && legsInLiquid) {
        m_velocity = glm::vec3(0, 10.f, 0);
    }

    m_acceleration *= scalarMultiplier;

    //Gravity:
    float gravity = -14.0f;

    m_acceleration.y = gravity;
}
