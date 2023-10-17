// Description:
//   CherriesView
//
// Copyright (C) 2001 Frank Becker
//
// This program is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free Software
// Foundation;  either version 2 of the License,  or (at your option) any  later
// version.
//
// This program is distributed in the hope that it will be useful,  but  WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details
//
#include "CherriesView.hpp"

#include <math.h>

#include <FPS.hpp>

#include <PNG.hpp>
#include <Trace.hpp>
#include <Config.hpp>
#include <Value.hpp>
#include <GameState.hpp>
#include <Constants.hpp>

#include <Hero.hpp>
#include <ParticleGroupManager.hpp>
#include <Timer.hpp>
#include <Camera.hpp>
#include <MenuManager.hpp>
#include <FontManager.hpp>
#include <BitmapManager.hpp>
#include <ModelManager.hpp>
#include <ScoreKeeper.hpp>
#include <TextureManager.hpp>
#include <PuckMaze.hpp>
#ifndef IPHONE
#include <GLExtension.hpp>
#endif
#include "StringUtils.hpp"

#include <ResourceManager.hpp>
#include <zrwops.hpp>

#include "VideoBase.hpp"

#include <GL/glew.h>
#include "gl3/Program.hpp"
#include "gl3/Shader.hpp"
#include "gl3/Buffer.hpp"
#include "gl3/VertexArray.hpp"
#include "gl3/ProgramManager.hpp"
#include "gl3/MatrixStack.hpp"

#include "glm/glm.hpp"
#include "glm/ext.hpp"
#include "GLVertexBufferObject.hpp"

#include <algorithm>
#include <string>
#include <locale>

using namespace std;

CherriesView::CherriesView() :
    _boardVisible(true),
    _boardPosX(0),
    _angle(0.0),
    _prevAngle(0.0),
    _numStarVertices(0),
    _starVertices(0),
    _shaftVerts(0),
    _shaftNormals(0),
    _shaftVindices(0),
    _shaftVao(0) {
    XTRACE();
}

CherriesView::~CherriesView() {
    XTRACE();
    LOG_INFO << "CherriesView shutdown..." << endl;

    delete[] _starVertices;
    delete _shaftVerts;
    delete _shaftNormals;
    delete _shaftVindices;
    delete _shaftVao;

    SkillS::cleanup();
    VideoBaseS::cleanup();
}

void CherriesView::initGL3Test() {
    while (!MatrixStack::model.empty()) {
        MatrixStack::model.pop();
    }
    while (!MatrixStack::projection.empty()) {
        MatrixStack::projection.pop();
    }

    MatrixStack::model.push(glm::mat4(1.0f));
    MatrixStack::projection.push(glm::mat4(1.0f));

    Program* progLight = ProgramManagerS::instance()->createProgram("lighting");
    progLight->use();

    _shaftVao = new VertexArray();
    _shaftVao->bind();

    _shaftVerts = new Buffer();
    GLuint shaftVertLoc = 0;
    glEnableVertexAttribArray(shaftVertLoc);
    _shaftVerts->bind(GL_ARRAY_BUFFER);
    glVertexAttribPointer(shaftVertLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);

    _shaftNormals = new Buffer();
    GLuint shaftNormLoc = 1;
    glEnableVertexAttribArray(shaftNormLoc);
    _shaftNormals->bind(GL_ARRAY_BUFFER);
    glVertexAttribPointer(shaftNormLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);

    _shaftVindices = new Buffer();
    _shaftVindices->bind(GL_ELEMENT_ARRAY_BUFFER);

    _shaftVao->unbind();
    progLight->release();

    Program* progTexture = ProgramManagerS::instance()->createProgram("texture");
    progTexture->use();
    progTexture->release();

    LOG_INFO << "initGL3Test DONE\n";
}

