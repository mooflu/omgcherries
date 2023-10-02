// Description:
//   All kinds of particles.
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
#include <Particles.hpp>
#include <GameState.hpp>
#include <RandomKnuth.hpp>
#include <ModelManager.hpp>
#include <Camera.hpp>
#include <ParticleGroup.hpp>
#include <ParticleGroupManager.hpp>
#include <Hero.hpp>
#include <Audio.hpp>
#include <FontManager.hpp>
#include <BitmapManager.hpp>
#include <ScoreKeeper.hpp>

#include "gl3/Program.hpp"
#include "gl3/ProgramManager.hpp"
#include "gl3/MatrixStack.hpp"
#include "glm/ext.hpp"

#include <Constants.hpp>

static RandomKnuth _random;

//------------------------------------------------------------------------------

float getPseudoRadius(Model* model) {
    //if most of the movement is vertical, width represents the radius
    //most realisticly (for non square objs).
    vec3 minpt, maxpt;
    model->getBoundingBox(minpt, maxpt);
    return (maxpt.x - minpt.x) / 2.0;
}

//------------------------------------------------------------------------------

GLBitmapCollection* BitmapParticleType::_bitmaps = 0;

BitmapParticleType::BitmapParticleType(const string& name) :
    ParticleType(name) {
    XTRACE();
    LoadBitmaps();
}

BitmapParticleType::~BitmapParticleType() {
    XTRACE();
}

void BitmapParticleType::LoadBitmaps(void) {
    XTRACE();
    static bool bitmapsLoaded = false;
    if (!bitmapsLoaded) {
        _bitmaps = BitmapManagerS::instance()->getBitmap("bitmaps/atlas");  //"bitmaps/ammo");
        if (!_bitmaps) {
            LOG_ERROR << "Unable to load bitmaps" << endl;
        }
        bitmapsLoaded = true;
    }
}

//------------------------------------------------------------------------------

SingleBitmapParticle::SingleBitmapParticle(const string& name, const char* bitmapName) :
    BitmapParticleType(name) {
    XTRACE();
    string bmName(bitmapName);
    _bmIndex = _bitmaps->getIndex(bmName);

    _bmHalfWidth = (float)(_bitmaps->getWidth(_bmIndex)) / 2.0f;
    _bmHalfHeight = (float)(_bitmaps->getHeight(_bmIndex)) / 2.0f;

    //if most of the movement is vertical, width represents the radius
    //most realisticly (for non square objs).
    //        _radius = sqrt(_bmHalfWidth*_bmHalfWidth + _bmHalfHeight*_bmHalfHeight);
    _radius = _bmHalfWidth;  // + _bmHalfHeight)/2.0;
}

//------------------------------------------------------------------------------

SmokePuff::SmokePuff(void) :
    SingleBitmapParticle("SmokePuff", "SmokePuff1") {
    XTRACE();
}

SmokePuff::~SmokePuff() {
    XTRACE();
}

void SmokePuff::init(ParticleInfo* p) {
    XTRACE();
    p->velocity.x = 0.0f * GAME_STEP_SCALE;
    p->velocity.y = 0.27f * GAME_STEP_SCALE;
    p->velocity.z = 0.0f * GAME_STEP_SCALE;

    p->extra.x = 0.025f * GAME_STEP_SCALE;
    p->extra.y = 0.004f * GAME_STEP_SCALE;
    p->extra.z = 0.36f;

    //init previous values for interpolation
    updatePrevs(p);
}

bool SmokePuff::update(ParticleInfo* p) {
    //    XTRACE();
    //update previous values for interpolation
    updatePrevs(p);

    p->extra.z = 0.36f - p->extra.x * 6.0f;
    //if alpha reaches 0, we can die
    if (p->extra.z < 0) {
        return false;
    }

    p->extra.x += p->extra.y;

    p->position.x += p->velocity.x;
    p->position.y += p->velocity.y;

    return true;
}

