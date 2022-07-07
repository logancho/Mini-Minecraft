#include "mygl.h"
#include <glm_includes.h>

#include <iostream>
#include <QApplication>
#include <QKeyEvent>
#include <QDateTime>
#include <ApplicationServices/ApplicationServices.h>

MyGL::MyGL(QWidget *parent)
    : OpenGLContext(parent),
      m_worldAxes(this),
      m_progLambert(this), m_progFlat(this), m_progInstanced(this), m_progLava(this), m_progWater(this), m_progNothing(this),
      m_frameBuffer(this, this->width(), this->height(), this->devicePixelRatio()),
      terrainTexture(), playerTexture(), bearTexture(),
      m_geomQuad(this),
      m_terrain(this), m_player(glm::vec3(5.f, 160.f, 15.f), m_terrain, this),
      inputB(),
      currentMSecs(QDateTime::currentMSecsSinceEpoch()),
      curMouseX(0.f),
      curMouseY(0.f),
      m_crosshair(this),
      m_inventory(this),
      m_blockSelection(this),
      m_placing(1),
      m_time(0),
      m_prevPos(glm::vec2()),
      m_bear(glm::vec3(2.f, 150.f, 13.f), m_terrain, this, m_player),
      m_bird(glm::vec3(2.f, 150.f, 12.f), m_terrain, this, m_player)
{
    // Connect the timer to a function so that when the timer ticks the function is executed
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(tick()));
    // Tell the timer to redraw 60 times per second
    m_timer.start(16);
    setFocusPolicy(Qt::ClickFocus);

    setMouseTracking(true); // MyGL will track the mouse's movements even if a mouse button is not pressed
    setCursor(Qt::BlankCursor); // Make the cursor invisible
}

MyGL::~MyGL() {
    makeCurrent();
    glDeleteVertexArrays(1, &vao);
    m_geomQuad.destroyVBOdata();
//    m_worldAxes.destroyVBOdata();
//    m_player.m_geomCube.destroyVBOdata();
//    m_bear.m_geomCube.destroyVBOdata();
//    m_bird.m_geomCube.destroyVBOdata();
//    m_frameBuffer.destroy();
//    m_crosshair.destroyVBOdata();
//    m_inventory.destroyVBOdata();
//    m_blockSelection.destroyVBOdata();
}


void MyGL::moveMouseToCenter() {
    QCursor::setPos(this->mapToGlobal(QPoint(width() / 2, height() / 2)));
}

void MyGL::initializeGL()
{
    if(AXIsProcessTrusted()) {
        std::cout << "trusted accessibility client, will be able to move the mouse" << std::endl;
    } else {
        std::cout << "not a trusted trusted accessibility client, wont be able to move the mouse" << std::endl;
    }

    // Create an OpenGL context using Qt's QOpenGLFunctions_3_2_Core class
    // If you were programming in a non-Qt context you might use GLEW (GL Extension Wrangler)instead
    initializeOpenGLFunctions();
    // Print out some information about the current OpenGL context
    debugContextVersion();

    // Set a few settings/modes in OpenGL rendering
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    //Textures
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // Set the color with which the screen is filled at the start of each render call.
    glClearColor(0.37f, 0.74f, 1.0f, 1);

    m_frameBuffer.create();

    printGLErrorLog();

    // Create a Vertex Attribute Object
    glGenVertexArrays(1, &vao);

    //Create the instance of the world axes
    m_worldAxes.destroyVBOdata();
    m_worldAxes.createVBOdata();

    //Player Model:
    std::cout << "Calling in MyGL create for m_geomCube of player\n";
    m_player.m_geomCube.destroyVBOdata();
    m_player.m_geomCube.createVBOdata();
    m_player.constructSceneGraph();

    //Bear Model:
    m_bear.m_geomCube.destroyVBOdata();
    m_bear.m_geomCube.createVBOdata();
    m_bear.constructSceneGraph();

    //Bird model:
    m_bird.m_geomCube.destroyVBOdata();
    m_bird.m_geomCube.createVBOdata();
    m_bird.constructSceneGraph();

    // Create and set up the diffuse shader
    m_progLambert.create(":/glsl/lambert.vert.glsl", ":/glsl/lambert.frag.glsl");
    // Create and set up the flat lighting shader
    m_progFlat.create(":/glsl/flat.vert.glsl", ":/glsl/flat.frag.glsl");

    //Here, instead of setTexture, I will call create on my texture object, terrainTexture
    terrainTexture = std::make_shared<Texture>(this);
    terrainTexture->create(":/textures/mctextureUnlabelled.png");
    terrainTexture->load(0);

    playerTexture = std::make_shared<Texture>(this);
    playerTexture->create(":/textures/SteveSkin.png");
    playerTexture->load(2);

    bearTexture = std::make_shared<Texture>(this);
    bearTexture->create(":/textures/Bear3.png");
    bearTexture->load(3);

    birdTexture = std::make_shared<Texture>(this);
    birdTexture->create(":/textures/bird2.png");
    birdTexture->load(4);

    m_progLava.create(":/glsl/lava.vert.glsl", ":/glsl/lava.frag.glsl");
    m_progWater.create(":/glsl/water.vert.glsl", ":/glsl/water.frag.glsl");
    m_progNothing.create(":/glsl/nothing.vert.glsl", ":/glsl/nothing.frag.glsl");

    // We have to have a VAO bound in OpenGL 3.2 Core. But if we're not
    // using multiple VAOs, we can just bind one once.
    glBindVertexArray(vao);

    //Set crosshair to player
    m_crosshair.m_player = &m_player;
    m_inventory.m_player = &m_player;
    m_blockSelection.m_player = &m_player;
    //moveMouseToCenter();

    glm::vec2 pPos(m_player.mcr_position.x, m_player.mcr_position.z);
    glm::ivec2 zone(64 * glm::ivec2(glm::floor(pPos / 64.f)));

    m_terrain.create(zone);
    m_prevPos = glm::vec2(pPos);
    m_geomQuad.destroyVBOdata();
    m_geomQuad.createVBOdata(); // check if this line is in the right place

    //have terrain create our platform!

}

