// Description:
//   Our Hero!
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
#include <Hero.hpp>
#include <ParticleGroup.hpp>
#include <ParticleGroupManager.hpp>
#include <Timer.hpp>
#include <Trace.hpp>
#include <GameState.hpp>
#include <Audio.hpp>
#include <Point.hpp>
#include <ScoreKeeper.hpp>
#include <Config.hpp>
#include <Game.hpp>
#include <Input.hpp>
#include <RandomKnuth.hpp>
#include <PuckMaze.hpp>
#include <Constants.hpp>

#include <BitmapManager.hpp>

#include <MazeNavigation.hpp>

#include <vmmlib/vector.hpp>
using namespace vmml;

#include <GL/glew.h>

using namespace std;

static RandomKnuth _random;

const float MAX_X = 63;   //(int)(47.5*4/3);
const float MIN_X = -63;  //-(int)(47.5*4/3);
const float MAX_Y = 45;
const float MIN_Y = -45;

Hero::Hero() :
    ParticleType("Hero"),
    pInfo(0),
    _tracer(0),
    _maxY(MIN_Y),
    _directions(0) {
    XTRACE();
    for (int i = 0; i < 360; i++) {
        _sint[i] = sin(i * ((float)M_PI / 180.0f));
        _cost[i] = cos(i * ((float)M_PI / 180.0f));
    }

    _atlas = BitmapManagerS::instance()->getBitmap("bitmaps/atlas");
    if (!_atlas) {
        LOG_ERROR << "Unable to load atlas" << endl;
    }
    _wheelsSmall = _atlas->getIndex("wheelsSmall");

    reset();
}

void Hero::reset(void) {
    XTRACE();
    //LOG_INFO << "Hero::reset\n";

    _isAlive = true;
    _isDying = false;
    _isDyingDelay = 0;
    _age = 0;
    _moveLeft = 0;
    _moveRight = 0;
    _moveUp = 0;
    _moveDown = 0;
    _energy = 0;
    _doTrace = false;
    _directions = 0;

    lastXPos = 0.0;
    lastYPos = 0.0;

    if (_tracer) {
        delete _tracer;
    }
    _tracer = new NEWTracer(PuckMazeS::instance());
}

void Hero::nextLevel(void) {
    _energy = 0;
    _invincibleUntil = _age + 2000;
}

Hero::~Hero() {
    XTRACE();

    delete _tracer;
}

void Hero::init(ParticleInfo* p) {
    XTRACE();
    //LOG_INFO << "Hero::init\n";
    pInfo = p;

    p->position.x = (float)(PuckMazeS::instance()->Width() / 2);
    p->position.y = (float)(PuckMazeS::instance()->Height() / 2);
    p->radius = OBJECT_RADIUS;
    p->damage = 500;
    p->tod = -1;

    lastXPos = p->position.x;
    lastYPos = p->position.y;

    updatePrevs(p);
}

void Hero::hit(ParticleInfo* p, ParticleInfo* p2, int /*radIndex*/) {
    //    XTRACE();
    if (!_isAlive || _isDying) {
        return;
    }

    if (_invincibleUntil > _age) {
        return;
    }

    if (_energy <= 0) {
#if 0
        static ParticleGroup *effects =
            ParticleGroupManagerS::instance()->getParticleGroup(EFFECTS_GROUP2);

        AudioS::instance()->playSample( "sounds/big_explosion");

        //spawn explosion
        for( int i=0; i<(int)(GameState::horsePower/1.5); i++)
        {
            effects->newParticle(
                "ExplosionPiece", p->position.x, p->position.y, p->position.z);
        }
#endif
        _isDyingDelay = 20;
        _isDying = true;
        ScoreKeeperS::instance()->addToCurrentScore(0);  //update playing time
        AudioS::instance()->playSample("sounds/gameOver");
        p->damage = 0;
    } else {
        ScoreKeeperS::instance()->addToCurrentScore(50);
    }
}

bool Hero::init(void) {
    XTRACE();
    return true;
}

