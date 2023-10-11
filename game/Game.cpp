// Description:
//   High level infrastructure for game.
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

#include <Game.hpp>
#include <GameState.hpp>
#include <Constants.hpp>
#include <Config.hpp>
#include <PausableTimer.hpp>
#include <ScoreKeeper.hpp>
#include <PuckMaze.hpp>
#include <RandomKnuth.hpp>

#include <Audio.hpp>
#include <Input.hpp>
#include "VideoBase.hpp"

#include <Hero.hpp>
#include <Enemy.hpp>
#include <ParticleGroup.hpp>
#include <ParticleGroupManager.hpp>

#include <ModelManager.hpp>
#include <MenuManager.hpp>
#include <ResourceManager.hpp>

static RandomKnuth _random;

Game::Game(void) {
    XTRACE();
}

Game::~Game() {
    XTRACE();

    LOG_INFO << "Shutting down..." << endl;

#ifndef DEMO
    // save config stuff
    ConfigS::instance()->saveToFile();

    // save leaderboard
    ScoreKeeperS::instance()->save();
#endif

    MenuManagerS::cleanup();
    ParticleGroupManagerS::cleanup();

    AudioS::cleanup();
    delete _view;  //calls SDL_Quit

    ModelManagerS::cleanup();
    ScoreKeeperS::cleanup();
    InputS::cleanup();

    PuckMazeS::cleanup();

    HeroS::cleanup();  //has to be after ParticleGroupManager

    // Note: this shuts down PHYSFS
    LOG_INFO << "ResourceManager cleanup..." << endl;
    ResourceManagerS::cleanup();
}

bool Game::init(void) {
    XTRACE();
    bool result = true;

    SkillS::instance()->updateSkill();
    ScoreKeeperS::instance()->load();
    ScoreKeeperS::instance()->setLeaderBoard(Skill::getString(GameState::skill));

    _view = new CherriesView();
    if (!_view->init()) {
        return false;
    }

    // init subsystems et al
    if (!ParticleGroupManagerS::instance()->init()) {
        return false;
    }

    if (!AudioS::instance()->init()) {
        return false;
    }
    AudioS::instance()->setDefaultSoundtrack("lg-criti.xm");

    if (!InputS::instance()->init()) {
        return false;
    }
    if (!MenuManagerS::instance()->init()) {
        return false;
    }
    if (!HeroS::instance()->init()) {
        return false;
    }

    ParticleGroupManager* pgm = ParticleGroupManagerS::instance();
    if (!pgm->init()) {
        return false;
    }

    //init all the paricle groups and links between them
    pgm->addGroup(HERO_GROUP, 1);
    pgm->addGroup(ENEMIES_GROUP, 300);
    //pgm->addGroup( ENEMIES_BULLETS_GROUP, 300);

    //there are 3 effect groups to give very simple control over the order
    //of drawing which is important for alpha blending.
    //pgm->addGroup( EFFECTS_GROUP1, 1000);
    pgm->addGroup(EFFECTS_GROUP2, 1000);
    //pgm->addGroup( EFFECTS_GROUP3, 1000);

    //pgm->addGroup( HERO_BULLETS_GROUP, 300);
    pgm->addGroup(BONUS_GROUP, 50);
    //pgm->addGroup( SHOOTABLE_ENEMY_BULLETS_GROUP, 100);
    //pgm->addGroup( SHOOTABLE_BONUS_GROUP, 50);

    //collision detect between the following groups
    //pgm->addLink( HERO_BULLETS_GROUP, SHOOTABLE_ENEMY_BULLETS_GROUP);
    //pgm->addLink( HERO_BULLETS_GROUP, ENEMIES_GROUP);
    //pgm->addLink( HERO_BULLETS_GROUP, SHOOTABLE_BONUS_GROUP);
    pgm->addLink(HERO_GROUP, ENEMIES_GROUP);
    //pgm->addLink( HERO_GROUP, ENEMIES_BULLETS_GROUP);
    pgm->addLink(HERO_GROUP, BONUS_GROUP);
    //pgm->addLink( HERO_GROUP, SHOOTABLE_ENEMY_BULLETS_GROUP);
    //pgm->addLink( HERO_GROUP, SHOOTABLE_BONUS_GROUP);

    //reset our stopwatch
    GameState::stopwatch.reset();
    GameState::stopwatch.pause();

    GameState::mainTimer.reset();

#ifdef IPHONE
    GameState::horsePower = 40.0;
#else
    ConfigS::instance()->getFloat("horsePower", GameState::horsePower);
#endif

    //add our hero...
    ParticleGroupManagerS::instance()->getParticleGroup(HERO_GROUP)->newParticle(string("Hero"), 0, 0, -100);

    //make sure we start of in menu mode
    MenuManagerS::instance()->turnMenuOn();

    GameState::startOfStep = GameState::mainTimer.getTime();
    GameState::startOfGameStep = GameState::stopwatch.getTime();

    LOG_INFO << "Initialization complete OK." << endl;

    return result;
}

void Game::nextLevel(void) {
    GameState::worminess++;
    PuckMazeS::instance()->reset();
    HeroS::instance()->nextLevel();

    int wormCount = (GameState::worminess * ((int)GameState::skill + 1)) / 2 + 1;
    //LOG_INFO << "Worm count=" << wormCount << endl;

    PuckMazeS::instance()->AddPowerpoints((int)GameState::skill + 1);

    ParticleGroup* pg = ParticleGroupManagerS::instance()->getParticleGroup(ENEMIES_GROUP);
    for (int i = 0; i < wormCount; i++) {
        pg->newParticle(string("Worm"), 0, 0, 0);
    }
}