void MyGL::resizeGL(int w, int h) {
    //This code sets the concatenated view and perspective projection matrices used for
    //our scene's camera view.
    m_player.setCameraWidthHeight(static_cast<unsigned int>(w), static_cast<unsigned int>(h));
    glm::mat4 viewproj = m_player.mcr_camera->getViewProj();

    // Upload the view-projection matrix to our shaders (i.e. onto the graphics card)

    m_progLambert.setViewProjMatrix(viewproj);
    m_progFlat.setViewProjMatrix(viewproj);

    glm::vec2 pPos(m_player.mcr_position.x, m_player.mcr_position.z);

    m_prevPos = glm::vec2(pPos);

    // resize framebuffer
    m_frameBuffer.resize(w, h, this->devicePixelRatio());
    m_frameBuffer.destroy();
    m_frameBuffer.create();

    printGLErrorLog();
}

// MyGL's constructor links tick() to a timer that fires 60 times per second.
// We're treating MyGL as our game engine class, so we're going to perform
// all per-frame actions here, such as performing physics updates on all
// entities in the scene.

void MyGL::tick() {

    update(); // Calls paintGL() as part of a larger QOpenGLWidget pipeline
    sendPlayerDataToGUI(); // Updates the info in the secondary window displaying player data

    //Input Bundle:

    //Physics Update via:

    qint64 dT = (QDateTime::currentMSecsSinceEpoch() - currentMSecs);


    m_player.tick(dT, inputB);

    if (!inputB.flightMode) {
        m_bear.tickNPC(dT);
        m_bird.tickNPC(dT);
    }
    //Update fields:
    //time:
    currentMSecs = QDateTime::currentMSecsSinceEpoch();
}

void MyGL::sendPlayerDataToGUI() const {
    emit sig_sendPlayerPos(m_player.posAsQString());
    emit sig_sendPlayerVel(m_player.velAsQString());
    emit sig_sendPlayerAcc(m_player.accAsQString());
    emit sig_sendPlayerLook(m_player.lookAsQString());
    glm::vec2 pPos(m_player.mcr_position.x, m_player.mcr_position.z);
    glm::ivec2 chunk(16 * glm::ivec2(glm::floor(pPos / 16.f)));
    glm::ivec2 zone(64 * glm::ivec2(glm::floor(pPos / 64.f)));
    emit sig_sendPlayerChunk(QString::fromStdString("( " + std::to_string(chunk.x) + ", " + std::to_string(chunk.y) + " )"));
    emit sig_sendPlayerTerrainZone(QString::fromStdString("( " + std::to_string(zone.x) + ", " + std::to_string(zone.y) + " )"));
}