void SmokePuff::draw(ParticleInfo* p) {
    //    XTRACE();
    ParticleInfo pi;
    interpolate(p, pi);

    //glEnable(GL_TEXTURE_2D);
    //glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    glPushMatrix();

    float shiftX = _bmHalfWidth * pi.extra.x;
    float shiftY = _bmHalfHeight * pi.extra.x;
    glTranslatef(pi.position.x - shiftX, pi.position.y - shiftY, pi.position.z);

    //rotate towards the camera
    CameraS::instance()->billboard();

    //	LOG_ERROR << pi.extra.x << endl;
    //glColor4f(1.0,1.0,1.0, pi.extra.z);

    bindTexture();
    _bitmaps->setColor(1.0, 1.0, 1.0, pi.extra.z);
    _bitmaps->Draw(_bmIndex, 0, 0, pi.extra.x, pi.extra.x);

    glPopMatrix();

    glEnable(GL_DEPTH_TEST);
    //glEnable(GL_LIGHTING);
    //glDisable(GL_TEXTURE_2D);
}

//------------------------------------------------------------------------------

MiniSmoke::MiniSmoke(void) :
    SingleBitmapParticle("MiniSmoke", "SmokePuff1") {
    XTRACE();
}

MiniSmoke::~MiniSmoke() {
    XTRACE();
}

void MiniSmoke::init(ParticleInfo* p) {
    XTRACE();

    p->extra.x = 0.020f * GAME_STEP_SCALE;
    p->extra.y = 0.002f * GAME_STEP_SCALE;
    p->extra.z = 0.80f;

    //init previous values for interpolation
    updatePrevs(p);
}

bool MiniSmoke::update(ParticleInfo* p) {
    //    XTRACE();
    //update previous values for interpolation
    updatePrevs(p);

    p->extra.z = 0.80f - p->extra.x * 12.0f;
    //if alpha reaches 0, we can die
    if (p->extra.z < 0) {
        return false;
    }

    p->extra.x += p->extra.y;

    p->position.x += p->velocity.x;
    p->position.y += p->velocity.y;

    return true;
}

void MiniSmoke::draw(ParticleInfo* p) {
    //    XTRACE();
    ParticleInfo pi;
    interpolate(p, pi);

    glEnable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    glPushMatrix();

    float shiftX = _bmHalfWidth * pi.extra.x;
    float shiftY = _bmHalfHeight * pi.extra.x;
    glTranslatef(pi.position.x - shiftX, pi.position.y - shiftY, pi.position.z);

    //rotate towards the camera
    CameraS::instance()->billboard();

    //	LOG_ERROR << pi.extra.x << endl;
    glColor4f(1.0, 1.0, 1.0, pi.extra.z);

    bindTexture();
    _bitmaps->Draw(_bmIndex, 0, 0, pi.extra.x, pi.extra.x);

    glPopMatrix();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
}

//------------------------------------------------------------------------------

Spark::Spark(void) :
    SingleBitmapParticle("Spark", "Spark1") {
    XTRACE();
}

Spark::~Spark() {
    XTRACE();
}

void Spark::init(ParticleInfo* p) {
    XTRACE();
    p->velocity.x = (float)(_random.random() & 0xf) * 0.82f * GAME_STEP_SCALE;
    p->velocity.y = -(float)(_random.random() & 0xf) * 0.62f * GAME_STEP_SCALE;
    p->velocity.z = 0.0f * GAME_STEP_SCALE;

    p->extra.x = 0.10f * GAME_STEP_SCALE;
    //    p->extra.z = 0.36;

    //init previous values for interpolation
    updatePrevs(p);
}

bool Spark::update(ParticleInfo* p) {
    //    XTRACE();
    //update previous values for interpolation
    updatePrevs(p);

    p->velocity.y -= 0.8f * GAME_STEP_SCALE;

    p->extra.z = 0.9f - p->extra.x;
    //if alpha reaches 0, we can die
    if (p->extra.z < 0) {
        return false;
    }

    p->extra.x += 0.025f * GAME_STEP_SCALE;

    p->position.x += p->velocity.x;
    p->position.y += p->velocity.y;

    if (p->position.y < 0) {
        return false;
    }

    return true;
}

