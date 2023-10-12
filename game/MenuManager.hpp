#pragma once
// Description:
//   Menu Manager/Controller.
//
// Copyright (C) 2001 Frank Becker
//

#include <list>

#include "tinyxml.h"
#include "Singleton.hpp"
#include "InterceptorI.hpp"
#include "Context.hpp"
#include "Point.hpp"
#include "ParticleGroup.hpp"
#include "Model.hpp"

struct Trigger;
class Selectable;

class MenuManager : public InterceptorI {
    friend class Singleton<MenuManager>;

public:
    bool init(void);
    bool update(void);
    bool draw(void);

    virtual void input(const Trigger& trigger, const bool& isDown);
    void turnMenuOn(void);
    void turnMenuOff(void);
    bool canReturnToGame(void);
    void makeMenu(TiXmlNode* _node);

    //Menu navigation
    void Down(void);
    void Up(void);
    void Enter(void);
    bool Exit(bool delayed = false);
    void Goto(Selectable* s);

    void reload(void);

private:
    virtual ~MenuManager();
    MenuManager(void);
    MenuManager(const MenuManager&);
    MenuManager& operator=(const MenuManager&);

    void loadMenuLevel(void);
    void clearActiveSelectables(void);
    void updateSettings(void);
    void activateSelectableUnderMouse(const bool& useFallback = false);

    TiXmlDocument* _menu;

    TiXmlNode* _topMenu;
    TiXmlNode* _currentMenu;

    std::list<Selectable*> _activeSelectables;
    std::list<Selectable*>::iterator _currentSelectable;

    int _board;
    Point2Di _boardOffset;

    int _pointer;

    float _prevMouseX;
    float _prevMouseY;
    float _mouseX;
    float _mouseY;

    Context::ContextEnum _prevContext;
    bool _delayedExit;
    bool _newLevelLoaded;

    Model* _mapleLeaf;
    float _angle;
    float _prevAngle;

    bool _showSparks;
    ParticleGroup _burst;
};

typedef Singleton<MenuManager> MenuManagerS;