// This function is called whenever update() is called.
// MyGL's constructor links update() to a timer that fires 60 times per second,
// so paintGL() called at a rate of 60 frames per second.
void MyGL::paintGL() {
    //Time
    m_progLambert.setTime(m_time);
    m_time++;

    // Clear the screen so that we only see newly drawn images
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //Texture:

    //TERRAIN:
    terrainTexture->bind(0);
    m_progLambert.setUpSampler2D(0);

    m_progFlat.setViewProjMatrix(m_player.mcr_camera->getViewProj());
    m_progLambert.setViewProjMatrix(m_player.mcr_camera->getViewProj());
    //m_progInstanced.setViewProjMatrix(m_player.mcr_camera.getViewProj());

    m_progLambert.setModelMatrix(glm::mat4(1.f));
    m_progFlat.setModelMatrix(glm::mat4(1.f));

    // frame buffer
    m_frameBuffer.bindFrameBuffer();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    renderTerrain();

    //MOBS:
    playerTexture->bind(2);
    m_progLambert.setUpSampler2D(2);

    //Set up lambert and sampler2d for our player drawing
    //1. Bind texture to slot 2
            //    void FrameBuffer::bindToTextureSlot(unsigned int slot) {
            //        m_textureSlot = slot;
            //        mp_context->glActiveTexture(GL_TEXTURE0 + slot);
            //        mp_context->glBindTexture(GL_TEXTURE_2D, m_outputTexture);
            //    }
    //2. Set sampler2d to a different slot

    //DRAW MODELS BEFORE FRAME BUFFER UPDATE!

    //Draw Player
    if (m_player.playerTranslate) {
        m_progLambert.setModelMatrix(glm::mat4(1.f));
        traverse(m_player.playerTranslate, glm::mat4(1.f), 2);
    }

    //Draw Bear
    bearTexture->bind(3);
    m_progLambert.setUpSampler2D(3);
    if (m_bear.bearTranslate) {
        m_progLambert.setModelMatrix(glm::mat4(1.f));
        traverse(m_bear.bearTranslate, glm::mat4(1.f), 3);
    }

    //Draw Bird
    birdTexture->bind(4);
    m_progLambert.setUpSampler2D(4);
    if (m_bird.birdTranslate) {
        m_progLambert.setModelMatrix(glm::mat4(1.f));
        traverse(m_bird.birdTranslate, glm::mat4(1.f), 4);
    }

    // FRAME BUFFER: //
    glBindFramebuffer(GL_FRAMEBUFFER, this->defaultFramebufferObject());
    m_frameBuffer.bindToTextureSlot(1);
    m_progLava.setUpSampler2D(1);
    m_progWater.setUpSampler2D(1);
    m_progNothing.setUpSampler2D(1);

    if (m_player.cameraInLiquid) {
        if (m_player.cameraLiquid == LAVA) {
            m_progLava.draw(m_geomQuad, 1);
        } else if (m_player.cameraLiquid == WATER) {
            m_progWater.draw(m_geomQuad, 1);
        }
    } else {
       m_progNothing.draw(m_geomQuad, 1);
    }

    // draw world axes and crosshair ontop of post process shader
    m_progLambert.setUpSampler2D(0);
    m_progLambert.setModelMatrix(glm::mat4(1.f));

    glDisable(GL_DEPTH_TEST);

    m_progFlat.setModelMatrix(glm::mat4(1.f));
    m_progLambert.setModelMatrix(glm::mat4(1.f));
    //m_progFlat.draw(m_worldAxes);

    terrainTexture->bind(0);
    m_progLambert.setUpSampler2D(0);

    m_crosshair.destroyVBOdata();
    m_crosshair.createVBOdata();
    m_inventory.destroyVBOdata();
    m_inventory.createVBOdata();
    m_blockSelection.destroyVBOdata();
    m_blockSelection.createVBOdata();
    m_progFlat.draw(m_crosshair);
    m_progFlat.draw(m_inventory);
    m_progLambert.draw(m_blockSelection);
    glEnable(GL_DEPTH_TEST);
}

