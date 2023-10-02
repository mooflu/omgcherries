#pragma once
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

#include <ParticleType.hpp>
#include <PuckMaze.hpp>
#include <GLBitmapCollection.hpp>

class Enemy : public ParticleType {
private:
    GLBitmapCollection* _atlas;
    int _wormHead;
    int _wormTail;

public:
    Enemy(void);
    virtual ~Enemy();

    virtual void init(ParticleInfo* p);
    virtual bool update(ParticleInfo* p);
    virtual void draw(ParticleInfo* p);

    virtual void hit(ParticleInfo* p, int /*damage*/, int /*radIndex*/);
};