bool CherriesView::init(void) {
    XTRACE();
    if (!VideoBaseS::instance()->init()) {
        return false;
    }

    initGL3Test();

    _smallFont = FontManagerS::instance()->getFont("bitmaps/arial-small");
    if (!_smallFont) {
        LOG_ERROR << "Unable to get font... (arial-small)" << endl;
        return false;
    }

    _scoreFont = FontManagerS::instance()->getFont("bitmaps/vipnaUpper");
    if (!_scoreFont) {
        LOG_ERROR << "Unable to get font... (vipnaUpper)" << endl;
        return false;
    }

    _gameOFont = FontManagerS::instance()->getFont("bitmaps/gameover");
    if (!_gameOFont) {
        LOG_ERROR << "Unable to get font... (gameover)" << endl;
        return false;
    }

    _board = BitmapManagerS::instance()->getBitmap("bitmaps/atlas");  //"bitmaps/board");
    if (!_board) {
        LOG_ERROR << "Unable to load CritterBoard" << endl;
        return false;
    }
    _boardIndex = _board->getIndex("board");

    //_titleIndex = _board->getIndex( "Title");

    _frenzy = _board->getIndex("frenzy");

    VideoBaseS::instance()->registerResolutionObserver(this);

    return true;
}

void CherriesView::resolutionChanged(int /*w*/, int /*h*/) {
    ProgramManagerS::instance()->reset();
    initGL3Test();
}

void CherriesView::updateLogic(void) {
    _prevAngle = _angle;
    _angle += 5.0f;

    float boardWidth = (float)_board->getWidth(_boardIndex);
    if (_boardVisible && (_boardPosX < 0)) {
        _boardPosX += 9;
        if (_boardPosX > 0) {
            _boardPosX = 0;
        }
    }

    if (!_boardVisible && (_boardPosX > -boardWidth)) {
        _boardPosX -= 9;
    }
}

