// Description:c
//   A Selectable
//
// Copyright (C) 2010 Frank Becker
//
#include "Selectable.hpp"

#include "Trace.hpp"
#include "Tokenizer.hpp"
#include "Config.hpp"
#include "Value.hpp"
#include "Game.hpp"
#include "GameState.hpp"
#include "Audio.hpp"
#include "Input.hpp"
#include "Trigger.hpp"
#include "MenuManager.hpp"
#include "FontManager.hpp"
#include "BitmapManager.hpp"
#include "ScoreKeeper.hpp"
#include "Constants.hpp"
#include "GetDataPath.hpp"

#include <GL/glew.h>
#include "GLVertexBufferObject.hpp"

#include <algorithm>
using namespace std;

Selectable* Selectable::_active = 0;

Selectable::Selectable(bool enabled, const BoundingBox& r, const string& info) :
    _enabled(enabled),
    _inputBox(r),
    _boundingBox(r),
    _info(info),
    _fontWhite(0) {
    XTRACE();
#ifndef DEMO
    _enabled = true;
#endif
    _fontWhite = FontManagerS::instance()->getFont("bitmaps/menuWhite");
    if (!_fontWhite) {
        LOG_ERROR << "Unable to load menuWhite font." << endl;
    }
}

Selectable::~Selectable() {
    XTRACE();
}

void Selectable::draw(const Point2Di& /*offset*/) {
#if 0
    glColor4f( 1.0, 0.2, 0.2, 0.5);
    vec3f v[4] = {
        vec3f( _inputBox.min.x+offset.x, _inputBox.min.y+offset.y, 0 ),
        vec3f( _inputBox.min.x+offset.x, _inputBox.max.y+offset.y, 0 ),
        vec3f( _inputBox.max.x+offset.x, _inputBox.max.y+offset.y, 0 ),
        vec3f( _inputBox.max.x+offset.x, _inputBox.min.y+offset.y, 0 ),
    };
    GLVBO::DrawQuad( GL_TRIANGLE_FAN, v);
#endif

    if (_active == this) {
        //glColor4f(1.0f, 1.0f, 1.0f, 0.6f);
        //_fontWhite->DrawString( _info.c_str(), offset.x+34, offset.y+23, 0.8f, 0.8f);
        //_fontWhite->DrawString( _info.c_str(), 0, 0, 1.0f, 1.0f);
    }
}

void Selectable::updateActive(UserFeedback feedback) {
    if ((_active != this)) {
        if (_active) {
            _active->deactivate();
        }
        _active = this;
        MenuManagerS::instance()->Goto(this);

        switch (feedback) {
            case eBeep:
#ifndef IPHONE
                AudioS::instance()->playSample("sounds/beep.wav");
#endif
                break;

            case eTick:
                AudioS::instance()->playSample("sounds/click1");
                break;

            case eNoFeedback:
            default:
                break;
        }
    }
}

//------------------------------------------------------------------------------

EscapeSelectable::EscapeSelectable(bool enabled, const BoundingBox& r, float size) :
    Selectable(enabled, r, "Escape"),
    _size(size),
    _icons(0),
    _exitOn(0),
    _exitOff(0) {
    _icons = BitmapManagerS::instance()->getBitmap("bitmaps/atlas");  //"bitmaps/menuIcons");
    if (!_icons) {
        LOG_ERROR << "Unable to load menuIcons." << endl;
    }

    _exitOn = _icons->getIndex("cherry");
    if (_exitOn == -1) {
        LOG_ERROR << "ExitOn button not found" << endl;
    }

    _exitOff = _icons->getIndex("cherry");
    if (_exitOff == -1) {
        LOG_ERROR << "ExitOff button not found" << endl;
    }

    _fontShadow = FontManagerS::instance()->getFont("bitmaps/menuShadow");
    if (!_fontShadow) {
        LOG_ERROR << "Unable to load shadow font." << endl;
    }

    _boundingBox.max.x = _boundingBox.min.x + _icons->getWidth(_exitOn) * _size;
    _boundingBox.max.y = _boundingBox.min.y + _icons->getHeight(_exitOn) * _size;

    _inputBox = _boundingBox;
}

void EscapeSelectable::select(void) {
    MenuManagerS::instance()->Exit(true);
}

void EscapeSelectable::input(const Trigger& trigger, const bool& isDown, const Point2Di& /*offset*/) {
    if (!_enabled) {
        return;
    }

    if (!isDown) {
        return;
    }

    switch (trigger.type) {
        case eButtonTrigger:
#ifdef IPHONE
            this->activate();
#endif
            MenuManagerS::instance()->Exit(true);
            break;

        case eMotionTrigger:
            this->activate();
            break;

        default:
            break;
    }
}

void EscapeSelectable::activate(bool beQuiet) {
    updateActive(beQuiet ? eNoFeedback : eBeep);
}

