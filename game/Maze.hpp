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
#ifndef __Maze_hpp__
#define __Maze_hpp__

// Maze Class

//This base maze class privides only the actual maze
//(to be thought of as a collection of walls) and some
//primitive info functions, all of which are inlined.
//Note that the bit representation for the walls
//(and other stuff) restricts what can be done in
//derived classes, since in the end everything has to
//fit into a 32-bit field.
//A better way might be to do the bit assignment at constructor time,
//oh well...

#include <SDL2/SDL_stdinc.h>
#include <Trace.hpp>

enum MazeElements {
    // element bit representation for walls
    WallUP = 1 << 0,
    WallDN = 1 << 1,
    WallLT = 1 << 2,
    WallRT = 1 << 3,

    // element bit representation for other elements
    ENEMY = 1 << 4,
    CHERRY = 1 << 5,

    //powerpoints make you stronger than the enemy
    POWERPOINT = 1 << 6,
};

class Maze {
protected:
    int* y2off;
    Uint32* map;

    int width;
    int height;

    void Create(void);
    void Simplify(void);

public:
    Maze(void);

    virtual ~Maze();

    void init(int width, int height);
    void reset(void);

    int Width(void) { return (width); }

    int Height(void) { return (height); }

    bool isElement(int x, int y, Uint32 element) {
        if ((x < 0) || (x >= width) || (y < 0) || (y >= height)) {
            LOG_ERROR << "isElement out of bounds " << x << "," << y << "\n";
            return false;
        }
        return ((map[y2off[y] + x] & element) != 0);
    }

    bool isInside(int x, int y) { return !((x < 0) || (x >= width) || (y < 0) || (y >= height)); }

    void RemoveElement(int x, int y, Uint32 element) { map[y2off[y] + x] &= ~element; }

    void AddElement(int x, int y, Uint32 element) { map[y2off[y] + x] |= element; }
};

#endif
