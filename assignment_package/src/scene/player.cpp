#include "player.h"
#include <QString>
#include <iostream>

const float thirdPersonDist = 5.f;

Player::Player(glm::vec3 pos, const Terrain &terrain, OpenGLContext* context)
    : Entity(pos), m_velocity(0,0,0), m_acceleration(0,0,0),
      m_camera(pos + glm::vec3(0, 1.5f, 0.f)), m_cameraThirdPerson(pos + glm::vec3(0.f, thirdPersonDist, thirdPersonDist)),
      mcr_terrain(terrain),
      jumpReady(false), thirdPerson(false),
      angleX(0.f), angleY(0.f),
      legsInLiquid(false), legsLiquid(EMPTY),
      cameraInLiquid(false), cameraLiquid(EMPTY),
      mcr_camera(&m_camera),
      playerTranslate(nullptr), playerRotate(nullptr), torsoScale(nullptr),
      leftArmRotate(nullptr), leftArmScale(nullptr),
      rightArmRotate(nullptr), rightArmRotate2(nullptr), rightArmRotate0(nullptr), rightArmScale(nullptr),
      leftLegRotate(nullptr), leftLegScale(nullptr),
      rightLegRotate(nullptr), rightLegScale(nullptr),
      headRotate(nullptr), headScale(nullptr),
      m_geomCube(context),
      defaultArmAngle(90), running(false),
      animationTimer(0.f),
      armAnimation(false)
{}

Player::~Player()
{}

void Player::playerAnimation(float dT) {
    //Animations
    animationTimer += (dT / 1000.f);
    if (animationTimer > M_PI * 100.f) {
        animationTimer = 0.f;
    }
    float angle = 60.f;
    //Arm animation:
    if (thirdPerson) {
        if (running) {
            if (leftArmRotate != nullptr) {
                (static_cast<RotateNode*>(leftArmRotate))->deg = angle * glm::sin(animationTimer * 10.f + M_PI);
            }
            if (rightArmRotate != nullptr && !armAnimation) {
                (static_cast<RotateNode*>(rightArmRotate))->deg = angle * glm::sin(animationTimer * 10.f);
            }
            if (leftLegRotate != nullptr) {
                (static_cast<RotateNode*>(leftLegRotate))->deg = angle * glm::sin(animationTimer * 10.f);
            }
            if (rightLegRotate != nullptr) {
                (static_cast<RotateNode*>(rightLegRotate))->deg = angle * glm::sin(animationTimer * 10.f + M_PI);
            }
        } else {
//            std::cout << "Not running\n";
            if (leftArmRotate != nullptr && glm::abs((static_cast<RotateNode*>(leftArmRotate))->deg) >= 5.0f) {
                (static_cast<RotateNode*>(leftArmRotate))->deg = angle * glm::sin(animationTimer * 10.f + M_PI);
            } else {
                (static_cast<RotateNode*>(leftArmRotate))->deg = 0.f;
            }
            ////////////////////2:43pm Jul 7
            if (rightArmRotate != nullptr && glm::abs((static_cast<RotateNode*>(rightArmRotate))->deg) >= 5.0f) {
                (static_cast<RotateNode*>(rightArmRotate))->deg = angle * glm::sin(animationTimer * 10.f);
            } else {
                (static_cast<RotateNode*>(rightArmRotate))->deg = 0.f;
            }
            ////////////////////2:43pm Jul 7
            if (leftLegRotate != nullptr && glm::abs((static_cast<RotateNode*>(leftLegRotate))->deg) >= 5.0f) {
                (static_cast<RotateNode*>(leftLegRotate))->deg = angle * glm::sin(animationTimer * 10.f);
            } else {
                (static_cast<RotateNode*>(leftLegRotate))->deg = 0.f;
            }
            if (rightLegRotate != nullptr && glm::abs((static_cast<RotateNode*>(rightLegRotate))->deg) >= 5.0f) {
                (static_cast<RotateNode*>(rightLegRotate))->deg = angle * glm::sin(animationTimer * 10.f + M_PI);
            } else {
                (static_cast<RotateNode*>(rightLegRotate))->deg = 0.f;
            }
        }
    }
}

void Player::tick(float dT, InputBundle &input) {
    processInputs(input);
    if (!input.flightMode) {
        playerAnimation(dT);
    }
    computePhysics(dT, mcr_terrain, input.flightMode);
}

