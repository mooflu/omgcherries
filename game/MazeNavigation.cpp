#include "MazeNavigation.hpp"

#include "PuckMaze.hpp"
#include "Constants.hpp"

MazeNavigation::MazeNavigation(void) {
    b2Vec2 gravity(0, 0);
    bool doSleep = true;

    _world = new b2World(gravity);
    _world->SetAllowSleeping(doSleep);

    b2BodyDef wallBodyDef;

    b2PolygonShape vWallShape;
    vWallShape.SetAsBox(0.01, 0.5);

    b2FixtureDef vWallFicture;
    vWallFicture.shape = &vWallShape;

    for (int i = 0; i < 6; i++) {
        _vWalls[i] = _world->CreateBody(&wallBodyDef);
        _vWalls[i]->CreateFixture(&vWallFicture);
    }

    b2PolygonShape hWallShape;
    hWallShape.SetAsBox(0.5, 0.01);

    b2FixtureDef hWallFicture;
    hWallFicture.shape = &hWallShape;

    for (int i = 0; i < 6; i++) {
        _hWalls[i] = _world->CreateBody(&wallBodyDef);
        _hWalls[i]->CreateFixture(&hWallFicture);
    }

    b2BodyDef itemBodyDef;

    b2CircleShape itemShapeDef;
    itemShapeDef.m_radius = OBJECT_RADIUS;

    b2FixtureDef hItemFixture;
    hItemFixture.shape = &itemShapeDef;
    hItemFixture.density = 1.0f;

    _item = _world->CreateBody(&itemBodyDef);
    _item->CreateFixture(&hItemFixture);
    _item->SetType(b2_dynamicBody);
    _item->SetSleepingAllowed(false);
}

MazeNavigation::~MazeNavigation() {
    delete _world;
}

vec2f MazeNavigation::getNextPosition(const vec2f& itemPos, const vec2f& velocity) {
    _item->SetTransform(b2Vec2(itemPos.x(), itemPos.y()), 0);
    _item->SetLinearVelocity(b2Vec2(velocity.x(), velocity.y()));

    PuckMaze* pm = PuckMazeS::instance();

    //update walls
    vec2i pos(lroundf(itemPos.x()), lroundf(itemPos.y()));

    float x;
    float y;

    int wallIndex = 0;

    vec2i p = pos;
    x = p.x();
    y = p.y();
    if (pm->isElement(p.x(), p.y(), WallLT)) {
        _vWalls[wallIndex++]->SetTransform(b2Vec2(x - 0.5f, y), 0);
    }
    if (pm->isElement(p.x(), p.y(), WallRT)) {
        _vWalls[wallIndex++]->SetTransform(b2Vec2(x + 0.5f, y), 0);
    }

    p = vec2i(pos.x(), pos.y() + 1);
    if (pm->isInside(p.x(), p.y())) {
        x = p.x();
        y = p.y();

        if (pm->isElement(p.x(), p.y(), WallLT)) {
            _vWalls[wallIndex++]->SetTransform(b2Vec2(x - 0.5f, y), 0);
        }
        if (pm->isElement(p.x(), p.y(), WallRT)) {
            _vWalls[wallIndex++]->SetTransform(b2Vec2(x + 0.5f, y), 0);
        }
    }

    p = vec2i(pos.x(), pos.y() - 1);
    if (pm->isInside(p.x(), p.y())) {
        x = p.x();
        y = p.y();

        if (pm->isElement(p.x(), p.y(), WallLT)) {
            _vWalls[wallIndex++]->SetTransform(b2Vec2(x - 0.5f, y), 0);
        }
        if (pm->isElement(p.x(), p.y(), WallRT)) {
            _vWalls[wallIndex++]->SetTransform(b2Vec2(x + 0.5f, y), 0);
        }
    }

    for (int i = wallIndex; i < 6; i++) {
        _vWalls[i]->SetTransform(b2Vec2(-10, -10), 0);
    }

    wallIndex = 0;
    p = pos;
    x = p.x();
    y = p.y();
    if (pm->isElement(p.x(), p.y(), WallUP)) {
        _hWalls[wallIndex++]->SetTransform(b2Vec2(x, y - 0.5f), 0);
    }
    if (pm->isElement(p.x(), p.y(), WallDN)) {
        _hWalls[wallIndex++]->SetTransform(b2Vec2(x, y + 0.5), 0);
    }

    p = vec2i(pos.x() - 1, pos.y());
    if (pm->isInside(p.x(), p.y())) {
        x = p.x();
        y = p.y();

        if (pm->isElement(p.x(), p.y(), WallUP)) {
            _hWalls[wallIndex++]->SetTransform(b2Vec2(x, y - 0.5f), 0);
        }
        if (pm->isElement(p.x(), p.y(), WallDN)) {
            _hWalls[wallIndex++]->SetTransform(b2Vec2(x, y + 0.5), 0);
        }
    }

    p = vec2i(pos.x() + 1, pos.y());
    if (pm->isInside(p.x(), p.y())) {
        x = p.x();
        y = p.y();

        if (pm->isElement(p.x(), p.y(), WallUP)) {
            _hWalls[wallIndex++]->SetTransform(b2Vec2(x, y - 0.5f), 0);
        }
        if (pm->isElement(p.x(), p.y(), WallDN)) {
            _hWalls[wallIndex++]->SetTransform(b2Vec2(x, y + 0.5), 0);
        }
    }

    for (int i = wallIndex; i < 6; i++) {
        _hWalls[i]->SetTransform(b2Vec2(-10, -10), 0);
    }

    for (int i = 0; i < 1; i++) {
        _world->Step(1.0f / 1.0f, 1, 1);
    }

    b2Vec2 newPos = _item->GetPosition();
    return vec2f(newPos.x, newPos.y);
}