void EscapeSelectable::draw(const Point2Di& offset) {
    Selectable::draw(offset);

    _icons->bind();
    //glColor4f(1.0, 1.0, 1.0, 1.0);
    //glEnable(GL_TEXTURE_2D);
    _icons->setColor(1.0, 1.0, 1.0, 1.0);
    if ((_active == this)) {
        _icons->Draw(_exitOn, _boundingBox.min.x + offset.x, _boundingBox.min.y + offset.y, _size, _size);
    } else {
        _icons->Draw(_exitOff, _boundingBox.min.x + offset.x, _boundingBox.min.y + offset.y, _size, _size);
    }

    //glColor4f(1.0f,0.852f,0.0f,1.0f);
    float size = 1.2;

    string text = "Back";
    _fontShadow->setColor(1.0f, 0.852f, 0.0f, 1.0f);
    _fontShadow->DrawString(text.c_str(), _boundingBox.min.x + offset.x + 30 + FONTSHADOW_OFFSET * size,
                            _boundingBox.min.y + offset.y + 60 - FONTSHADOW_OFFSET * size, size, size);

    _fontWhite->setColor(1.0f, 0.852f, 0.0f, 1.0f);
    _fontWhite->DrawString(text.c_str(), _boundingBox.min.x + offset.x + 30, _boundingBox.min.y + offset.y + 60, size,
                           size);

    //glDisable(GL_TEXTURE_2D);
}

//------------------------------------------------------------------------------

TextOnlySelectable::TextOnlySelectable(bool enabled, const BoundingBox& rect, const string& text, const string& info,
                                       bool center, float size, float red, float green, float blue) :

    Selectable(enabled, rect, info),
    _text(text),
    _fontShadow(0),
    _icons(0),
    r(red),
    g(green),
    b(blue),
    _size(size) {
    if (text == "VERSION") {
        r = 0.5;
        g = 0.5;
        b = 0.5;
        _text = "Version " + GAMEVERSION;
    }
    _fontShadow = FontManagerS::instance()->getFont("bitmaps/menuShadow");
    if (!_fontShadow) {
        LOG_ERROR << "Unable to load shadow font." << endl;
    }

    float width = _fontWhite->GetWidth(_text.c_str(), _size);
    float height = _fontWhite->GetHeight(_size);

    if (center) {
        _boundingBox.min.x -= width * 0.5f;
    }
    _boundingBox.max.x = _boundingBox.min.x + width;
    _boundingBox.max.y = _boundingBox.min.y + height;

    _inputBox = _boundingBox;

    _icons = BitmapManagerS::instance()->getBitmap("bitmaps/atlas");  //"bitmaps/menuIcons");
    if (!_icons) {
        LOG_ERROR << "Unable to load menuIcons." << endl;
    }
}

void TextOnlySelectable::input(const Trigger& trigger, const bool& /*isDown*/, const Point2Di& /*offset*/) {
    if (!_enabled) {
        return;
    }

    switch (trigger.type) {
#ifdef IPHONE
        case eButtonTrigger:
            this->activate();
            break;
#endif
        case eMotionTrigger:
            this->activate();
            break;

        default:
            break;
    }
}

void TextOnlySelectable::activate(bool /*beQuiet*/) {
    updateActive(eNoFeedback);
}

void TextOnlySelectable::draw(const Point2Di& offset) {
    Selectable::draw(offset);

    vec4f color(1.0, 1.0, 1.0, 1.0);

    _fontShadow->setColor(color);
    _fontShadow->DrawString(_text.c_str(), _boundingBox.min.x + offset.x + FONTSHADOW_OFFSET * _size,
                            _boundingBox.min.y + offset.y - FONTSHADOW_OFFSET * _size, _size, _size);

    if (!_enabled) {
        color = vec4f(0.5, 0.5, 0.5, 1.0);
    } else {
        color = vec4f(r, g, b, 1.0);
    }

    _fontWhite->setColor(color);
    _fontWhite->DrawString(_text.c_str(), _boundingBox.min.x + offset.x, _boundingBox.min.y + offset.y, _size, _size);
}

//------------------------------------------------------------------------------

ImageOnlySelectable::ImageOnlySelectable(bool enabled, const BoundingBox& rect, const string& text, const string& info,
                                         bool center, float size, float red, float green, float blue) :

    Selectable(enabled, rect, info),
    _text(text),
    _fontShadow(0),
    _icons(0),
    _image(0),
    r(red),
    g(green),
    b(blue),
    _size(size) {
    _fontShadow = FontManagerS::instance()->getFont("bitmaps/menuShadow");
    if (!_fontShadow) {
        LOG_ERROR << "Unable to load shadow font." << endl;
    }

    _icons = BitmapManagerS::instance()->getBitmap("bitmaps/atlas");  //"bitmaps/menuIcons");
    if (!_icons) {
        LOG_ERROR << "Unable to load menuIcons." << endl;
    }
    _image = _icons->getIndex(_text);

    float width = _icons->getWidth(_image);
    float height = _icons->getHeight(_image);

    if (center) {
        _boundingBox.min.x -= width * 0.5f;
        _boundingBox.min.y -= height * 0.5f;
    }

    _boundingBox.max.x = _boundingBox.min.x + width;
    _boundingBox.max.y = _boundingBox.min.y + height;

    _inputBox = _boundingBox;
}

void ImageOnlySelectable::input(const Trigger& trigger, const bool& /*isDown*/, const Point2Di& /*offset*/) {
    if (!_enabled) {
        return;
    }

    switch (trigger.type) {
#ifdef IPHONE
        case eButtonTrigger:
            this->activate();
            break;
#endif
        case eMotionTrigger:
            this->activate();
            break;

        default:
            break;
    }
}

void ImageOnlySelectable::activate(bool /*beQuiet*/) {
    updateActive(eNoFeedback);
}

