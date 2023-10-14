#pragma once
// Description:
//   Collection of game state variables.
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

#include <Context.hpp>
#include <Skill.hpp>
#include <PausableTimer.hpp>

struct GameState {
    static bool isDeveloper;
    static bool isAlive;
    static bool requestExit;

    static bool showFPS;

    static float startOfStep;
    static float frameFractionOther;
    static float startOfGameStep;
    static float frameFraction;
    static float startOfGame;

    static float horsePower;
    static int numObjects;
    static int enemyBulletCount;
    static int enemyAttackCount;

    static PausableTimer mainTimer;
    static PausableTimer stopwatch;
    static Context::ContextEnum context;
    static Skill::SkillEnum skill;
    static int worminess;

    static std::string deviceId;
    static bool isPirate;
};