void Player::defaultPlayerRotation() {
    if (leftArmRotate != nullptr && rightArmRotate != nullptr && leftLegRotate != nullptr && rightLegRotate != nullptr) {
        (static_cast<RotateNode*>(leftArmRotate))->deg = 0.f;

        (static_cast<RotateNode*>(rightArmRotate))->deg = 0.f;

        (static_cast<RotateNode*>(leftLegRotate))->deg = 0.f;

        (static_cast<RotateNode*>(rightLegRotate))->deg = 0.f;
    }
}

void Player::processInputs(InputBundle &inputs) {
    // TODO: Update the Player's velocity and acceleration based on the
    // state of the inputs.
    //Acceleration
    thirdPerson = inputs.thirdPerson;

    float scalarMultiplier = 25.f;
    m_acceleration = glm::vec3(0, 0, 0);

    if (inputs.flightMode) {

        if (inputs.wPressed) {
            m_acceleration += m_forward;
        }
        if (inputs.aPressed) {
            m_acceleration -= m_right;
        }
        if (inputs.sPressed) {
            m_acceleration -= m_forward;
        }
        if (inputs.dPressed) {
            m_acceleration += m_right;
        }
        if (inputs.ePressed) {
            m_acceleration += glm::vec3(0, 1.f, 0);
        }
        if (inputs.qPressed) {
            m_acceleration -= glm::vec3(0, 1.f, 0);
        }
        m_acceleration *= scalarMultiplier;

    } else {
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

    float drag = 0.88f;
    float sensitivity = 0.15f;

    angleX += inputs.mouseX;
    angleY += inputs.mouseY;

    float finalAngleX = angleX * sensitivity;
    float finalAngleY = angleY * sensitivity;



    rotateOnUpGlobal(finalAngleX);
    float curAngle = (glm::angle(glm::vec3(0, 1, 0), m_forward)) * (180.f / M_PI);

    float minAngle = 0.f;
    float maxAngle = 180.f;
    if (thirdPerson) {
        minAngle = 30.f;
    }
    if (curAngle - finalAngleY > maxAngle) {
            rotateOnRightLocal(glm::max(maxAngle - curAngle, 0.f));
    } else if (curAngle - finalAngleY < minAngle) {
            rotateOnRightLocal(glm::min(minAngle - curAngle, 0.f));
    } else {
            rotateOnRightLocal(finalAngleY);
    }

    angleX *= drag;
    angleY *= drag;

}

void Player::computePhysics(float dT, const Terrain &terrain, bool flightmode) {
    //Movement Physics:
    // always reset movement speed and gravity
    // TODO: Update the Player's position based on its acceleration
    // and velocity, and also perform collision detection.
    float epsilon = 0.005f;

    float friction;

    if (flightmode) {
        friction = 0.98f;
        m_velocity.x *= friction;
        m_velocity.z *= friction;
        m_velocity.y *= 0.95;
    } else {
        friction = 0.85f;
        m_velocity.x *= friction;
        m_velocity.z *= friction;
    }

    m_velocity += m_acceleration * (dT / 1000.f);
    glm::vec3 rayDir = m_velocity * (dT / 1000.f);

    legsInLiquid = false;
    cameraInLiquid = false;

    // check if legs and camera are in liquid
    glm::ivec3 currCellLegs = glm::ivec3(glm::floor(m_position));glm::ivec3 currCellCamera = glm::ivec3(glm::floor(m_position + glm::vec3(0.f, 1.5f, 0.f)));
    BlockType cellTypeLegs = terrain.getBlockAt(currCellLegs.x, currCellLegs.y, currCellLegs.z);
    BlockType cellTypeCamera = terrain.getBlockAt(currCellCamera.x, currCellCamera.y, currCellCamera.z);
    if (cellTypeLegs == BlockType::LAVA || cellTypeLegs == BlockType::WATER) {
        legsInLiquid = true;
    }

    if (cellTypeCamera == BlockType::LAVA || cellTypeCamera == BlockType::WATER) {
        cameraInLiquid = true;
        cameraLiquid = cellTypeCamera;
    }

    if (!flightmode) {
        std::vector<glm::vec3> rayOrigins;

        float w = 0.25f;
        float h = 0.99f;
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

//        float autoJumpThreshold = 4.0f;
        float autoJumpThreshold = 2.0f;
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
                    m_velocity.y = 6.0f;
                    jumpReady = false;
                }
            } else {
                rayDir.z = (minOutDistZ * 0.8) * glm::sign(rayDir.z);
            }
        }
    }

    if (legsInLiquid) {
        // slow down movement by a lot more than 2/3 as it is not noticeable
        rayDir *= 0.3;
    }

    running = (glm::abs(rayDir.x / (dT / 1000.f)) >= 2.f) || (glm::abs(rayDir.z / (dT / 1000.f)) >= 2.f);
    moveAlongVector(rayDir);
}

