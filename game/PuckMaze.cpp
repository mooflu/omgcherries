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
#include <Trace.hpp>
#include <RandomKnuth.hpp>
#include <PuckMaze.hpp>

#include "GLVertexBufferObject.hpp"

#include "vmmlib/vector.hpp"
using namespace vmml;

#include <string>
using namespace std;

static RandomKnuth _random;

//make new maze and add elements
PuckMaze::PuckMaze(void) :
    _maze(0),
    _points(0),
    _cellSize(4),
    _cellBuf(0) {
    init(10, 10, 5);
}

PuckMaze::~PuckMaze() {
    delete[] _cellBuf;
    delete _maze;
}

void PuckMaze::init(int w, int h, int cellSize) {
    LOG_INFO << "Maze size: " << w << "x" << h << "\n";
    _cellSize = cellSize;
    delete[] _cellBuf;
    _cellBuf = new char[_cellSize * _cellSize];
    Maze::init(w, h);
    reset();
}

//add elements
void PuckMaze::AddPoints(void) {
    int i;
    for (i = 0; i < width * height; i++) {
        map[i] |= CHERRY;
    }
    _points = width * height;
}

void PuckMaze::AddPowerpoints(int numPoints) {
    for (int i = 0; i < numPoints; i++) {
        int pos = _random.random() % (width * height);
        map[pos] |= POWERPOINT;
    }
}

//redo the maze
void PuckMaze::reset(void) {
    Maze::reset();
    AddPoints();
    UpdateTexture();
}

static inline void SetPixel(SDL_Surface* img, int x, int y, int c) {
    char* data = (char*)img->pixels;
    data[y * img->pitch + x * 4 + 0] = c;
    data[y * img->pitch + x * 4 + 1] = c;
    data[y * img->pitch + x * 4 + 2] = c;
    data[y * img->pitch + x * 4 + 3] = 255;
}

void PuckMaze::Redraw(SDL_Surface* img, int X, int Y, int W, int H) {
    int BGCOLOR = 0;
    int WALLCOLOR = 255;

    int pos = 0;
    int x, y;

    if ((Y + H) > height) {
        H = height - Y;
    }
    if ((X + W) > width) {
        W = width - X;
    }

    int maxPos = width * height;

    for (y = Y; y < (Y + H); y++) {
        pos = y * width + X;
        for (x = X; x < (X + W); x++) {
            int cellPixelCount = _cellSize * _cellSize;
            char* c = _cellBuf;
            for (int i = 0; i < cellPixelCount; i++) {
                c[i] = BGCOLOR;
            }

            if (map[pos] & WallDN) {
                for (int i = 1; i <= _cellSize; i++) {
                    c[cellPixelCount - i] = WALLCOLOR;
                }
            }
            if (map[pos] & WallRT) {
                for (int i = 0; i < _cellSize; i++) {
                    c[_cellSize - 1 + i * _cellSize] = WALLCOLOR;
                }
            }
            if ((map[pos + 1] & WallDN) || ((pos + width) < maxPos) && (map[pos + width] & WallRT)) {
                c[cellPixelCount - 1] = WALLCOLOR;
            }

            for (int yy = 0; yy < _cellSize; yy++) {
                for (int xx = 0; xx < _cellSize; xx++) {
                    SetPixel(img, x * _cellSize + xx + 1, y * _cellSize + yy + 1, c[yy * _cellSize + xx]);
                }
            }

            pos++;
        }
    }

    pos = Y * width;
    for (y = Y; y < (Y + H); y++) {
        if (map[pos] & WallLT) {
            for (int i = 0; i < _cellSize; i++) {
                SetPixel(img, 0, y * _cellSize + i, WALLCOLOR);
            }
        }
        pos += width;
    }

    pos = X;
    for (x = X; x < (X + W); x++) {
        if (map[pos] & WallUP) {
            for (int i = 0; i < _cellSize; i++) {
                SetPixel(img, x * _cellSize + i, 0, WALLCOLOR);
            }
        }
        pos++;
    }
}

//draw the maze...
void PuckMaze::UpdateTexture(void) {
    delete _maze;
    _maze = 0;

#ifdef IPHONE
    string extensions = (char*)glGetString(GL_EXTENSIONS);
    _hasTexRectExt = extensions.find("GL_OES_draw_texture") != string::npos;
#endif

    SDL_Surface* img = SDL_CreateRGBSurface(SDL_SWSURFACE, 512, 512, 8 * 4, 0, 0, 0, 1);

    Redraw(img, 0, 0, width, height);

    _maze = new GLTexture(GL_TEXTURE_2D, img, false);
}

void PuckMaze::draw(const float& x, const float& y, const float& z, const float& scalex, const float& scaley) {
    float bw = width * _cellSize + 1;
    float bh = height * _cellSize + 1;
    float textureSize = 512;

    //glEnable(GL_TEXTURE_2D);
    _maze->bind();
#ifdef IPHONE
    if (false)  //_hasTexRectExt)
    {
        //TODO: fix offset/rotation
        GLint cropRect[4];
        cropRect[0] = 0;
        cropRect[1] = 0;
        cropRect[2] = bw;
        cropRect[3] = bh;
        glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_CROP_RECT_OES, cropRect);
        glDrawTexsOES(0, 0, 0, bw, bh);
    } else
#endif
    {
        float tx, ty, fxsize, fysize;
        float dxsize, dysize;

        dxsize = (float)bw * scalex;
        dysize = (float)bh * scaley;
        fxsize = (float)bw / textureSize;
        fysize = (float)bh / textureSize;
        tx = (float)0 / textureSize;
        ty = (float)0 / textureSize;

#if 0
        GLfloat squareVertices[] = {
            x       ,y+dysize ,z,
            x+dxsize,y+dysize ,z,
            x+dxsize,y        ,z,
            x       ,y        ,z
        };

        GLfloat squareTexCoords[] = {
            tx       ,ty+fysize,
            tx+fxsize,ty+fysize,
            tx+fxsize,ty,
            tx       ,ty
        };

        glVertexPointer(3, GL_FLOAT, 0, squareVertices);
        glTexCoordPointer(2, GL_FLOAT, 0, squareTexCoords);

        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);

        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
#else
        vec4f v[4] = {
            vec4f(x, y + dysize, z, 1),
            vec4f(x + dxsize, y + dysize, z, 1),
            vec4f(x + dxsize, y, z, 1),
            vec4f(x, y, z, 1),
        };
        vec2f t[4] = {
            vec2f(tx, ty + fysize),
            vec2f(tx + fxsize, ty + fysize),
            vec2f(tx + fxsize, ty),
            vec2f(tx, ty),
        };

        GLVBO vbo;
        vbo.setColor(1.0f, 1.0f, 1.0f, 1.0f);
        vbo.DrawTexQuad(v, t);
#endif
    }
    //glDisable(GL_TEXTURE_2D);
}
