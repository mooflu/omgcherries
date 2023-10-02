#pragma once
// Description:
//
// Copyright (C) 2001-2008 Frank Becker
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

#include <Maze.hpp>
#include <Singleton.hpp>
#include <GLTexture.hpp>

#include "SDL.h"

//the pacmaze class add the screen handling, and adds some new elements
//(in addition to the walls).
class PuckMaze : public Maze {
    friend class Singleton<PuckMaze>;

private:
    int _points;
    GLTexture* _maze;
    bool _hasTexRectExt;
    int _cellSize;
    char* _cellBuf;

    void AddPoints(void);

    void Redraw(SDL_Surface* img, int X, int Y, int W, int H);

public:
    PuckMaze();
    virtual ~PuckMaze();

    void init(int width, int height, int cellSize);
    void reset(void);
    void draw(const float& x, const float& y, const float& z, const float& scalex, const float& scaley);

    void AddPowerpoints(int numPoints);

    void ClearPoint(int x, int y) {
        RemoveElement(x, y, CHERRY);
        _points--;
    }

    void UpdateTexture(void);

    int CellSize(void) { return _cellSize; }

    int Points(void) { return (_points); }
};

typedef Singleton<PuckMaze> PuckMazeS;
