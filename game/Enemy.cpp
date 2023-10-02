// Description:
//
// Copyright (C) 2008 Frank Becker
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
#include <Enemy.hpp>
#include <Hero.hpp>

#include <Trace.hpp>
#include <RandomKnuth.hpp>
#include <Point.hpp>
#include <Constants.hpp>

#include <BitmapManager.hpp>

#include <MazeNavigation.hpp>

using namespace std;

static RandomKnuth _random;

Enemy::Enemy(void) :
    ParticleType("Worm", true) {
    _atlas = BitmapManagerS::instance()->getBitmap("bitmaps/atlas");
    if (!_atlas) {
        LOG_ERROR << "Unable to load atlas" << endl;
    }
    _wormHead = _atlas->getIndex("wormHead");
    _wormTail = _atlas->getIndex("wormTail");
}

Enemy::~Enemy() {}

void Enemy::init(ParticleInfo* p) {
    XTRACE();
    //LOG_INFO << "Enemy::init\n";

    Point3D heroPos(HeroS::instance()->lastXPos, HeroS::instance()->lastYPos, 0.0);
    Point3D myPos((float)(_random.random() % (PuckMazeS::instance()->Width())),
                  (float)(_random.random() % (PuckMazeS::instance()->Height())), 0.0);

    while (dist(myPos - heroPos) < 5.0) {
        myPos.x = (float)(_random.random() % (PuckMazeS::instance()->Width()));
        myPos.y = (float)(_random.random() % (PuckMazeS::instance()->Height()));
    }

    p->position.x = myPos.x;
    p->position.y = myPos.y;

    p->extra = p->position;
    p->color = p->position;
    p->velocity = p->position;

    p->radius = OBJECT_RADIUS;
    p->damage = 0;
    p->tod = -1;

    updatePrevs(p);
}

bool Enemy::update(ParticleInfo* p) {
    if (p->tod == 0) {
        return false;
    }

    updatePrevs(p);

    p->damage++;
    if (p->damage > 2) {
        p->extra = p->color;
        p->color = p->velocity;
        p->velocity = p->position;
        p->damage = 0;
    }

    float& _xPos = p->position.x;
    float& _yPos = p->position.y;

    Point2D delta(HeroS::instance()->lastXPos - _xPos, HeroS::instance()->lastYPos - _yPos);

    float dist = sqrt(delta.x * delta.x + delta.y * delta.y);
    if (dist < 0.001) {
        //LOG_INFO << "tiny delta\n";
        return true;
    }

    norm(delta);

    if (HeroS::instance()->Energy() > 0) {
        //run away!
        delta = delta * -1;
    }

    float stepX = delta.x * 0.2;
    float stepY = delta.y * 0.2;
    Clamp(stepX, -0.4, 0.4);
    Clamp(stepY, -0.4, 0.4);

    vec2f newPos = MazeNavigationS::instance()->getNextPosition(vec2f(_xPos, _yPos), vec2f(stepX, stepY));
    _xPos = newPos.x();
    _yPos = newPos.y();

    return true;
}

void Enemy::draw(ParticleInfo* p) {
    ParticleInfo pi;
    interpolate(p, pi);

    float mazeOffsetX = 75.0;

    float cellSize = PuckMazeS::instance()->CellSize();
    float posX = pi.position.x * cellSize + (cellSize / 2.0) + 0.5 + mazeOffsetX;
    float posY = pi.position.y * cellSize + (cellSize / 2.0) + 0.5;

    float posX1 = p->velocity.x * cellSize + (cellSize / 2.0) + 0.5 + mazeOffsetX;
    float posY1 = p->velocity.y * cellSize + (cellSize / 2.0) + 0.5;

    float posX2 = p->color.x * cellSize + (cellSize / 2.0) + 0.5 + mazeOffsetX;
    float posY2 = p->color.y * cellSize + (cellSize / 2.0) + 0.5;

    float posX3 = p->extra.x * cellSize + (cellSize / 2.0) + 0.5 + mazeOffsetX;
    float posY3 = p->extra.y * cellSize + (cellSize / 2.0) + 0.5;

    //glEnable(GL_TEXTURE_2D);
    _atlas->bind();
    //glColor4f(1.0,1.0,1.0,1.0);
    _atlas->setColor(1.0, 1.0, 1.0, 1.0);
    _atlas->DrawC(_wormTail, posX3, posY3, cellSize / 28.0, cellSize / 28.0);
    _atlas->DrawC(_wormTail, posX2, posY2, cellSize / 22.0, cellSize / 22.0);
    _atlas->DrawC(_wormTail, posX1, posY1, cellSize / 14.0, cellSize / 14.0);
    _atlas->DrawC(_wormHead, posX, posY, cellSize / 24.0, cellSize / 24.0);

    //(GL_TEXTURE_2D);
}

void Enemy::hit(ParticleInfo* p, int /*damage*/, int /*radIndex*/) {
    //LOG_INFO << "Enemy::hit\n";
    if (HeroS::instance()->Energy() > 0) {
        p->tod = 0;
    }
}