void ImageOnlySelectable::draw(const Point2Di& offset) {
    //Selectable::draw(offset);

    //glColor4f(1.0, 1.0, 1.0, 1.0);

    //glEnable(GL_TEXTURE_2D);
    _icons->bind();
    _icons->setColor(1.0, 1.0, 1.0, 1.0);
    _icons->DrawC(_image, _boundingBox.min.x + offset.x, _boundingBox.min.y + offset.y, _size, _size);
    //glDisable(GL_TEXTURE_2D);
}

//------------------------------------------------------------------------------

FloatSelectable::FloatSelectable(bool enabled, const BoundingBox& rect, const string& text, const string& info,
                                 const string& variable, const string& range, const string& sliderOffset) :

    TextOnlySelectable(enabled, rect, text, info, false, 0.7f, 1.0f, 1.0f, 1.0f),
    _variable(variable),
    _startX(-1.0f) {
    Tokenizer t(range);

    _min = (float)atof(t.next().c_str());
    _max = (float)atof(t.next().c_str());

    _sliderOffset = atoi(sliderOffset.c_str());

    float curVal = _min;
    ConfigS::instance()->getFloat(_variable, curVal);
    _xPos = (curVal - _min) * 140.0f / (_max - _min);

    //bounding box for double arrow
    _bRect.min.x = _boundingBox.min.x + 102 + _sliderOffset;
    _bRect.max.x = _bRect.min.x + 20;
    _bRect.min.y = _boundingBox.min.y;
    _bRect.max.y = _bRect.min.y + 30;

    _slider = _icons->getIndex("Slider");
    if (_slider == -1) {
        LOG_ERROR << "Slider not found" << endl;
    }
    _doubleArrow = _icons->getIndex("DoubleArrow");
    if (_doubleArrow == -1) {
        LOG_ERROR << "DoubleArrow not found" << endl;
    }

    _boundingBox.max.x = _bRect.min.x + _icons->getWidth(_slider) * 1.6f;

    _inputBox = _boundingBox;
}

void FloatSelectable::incBy(float delta) {
    float curVal = _min;
    ConfigS::instance()->getFloat(_variable, curVal);
    curVal += delta;
    Clamp(curVal, _min, _max);

    LOG_INFO << "New val = " << curVal << "for " << _variable << "\n";
    _xPos = (curVal - _min) * 140.0f / (_max - _min);

    Value* v = new Value(curVal);
    ConfigS::instance()->updateKeyword(_variable, v);
    AudioS::instance()->playSample("sounds/tick1");
}

void FloatSelectable::input(const Trigger& trigger, const bool& isDown, const Point2Di& offset) {
    if (!_enabled) {
        return;
    }

    float mouseX = trigger.fData1;
    float mouseY = trigger.fData2;

    switch (trigger.type) {
        case eKeyTrigger:
            if (isDown && (_active == this)) {
                switch (trigger.data1) {
                    case SDLK_LEFT:
                        incBy(-0.1);
                        break;

                    case SDLK_RIGHT:
                        incBy(0.1);
                        break;

                    default:
                        break;
                }
            }
            break;

        case eButtonTrigger:
#ifdef IPHONE
            this->activate();
#endif
            if (isDown) {
                if ((mouseX >= (_bRect.min.x + offset.x + _xPos)) && (mouseX <= (_bRect.max.x + offset.x + _xPos)) &&
                    (mouseY >= (_bRect.min.y + offset.y)) && (mouseY <= (_bRect.max.y + offset.y))) {
                    _startX = mouseX;
                }
            } else {
                _startX = -1.0f;
                float curVal = _min;
                ConfigS::instance()->getFloat(_variable, curVal);
                curVal = _min + _xPos * (_max - _min) / 140.0f;
                //LOG_INFO << "New val = " << curVal << "\n";

                Value* v = new Value(curVal);
                ConfigS::instance()->updateKeyword(_variable, v);
            }
            break;

        case eMotionTrigger:
            if (_startX >= 0.0f) {
                float dx = mouseX - _startX;
                _startX = mouseX;

                _xPos += dx;
                Clamp(_xPos, 0.0, 140.0);
            }

            if ((mouseX >= (_boundingBox.min.x + offset.x)) && (mouseX <= (_boundingBox.max.x + offset.x)) &&
                (mouseY >= (_boundingBox.min.y + offset.y)) && (mouseY <= (_boundingBox.max.y + offset.y))) {
                this->activate();
            }
            break;

        default:
            break;
    }
}

void FloatSelectable::activate(bool beQuiet) {
    updateActive(beQuiet ? eNoFeedback : eBeep);
}

void FloatSelectable::draw(const Point2Di& offset) {
    TextOnlySelectable::draw(offset);

    _icons->bind();
    vec4f color;
    if (_active == this) {
        if (!_enabled) {
            color = vec4f(0.7, 0.7, 0.7, 1.0);
        } else {
            color = vec4f(1.0f, 0.0f, 0.0f, 1.0f);
        }
    } else {
        if (!_enabled) {
            color = vec4f(0.5, 0.5, 0.5, 1.0);
        } else {
            color = vec4f(1.0, 1.0, 1.0, 1.0);
        }
    }

    _icons->setColor(color);
    _icons->DrawC(_slider, _boundingBox.min.x + offset.x + 180 + _sliderOffset, _boundingBox.min.y + offset.y + 15,
                  1.6f, 0.4f);
    _icons->DrawC(_doubleArrow, _boundingBox.min.x + offset.x + 112 + _sliderOffset + _xPos,
                  _boundingBox.min.y + offset.y + 15, 0.8f, 0.8f);

    float curVal = _min + _xPos * (_max - _min) / 140.0f;
    char valStr[10];
    sprintf(valStr, "%2.2f", curVal);
    _fontWhite->DrawString(valStr, _boundingBox.max.x + offset.x,
        _boundingBox.min.y + offset.y + 5, 0.5f, 0.5f);
}