void Player::setCameraWidthHeight(unsigned int w, unsigned int h) {
    m_camera.setWidthHeight(w, h);
    m_cameraThirdPerson.setWidthHeight(w, h);
}

void Player::moveAlongVector(glm::vec3 dir) {
    Entity::moveAlongVector(dir);
    m_camera.moveAlongVector(dir);
    m_cameraThirdPerson.moveAlongVector(dir);

    //Update translate
    if (playerTranslate != nullptr) {
        (static_cast<TranslateNode*>(playerTranslate.get()))->trans = glm::vec3(m_position.x, m_position.y + 1.05f, m_position.z);
    }
}

void Player::moveForwardLocal(float amount) {
    Entity::moveForwardLocal(amount);
    m_camera.moveForwardLocal(amount);
    m_cameraThirdPerson.moveForwardLocal(amount);
}
void Player::moveRightLocal(float amount) {
    Entity::moveRightLocal(amount);
    m_camera.moveRightLocal(amount);
    m_cameraThirdPerson.moveRightLocal(amount);
}
void Player::moveUpLocal(float amount) {
    Entity::moveUpLocal(amount);
    m_camera.moveUpLocal(amount);
    m_cameraThirdPerson.moveUpLocal(amount);
}
void Player::moveForwardGlobal(float amount) {
    Entity::moveForwardGlobal(amount);
    m_camera.moveForwardGlobal(amount);
    m_cameraThirdPerson.moveForwardGlobal(amount);
}
void Player::moveRightGlobal(float amount) {
    Entity::moveRightGlobal(amount);
    m_camera.moveRightGlobal(amount);
    m_cameraThirdPerson.moveRightGlobal(amount);
}
void Player::moveUpGlobal(float amount) {
    Entity::moveUpGlobal(amount);
    m_camera.moveUpGlobal(amount);
    m_cameraThirdPerson.moveUpGlobal(amount);
}

void Player::rotateOnRightLocalThirdPerson() {
    m_cameraThirdPerson.m_forward = glm::normalize(m_camera.mcr_position - m_cameraThirdPerson.mcr_position);
    m_cameraThirdPerson.m_right = m_camera.m_right;
    m_cameraThirdPerson.m_up = - glm::cross(m_cameraThirdPerson.m_forward, m_cameraThirdPerson.m_right);
}

//Rotate Functions:
void Player::rotateOnForwardLocal(float degrees) {
    Entity::rotateOnForwardLocal(degrees);
    m_camera.rotateOnForwardLocal(degrees);

    m_cameraThirdPerson.mcr_position = m_camera.mcr_position + glm::normalize(m_camera.m_forward) * -thirdPersonDist + glm::vec3(0.f, 1.f, 0.f) * thirdPersonDist;
    rotateOnRightLocalThirdPerson();
}
void Player::rotateOnRightLocal(float degrees) {
    Entity::rotateOnRightLocal(degrees);
    m_camera.rotateOnRightLocal(degrees);

    m_cameraThirdPerson.mcr_position = m_camera.mcr_position + glm::normalize(m_camera.m_forward) * -thirdPersonDist + glm::vec3(0.f, 1.f, 0.f) * thirdPersonDist;
    rotateOnRightLocalThirdPerson();

    //Right Arm Rotation
    if  (rightArmRotate != nullptr && rightArmRotate2 != nullptr) {
        defaultArmAngle += degrees;
        //Rotate the axis of rotation to keep hit animation the same at any angle
        (static_cast<RotateNode*>(rightArmRotate2))->axisOfRotation = glm::vec3(0.f,
                                                                                glm::sin(glm::radians(defaultArmAngle)),
                                                                                -glm::cos(glm::radians(defaultArmAngle)));
        if (armAnimation) {
            //Check if armAnimation complete:
            if ((static_cast<RotateNode*>(rightArmRotate2))->deg >= 40.f) {
                armAnimation = false;
                (static_cast<RotateNode*>(rightArmRotate2))->deg = 0.5f * ((static_cast<RotateNode*>(rightArmRotate2))->deg);
                (static_cast<RotateNode*>(rightArmRotate))->deg = 0.5f * (defaultArmAngle + (static_cast<RotateNode*>(rightArmRotate))->deg);
            } else {
                (static_cast<RotateNode*>(rightArmRotate2))->deg += 3.5f;
                (static_cast<RotateNode*>(rightArmRotate))->deg -= 0.5f;
            }
        } else {
            //If no hit animation:
            (static_cast<RotateNode*>(rightArmRotate2))->deg = 0.f;
            if (!thirdPerson) {
                (static_cast<RotateNode*>(rightArmRotate))->deg = defaultArmAngle;
            }
        }
    }
}
void Player::rotateOnUpLocal(float degrees) {
    Entity::rotateOnUpLocal(degrees);
    m_camera.rotateOnUpLocal(degrees);

    m_cameraThirdPerson.mcr_position = m_camera.mcr_position + glm::normalize(m_camera.m_forward) * -thirdPersonDist + glm::vec3(0.f, 1.f, 0.f) * thirdPersonDist;
    rotateOnRightLocalThirdPerson();
}
void Player::rotateOnForwardGlobal(float degrees) {
    Entity::rotateOnForwardGlobal(degrees);
    m_camera.rotateOnForwardGlobal(degrees);

    m_cameraThirdPerson.mcr_position = m_camera.mcr_position + glm::normalize(m_camera.m_forward) * -thirdPersonDist + glm::vec3(0.f, 1.f, 0.f) * thirdPersonDist;
    rotateOnRightLocalThirdPerson();
}
void Player::rotateOnRightGlobal(float degrees) {
    Entity::rotateOnRightGlobal(degrees);
    m_camera.rotateOnRightGlobal(degrees);

    m_cameraThirdPerson.mcr_position = m_camera.mcr_position + glm::normalize(m_camera.m_forward) * -thirdPersonDist + glm::vec3(0.f, 1.f, 0.f) * thirdPersonDist;

}
void Player::rotateOnUpGlobal(float degrees) {
    Entity::rotateOnUpGlobal(degrees);
    m_camera.rotateOnUpGlobal(degrees);

    m_cameraThirdPerson.mcr_position = m_camera.mcr_position + glm::normalize(m_camera.m_forward) * -thirdPersonDist + glm::vec3(0.f, 1.f, 0.f) * thirdPersonDist;

    rotateOnRightLocalThirdPerson();

    //Rotate our playermodel
    if (playerRotate != nullptr) {
        (static_cast<RotateNode*>(playerRotate))->deg += degrees;
    }
}