void Spark::draw(ParticleInfo* p) {
    //    XTRACE();
    ParticleInfo pi;
    interpolateOther(p, pi);

    //glColor4f(1.0,1.0,1.0, pi.extra.z);

    bindTexture();
    _bitmaps->setColor(1.0, 1.0, 1.0, pi.extra.z);
    _bitmaps->DrawC(_bmIndex, pi.position.x, pi.position.y, 1.0, 1.0);
}

//------------------------------------------------------------------------------

FireSpark::FireSpark(const string& sp) :
    SingleBitmapParticle(sp, sp.c_str()) {
    XTRACE();
}

FireSpark::~FireSpark() {
    XTRACE();
}

void FireSpark::init(ParticleInfo* p) {
    XTRACE();
    p->velocity.x = (float)((_random.random() & 0xff) - 0x80) * 0.0010f * GAME_STEP_SCALE;
    p->velocity.y = (float)((_random.random() & 0xff) - 0x80) * 0.0010f * GAME_STEP_SCALE;
    p->velocity.z = 0.0f * GAME_STEP_SCALE;

    p->extra.x = 0.10f * GAME_STEP_SCALE;
    //    p->extra.z = 0.36;

    //init previous values for interpolation
    updatePrevs(p);
}

bool FireSpark::update(ParticleInfo* p) {
    //    XTRACE();
    //update previous values for interpolation
    updatePrevs(p);

    //    p->velocity.y -= 0.8 * GAME_STEP_SCALE;

    p->extra.z = 0.9f - p->extra.x;
    //if alpha reaches 0, we can die
    if (p->extra.z < 0) {
        return false;
    }

    p->extra.x += 0.10f * GAME_STEP_SCALE;

    p->position.x += p->velocity.x;
    p->position.y += p->velocity.y;

    return true;
}

void FireSpark::draw(ParticleInfo* p) {
    //    XTRACE();
    ParticleInfo pi;
    interpolate(p, pi);

    glEnable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    glPushMatrix();

    glTranslatef(pi.position.x, pi.position.y, pi.position.z);

    //rotate towards the camera
    CameraS::instance()->billboard();

    glColor4f(1.0, 1.0, 1.0, p->extra.z);

    bindTexture();

    //    glRotatef( pi.extra.y, 0,0,1);

    _bitmaps->DrawC(_bmIndex, 0, 0, 0.2f, 0.2f);

    glPopMatrix();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
}

//------------------------------------------------------------------------------

StatusMessage::StatusMessage(void) :
    ParticleType("StatusMessage") {
    XTRACE();
    _smallFont = FontManagerS::instance()->getFont("bitmaps/arial-small");
    if (!_smallFont) {
        LOG_ERROR << "Unable to get font... (arial-small)" << endl;
    }
}

StatusMessage::~StatusMessage() {
    XTRACE();
}

void StatusMessage::init(ParticleInfo* p) {
    //    XTRACE();
    p->velocity.x = -1.0f * GAME_STEP_SCALE;

    p->extra.x = _smallFont->GetWidth(p->text.c_str(), 0.1f);
    p->position.x = 70.0f;

    LOG_INFO << "StatusMsg = [" << p->text << "] " /*<< p->position.y*/ << endl;

    p->tod = -1;

    //init previous values for interpolation
    updatePrevs(p);
}

bool StatusMessage::update(ParticleInfo* p) {
    //    XTRACE();
    //update previous values for interpolation
    updatePrevs(p);

    p->position.x += p->velocity.x;

    if (p->position.x < -(70.0f + p->extra.x)) {
        return false;
    }

    return true;
}

void StatusMessage::draw(ParticleInfo* p) {
    //    XTRACE();
    ParticleInfo pi;
    interpolate(p, pi);

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    glPushMatrix();

    glTranslatef(pi.position.x, pi.position.y, pi.position.z);

    glColor4f(p->color.x, p->color.y, p->color.z, 0.8f);
    _smallFont->DrawString(p->text.c_str(), 0, 0, p->extra.y, p->extra.z);

    glPopMatrix();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}

