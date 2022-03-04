#pragma once
#include "entity.h"
#include "camera.h"
#include "terrain.h"
#include <glm/gtx/vector_angle.hpp>
#include <utility>
#include "translatenode.h"
#include "rotatenode.h"
#include "scalenode.h"

class Player : public Entity {
public:
    glm::vec3 m_velocity, m_acceleration;
    Camera m_camera;
    Camera m_cameraThirdPerson;
    const Terrain &mcr_terrain;
    bool jumpReady;
    bool thirdPerson;
    float angleX, angleY;

    bool legsInLiquid;
    BlockType legsLiquid;


    void processInputs(InputBundle &inputs);
    void computePhysics(float dT, const Terrain &terrain, bool flightMode);


    bool cameraInLiquid;
    BlockType cameraLiquid;
    // Readonly public pointer to our camera
    // for easy access from MyGL
    Camera* mcr_camera;

    //Player Model Fields
    uPtr<Node> playerTranslate;
    Node* playerRotate;
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
    float defaultArmAngle;
    bool running;
    float animationTimer;
    bool armAnimation;

    Player(glm::vec3 pos, const Terrain &terrain, OpenGLContext* context);
    virtual ~Player() override;

    void setCameraWidthHeight(unsigned int w, unsigned int h);

    void tick(float dT, InputBundle &input) override;

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
    void rotateOnRightLocalThirdPerson();

    glm::vec3 getLocalForward();
    glm::vec3 getLocalRight();
    glm::vec3 getLocalUp();
    // For sending the Player's data to the GUI
    // for display
    QString posAsQString() const;
    QString velAsQString() const;
    QString accAsQString() const;
    QString lookAsQString() const;

    //
    void constructSceneGraph();
    void toggleThirdPerson(bool b);
    void toggleNodeVisibility();
    void playerAnimation(float dT);
    void defaultPlayerRotation();
};
