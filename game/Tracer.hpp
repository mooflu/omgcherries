// Description:
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
#ifndef __Tracer_hpp__
#define __Tracer_hpp__

#include "SDL.h"
#include <PuckMaze.hpp>

class Tracer {
protected:
    typedef struct Info {
        int x;
        int y;
        int direction;
        int distance;
    } Info;

    typedef struct Node {
        Info* info;
        Node* next;
    } Node;

    Node* head;
    Node* tail;

    void Add(int x, int y, int dir, int dist);

    char* map;

    PuckMaze* maze;

public:
    Tracer(PuckMaze* maze);
    ~Tracer();
    int Find(int x, int y, Uint32 element, int len = 0);
    char* FindPath(int x, int y, Uint32 element);
};

class NEWTracer : public Tracer {
private:
    int _maxDist;

public:
    NEWTracer(PuckMaze* maze) :
        Tracer(maze),
        _maxDist(16) {
        ;
    }

    int Find2(int x, int y, Uint32 element);
};

#endif
