#pragma once
#include "entity.h"
#include "player.h"
#include "terrain.h"
#include <glm/gtx/vector_angle.hpp>
#include <utility>
#include "translatenode.h"
#include "rotatenode.h"
#include "scalenode.h"
#include <queue>
#include <deque>

class Bird : public Entity {
public:
    OpenGLContext* contextSave;
    const Player &m_player;
    glm::vec3 m_velocity, m_acceleration;
    glm::vec3 playerDir;
    const Terrain &mcr_terrain;
    bool jumpReady;
    float angleX, angleY;
    float epsilon;

    bool legsInLiquid;
    BlockType legsLiquid;

    void processInputs(InputBundle &inputs);
    void processInputsNPC();
    void computePhysics(float dT, const Terrain &terrain);

    //Bird Model Fields
    uPtr<Node> birdTranslate;
    Node* birdRotate;
    Node* torsoScale;

    Node* leftArmRotate;
    Node* leftArmScale;

    Node* rightArmRotate;
    Node* rightArmRotate2;
    Node* rightArmRotate0;
    Node* rightArmScale;

    Node* leftLegRotate;
    Node* leftLegScale;

    Node* rightLegRotate;
    Node* rightLegScale;

    Node* headRotate;
    Node* headScale;

    MobCuboid m_geomCube;
    MobCuboid m_geomCube2;
    float defaultArmAngle;
    bool running;
    float animationTimer;
    bool armAnimation;
    bool pathFinder;
    std::deque<MobDirection> path;
    glm::ivec3 prevPos;

    Bird(glm::vec3 pos, const Terrain &terrain, OpenGLContext* context, const Player &p);
    virtual ~Bird() override;

    void tick(float dT, InputBundle &input) override;
    void tickNPC(float dT);

    std::pair<bool, BlockType> gridMarch(glm::vec3 rayOrigin, glm::vec3 rayDirection, const Terrain &terrain, float *out_dist, glm::ivec3 *out_blockHit);

    void removeBlockPointedTo();

    // Player overrides all of Entity's movement
    // functions so that it transforms its camera
    // by the same amount as it transforms itself.

    void moveAlongVector(glm::vec3 dir) override;
    void moveForwardLocal(float amount) override;
    void moveRightLocal(float amount) override;
    void moveUpLocal(float amount) override;
    void moveForwardGlobal(float amount) override;
    void moveRightGlobal(float amount) override;
    void moveUpGlobal(float amount) override;
    void rotateOnForwardLocal(float degrees) override;
    void rotateOnRightLocal(float degrees) override;
    void rotateOnUpLocal(float degrees) override;
    void rotateOnForwardGlobal(float degrees) override;
    void rotateOnRightGlobal(float degrees) override;
    void rotateOnUpGlobal(float degrees) override;

    // For sending the Bird's data to the GUI
    // for display
    QString posAsQString() const;
    QString velAsQString() const;
    QString accAsQString() const;
    QString lookAsQString() const;

    //
    void constructSceneGraph();
    void toggleNodeVisibility();
    void defaultBirdRotation();
    void checkLegsInLiquid(const Terrain &terrain);
    void physicsCollisions(glm::vec3 &rayD, const Terrain &terrain);
    void runAnimation(float dT);
    void activatePathFinder();
};
