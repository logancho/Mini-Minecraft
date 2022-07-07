#include "bear.h"
#include <QString>
#include <iostream>

Bear::Bear(glm::vec3 pos, const Terrain &terrain, OpenGLContext* context, const Player &p)
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
const int maxDist = 15;

void Bear::processInputsNPC() {
    playerDir = m_player.m_position - m_position;

    //Check to see if Player is within visible range of NPC
    if (glm::length(playerDir) >= 1.f && glm::length(playerDir) <= maxDist) {
        float xzAngle = glm::orientedAngle(glm::vec3(m_forward.x, 0.f, m_forward.z), glm::normalize(glm::vec3(playerDir.x, 0.f, playerDir.z)),
                                           glm::vec3(0.f, 1.f, 0.f));
        //Rotate NPC's torso and head towards Player
        rotateOnUpGlobal(xzAngle * 180.f / M_PI);
        float yAngle = glm::orientedAngle(m_forward, glm::normalize(playerDir),  m_right);
        rotateOnRightLocal(yAngle * 180.f / M_PI);

        //BFS NPC AI Player Pathfinder:
        if (pathFinder && path.empty()) {
            //0. Find block positions of us (the NPC) and Player
            glm::ivec3 currCell = glm::ivec3(glm::floor(m_position));
            glm::ivec3 currCellPlayer = glm::ivec3(glm::floor(m_player.m_position));
            glm::ivec3 offset = glm::ivec3(maxDist, 0, maxDist) - glm::ivec3(currCell.x, 0, currCell.z);

            //1. Create a 2D array that is (maxDist * 2 + 1) x (maxDist * 2 + 1) large from mob position, set all initial values as -1
            float graph[maxDist * 2 + 1][maxDist * 2 + 1];
            glm::ivec2 prev[maxDist * 2 + 1][maxDist * 2 + 1];

            for (int i = 0; i < maxDist * 2 + 1; i++) {
                for (int j = 0; j < maxDist * 2 + 1; j++) {
                    graph[i][j] = -1.f;
                    prev[i][j] = glm::ivec2(-1, -1);
                }
            }

            //2. Set the center of our grid to be 0
            graph[maxDist][maxDist] = 0;

            //3. Perform BFS on this 2D array with specific cases to handle 3Dimensional traversal
            std::queue<glm::ivec3> q;
            //Add our center to the queue to begin BFS
            q.push(glm::ivec3(maxDist, currCell.y, maxDist));

            while (!q.empty()) {
                glm::ivec3 cur = q.front();
                q.pop();
                //Check direct neighbors in 4 directions of current block, North, East, South, West:
                for (int i = 0; i < 4; i++) {
                    glm::ivec3 neighbor = cur + surroundingGrid[i]; //neighbor in 3D space (x, z relative to grid, y in world space)
                    //Check if the neighbor is in bounds of our 2d array
                    if (neighbor.x >= 0 && neighbor.x <= maxDist * 2 && neighbor.z >= 0 && neighbor.z <= maxDist * 2) {
                        //Check if the neighboring x, z coordinate has already been recorded as impossible to reach
                        if (graph[neighbor.x][neighbor.z] != -2) {
                            //If not, then we can now check whether there exists a valid block at this grid, and if there is
                            //not any such block, we will replace graph[neighbor.x][neighbor.z] with -2

                            glm::vec3 worldNeighborF = glm::vec3(neighbor - offset);
                            //Casework:

                            //CASE 1: FLAT - y is unchanged
                            if (mcr_terrain.getBlockAt(worldNeighborF.x, worldNeighborF.y, worldNeighborF.z) == EMPTY
                                    && mcr_terrain.getBlockAt(worldNeighborF.x, worldNeighborF.y + 1, worldNeighborF.z) == EMPTY
                                    && mcr_terrain.getBlockAt(worldNeighborF.x, worldNeighborF.y - 1, worldNeighborF.z) != EMPTY) {
                                if (graph[neighbor.x][neighbor.z] == -1) {
                                    q.push(neighbor);
                                    graph[neighbor.x][neighbor.z] = graph[cur.x][cur.z] + 1;
                                    //Update prev
                                    prev[neighbor.x][neighbor.z] = glm::vec2(cur.x, cur.z);
                                } else if (graph[neighbor.x][neighbor.z] > graph[cur.x][cur.z] + 1){
                                    q.push(neighbor);
                                    graph[neighbor.x][neighbor.z] = graph[cur.x][cur.z] + 1;
                                    //Update prev
                                    prev[neighbor.x][neighbor.z] = glm::vec2(cur.x, cur.z);
                                }
                            }
                            //Case 2: UP HILL - change in y is +1
                            else if (mcr_terrain.getBlockAt(worldNeighborF.x, worldNeighborF.y, worldNeighborF.z) != EMPTY
                                    && mcr_terrain.getBlockAt(worldNeighborF.x, worldNeighborF.y + 1, worldNeighborF.z) == EMPTY
                                    && mcr_terrain.getBlockAt(worldNeighborF.x, worldNeighborF.y + 2, worldNeighborF.z) == EMPTY) {
                                if (graph[neighbor.x][neighbor.z] == -1) {
                                    q.push(glm::ivec3(neighbor.x, neighbor.y + 1, neighbor.z));
                                    graph[neighbor.x][neighbor.z] = graph[cur.x][cur.z] + 1;
                                    //Update prev
                                    prev[neighbor.x][neighbor.z] = glm::vec2(cur.x, cur.z);
                                } else if (graph[neighbor.x][neighbor.z] > graph[cur.x][cur.z] + 1){
                                    q.push(neighbor);
                                    graph[neighbor.x][neighbor.z] = graph[cur.x][cur.z] + 1;
                                    //Update prev
                                    prev[neighbor.x][neighbor.z] = glm::vec2(cur.x, cur.z);
                                }
                            }
                            //Case 3: DOWN HILL - change in y is -1
                            else if (mcr_terrain.getBlockAt(worldNeighborF.x, worldNeighborF.y, worldNeighborF.z) == EMPTY
                                    && mcr_terrain.getBlockAt(worldNeighborF.x, worldNeighborF.y - 1, worldNeighborF.z) == EMPTY
                                    && mcr_terrain.getBlockAt(worldNeighborF.x, worldNeighborF.y - 2, worldNeighborF.z) != EMPTY) {
                                if (graph[neighbor.x][neighbor.z] == -1) {
                                    q.push(glm::ivec3(neighbor.x, neighbor.y - 1, neighbor.z));
                                    graph[neighbor.x][neighbor.z] = graph[cur.x][cur.z] + 1;
                                    //Update prev
                                    prev[neighbor.x][neighbor.z] = glm::vec2(cur.x, cur.z);
                                } else if (graph[neighbor.x][neighbor.z] > graph[cur.x][cur.z] + 1){
                                    q.push(neighbor);
                                    graph[neighbor.x][neighbor.z] = graph[cur.x][cur.z] + 1;
                                    //Update prev
                                    prev[neighbor.x][neighbor.z] = glm::vec2(cur.x, cur.z);
                                }
                            }
                            else {
                                //Case 4: Unreachable
                                graph[neighbor.x][neighbor.z] = -2;
                            }
                        }
                    }
                }
            }

            //Now, since we have our finished weighted grid, we can make a path between the NPC and the player if
            //prev[playerPos] is not null, i.e. graph[currCellPlayer.x - currCell.x + maxDist][currCellPlayer.z - currCell.z + maxDist] > -1
            if (graph[currCellPlayer.x - currCell.x + maxDist][currCellPlayer.z - currCell.z + maxDist] > -1) {
                //Replace old path with new path:
                path.clear();
                glm::ivec2 cur = glm::ivec2(currCellPlayer.x - currCell.x + maxDist, currCellPlayer.z - currCell.z + maxDist);
                if (prev[cur.x][cur.y] != glm::ivec2(-1, -1)) {
                    cur = prev[cur.x][cur.y];
                }
                while (prev[cur.x][cur.y] != glm::ivec2(-1, -1)) {
                    glm::ivec2 diff = cur - prev[cur.x][cur.y];
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
                    cur = prev[cur.x][cur.y];
                }
            }
        }
    }

    //Bear's Acceleration
    float scalarMultiplier = 2.f;
    m_acceleration = glm::vec3(0, 0, 0);

    //If bear is in player tracking mode, and there exists a valid path to the player:
    if (pathFinder && !path.empty()) {
        MobDirection cur = path.front();
        glm::ivec3 currCell = glm::ivec3(glm::floor(m_position));

        switch(cur) {
        case FORWARDS:
            m_acceleration.z = 1.f;
            if (currCell.z - prevPos.z >= 0.9f) {
                prevPos = currCell;
                path.pop_front();
            }
            break;
        case BACKWARDS:
            m_acceleration.z = -1.f;
            if (currCell.z - prevPos.z <= -0.9f) {
                prevPos = currCell;
                path.pop_front();
            }
            break;
        case LEFT:
            m_acceleration.x = 1.f;
            if (currCell.x - prevPos.x >= 0.9f) {
                prevPos = currCell;
                path.pop_front();
            }
            break;
        case RIGHT:
            m_acceleration.x = -1.f;
            if (currCell.x - prevPos.x <= -0.9f) {
                prevPos = currCell;
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

void Bear::computePhysics(float dT, const Terrain &terrain) {
    // TODO: Update the Bear's position based on its acceleration
    // and velocity, and also perform collision detection.

    checkLegsInLiquid(terrain);

    //Friction:
    float friction;

    friction = 0.90f;
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
//                Allow to jump over block of height unit 1
            if (terrain.getBlockAt(closestOut_BlockHitX.x, closestOut_BlockHitX.y + 1, closestOut_BlockHitX.z) == BlockType::EMPTY
                    && jumpReady && glm::abs(m_velocity.x) > autoJumpThreshold) {
                if (prevAutoJumpBlock == glm::ivec3(closestOut_BlockHitZ.x, closestOut_BlockHitZ.y + 1, closestOut_BlockHitZ.z)) {
                    //compare center of block to bear's position, then determine whether we need to move left or right
                    //clearly, we are jumping in the +z direction in this case, therefore, we will adjust either +- x
//                    float diff = closestOut_BlockHitZ.x + 0.5 - m_position.x;
                    std::cout << "Yo! x repeat\n";
                    if (closestOut_BlockHitZ.z + 0.5 - m_position.z > 0) {
                        m_velocity.z = 2.f;
                    } else {
                        m_velocity.z = -2.f;
                    }
                }
                prevAutoJumpBlock = glm::ivec3(closestOut_BlockHitZ.x, closestOut_BlockHitZ.y + 1, closestOut_BlockHitZ.z);
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
                    std::cout << "Yo! z repeat\n";
                    //compare center of block to bear's position, then determine whether we need to move left or right
                    //clearly, we are jumping in the +z direction in this case, therefore, we will adjust either +- x
//                    float diff = closestOut_BlockHitZ.x + 0.5 - m_position.x;
                    if (closestOut_BlockHitZ.x + 0.5 - m_position.x > 0) {
                        m_velocity.x = 2.f;
                    } else {
                        m_velocity.x = -2.f;
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
