#pragma once
// Description:
//   Action callbacks for mouse and keyboard events.
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
#include <Callback.hpp>
#include <GameState.hpp>

class TapAction : public Callback {
public:
    TapAction(void) :
        Callback("TapAction", "TAP") {
        XTRACE();
    }

    virtual ~TapAction() { XTRACE(); }

    virtual void performAction(Trigger&, bool isDown);

private:
    int _numWeap;
};

class MotionAction : public Callback {
public:
    MotionAction(void) :
        Callback("Motion", "MOTION") {
        XTRACE();
    }

    virtual ~MotionAction() { XTRACE(); }

    virtual void performAction(Trigger& trigger, bool isDown);
};

class MotionLeftAction : public Callback {
public:
    MotionLeftAction(void) :
        Callback("MotionLeft", "A") {
        XTRACE();
    }

    virtual ~MotionLeftAction() { XTRACE(); }

    virtual void performAction(Trigger&, bool isDown);
};

class MotionRightAction : public Callback {
public:
    MotionRightAction(void) :
        Callback("MotionRight", "D") {
        XTRACE();
    }

    virtual ~MotionRightAction() { XTRACE(); }

    virtual void performAction(Trigger&, bool isDown);
};

class MotionUpAction : public Callback {
public:
    MotionUpAction(void) :
        Callback("MotionUp", "W") {
        XTRACE();
    }

    virtual ~MotionUpAction() { XTRACE(); }

    virtual void performAction(Trigger&, bool isDown);
};

class MotionDownAction : public Callback {
public:
    MotionDownAction(void) :
        Callback("MotionDown", "S") {
        XTRACE();
    }

    virtual ~MotionDownAction() { XTRACE(); }

    virtual void performAction(Trigger&, bool isDown);
};

class SnapshotAction : public Callback {
public:
    SnapshotAction(void) :
        Callback("Snapshot", "F6") {
        XTRACE();
    }

    virtual ~SnapshotAction() { XTRACE(); }

    virtual void performAction(Trigger& trigger, bool isDown);
};

class ConfirmAction : public Callback {
public:
    ConfirmAction(void) :
        Callback("Confirm", "RETURN") {
        XTRACE();
    }

    virtual ~ConfirmAction() { XTRACE(); }

    virtual void performAction(Trigger&, bool isDown);
};

class ChangeContext : public Callback {
public:
    ChangeContext(void) :
        Callback("ChangeContext", "C") {
        XTRACE();
    }

    virtual ~ChangeContext() { XTRACE(); }

    virtual void performAction(Trigger&, bool isDown);
};

class CritterBoard : public Callback {
public:
    CritterBoard(void) :
        Callback("CritterBoard", "ESCAPE") {
        XTRACE();
    }

    virtual ~CritterBoard() { XTRACE(); }

    virtual void performAction(Trigger&, bool isDown);
};

class PauseGame : public Callback {
public:
    PauseGame(void) :
        Callback("PauseGame", "P"),
        _prevContext(Context::eUnknown) {
        XTRACE();
    }

    virtual ~PauseGame() { XTRACE(); }

    virtual void performAction(Trigger&, bool isDown);

private:
    Context::ContextEnum _prevContext;
};

class EscapeAction : public Callback {
public:
#if defined(EMSCRIPTEN)
    EscapeAction(void) :
        Callback("EscapeAction", "BACKSPACE"),
#else
    EscapeAction(void) :
        Callback("EscapeAction", "ESCAPE"),
#endif
        _prevContext(Context::eUnknown) {
        XTRACE();
    }

    virtual ~EscapeAction() { XTRACE(); }

    virtual void performAction(Trigger&, bool isDown);

private:
    Context::ContextEnum _prevContext;
};