//------------------------------------------------------------------------------
#if 0
ExplosionPiece::ExplosionPiece( void):
    ParticleType( "ExplosionPiece")
{
    XTRACE();
    _cloud = ModelManagerS::instance()->getModel("models/E1");
}

ExplosionPiece::~ExplosionPiece()
{
    XTRACE();
}

void ExplosionPiece::init( ParticleInfo *p)
{
//    XTRACE();
    p->velocity.x = (float)((_random.random()&0xff)-128)*0.002f* GAME_STEP_SCALE;
    p->velocity.y = (float)((_random.random()&0xff)-128)*0.002f* GAME_STEP_SCALE;
    p->velocity.z = (float)((_random.random()&0xff)-128)*0.002f* GAME_STEP_SCALE;

    p->extra.x = ((float)(_random.random()%90)-45.0f)*0.1f * GAME_STEP_SCALE;
    p->extra.y = 1.0f;
    p->tod = -1;

    p->extra.z = 0;

    //init previous values for interpolation
    updatePrevs(p);
}

bool ExplosionPiece::update( ParticleInfo *p)
{
//    XTRACE();
    //update previous values for interpolation
    updatePrevs(p);

    p->extra.y -= 0.03f * GAME_STEP_SCALE;
    if( p->extra.y <= 0) return false;

    p->extra.z += p->extra.x;

    p->position.x += p->velocity.x;
    p->position.y += p->velocity.y;
    p->position.z += p->velocity.z;

    return true;
}

void ExplosionPiece::draw( ParticleInfo *p)
{
//    XTRACE();
    ParticleInfo pi;
    interpolate( p, pi);

    glColor4f(1.0f,0.2f,0.1f, pi.extra.y);

    glPushMatrix();

    glTranslatef( pi.position.x, pi.position.y, pi.position.z);
    glRotatef(pi.extra.z, 1,1,0);

    _cloud->draw();

    glPopMatrix();
}
#endif
//------------------------------------------------------------------------------

ScoreHighlight::ScoreHighlight(void) :
    ParticleType("ScoreHighlight") {
    _font = FontManagerS::instance()->getFont("bitmaps/arial-small");
    if (!_font) {
        LOG_ERROR << "Unable to get font... (arial-small)" << endl;
    }
}

ScoreHighlight::~ScoreHighlight() {}

void ScoreHighlight::init(ParticleInfo* p) {
    p->velocity.x = (float)((_random.random() & 0xff) - 168) * 0.003f * GAME_STEP_SCALE;
    p->velocity.y = (float)((_random.random() & 0xff) - 128) * 0.002f * GAME_STEP_SCALE;
    p->velocity.z = (float)((_random.random() & 0xff) - 128) * 0.002f * GAME_STEP_SCALE;

    p->extra.x = _random.rangef0_1() * 40.0f - 20.0f;
    p->extra.y = 0.05f;
    p->extra.z = 0.8f;

    p->tod = -1;

    //init previous values for interpolation
    updatePrevs(p);
}

bool ScoreHighlight::update(ParticleInfo* p) {
    //    XTRACE();
    //update previous values for interpolation
    updatePrevs(p);

    p->extra.z -= 0.02f * GAME_STEP_SCALE;
    //if alpha reaches 0, we can die
    if (p->extra.z < 0) {
        return false;
    }

    p->extra.x += 1.00f * GAME_STEP_SCALE;
    p->extra.y += 0.005f * GAME_STEP_SCALE;

    p->position.x += p->velocity.x;
    p->position.y += p->velocity.y;
    p->position.z += p->velocity.z;

    return true;
}

void ScoreHighlight::draw(ParticleInfo* p) {
    //    XTRACE();
    ParticleInfo pi;
    interpolate(p, pi);

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    glPushMatrix();

    glTranslatef(pi.position.x, pi.position.y, pi.position.z);

    //rotate towards the camera
    CameraS::instance()->billboard();

    //    glRotatef( pi.extra.x, 0,0,1);

    glColor4f(p->color.x, p->color.y, p->color.z, pi.extra.z);
    _font->DrawString(p->text.c_str(), 0, 0, pi.extra.y, pi.extra.y);
    glPopMatrix();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}

//------------------------------------------------------------------------------