// TODO: Change this so it renders the nine zones of generated
// terrain that surround the player (refer to Terrain::m_generatedTerrain
// for more info)
void MyGL::renderTerrain() {
//    terrainReadyForNPCs = false;
    glm::vec2 pPos(m_player.mcr_position.x, m_player.mcr_position.z);


    glm::ivec2 zone(64 * glm::ivec2(glm::floor(pPos / 64.f)));

    m_terrain.expand(pPos, m_prevPos);
        //create a new function to use instead of expand!
    m_terrain.draw(zone, &m_progLambert);

//    terrainReadyForNPCs = true;
}

void MyGL::keyPressEvent(QKeyEvent *e) {
    float amount = 2.0;
    if(e->modifiers() & Qt::ShiftModifier){
        amount = 10.0f;
    }
    // http://doc.qt.io/qt-5/qt.html#Key-enum
    // This could all be much more efficient if a switch
    // statement were used, but I really dislike their
    // syntax so I chose to be lazy and use a long
    // chain of if statements instead

    if (e->key() == Qt::Key_Escape) {
        QApplication::quit();
    } else if (e->key() == Qt::Key_Right) {
        //Same as D
        inputB.dPressed = true;
    } else if (e->key() == Qt::Key_Left) {
        //Same as A
        inputB.aPressed = true;
    } else if (e->key() == Qt::Key_Up) {
        //Same as W
        inputB.wPressed = true;
    } else if (e->key() == Qt::Key_Down) {
        //Same as S
        inputB.sPressed = true;
    } else if (e->key() == Qt::Key_W) {
        inputB.wPressed = true;
    } else if (e->key() == Qt::Key_S) {
        inputB.sPressed = true;
    } else if (e->key() == Qt::Key_D) {
        inputB.dPressed = true;
    } else if (e->key() == Qt::Key_A) {
        inputB.aPressed = true;
    } else if (e->key() == Qt::Key_Q) {
        inputB.qPressed = true;
    } else if (e->key() == Qt::Key_E) {
        inputB.ePressed = true;
    } else if (e->key() == Qt::Key_F) {
        inputB.flightMode = !inputB.flightMode;
        if (inputB.thirdPerson) {
            m_player.defaultPlayerRotation();
        }
    } else if (e->key() == Qt::Key_Space) {
        inputB.spacePressed = true;
    } else if (e->key() == Qt::Key_T) {
        inputB.thirdPerson = !inputB.thirdPerson;
        m_player.toggleThirdPerson(inputB.thirdPerson);
    } else if (e->key() == Qt::Key_1) {
        m_placing = 1;
    } else if (e->key() == Qt::Key_2) {
        m_placing = 2;
    } else if (e->key() == Qt::Key_3) {
        m_placing = 3;
    } else if (e->key() == Qt::Key_4) {
        m_placing = 4;
    } else if (e->key() == Qt::Key_5) {
        m_placing = 5;
    } else if (e->key() == Qt::Key_6) {
        m_placing = 6;
    } else if (e->key() == Qt::Key_7) {
        m_placing = 7;
    } else if (e->key() == Qt::Key_8) {
        m_placing = 8;
    } else if (e->key() == Qt::Key_9) {
        m_placing = 9;
    } else if (e->key() == Qt::Key_0) {
        m_placing = 0;
    } else if (e->key() == Qt::Key_Y) {
        m_bear.activatePathFinder();
    } else if (e->key() == Qt::Key_U) {
        m_bird.activatePathFinder();
    }
}

void MyGL::keyReleaseEvent(QKeyEvent *e) {
    if (e->key() == Qt::Key_Right) {
        inputB.dPressed = false;
    } else if (e->key() == Qt::Key_Left) {
        inputB.aPressed = false;
    } else if (e->key() == Qt::Key_Up) {
        inputB.wPressed = false;
    } else if (e->key() == Qt::Key_Down) {
        inputB.sPressed = false;
    } else if (e->key() == Qt::Key_W) {
        inputB.wPressed = false;
    } else if (e->key() == Qt::Key_S) {
        inputB.sPressed = false;
    } else if (e->key() == Qt::Key_D) {
        inputB.dPressed = false;
    } else if (e->key() == Qt::Key_A) {
        inputB.aPressed = false;
    } else if (e->key() == Qt::Key_Q) {
        inputB.qPressed = false;
    } else if (e->key() == Qt::Key_E) {
        inputB.ePressed = false;
    } else if (e->key() == Qt::Key_Space) {
        inputB.spacePressed = false;
    }
}