//------------------------------------------------------------------------------

EnumSelectable::EnumSelectable(bool enabled, const BoundingBox& rect, const string& text, const string& info,
                               const string& variable, const string& values) :

    TextOnlySelectable(enabled, rect, text, info, false, 2.0f, 1.0f, 1.0f, 1.0f),
    _variable(variable) {
    _activeEnum = 0;
    ConfigS::instance()->getInteger(_variable, _activeEnum);

    Tokenizer t(values, ":");
    string s = t.next();
    float maxW = 0;
    while (s != "") {
        _enumList.insert(_enumList.end(), s);
        float w = _fontWhite->GetWidth(s.c_str(), _size);
        if (w > maxW) {
            maxW = w;
        }
        s = t.next();
    }

    _xOff = _boundingBox.max.x + 5.0f;
    _boundingBox.max.x += maxW;

    _inputBox = _boundingBox;
}

void EnumSelectable::nextEnum(void) {
    _activeEnum++;
    if (_activeEnum == _enumList.size()) {
        _activeEnum = 0;
    }

    Value* v = new Value(_activeEnum);
    ConfigS::instance()->updateKeyword(_variable, v);
    AudioS::instance()->playSample("sounds/click1");
}

void EnumSelectable::prevEnum(void) {
    if (_activeEnum == 0) {
        _activeEnum = _enumList.size();
    }
    _activeEnum--;

    Value* v = new Value(_activeEnum);
    ConfigS::instance()->updateKeyword(_variable, v);
    AudioS::instance()->playSample("sounds/click1");
}

void EnumSelectable::input(const Trigger& trigger, const bool& isDown, const Point2Di& /*offset*/) {
    if (!_enabled) {
        return;
    }

    if (!isDown) {
        return;
    }

    switch (trigger.type) {
        case eKeyTrigger:
            switch (trigger.data1) {
                case SDLK_LEFT:
                    prevEnum();
                    break;

                case SDLK_RIGHT:
                    nextEnum();
                    break;

                default:
                    break;
            }
            break;

        case eButtonTrigger: {
#ifdef IPHONE
            this->activate();
#endif
            if (trigger.data1 == SDL_MOUSEWHEEL) {
                if (trigger.data3 > 0) {
                    nextEnum();
                } else {
                    prevEnum();
                }
            }
        } break;

        case eMotionTrigger:
            this->activate();
            break;

        default:
            break;
    }
}

void EnumSelectable::activate(bool beQuiet) {
    updateActive(beQuiet ? eNoFeedback : eBeep);
}

void EnumSelectable::draw(const Point2Di& offset) {
    TextOnlySelectable::draw(offset);

    string val = _enumList[_activeEnum];

    vec4f color(1.0, 1.0, 1.0, 1.0);
    //glColor4f(1.0, 1.0, 1.0, 1.0);
    _fontShadow->setColor(color);
    _fontShadow->DrawString(val.c_str(), _xOff + offset.x + FONTSHADOW_OFFSET * _size,
                            _boundingBox.min.y + offset.y - FONTSHADOW_OFFSET * _size, _size, _size);

    if (_active == this) {
        if (!_enabled) {
            color = vec4f(0.7, 0.7, 0.7, 1.0);
        } else {
            color = vec4f(1.0f, 0.0f, 0.0f, 1.0f);
        }
    } else {
        if (!_enabled) {
            color = vec4f(0.5, 0.5, 0.5, 1.0);
        } else {
            color = vec4f(1.0f, 0.852f, 0.0f, 1.0f);
        }
    }

    _fontWhite->setColor(color);
    _fontWhite->DrawString(val.c_str(), _xOff + offset.x, _boundingBox.min.y + offset.y, _size, _size);
}

//------------------------------------------------------------------------------

BoolSelectable::BoolSelectable(bool enabled, const BoundingBox& rect, const string& text, const string& info,
                               const string& variable) :

    TextOnlySelectable(enabled, rect, text, info, false, 2.0f, 1.0f, 1.0f, 1.0f),
    _variable(variable) {
    _checkmark = _icons->getIndex("Checkmark");
    _checkmarkOff = _icons->getIndex("CheckmarkOff");

    _xOff = _boundingBox.max.x + 10.0f;
    _boundingBox.max.x = _xOff + _icons->getWidth(_checkmark) * _size;

    _inputBox = _boundingBox;
}

void BoolSelectable::select(void) {
    if (!_enabled) {
        return;
    }

    toggle();
}

void BoolSelectable::toggle(void) {
    bool val = false;
    ConfigS::instance()->getBoolean(_variable, val);
    Value* v = new Value(!val);

    //    LOG_INFO << "New value for " << _variable
    //             << " is " << v->getString() << endl;

    ConfigS::instance()->updateKeyword(_variable, v);
    AudioS::instance()->playSample("sounds/click1");
}