bool Hero::update(ParticleInfo* p) {
    //    XTRACE();
    if (p->tod == 0) {
        return false;
    }

    if (_isDying) {
        if (_isDyingDelay-- < 0) {
            _isAlive = false;
            return false;
        }
        return true;
    }

    if (!_isAlive) {
        return false;
    }

    updatePrevs(p);

    float& _xPos = p->position.x;
    float& _yPos = p->position.y;

    _age = (int)(1000.0 * (GameState::stopwatch.getTime() - GameState::startOfGame));

    if (_energy > 0) {
        _energy--;
    }

    if (_doTrace) {
        int newX = lroundf(_xPos);
        int newY = lroundf(_yPos);
        Track(newX, newY, CHERRY);
        _xPos = newX;
        _yPos = newY;
    } else {
        if (_directions & Direction::eDown) {
            move(Direction::eDown, true);
        }
        if (_directions & Direction::eUp) {
            move(Direction::eUp, true);
        }
        if (_directions & Direction::eLeft) {
            move(Direction::eLeft, true);
        }
        if (_directions & Direction::eRight) {
            move(Direction::eRight, true);
        }
    }

    lastXPos = _xPos;
    lastYPos = _yPos;

    return true;
}

bool Hero::Move(int& x, int& y, int dir) {
    if (PuckMazeS::instance()->isElement(x, y, dir)) {
        return false;
    }

    switch (dir) {
        case WallLT:
            x--;
            break;
        case WallRT:
            x++;
            break;
        default:
            break;
    }

    switch (dir) {
        case WallUP:
            y--;
            break;
        case WallDN:
            y++;
            break;
        default:
            break;
    }

    Check(x, y);

    return true;
}

void Hero::Check(int x, int y) {
    if (PuckMazeS::instance()->isElement(x, y, CHERRY)) {
        PuckMazeS::instance()->ClearPoint(x, y);
        ScoreKeeperS::instance()->addToCurrentScore(1);
        AudioS::instance()->playSample("sounds/tick");
    }

    if (PuckMazeS::instance()->isElement(x, y, POWERPOINT)) {
        PuckMazeS::instance()->RemoveElement(x, y, POWERPOINT);
        ScoreKeeperS::instance()->addToCurrentScore(100);
        _energy += 120 + (30 * ((int)GameState::skill + 1));
        //LOG_INFO << "energy = " << _energy << "\n";
    }

    if (PuckMazeS::instance()->Points() == 0) {
        //_isDyingDelay = 60; //2 sec
#if 0
        _isDying = true;
#endif
        GameS::instance()->nextLevel();

        ScoreKeeperS::instance()->addToCurrentScore(0);  //update playing time
        AudioS::instance()->playSample("sounds/gameOverWon");
    }
    //LOG_INFO << "Remaining: " << PuckMazeS::instance()->Points() << "\n";
}

int Hero::Reverse(int dir) {
    switch (dir) {
        case WallLT:
            return WallRT;

        case WallRT:
            return WallLT;

        case WallUP:
            return WallDN;

        case WallDN:
            return WallUP;
    }

    return 0;
}

void Hero::Track(int& x, int& y, int element) {
    static int oldDir = WallDN;
    static int dir;
    int count = 0;
    static int nfCount = 0;

    dir = _tracer->Find2(x, y, element);

    //the chunk below makes sure that random movement
    //isn't too wild.
    if (dir == 0) {
        nfCount++;
        if (nfCount < 10) {
            dir = oldDir;
            while (PuckMazeS::instance()->isElement(x, y, dir)) {
                dir = 1 << (_random.random() % 4);
                while (dir == oldDir || dir == Reverse(oldDir)) {
                    dir = 1 << (_random.random() % 4);
                }
                count++;
                if (count > (_random.random() % 8)) {
                    dir = 1 << (_random.random() % 4);
                    count = 0;
                }
            }
            oldDir = dir;
        } else {
            dir = 1 << (_random.random() % 4);
            nfCount = 0;
        }
    } else {
        nfCount = 0;
    }

    Move(x, y, dir);
}