void Game::reset(void) {
    //reset in order to start new game
    SkillS::instance()->updateSkill();

    ScoreKeeperS::instance()->updateScoreBoardWithLeaderBoard();

    PuckMazeS::cleanup();
    int cellSizes[] = {26, 18, 12, 8, 6};
    int cellSize = cellSizes[(int)GameState::skill];
    int boardSize = 319 / cellSize;
    PuckMazeS::instance()->init(boardSize, boardSize, cellSize);

    ParticleGroupManagerS::instance()->reset();  //updates all particles one more time so they can die
    HeroS::instance()->reset();
    ParticleGroupManagerS::instance()->getParticleGroup(HERO_GROUP)->newParticle(string("Hero"), 0, 0, -100);

    GameState::worminess = 0;
    nextLevel();

    ScoreKeeperS::instance()->setLeaderBoard(Skill::getString(GameState::skill));
    ScoreKeeperS::instance()->resetCurrentScore();

    GameState::startOfStep = GameState::mainTimer.getTime();  //get fresh start for other logic
    GameState::stopwatch.start();
    LOG_INFO << "Starting game timer\n";

    GameState::startOfGame = GameState::stopwatch.getTime();
    GameState::startOfGameStep = GameState::stopwatch.getTime();
}

void Game::startNewGame(void) {
    //VideoBase::instance()->showFullscreenImage("Loading");
    // #warning FIXME

    GameS::instance()->reset();
    GameState::context = Context::eInGame;
    InputS::instance()->disableInterceptor();
    AudioS::instance()->playSample("sounds/chirp2");

    bool allowVerticalMovement = false;
    ConfigS::instance()->getBoolean("allowVerticalMovement", allowVerticalMovement);
}

void Game::toggleCritterBoard(void) {
    _view->toggleCritterBoard();
}

void Game::updateOtherLogic(void) {
    int stepCount = 0;
    float currentTime = GameState::mainTimer.getTime();
    while ((currentTime - GameState::startOfStep) > GAME_STEP_SIZE) {
        //FIXME: shouldn't run all the time...
        MenuManagerS::instance()->update();

        //advance to next start-of-game-step point in time
        GameState::startOfStep += GAME_STEP_SIZE;
        currentTime = GameState::mainTimer.getTime();

        stepCount++;
        if (stepCount > MAX_GAME_STEPS) {
            break;
        }
    }

    GameState::frameFractionOther = (currentTime - GameState::startOfStep) / GAME_STEP_SIZE;

    if (stepCount > 1) {
        //LOG_WARNING << "Skipped " << stepCount << " frames." << endl;

        if (GameState::frameFractionOther > 1.0) {
            //Our logic is still way behind where it should be at this
            //point in time. If we get here we already ran through
            //MAX_GAME_STEPS logic runs trying to catch up.

            //We clamp the value to 1.0, higher values would try
            //to predict were we are visually. Maybe not a bad idead
            //to allow values up to let's say 1.3...
            GameState::frameFractionOther = 1.0;
        }
    }
}

void Game::updateInGameLogic(void) {
    int stepCount = 0;
    float currentGameTime = GameState::stopwatch.getTime();
    while ((currentGameTime - GameState::startOfGameStep) > GAME_STEP_SIZE) {
        // update all objects, particles, etc.
        ParticleGroupManagerS::instance()->update();

        //FIXME: Currently the Critterboard is updated in the video system. Should be on its own.
        _view->updateLogic();

        //advance to next start-of-game-step point in time
        GameState::startOfGameStep += GAME_STEP_SIZE;
        currentGameTime = GameState::stopwatch.getTime();

        stepCount++;
        if (stepCount > MAX_GAME_STEPS) {
            break;
        }
    }

    GameState::frameFraction = (currentGameTime - GameState::startOfGameStep) / GAME_STEP_SIZE;

    if (stepCount > 1) {
        //LOG_WARNING << "Skipped2 " << stepCount << " frames." << endl;

        if (GameState::frameFraction > 1.0) {
            //Our logic is still way behind where it should be at this
            //point in time. If we get here we already ran through
            //MAX_GAME_STEPS logic runs trying to catch up.

            //We clamp the value to 1.0, higher values would try
            //to predict were we are visually. Maybe not a bad idead
            //to allow values up to let's say 1.3...
            GameState::frameFraction = 1.0;
        }
    }
}

void Game::run(void) {
    XTRACE();

    Audio& audio = *AudioS::instance();
    Input& input = *InputS::instance();

#ifndef IPHONE
    while (GameState::isAlive)
#endif
    {
        switch (GameState::context) {
            case Context::eInGame:
                //stuff that only needs updating when game is actually running
                updateInGameLogic();
                break;

            default:
                break;
        }

        //stuff that should run all the time
        updateOtherLogic();

        input.update();
        audio.update();
        _view->draw();

        _random.random();  //randomize a bit

        GLenum err;
        while ((err = glGetError()) != GL_NO_ERROR) {
            LOG_ERROR << "GL ERROR: " << std::hex << err << "\n";
        }

        VideoBaseS::instance()->swap();
    }
}