void BoolSelectable::input(const Trigger& trigger, const bool& isDown, const Point2Di& /*offset*/) {
    if (!_enabled) {
        return;
    }

    if (!isDown) {
        return;
    }

    switch (trigger.type) {
        case eButtonTrigger: {
#ifdef IPHONE
            this->activate();
#endif
            toggle();
        } break;

        case eMotionTrigger:
            this->activate();
            break;

        default:
            break;
    }
}

void BoolSelectable::activate(bool beQuiet) {
    updateActive(beQuiet ? eNoFeedback : eBeep);
}

void BoolSelectable::draw(const Point2Di& offset) {
    TextOnlySelectable::draw(offset);

    bool val = false;
    ConfigS::instance()->getBoolean(_variable, val);

    _icons->bind();
    vec4f color(1.0, 1.0, 1.0, 1.0);
    if (_active == this) {
        if (!_enabled) {
            color = vec4f(0.7, 0.7, 0.7, 1.0);
        } else {
            color = vec4f(1.0f, 0.0f, 0.0f, 1.0f);
        }
    } else {
        if (!_enabled) {
            color = vec4f(0.5, 0.5, 0.5, 1.0);
        } else {
            color = vec4f(1.0, 1.0, 1.0, 1.0);
        }
    }

    //glEnable(GL_TEXTURE_2D);
    _icons->setColor(color);
    if (val) {
        _icons->Draw(_checkmark, _xOff + offset.x, _boundingBox.min.y + offset.y + 2, _size * 5.0 / 7.0,
                     _size * 5.0 / 7.0);
    } else {
        _icons->Draw(_checkmarkOff, _xOff + offset.x, _boundingBox.min.y + offset.y + 2, _size * 5.0 / 7.0,
                     _size * 5.0 / 7.0);
    }
    //glDisable(GL_TEXTURE_2D);
}

//------------------------------------------------------------------------------

LeaderBoardSelectable::LeaderBoardSelectable(bool enabled, const BoundingBox& r, const string& text,
                                             const string& info) :

    Selectable(enabled, r, info),
    _text(text),
    _size(1.0f) {
    _fontShadow = FontManagerS::instance()->getFont("bitmaps/menuShadow");

    float width = _fontWhite->GetWidth(_text.c_str(), _size);
    float height = _fontWhite->GetHeight(_size);

    _boundingBox.min.x -= width * 0.5f;
    _boundingBox.max.x = _boundingBox.min.x + width;
    _boundingBox.max.y = _boundingBox.min.y + height;

    _inputBox.min.x = 0;
    _inputBox.min.y = 30;
    _inputBox.max.x = 700;
    _inputBox.max.y = 605;

    ScoreKeeperS::instance()->setActive("");
}

LeaderBoardSelectable::~LeaderBoardSelectable() {}

void LeaderBoardSelectable::input(const Trigger& trigger, const bool& isDown, const Point2Di& offset) {
    if (!_enabled) {
        return;
    }

    if (!isDown) {
        return;
    }

    float mouseX = trigger.fData1;
    float mouseY = trigger.fData2;

    switch (trigger.type) {
#ifdef IPHONE
        case eButtonTrigger:
            if (trigger.data1 == SDL_BUTTON(1)) {
                if (ScoreKeeperS::instance()->numBoards() > 1) {
                    if ((mouseY > 505) && (mouseY < 605)) {
                        if ((mouseX > 50) && (mouseX < 150)) {
                            ScoreKeeperS::instance()->prevBoard();
                            AudioS::instance()->playSample("sounds/click1");
                        }
                        if ((mouseX > 580) && (mouseX < 680)) {
                            ScoreKeeperS::instance()->nextBoard();
                            AudioS::instance()->playSample("sounds/click1");
                        }
                    }
                }
            }
            break;
#else
        case eKeyTrigger:
            switch (trigger.data1) {
                case SDLK_LEFT:
                    ScoreKeeperS::instance()->prevBoard();
                    AudioS::instance()->playSample("sounds/click1");
                    break;

                case SDLK_RIGHT:
                    ScoreKeeperS::instance()->nextBoard();
                    AudioS::instance()->playSample("sounds/click1");
                    break;

                default:
                    break;
            }
            break;

        case eButtonTrigger:
            if (trigger.data1 == -99)  //TODO: handle mouse wheel - sdl1 SDL_BUTTON_WHEELDOWN)
            {
                ScoreKeeperS::instance()->prevBoard();
                AudioS::instance()->playSample("sounds/click1");

            } else {
                ScoreKeeperS::instance()->nextBoard();
                AudioS::instance()->playSample("sounds/click1");
            }
            break;
#endif

        case eMotionTrigger: {
            float spacing = 36.0f;

            int idx = (int)((mouseY + offset.y - 145.0f) / spacing);
            if (idx < 0) {
                idx = 0;
            }
            if (idx > 9) {
                idx = 9;
            }
            _info = ScoreKeeperS::instance()->getInfoText(9 - idx);
            //                LOG_INFO << "spot = " << idx << endl;
        }
            this->activate();
            break;

        default:
            break;
    }
}

void LeaderBoardSelectable::activate(bool /*beQuiet*/) {
    updateActive(eNoFeedback);
}