void MyGL::mouseMoveEvent(QMouseEvent *e) {
//    inputB.mouseX = glm::asin((this->size().width() / 2 - e->position().x()) / this->size().width()) * M_PI * 180;
//    inputB.mouseY = glm::asin(((this->size().height() / 2 - e->position().y())) / this->size().height()) * M_PI * 180;
    inputB.mouseX = (this->size().width() / 2 - e->position().x()) / this->size().width() * M_PI * 180;
    inputB.mouseY = (this->size().height() / 2 - e->position().y()) / this->size().height() * M_PI * 180;
    moveMouseToCenter();
}

void MyGL::mousePressEvent(QMouseEvent *e) {
    glm::vec3 offset = glm::vec3(0.f);
    if (m_player.thirdPerson) {
        offset = glm::sqrt(50.f) * m_player.mcr_camera->m_forward;
    }
    if(e->button() == Qt::LeftButton) {
        m_player.armAnimation = true;
        float out_dist = 0.f;
        glm::ivec3 out_blockHitPos = glm::ivec3();
        if (m_player.gridMarch(m_player.mcr_camera->mcr_position + offset, m_player.getLocalForward() * 3.f, m_terrain, &out_dist, &out_blockHitPos).first
                && m_terrain.getBlockAt(out_blockHitPos.x, out_blockHitPos.y, out_blockHitPos.z) != BlockType::BEDROCK
                && m_terrain.getBlockAt(out_blockHitPos.x, out_blockHitPos.y, out_blockHitPos.z) != BlockType::LAVA
                && m_terrain.getBlockAt(out_blockHitPos.x, out_blockHitPos.y, out_blockHitPos.z) != BlockType::WATER) {
          
            if (out_dist <= 3) {
                m_terrain.setBlockAt(out_blockHitPos.x, out_blockHitPos.y, out_blockHitPos.z, BlockType::EMPTY);
                Chunk* c = m_terrain.getChunkAt(out_blockHitPos.x, out_blockHitPos.z).get();

                m_terrain.appendEditedChunk(c);

                std::vector<Chunk*> toAdd = c->checkFromNeighbor(out_blockHitPos);

                for (Chunk* chunk : toAdd) {
                    m_terrain.appendEditedChunk(chunk);
                }
            }
        }
    } else if (e->button() == Qt::RightButton) {
        m_player.armAnimation = true;
        addAdjacentBlock(offset);
    }
}

