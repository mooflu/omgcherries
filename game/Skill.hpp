#pragma once
// Description:
//   Skill settings.
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
#include <Singleton.hpp>

const std::string SKILL_1 = "Beginner";
const std::string SKILL_2 = "Normal";
const std::string SKILL_3 = "Expert";
const std::string SKILL_4 = "Eagle-Eye";
const std::string SKILL_5 = "OMG!";
const std::string SKILL_ERROR = "Unknown skill";

class Skill {
    friend class Singleton<Skill>;

public:
    enum SkillEnum {
        eBeginner = 0,
        eNormal = 1,
        eExpert = 2,
        eEagleEye = 3,
        eOMG = 4,
        eLAST
    };

    inline static const std::string& getString(SkillEnum e) {
        switch (e) {
            case eBeginner:
                return SKILL_1;

            case eNormal:
                return SKILL_2;

            case eExpert:
                return SKILL_3;

            case eEagleEye:
                return SKILL_4;

            case eOMG:
                return SKILL_5;

            case eLAST:
            default:
                break;
        }
        return SKILL_ERROR;
    }

    static inline Skill::SkillEnum getSkill(std::string name) {
        if (name == SKILL_1) {
            return Skill::eBeginner;
        }
        if (name == SKILL_2) {
            return Skill::eNormal;
        }
        if (name == SKILL_3) {
            return Skill::eExpert;
        }
        if (name == SKILL_4) {
            return Skill::eEagleEye;
        }
        if (name == SKILL_5) {
            return Skill::eOMG;
        }

        return Skill::eBeginner;
    }

    void updateSkill(const Skill::SkillEnum& skill);
    void updateSkill(void);
    void incrementSkill(void);

private:
    ~Skill();
    Skill(void);
    Skill(const Skill&);
    Skill& operator=(const Skill&);
};

typedef Singleton<Skill> SkillS;