void LeaderBoardSelectable::draw(const Point2Di& offset) {
#if 0
    glColor4f( 0.2, 0.2, 1.0, 0.5);
    vec3f v[4] = {
        vec3f( _inputBox.min.x+offset.x, _inputBox.min.y+offset.y, 0 ),
        vec3f( _inputBox.min.x+offset.x, _inputBox.max.y+offset.y, 0 ),
        vec3f( _inputBox.max.x+offset.x, _inputBox.max.y+offset.y, 0 ),
        vec3f( _inputBox.max.x+offset.x, _inputBox.min.y+offset.y, 0 ),
    };
    GLVBO::DrawQuad( GL_TRIANGLE_FAN, v);
#endif

    Selectable::draw(offset);
#if 0
    //Draw "Leader Board:" sub-header
    glColor4f(1.0, 1.0, 1.0, 1.0);
    _fontShadow->DrawString(
        _text.c_str(),
        _boundingBox.min.x + offset.x+FONTSHADOW_OFFSET*_size,
        _boundingBox.min.y + offset.y-FONTSHADOW_OFFSET*_size,
        _size, _size);

    glColor4f(1.0, 1.0, 1.0, 1.0);
    _fontWhite->DrawString(
        _text.c_str(),
        _boundingBox.min.x + offset.x,
        _boundingBox.min.y + offset.y,
        _size, _size);
#endif
    ScoreKeeperS::instance()->draw(offset);
}

//------------------------------------------------------------------------------

bool ResolutionSelectable::DescendingAreaOrder(const Resolution& res1, const Resolution& res2) {
    return res1.width * res1.height < res2.width * res2.height;
}

ResolutionSelectable::ResolutionSelectable(bool enabled, const BoundingBox& rect, const string& text,
                                           const string& info) :

    TextOnlySelectable(enabled, rect, text, info, false, 0.7f, 1.0f, 1.0f, 1.0f) {
    _fontShadow = FontManagerS::instance()->getFont("bitmaps/menuShadow");

    _checkmark = _icons->getIndex("Checkmark");
    _checkmarkOff = _icons->getIndex("CheckmarkOff");

    //Get current resolution
    int currentWidth = 640;
    ConfigS::instance()->getInteger("width", currentWidth);
    int currentHeight = 480;
    ConfigS::instance()->getInteger("height", currentHeight);

    //get custom resolutions from config file
    string resolutions;
    if (!ConfigS::instance()->getString("resolutions", resolutions)) {
        resolutions = "512x384,640x480,800x600,1024x768,1152x864,1280x960,1600x1200";
        Value* res = new Value(resolutions);
        ConfigS::instance()->updateKeyword("resolutions", res);
    }

    //extract resolutions
    Tokenizer rToken(resolutions, ",");
    int count = 1;
    while (count < 20) {
        string nextRes = rToken.next();
        if (nextRes == "") {
            break;
        }

        Tokenizer t(nextRes, "x");

        int width = atoi(t.next().c_str());
        int height = atoi(t.next().c_str());

        _resolutionList.push_back(Resolution(width, height));

        count++;
    }

    //Add other fullscreen resolutions
    addFullscreenResolutions();

    if (_resolutionList.size() == 0) {
        _resolutionList.push_back(Resolution(currentWidth, currentHeight));
    }

    //sort resolutions by area
    std::sort(_resolutionList.begin(), _resolutionList.end(), DescendingAreaOrder);

    float fwidth = 0;
    float fheight = _fontWhite->GetHeight(_size);

    _activeResolution = _resolutionList.begin();
    vector<Resolution>::iterator i;
    for (i = _resolutionList.begin(); i != _resolutionList.end(); i++) {
        Resolution res = (*i);
        if (((*i).width == currentWidth) && ((*i).height == currentHeight)) {
            _activeResolution = i;
        }

        float nextfwidth = _fontWhite->GetWidth((*i).text.c_str(), _size);
        if (nextfwidth > fwidth) {
            fwidth = nextfwidth;
        }
    }

    //    _boundingBox.min.x -= fwidth*0.5;
    _boundingBox.max.x = _boundingBox.min.x + fwidth + _fontWhite->GetWidth(_text.c_str(), _size) +
                         _icons->getWidth(_checkmark) * 0.5f + 10;
    _boundingBox.max.y = _boundingBox.min.y + fheight;

    //bounding box for checkmark
    _bRect.min.x = _boundingBox.max.x - _icons->getWidth(_checkmark) * 0.5f;
    _bRect.max.x = _boundingBox.max.x;
    _bRect.min.y = _boundingBox.min.y;
    _bRect.max.y = _bRect.min.y + 30;

    _inputBox = _boundingBox;
}

void ResolutionSelectable::select(void) {
    applyResolution();
}

void ResolutionSelectable::applyResolution(void) {
    int width = (*_activeResolution).width;
    int height = (*_activeResolution).height;

    Value* w = new Value(width);
    ConfigS::instance()->updateKeyword("width", w);
    Value* h = new Value(height);
    ConfigS::instance()->updateKeyword("height", h);
    AudioS::instance()->playSample("sounds/click1");
}

void ResolutionSelectable::addFullscreenResolutions(void) {
#ifndef IPHONE
    int numDisplayModes = SDL_GetNumDisplayModes(0);
    // SDL_Rect **modes=SDL_ListModes(NULL, SDL_FULLSCREEN | SDL_OPENGL);

    if (numDisplayModes == 0) {
        return;
    }

    for (int i = 0; i < numDisplayModes; i++) {
        SDL_DisplayMode mode;
        SDL_GetDisplayMode(0, i, &mode);
        Resolution newRes(mode.w, mode.h);

        bool add = true;
        vector<Resolution>::iterator res;
        for (res = _resolutionList.begin(); res != _resolutionList.end(); res++) {
            if ((*res) == newRes) {
                //LOG_INFO << "Found dup " << buf << endl;
                add = false;
                break;
            }
        }

        if (add) {
            //LOG_INFO << "Adding new " << newRes.text << endl;
            _resolutionList.push_back(newRes);
        }
    }
#endif
}

