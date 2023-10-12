#pragma once
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
#include <string>

#include <GLTexture.hpp>
#include <GLBitmapFont.hpp>
#include <TextInput.hpp>
#include "VideoBase.hpp"

#ifndef IPHONE
const int VIDEO_DEFAULT_WIDTH = 640;
const int VIDEO_DEFAULT_HEIGHT = 480;
#else
const int VIDEO_DEFAULT_WIDTH = 320;
const int VIDEO_DEFAULT_HEIGHT = 480;
#endif

const int VIDEO_ORTHO_WIDTH = 1000;
const int VIDEO_ORTHO_HEIGHT = 750;

class Buffer;
class VertexArray;

class CherriesView : public ResolutionChangeObserverI {
public:
    CherriesView(void);
    virtual ~CherriesView();

    bool init(void);
    bool draw(void);
    void updateLogic(void);

    void toggleCritterBoard(void) { _boardVisible = !_boardVisible; }
    virtual void resolutionChanged(int w, int h);

private:
    CherriesView(const CherriesView&);
    CherriesView& operator=(const CherriesView&);

    void initGL3Test();

    GLBitmapFont* _smallFont;
    GLBitmapFont* _scoreFont;
    GLBitmapFont* _gameOFont;

    GLBitmapCollection* _board;
    int _boardIndex;
    bool _boardVisible;
    float _boardPosX;
    int _titleIndex;
    int _frenzy;

    std::string _statusMsg;
    bool _statusMsgActive;
    float _statusMsgWidth;
    float _statusMsgPos;

    float _angle;
    float _prevAngle;

    TextInput _textInput;

    int _numStarVertices;
    GLfloat* _starVertices;

    Buffer* _shaftVerts;
    Buffer* _shaftNormals;
    Buffer* _shaftVindices;

    VertexArray* _shaftVao;
};