void Hero::tap(bool isDown) {
    _doTrace = isDown;
}

void Hero::move(float dx, float dy) {
    //    XTRACE();
    if (!_isAlive || _isDying) {
        return;
    }
    if (!pInfo) {
        return;
    }

    float& _xPos = pInfo->position.x;
    float& _yPos = pInfo->position.y;

    float stepX = dx * 0.2;
    float stepY = dy * 0.2;
    Clamp(stepX, -0.8, 0.8);
    Clamp(stepY, -0.8, 0.8);

    vec2f newPos = MazeNavigationS::instance()->getNextPosition(vec2f(_xPos, _yPos), vec2f(stepX, stepY));
    _xPos = newPos.x();
    _yPos = newPos.y();

    Check(lroundf(_xPos), lroundf(_yPos));
}

void Hero::applyDirection(Direction::DirectionEnum dir, bool isDown) {
    if (isDown) {
        _directions = _directions | dir;
    }
    else {
        _directions = _directions & ~dir;
    }
}

void Hero::move(Direction::DirectionEnum dir, bool isDown) {
    //    XTRACE();
    if (!_isAlive || _isDying) {
        return;
    }
    float delta;

    float stepX = 0.0f;
    float stepY = 0.0f;

    if (isDown) {
        delta = 0.6f * GAME_STEP_SCALE;
    } else {
        delta = 0;
    }

    float& _xPos = pInfo->position.x;
    float& _yPos = pInfo->position.y;

    //LOG_INFO << "delta = " << delta << endl;

    switch (dir) {
        case Direction::eDown:
            stepY = -delta;
            break;
        case Direction::eUp:
            stepY = delta;
            break;

        case Direction::eLeft:
            stepX = -delta;
            break;
        case Direction::eRight:
            stepX = delta;
            break;

        default:
            break;
    }
    vec2f newPos = MazeNavigationS::instance()->getNextPosition(vec2f(_xPos, _yPos), vec2f(stepX, stepY));
    _xPos = newPos.x();
    _yPos = newPos.y();

    Check(lroundf(_xPos), lroundf(_yPos));
}

void Hero::draw(void) {
    //    XTRACE();
    if (!_isAlive || _isDying) {
        return;
    }
    if (!pInfo) {
        return;
    }

    ParticleInfo pi;
    interpolate(pInfo, pi);

    float mazeOffsetX = 75.0;

    float cellSize = PuckMazeS::instance()->CellSize();
    float posX = pi.position.x * cellSize + (cellSize / 2.0) + 0.5 + mazeOffsetX;
    float posY = pi.position.y * cellSize + (cellSize / 2.0) + 0.5;
#if 1
    //glEnable(GL_TEXTURE_2D);
    _atlas->bind();

    vec4f color;
    static int flicker = 10;
    if (_invincibleUntil > _age) {
        //LOG_INFO << flicker << endl;
        flicker--;
        if (flicker <= 0) {
            flicker = 10;
        }
        if (flicker > 5) {
            color = vec4f(1.0, 0.0, 0.0, 0.2);
        } else {
            color = vec4f(1.0, 1.0, 1.0, 1.0);
        }
    } else {
        color = vec4f(1.0, 1.0, 1.0, 1.0);
    }
    _atlas->setColor(color);
    _atlas->DrawC(_wheelsSmall, posX, posY, cellSize / 24.0, cellSize / 24.0);
    //glDisable(GL_TEXTURE_2D);
#else
    GLfloat vertices[3];
    int vIdx = 0;
    vertices[vIdx++] = posX;
    vertices[vIdx++] = posY;
    vertices[vIdx++] = 0;

    glColor4f(1.0, 1.0, 0.0, 1.0f);
    glPointSize(cellSize - 1.0);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, vertices);
    glDrawArrays(GL_POINTS, 0, NUM_STARS);
    glDisableClientState(GL_VERTEX_ARRAY);
    glPointSize(1.0);
#endif
}