bool CherriesView::draw(void) {
    //    XTRACE();

    static float nextTime = Timer::getTime() + 0.5f;
    float thisTime = Timer::getTime();
    if (thisTime > nextTime) {
        nextTime = thisTime + 0.5f;
        VideoBaseS::instance()->updateSettings();
    }

    FPS::Update();

    GLBitmapFont& smallFont = *_smallFont;
    GLBitmapFont& scoreFont = *_scoreFont;
    GLBitmapFont& gameOFont = *_gameOFont;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClear(GL_COLOR_BUFFER_BIT);  // | GL_DEPTH_BUFFER_BIT);

    //--- Ortho stuff from here on ---
    glm::mat4& projection = MatrixStack::projection.top();
    // projection = glm::ortho(-0.5f, VIDEO_ORTHO_HEIGHT + 0.5f, -0.5f, VIDEO_ORTHO_WIDTH + 0.5f, -1000.0f, 1000.0f);
    projection = glm::ortho(-0.5f, VIDEO_ORTHO_WIDTH + 0.5f, -0.5f, VIDEO_ORTHO_HEIGHT + 0.5f, -1000.0f, 1000.0f);

    glm::mat4& modelview = MatrixStack::model.top();
    modelview = glm::mat4(1.0);
    {
        Program* prog = ProgramManagerS::instance()->getProgram("texture");
        prog->use();  //needed to set uniforms
        GLint modelViewMatrixLoc = glGetUniformLocation(prog->id(), "modelViewMatrix");
        glUniformMatrix4fv(modelViewMatrixLoc, 1, GL_FALSE, glm::value_ptr(projection * modelview));
    }

#ifdef IPHONE
    glTranslatef(0, 1000, 0);
    glRotatef(-90.0, 0, 0, 1);
#endif

    glDisable(GL_DEPTH_TEST);

    bool showFPS = false;
    ConfigS::instance()->getBoolean("showFPS", showFPS);
    if (showFPS) {
        smallFont.setColor(1.0, 1.0, 1.0, 1.0);
        smallFont.DrawString(FPS::GetFPSString(), 0, 0, 1.0f, 1.0f);
    }

    if (GameState::context != Context::eMenu) {
#ifdef IPHONE
        projection = glm::ortho(0.0, 320.0, 0.0, 480.0, -1000.0, 1000.0);
#else
        projection = glm::ortho(0.0, 480.0, 0.0, 320.0, -1000.0, 1000.0);
#endif

        modelview = glm::mat4(1.0);
        {
            Program* prog = ProgramManagerS::instance()->getProgram("texture");
            prog->use();  //needed to set uniforms
            GLint modelViewMatrixLoc = glGetUniformLocation(prog->id(), "modelViewMatrix");
            glUniformMatrix4fv(modelViewMatrixLoc, 1, GL_FALSE, glm::value_ptr(projection * modelview));
        }

#ifdef IPHONE
        glTranslatef(0, 480.0, 0);
        glRotatef(-90.0, 0, 0, 1);
#endif

        //glColor4f(1.0,1.0,1.0,0.3f);
        _board->setColor(1.0, 1.0, 1.0, 0.3f);

        float mazeOffsetX = 75.0;

        if (HeroS::instance()->alive()) {
            PuckMazeS::instance()->draw(mazeOffsetX, 0, 0, 1., 1.);

            int puckCount = PuckMazeS::instance()->Width() * PuckMazeS::instance()->Height();
            if (puckCount != _numStarVertices) {
                _numStarVertices = puckCount;
                delete[] _starVertices;
                _starVertices = new GLfloat[_numStarVertices * 3];
            }

            float cellSize = PuckMazeS::instance()->CellSize();
            //LOG_INFO << "cellsize = " << cellSize << "\n";

            int _cherrySmall = _board->getIndex("cherrySmall");
            int _banana = _board->getIndex("banana");

            //glEnable(GL_TEXTURE_2D);
            _board->bind();


            if (GameState::context == Context::ePaused) {
                smallFont.setColor(1.0, 1.0, 1.0, 1.0);
                float cx = (480.0 - smallFont.GetWidth("Paused", 1.0f)) / 2.0;
                smallFont.DrawString("Paused", cx, 160, 1.0f, 1.0f);
                return true;
            }

            vmml::vec4f frenzyColor;
            if (HeroS::instance()->Frenzy()) {
                frenzyColor = vmml::vec4f(1.0, 1.0, 1.0, 1.0);
            } else {
                frenzyColor = vmml::vec4f(1.0, 1.0, 1.0, 0.15);
            }
            _board->setColor(frenzyColor);
            _board->Draw(_frenzy, 480.0 - 90.0, 20.0, 1.0, 1.0);

            //glColor4f(1.0,1.0,1.0,1.0);
            _board->setColor(1.0, 1.0, 1.0, 1.0);

            bool detailCherry = cellSize > 10;

            for (int y = 0; y < PuckMazeS::instance()->Height(); y++) {
                for (int x = 0; x < PuckMazeS::instance()->Width(); x++) {
                    float posX = (float)x * cellSize + (cellSize / 2.0) + 0.5 + mazeOffsetX;
                    float posY = (float)y * cellSize + (cellSize / 2.0) + 0.5;

                    if (PuckMazeS::instance()->isElement(x, y, POWERPOINT)) {
                        _board->DrawC(_banana, posX, posY, cellSize / 32.0, cellSize / 32.0);
                    } else if (detailCherry && PuckMazeS::instance()->isElement(x, y, CHERRY)) {
                        _board->DrawC(_cherrySmall, posX, posY, cellSize / 64.0, cellSize / 64.0);
                    }
                }
            }

            //glDisable(GL_TEXTURE_2D);

            if (!detailCherry) {
                int vIdx = 0;
                for (int y = 0; y < PuckMazeS::instance()->Height(); y++) {
                    for (int x = 0; x < PuckMazeS::instance()->Width(); x++) {
                        _starVertices[vIdx++] = (float)x * cellSize + (cellSize) / 2.0 + 0.25 + mazeOffsetX;
                        _starVertices[vIdx++] = (float)y * cellSize + (cellSize) / 2.0 + 0.25;
                        if (PuckMazeS::instance()->isElement(x, y, CHERRY)) {
                            _starVertices[vIdx++] = 0;
                        } else {
                            _starVertices[vIdx++] = 2000;
                        }
                    }
                }

#ifndef EMSCRIPTEN
                if (PuckMazeS::instance()->Points() < (_numStarVertices / 50)) {
                    glPointSize(cellSize - 1.0);
                } else {
                    float ptSize = (max)((cellSize - 1.0) / 3.0, 1.0);
                    glPointSize(ptSize);
                }
#endif
#if OLD_DRAW
                glColor4f(0.6, 0.0, 0.0, 1.0f);
                glEnable(GL_POINT_SMOOTH);
                glEnableClientState(GL_VERTEX_ARRAY);
                glVertexPointer(3, GL_FLOAT, 0, _starVertices);
                glDrawArrays(GL_POINTS, 0, _numStarVertices);
                glDisableClientState(GL_VERTEX_ARRAY);
#else
                GLVBO vbo;
                vbo.setColor(0.6, 0.0, 0.0, 1.0f);
                vbo.DrawPoints(_starVertices, _numStarVertices);
#endif
            }

            ParticleGroupManagerS::instance()->draw();

            if (HeroS::instance()->alive()) {
                HeroS::instance()->draw();
            }
        }

        // projection = glm::ortho(-0.5f, VIDEO_ORTHO_HEIGHT + 0.5f, -0.5f, VIDEO_ORTHO_WIDTH + 0.5f, -1000.0f, 1000.0f);
        projection = glm::ortho(-0.5f, VIDEO_ORTHO_WIDTH + 0.5f, -0.5f, VIDEO_ORTHO_HEIGHT + 0.5f, -1000.0f, 1000.0f);

        modelview = glm::mat4(1.0);
        {
            Program* prog = ProgramManagerS::instance()->getProgram("texture");
            prog->use();  //needed to set uniforms
            GLint modelViewMatrixLoc = glGetUniformLocation(prog->id(), "modelViewMatrix");
            glUniformMatrix4fv(modelViewMatrixLoc, 1, GL_FALSE, glm::value_ptr(projection * modelview));
        }

#ifdef IPHONE
        glTranslatef(0, 1000, 0);
        glRotatef(-90.0, 0, 0, 1);
#endif

        if (!HeroS::instance()->alive()) {
            float cx = (1000.0 - gameOFont.GetWidth("GAME OVER", 0.8f)) / 2.0;
            gameOFont.setColor(1.0f, 1.0f, 1.0f, 0.8f);
            gameOFont.DrawString("GAME OVER", cx, 700, 0.8f, 0.8f);

            string text;
            text = "Willy got ya. Try again!";

            cx = (1000.0 - smallFont.GetWidth(text.c_str(), 1.9f)) / 2.0;
            smallFont.setColor(1.0f, 1.0f, 1.0f, 1.0f);
            smallFont.DrawString(text.c_str(), cx, 640, 1.9f, 1.9f);

            if (ScoreKeeperS::instance()->currentIsTopTen()) {
                if (!_textInput.isOn()) {
                    _textInput.turnOn();
                }
                string currentText = _textInput.getText();
                if (!_textInput.isOn()) {
                    MenuManagerS::instance()->turnMenuOn();
                }

                text = "Top Ten Finish!";
                cx = (1000.0 - smallFont.GetWidth(text.c_str(), 1.5f)) / 2.0;
                smallFont.DrawString(text.c_str(), cx, 530, 1.5f, 1.5f);

                string pname = "Enter name: ";
                pname += currentText + "_";
                smallFont.DrawString(pname.c_str(), 115, 420, 2.0f, 2.0f);

                ScoreKeeperS::instance()->setNameForCurrent(currentText);
            }

            //glColor4f(1.0f,1.0f,1.0f,1.0f);
        }
    }

    char buff[128];
    if (GameState::isDeveloper) {
        static float nextShow = 0;
        static int aCount = 0;
        float thisTime = Timer::getTime();
        if (thisTime > nextShow) {
            nextShow = thisTime + 0.5f;
            aCount = ParticleGroupManagerS::instance()->getAliveCount();
        }
        sprintf(buff, "p=%d", aCount);
        smallFont.setColor(1.0f, 1.0f, 1.0f, 1.0f);
        smallFont.DrawString(buff, 0, 40, 1.0, 1.0);
    }

    if (GameState::context == Context::eMenu) {
        //glEnable(GL_TEXTURE_2D);
        _board->bind();
        //glColor4f(1.0,1.0,1.0,1.0);
        //float scale = 1.5;
        //float xOff = (VIDEO_ORTHO_WIDTH-(_board->getWidth(_titleIndex)*scale))/2.0;
        //_board->setColor(1.0,1.0,1.0,1.0);
        //_board->Draw(_titleIndex,xOff,VIDEO_ORTHO_HEIGHT-200,scale,scale);
        //glDisable(GL_TEXTURE_2D);

        MenuManagerS::instance()->draw();

        //glColor4f(1.0,1.0,1.0,0.5);
        string gVersion = "v" + GAMEVERSION;
        float width = smallFont.GetWidth(gVersion.c_str(), 0.7f);
        smallFont.setColor(1.0, 1.0, 1.0, 0.5);
        smallFont.DrawString(gVersion.c_str(), 995.0f - width, 0.0f, 0.7f, 0.7f);
    } else {
        float boardWidth = (float)_board->getWidth(_boardIndex);

        //draw board if at least partially visible
        if (_boardPosX > -boardWidth) {
            float size = 0.54f;
            float tdy = -50.0f;
            float ty = (float)VIDEO_ORTHO_HEIGHT - 49.0f;
            float tx = 8.0f + _boardPosX;

            //glEnable(GL_TEXTURE_2D);
            _board->bind();
            //glColor4f(1.0,1.0,1.0,0.8);
            _board->setColor(1.0, 1.0, 1.0, 0.8);
            _board->Draw(_boardIndex, _boardPosX, VIDEO_ORTHO_HEIGHT - 256, 1.0, 1.0);
            //glDisable(GL_TEXTURE_2D);

            sprintf(buff, "%d", ScoreKeeperS::instance()->getCurrentScore());
            scoreFont.setColor(1.0, 1.0, 1.0, 1.0);
            scoreFont.DrawString(buff, tx, ty, size, size);
            ty += tdy;

            sprintf(buff, "%d", ScoreKeeperS::instance()->getHighScore());
            scoreFont.setColor(1.0, 1.0, 1.0, 1.0);
            scoreFont.DrawString(buff, tx, ty, size, size);
            ty += tdy;

            float bLen = 1.0f;
            float he = HeroS::instance()->Energy() / 3.0f; // Banana timer
            Clamp(he, 0.0, 100.0);
#if OLD_DRAW
            glColor4f(1.0f, 1.0f, 0.1f, 0.5f);
            GLfloat quadVerticesEnergy[] = {
                tx - 2, ty + 2,  -1, tx - 2 + he * bLen, ty + 2, -1, tx - 2 + he * bLen, ty + 24, -1,
                tx - 2, ty + 24, -1};
            glVertexPointer(3, GL_FLOAT, 0, quadVerticesEnergy);
            glEnableClientState(GL_VERTEX_ARRAY);
            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
            glDisableClientState(GL_VERTEX_ARRAY);
#else
            vec4f v[4] = {
                vec4f(tx - 2, ty + 2, -1, 1),
                vec4f(tx - 2 + he * bLen, ty + 2, -1, 1),
                vec4f(tx - 2 + he * bLen, ty + 24, -1, 1),
                vec4f(tx - 2, ty + 24, -1, 1),
            };
            GLVBO vbo;
            vbo.setColor(1.0f, 1.0f, 0.1f, 0.5f);
            vbo.DrawQuad(v);
#endif
            ty += tdy;
            //glColor4f(1.0,1.0,1.0,1.0);
            int matchTime = HeroS::instance()->Age();
            sprintf(buff, "%d.%d", matchTime / 1000, (matchTime % 1000) / 100);
            scoreFont.setColor(1.0, 1.0, 1.0, 1.0);
            scoreFont.DrawString(buff, tx, ty, size, size);

            ty += tdy;
            //glColor4f(1.0,1.0,1.0,1.0);
            SkillS::instance();
            string theSkill = StringUtils(Skill::getString(GameState::skill)).toUpper();
            scoreFont.setColor(1.0, 1.0, 1.0, 1.0);
            scoreFont.DrawString(theSkill.c_str(), tx, ty, size, size);
        }
    }

    return true;
}