void ResolutionSelectable::prevResolution(void) {
    if (_activeResolution == _resolutionList.begin()) {
        _activeResolution = _resolutionList.end();
    }
    _activeResolution--;
    AudioS::instance()->playSample("sounds/tick1");
}

void ResolutionSelectable::nextResolution(void) {
    _activeResolution++;
    if (_activeResolution == _resolutionList.end()) {
        _activeResolution = _resolutionList.begin();
    }
    AudioS::instance()->playSample("sounds/tick1");
}

void ResolutionSelectable::input(const Trigger& trigger, const bool& isDown, const Point2Di& offset) {
    if (!_enabled) {
        return;
    }

    if (!isDown) {
        return;
    }

    float mouseX = trigger.fData1;
    float mouseY = trigger.fData2;

    switch (trigger.type) {
        case eKeyTrigger:
            switch (trigger.data1) {
                case SDLK_LEFT:
                    prevResolution();
                    break;

                case SDLK_RIGHT:
                    nextResolution();
                    break;

                default:
                    break;
            }
            break;

        case eButtonTrigger: {
#ifdef IPHONE
            this->activate();
#endif
            if (isDown) {
                if ((mouseX >= (_bRect.min.x + offset.x)) && (mouseX <= (_bRect.max.x + offset.x)) &&
                    (mouseY >= (_bRect.min.y + offset.y)) && (mouseY <= (_bRect.max.y + offset.y))) {
                    applyResolution();
                } else {
                    if (trigger.data1 == SDL_MOUSEWHEEL) {
                        if (trigger.data3 > 0) {
                            nextResolution();
                        } else {
                            prevResolution();
                        }
                    }
                }
            }
        } break;

        case eMotionTrigger:
            this->activate();
            break;

        default:
            break;
    }
}

void ResolutionSelectable::activate(bool beQuiet) {
    updateActive(beQuiet ? eNoFeedback : eBeep);
}

void ResolutionSelectable::draw(const Point2Di& offset) {
    //    Selectable::draw(offset);
    TextOnlySelectable::draw(offset);
    string resolution = /*_text +*/ (*_activeResolution).text;

    float xOff = _fontWhite->GetWidth(_text.c_str(), _size);
    //glColor4f(1.0, 1.0, 1.0, 1.0);
    _fontShadow->setColor(1.0, 1.0, 1.0, 1.0);
    _fontShadow->DrawString(resolution.c_str(), xOff + _boundingBox.min.x + offset.x + FONTSHADOW_OFFSET * _size,
                            _boundingBox.min.y + offset.y - FONTSHADOW_OFFSET * _size, _size, _size);

    //glColor4f(1.0f, 0.852f, 0.0f, 1.0f);
    //    glColor4f(1.0, 1.0, 1.0, 1.0);
    _fontWhite->setColor(1.0f, 0.852f, 0.0f, 1.0f);
    _fontWhite->DrawString(resolution.c_str(), xOff + _boundingBox.min.x + offset.x, _boundingBox.min.y + offset.y,
                           _size, _size);

    _icons->bind();
    vec4f color;
    if (_active == this) {
        if (!_enabled) {
            color = vec4f(0.7, 0.7, 0.7, 1.0);
        } else {
            color = vec4f(1.0f, 0.0f, 0.0f, 1.0f);
        }
    } else {
        if (!_enabled) {
            color = vec4f(0.5, 0.5, 0.5, 1.0);
        } else {
            color = vec4f(1.0, 1.0, 1.0, 1.0);
        }
    }

    //glEnable(GL_TEXTURE_2D);
    float _xOff = _bRect.min.x;

    int currentWidth;
    ConfigS::instance()->getInteger("width", currentWidth);
    int currentHeight;
    ConfigS::instance()->getInteger("height", currentHeight);

    int width = (*_activeResolution).width;
    int height = (*_activeResolution).height;

    _icons->setColor(color);
    if ((currentWidth == width) && (currentHeight == height)) {
        _icons->Draw(_checkmark, _xOff + offset.x, _boundingBox.min.y + offset.y + 2, 0.5, 0.5);
    } else {
        _icons->Draw(_checkmarkOff, _xOff + offset.x, _boundingBox.min.y + offset.y + 2, 0.5, 0.5);
    }
    //glDisable(GL_TEXTURE_2D);
}

//------------------------------------------------------------------------------

TextSelectable::TextSelectable(bool enabled, const BoundingBox& rect, const string& text, const string& info) :

    TextOnlySelectable(enabled, rect, text, info),
    _ds(0.0) {
    _prevSize = _size;
}

void TextSelectable::input(const Trigger& trigger, const bool& isDown, const Point2Di& /*offset*/) {
    if (!_enabled) {
        return;
    }

    if (!isDown) {
        return;
    }

    switch (trigger.type) {
        case eButtonTrigger:
#ifdef IPHONE
            this->activate();
#endif
            MenuManagerS::instance()->Enter();
            break;

        case eMotionTrigger:
            this->activate();
            break;

        default:
            break;
    }
}

