// Description:
//   Skill settings
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
#include "Skill.hpp"

#include <Trace.hpp>
#include <Config.hpp>
#include <Value.hpp>
#include <GameState.hpp>
#include <Hero.hpp>

using namespace std;

Skill::Skill(void) {
    XTRACE();
    updateSkill();
}

Skill::~Skill() {
    XTRACE();
}

void Skill::updateSkill(void) {
    int i = (int)Skill::eBeginner;
    ConfigS::instance()->getInteger("skill", i);
    updateSkill((Skill::SkillEnum)i);
}

void Skill::updateSkill(const Skill::SkillEnum& skill) {
    XTRACE();

    //if( GameState::skill == skill) return;

    GameState::skill = skill;

    string newSkill = getString(GameState::skill);
    LOG_INFO << "Setting skill to " << newSkill << endl;

    Value* v = new Value((int)GameState::skill);
    ConfigS::instance()->updateKeyword("skill", v);

    switch (GameState::skill) {
        case Skill::eBeginner:
            break;

        case Skill::eNormal:
            break;

        case Skill::eExpert:
            break;

        case Skill::eEagleEye:
            break;

        case Skill::eOMG:
            break;

        default:
            break;
    }
}

void Skill::incrementSkill(void) {
    if ((GameState::skill + 1) != Skill::eLAST) {
        updateSkill((Skill::SkillEnum)(GameState::skill + 1));
    }
}
