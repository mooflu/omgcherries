#pragma once
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

#include <Singleton.hpp>
#include <Model.hpp>
#include <Direction.hpp>
#include <ParticleType.hpp>
#include <Skill.hpp>
#include <Tracer.hpp>
#include <GLBitmapCollection.hpp>

#include <string>

class Hero : public ParticleType {
    friend class Singleton<Hero>;

public:
    bool init(void);
    void reset(void);

    virtual void init(ParticleInfo* p);
    virtual bool update(ParticleInfo* p);
    virtual void hit(ParticleInfo* p, ParticleInfo* p2, int /*radIndex*/);

    virtual void draw(ParticleInfo*) { /* not used, see other draw */
        ;
    }

    virtual void draw(void);

    void nextLevel(void);

    void tap(bool isDown);
    void move(float dx, float dy);
    void move(Direction::DirectionEnum d, bool isDown);

    bool Move(int& x, int& y, int dir);
    void Check(int x, int y);
    void Track(int& x, int& y, int element);
    int Reverse(int dir);

    int Energy(void) { return _energy; }

    bool alive(void) { return _isAlive; }

    void alive(bool a) { _isAlive = a; }

    int Age(void) { return _age; }

    bool Frenzy(void) { return _doTrace; }

    float lastXPos;
    float lastYPos;

private:
    virtual ~Hero();
    Hero(void);

    Hero(const Hero&);
    Hero& operator=(const Hero&);

    bool _doTrace;
    NEWTracer* _tracer;

    ParticleInfo* pInfo;

    int _energy;

    float _moveLeft;
    float _moveRight;
    float _moveUp;
    float _moveDown;
    float _maxY;
    bool _isAlive;
    bool _isDying;
    int _invincibleUntil;
    int _isDyingDelay;
    int _age;

    float _sint[360];
    float _cost[360];

    GLBitmapCollection* _atlas;
    int _wheelsSmall;
};

typedef Singleton<Hero> HeroS;