void TextSelectable::activate(bool beQuiet) {
    updateActive(beQuiet ? eNoFeedback : eBeep);
    _ds = 0.1f;
}

void TextSelectable::deactivate(void) {
    //        LOG_INFO << "Deactivate " << _text << endl;
    _ds = -0.1f;
}

void TextSelectable::update(void) {
    if (!_enabled) {
        return;
    }
#ifndef IPHONE
    _prevSize = _size;
    _size += _ds;
    Clamp(_size, 1.0, 1.8);  //any bigger and we'll have overlapping activation areas

    //adjust the input box according to the scaled text
    float dx = (float)(_boundingBox.max.x - _boundingBox.min.x) * (_size - 1.0f) / 2.0f;
    float dy = (float)(_boundingBox.max.y - _boundingBox.min.y) * (_size - 1.0f) / 2.0f;

    _inputBox.min.x = _boundingBox.min.x - dx;
    _inputBox.min.y = _boundingBox.min.y - dy;
    _inputBox.max.x = _boundingBox.max.x + dx;
    _inputBox.max.y = _boundingBox.max.y + dy;
#endif
}

void TextSelectable::draw(const Point2Di& offset) {
    Selectable::draw(offset);

#ifdef IPHONE
    float iSize = _size;
#else
    float iSize = _prevSize + (_size - _prevSize) * GameState::frameFractionOther;
    Clamp(iSize, 1.0, 2.0);
#endif

    float halfWidth = _fontWhite->GetWidth(_text.c_str(), iSize - 1.0f) / 2.0f;
    float halfHeight = _fontWhite->GetHeight(iSize - 1.0f) / 2.0f;

    //glColor4f(1.0, 1.0, 1.0, 1.0);
    _fontShadow->setColor(1.0, 1.0, 1.0, 1.0);
    _fontShadow->DrawString(_text.c_str(), _boundingBox.min.x + offset.x - halfWidth + FONTSHADOW_OFFSET * iSize,
                            _boundingBox.min.y + offset.y - halfHeight - FONTSHADOW_OFFSET * iSize, iSize, iSize);

    vec4f color;
    if (!_enabled) {
        color = vec4f(0.5, 0.5, 0.5, 1.0);
    } else {
        color = vec4f(r, g, b, 0.8f);
    }

    _fontWhite->setColor(color);
    _fontWhite->DrawString(_text.c_str(), _boundingBox.min.x + offset.x - halfWidth,
                           _boundingBox.min.y + offset.y - halfHeight, iSize, iSize);
}

//------------------------------------------------------------------------------

ActionSelectable::ActionSelectable(bool enabled, const BoundingBox& rect, const string& action, const string& text,
                                   const string& info, float size) :

    TextSelectable(enabled, rect, text, info),
    _action(action) {
    if (_action == "OpenURL") {
        _size = 1.5;
    } else {
        _size = 2.6;
    }
    _size = size;

    _inputBox.min.x = _boundingBox.min.x - 40 * _size;
    _inputBox.min.y = _boundingBox.min.y - 30 * _size;
    _inputBox.max.x = _boundingBox.max.x + 40 * _size;
    _inputBox.max.y = _boundingBox.max.y + 30 * _size;
}

void ActionSelectable::select(void) {
    //        LOG_INFO << "Selecting: " << _action << endl;
    if (_action == "NewGame") {
        GameS::instance()->startNewGame();

        //no confirm sound in this case
        return;
    } else if (_action == "Quit") {
        GameState::requestExit = true;
    } else if (_action == "OpenURL") {
        openURL("http://games.mooflu.com/");
    }
    AudioS::instance()->playSample("sounds/beep");
}

//------------------------------------------------------------------------------

MenuSelectable::MenuSelectable(TiXmlNode* node, bool enabled, const BoundingBox& rect, const string& text,
                               const string& info, float size) :

    TextSelectable(enabled, rect, text, info),
    _node(node) {
    _prevSize = size;
    _size = size;

    _cherry = _icons->getIndex("cherry");

    float cherrySize = 1.0;
    _inputBox.min.x = _boundingBox.min.x - 50 * cherrySize;
    _inputBox.min.y = _boundingBox.min.y - 90 * cherrySize;
    _inputBox.max.x = _boundingBox.max.x + 50 * cherrySize;
    _inputBox.max.y = _boundingBox.max.y + 70 * cherrySize;
}

void MenuSelectable::select(void) {
    MenuManagerS::instance()->makeMenu(_node);
    AudioS::instance()->playSample("sounds/beep");
}

void MenuSelectable::draw(const Point2Di& offset) {
    float cherrySize = 1.0;
    float xOff = _fontWhite->GetWidth(_text.c_str(), cherrySize);

    _icons->bind();

    //glColor4f(1.0, 1.0, 1.0, 0.7);
    //glEnable(GL_TEXTURE_2D);
    _icons->setColor(1.0, 1.0, 1.0, 0.7);
    _icons->DrawC(_cherry, _boundingBox.min.x + offset.x + xOff / 2.0,
                  _boundingBox.min.y + offset.y + (45.0 * cherrySize), cherrySize, cherrySize);
    //glDisable(GL_TEXTURE_2D);

    TextSelectable::draw(offset);
}

//------------------------------------------------------------------------------