void MyGL::addAdjacentBlock(glm::vec3 offset) {
    float out_dist = 0.f;
    glm::ivec3 out_blockHitPos = glm::ivec3();
    if (m_player.gridMarch(m_player.mcr_camera->mcr_position + offset, m_player.getLocalForward() * 3.f, m_terrain, &out_dist, &out_blockHitPos).first && (out_dist > glm::sqrt(3))) {

        glm::vec3 rayDir = m_player.getLocalForward();
        float t_near = -std::numeric_limits<float>::infinity();
        float t_far = std::numeric_limits<float>::infinity();

        //Ray cube intersection - find t_near and t_far

        //X Slabs
        float x_max = out_blockHitPos.x + 1;
        float x_min = out_blockHitPos.x;
        if (rayDir.x != 0) {
            float t_0 = (x_min - m_player.mcr_camera->mcr_position.x) / rayDir.x;
            float t_1 = (x_max - m_player.mcr_camera->mcr_position.x) / rayDir.x;
            if (t_0 > t_1) {
                float temp = t_0;
                t_0 = t_1;
                t_1 = temp;
            }
            if (t_0 > t_near) {
                t_near = t_0;
            }
            if (t_1 < t_far) {
                t_far = t_1;
            }
        }
        //Y Slabs
        float y_max = out_blockHitPos.y + 1;
        float y_min = out_blockHitPos.y;
        if (rayDir.y != 0) {
            float t_0 = (y_min - m_player.mcr_camera->mcr_position.y) / rayDir.y;
            float t_1 = (y_max - m_player.mcr_camera->mcr_position.y) / rayDir.y;
            if (t_0 > t_1) {
                float temp = t_0;
                t_0 = t_1;
                t_1 = temp;
            }
            if (t_0 > t_near) {
                t_near = t_0;
            }
            if (t_1 < t_far) {
                t_far = t_1;
            }
        }
        //Z Slabs
        float z_max = out_blockHitPos.z + 1;
        float z_min = out_blockHitPos.z;
        if (rayDir.z != 0) {
            float t_0 = (z_min - m_player.mcr_camera->mcr_position.z) / rayDir.z;
            float t_1 = (z_max - m_player.mcr_camera->mcr_position.z) / rayDir.z;
            if (t_0 > t_1) {
                float temp = t_0;
                t_0 = t_1;
                t_1 = temp;
            }
            if (t_0 > t_near) {
                t_near = t_0;
            }
            if (t_1 < t_far) {
                t_far = t_1;
            }
        }
        //Final check
        //If we do intersect with the box
        if (t_near < t_far) {
            glm::vec3 intersect = t_near * rayDir + m_player.mcr_camera->mcr_position;
            glm::ivec3 intersectInt = glm::ivec3(glm::floor(intersect.x), glm::floor(intersect.y), glm::floor(intersect.z));
            //Check which box we are in:

            glm::vec3 diff = intersect - glm::vec3(out_blockHitPos.x, out_blockHitPos.y, out_blockHitPos.z);

            BlockType b;

            switch (m_placing) {
            case 1:
                b = BlockType::STONE;
                break;
            case 2:
                b = BlockType::GRASS;
                break;
            case 3:
                b = BlockType::PLANK;
                break;
            case 4:
                b = BlockType::COBBLE;
                break;
            case 5:
                b = BlockType::SAND;
                break;
            case 6:
                b = BlockType::CACTUS;
                break;
            case 7:
                b = BlockType::YELLOW_FLOWER;
                break;
            case 8:
                b = BlockType::ICE;
                break;
            case 9:
                b = BlockType::BRICK;
                break;
            case 0:
                b = BlockType::SNOW;
                break;
            default:
                b = BlockType::STONE;
                break;
            }

            if (diff.x == 1) {
                //Right
                m_terrain.setBlockAt(out_blockHitPos.x + 1, out_blockHitPos.y, out_blockHitPos.z, b);

                Chunk* c = m_terrain.getChunkAt(out_blockHitPos.x + 1, out_blockHitPos.z).get();
                m_terrain.appendEditedChunk(c);
            } else if (diff.x == 0) {
                //Left
                m_terrain.setBlockAt(out_blockHitPos.x - 1, out_blockHitPos.y, out_blockHitPos.z, b);
                Chunk* c = m_terrain.getChunkAt(out_blockHitPos.x - 1, out_blockHitPos.z).get();
                m_terrain.appendEditedChunk(c);
            } else if (diff.y == 1) {
                //Top
                m_terrain.setBlockAt(out_blockHitPos.x, out_blockHitPos.y + 1, out_blockHitPos.z, b);
                Chunk* c = m_terrain.getChunkAt(out_blockHitPos.x, out_blockHitPos.z).get();
                m_terrain.appendEditedChunk(c);
            } else if (diff.y == 0) {
                //Bottom
                m_terrain.setBlockAt(out_blockHitPos.x, out_blockHitPos.y - 1, out_blockHitPos.z, b);
                Chunk* c = m_terrain.getChunkAt(out_blockHitPos.x, out_blockHitPos.z).get();
                m_terrain.appendEditedChunk(c);
            } else if (diff.z == 1) {
                //Front
                m_terrain.setBlockAt(out_blockHitPos.x, out_blockHitPos.y, out_blockHitPos.z + 1, b);
                Chunk* c = m_terrain.getChunkAt(out_blockHitPos.x, out_blockHitPos.z + 1).get();
                m_terrain.appendEditedChunk(c);
            } else if (diff.z == 0) {
                m_terrain.setBlockAt(out_blockHitPos.x, out_blockHitPos.y, out_blockHitPos.z - 1, b);
                Chunk* c = m_terrain.getChunkAt(out_blockHitPos.x, out_blockHitPos.z - 1).get();
                m_terrain.appendEditedChunk(c);
            }
        }
    }
}

void MyGL::traverse(const uPtr<Node>& cur, glm::mat4 t, int textureSlot) {
    t *= cur->transMatrix();

    for (const uPtr<Node>& n: cur->children) {
        traverse(n, t, textureSlot);
    }

    if (cur->mCuboid != nullptr && cur->visible) {
        //Set Trans
        m_progLambert.setModelMatrix(t);
        //Modify UV
        cur->mCuboid->type = cur->type;
        //Draw
        cur->mCuboid->destroyVBOdata();
        cur->mCuboid->createVBOdata();
        m_progLambert.draw(*(cur->mCuboid), textureSlot);
    }
    return;
}