QString Player::posAsQString() const {
    std::string str("( " + std::to_string(m_position.x) + ", " + std::to_string(m_position.y) + ", " + std::to_string(m_position.z) + ")");
    return QString::fromStdString(str);
}
QString Player::velAsQString() const {
    std::string str("( " + std::to_string(m_velocity.x) + ", " + std::to_string(m_velocity.y) + ", " + std::to_string(m_velocity.z) + ")");
    return QString::fromStdString(str);
}
QString Player::accAsQString() const {
    std::string str("( " + std::to_string(m_acceleration.x) + ", " + std::to_string(m_acceleration.y) + ", " + std::to_string(m_acceleration.z) + ")");
    return QString::fromStdString(str);
}
QString Player::lookAsQString() const {
    std::string str("( " + std::to_string(m_forward.x) + ", " + std::to_string(m_forward.y) + ", " + std::to_string(m_forward.z) + ")");
    return QString::fromStdString(str);
}

std::pair<bool, BlockType> Player::gridMarch(glm::vec3 rayOrigin, glm::vec3 rayDirection, const Terrain &terrain, float *out_dist, glm::ivec3 *out_blockHit) {

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


                // If the player is *exactly* on an interface then
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


glm::vec3 Player::getLocalForward() {
    if (thirdPerson) {
        return m_cameraThirdPerson.m_forward;
    }
    return m_forward;
}

glm::vec3 Player::getLocalRight() {
    if (thirdPerson) {
        return m_cameraThirdPerson.m_right;
    }
    return m_right;
}

glm::vec3 Player::getLocalUp() {
    if (thirdPerson) {
        return m_cameraThirdPerson.m_up;
    }
    return m_up;
}

void Player::constructSceneGraph() {
    ////// 1. Create Nodes

    //Torso
    uPtr<Node> torsoT = mkU<TranslateNode> (glm::vec3(m_position.x, m_position.y + 1.05f, m_position.z));
    uPtr<Node> torsoR = mkU<RotateNode> ();
    uPtr<Node> torsoS = mkU<ScaleNode> (0.5f, 0.75f, 0.25f);
    //Add geom to torsoS
    torsoS->mCuboid = (&m_geomCube);
    torsoS->type = TORSO;
    //Legs
    //Left Leg
    uPtr<Node> leftLegT = mkU<TranslateNode> (glm::vec3(-0.125f, -0.25f, 0.f));
    uPtr<Node> leftLegR = mkU<RotateNode> ();
    uPtr<Node> leftLegT2 = mkU<TranslateNode> (glm::vec3(0.f, -0.5f, 0.f));
    uPtr<Node> leftLegS = mkU<ScaleNode> (0.25f, 0.75f, 0.25f);
    leftLegS->mCuboid = (&m_geomCube);
    leftLegS->type = LEFTLEG;
    //Right Leg
    uPtr<Node> rightLegT = mkU<TranslateNode> (glm::vec3(0.125f, -0.25f, 0.f));
    uPtr<Node> rightLegR = mkU<RotateNode> ();
    uPtr<Node> rightLegT2 = mkU<TranslateNode> (glm::vec3(0.f, -0.5f, 0.f));
    uPtr<Node> rightLegS = mkU<ScaleNode> (0.25f, 0.75f, 0.25f);
    rightLegS->mCuboid = (&m_geomCube);
    rightLegS->type = RIGHTLEG;
    //Arms
    //Left Arm
    uPtr<Node> leftArmT = mkU<TranslateNode> (glm::vec3(-0.375f, 0.25f, 0.f));
    uPtr<Node> leftArmR = mkU<RotateNode> ();
    uPtr<Node> leftArmT2 = mkU<TranslateNode> (glm::vec3(0.f, -0.25f, 0.f));
    uPtr<Node> leftArmS = mkU<ScaleNode> (0.25f, 0.75f, 0.25f);
    leftArmS->mCuboid = (&m_geomCube);
    leftArmS->type = LEFTARM;
    //Right Arm
    uPtr<Node> rightArmR0 = mkU<RotateNode> ();
    uPtr<Node> rightArmT = mkU<TranslateNode> (glm::vec3(0.375f, 0.25f, 0.f));
    uPtr<Node> rightArmR2 = mkU<RotateNode> (0.f, glm::vec3(0.f, 1.f, 0.f)); //the axis for this should be local!!!!
    uPtr<Node> rightArmR = mkU<RotateNode> (90.f, glm::vec3(1.f, 0.f, 0.f)); //the axis for this should also be local!!!!
    uPtr<Node> rightArmT2 = mkU<TranslateNode> (glm::vec3(0.f, -0.25f, 0.f));
    uPtr<Node> rightArmS = mkU<ScaleNode> (0.25f, 0.75f, 0.25f);
    rightArmS->mCuboid = (&m_geomCube);
    rightArmS->type = RIGHTARM;

    //Head
    uPtr<Node> headT = mkU<TranslateNode> (glm::vec3(0.f, 0.625f, 0.f));
    uPtr<Node> headR = mkU<RotateNode> ();
    uPtr<Node> headS = mkU<ScaleNode> (0.5f, 0.5f, 0.5f);
    headS->mCuboid = (&m_geomCube);

    ////// 1.5 Assign pointers

    //playerRotate
    playerRotate = torsoR.get();
    (static_cast<RotateNode*>(playerRotate))->axisOfRotation = glm::vec3(0.f, 1.f, 0.f);

    //torsoScale
    torsoScale = torsoS.get();
    torsoScale->visible = false;

    //leftArmRotate
    leftArmRotate = leftArmR.get();
    //leftArmScale
    leftArmScale = leftArmS.get();
    leftArmScale->visible = false;

    //rightArmRotate
    rightArmRotate = rightArmR.get();
    //rightArmRotate2
    rightArmRotate2 = rightArmR2.get();
    //rightArmScale
    rightArmScale = rightArmS.get();
//    rightArmScale->visible = false;

    //leftLegRotate
    leftLegRotate = leftLegR.get();
    //leftLegScale
    leftLegScale = leftLegS.get();
    leftLegScale->visible = false;

    //rightLegRotate
    rightLegRotate = rightLegR.get();
    //rightLegScale
    rightLegScale = rightLegS.get();
    rightLegScale->visible = false;

    //headRotate
    headRotate = headR.get();

    //headScale
    headScale = headS.get();
    headScale->visible = false;

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

    ////// 3. Assign Main to Field
    playerTranslate = std::move(torsoT);
}

void Player::toggleThirdPerson(bool b) {
    toggleNodeVisibility();
    if (b) {
        //We are in third person
        mcr_camera = &m_cameraThirdPerson;
    } else {
        //We are in first person
        if (rightArmRotate != nullptr) {
            (static_cast<RotateNode*>(rightArmRotate))->deg = defaultArmAngle;
        }
        mcr_camera = &m_camera;
    }
}

void Player::toggleNodeVisibility() {
    std::vector<Node*> geomNodes = {headScale, torsoScale, leftArmScale, leftLegScale, rightLegScale};
    for (Node* n : geomNodes) {
        n->visible = !(n->visible);
    }
}
