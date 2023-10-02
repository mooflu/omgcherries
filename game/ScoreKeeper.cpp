// Description:
//   Score keeper.
//
// Copyright (C) 2004-2009 Frank Becker
//
#include "ScoreKeeper.hpp"

#include "Trace.hpp"
#include "Config.hpp"
#include "Value.hpp"
#include "Tokenizer.hpp"
#include "zStream.hpp"
#include <RandomKnuth.hpp>
#include "GetDataPath.hpp"

#include "GLBitmapFont.hpp"
#include "FontManager.hpp"

#include "GLBitmapCollection.hpp"
#include "BitmapManager.hpp"

#include "Constants.hpp"
#include "ParticleInfo.hpp"
#include "ParticleGroup.hpp"
#include "ParticleGroupManager.hpp"

#include "Tokenizer.hpp"
#include "GameState.hpp"
#include "FindHash.hpp"
#include "StringUtils.hpp"

#ifdef min
#undef min
#endif

#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <sstream>
#include <algorithm>
using namespace std;

const int LEADERBOARD_SIZE = 11;  //top-10 plus current score

static RandomKnuth _random;

ScoreKeeper::ScoreKeeper(void) :
    _currentIndex(LEADERBOARD_SIZE - 1),
    _infoIndex(0),
    _leaderBoard(LEADERBOARD_SIZE),
    _practiceMode(false) {
    XTRACE();
    resetLeaderBoard(_leaderBoard);
    _currentScoreboard = _scoreBoards.end();
}

ScoreKeeper::~ScoreKeeper(void) {
    XTRACE();
    _scoreBoards.clear();
}

void ScoreKeeper::resetCurrentScore(void) {
    _currentIndex = LEADERBOARD_SIZE - 1;
    _leaderBoard[_currentIndex].score = 0;
    _leaderBoard[_currentIndex].name = "Anonymous";
    _leaderBoard[_currentIndex].time = 0;
    _leaderBoard[_currentIndex].points = 0;
    _leaderBoard[_currentIndex].msPlayed = 0;
}

int ScoreKeeper::addToCurrentScore(int score) {
    XTRACE();
    float multiplier = 1.0f;

    int newValue = (int)((float)score * multiplier);

#if 0
    bool showScoreUpdates = true;
    if( ! ConfigS::instance()->getBoolean( "showScoreUpdates", showScoreUpdates))
    {
        Value *v = new Value( showScoreUpdates);
        ConfigS::instance()->updateKeyword( "showScoreUpdates", v);
    }

    if( showScoreUpdates && (newValue > 0))
    {
        static ParticleGroup *effects =
        ParticleGroupManagerS::instance()->getParticleGroup( EFFECTS_GROUP1);

        ParticleInfo pi;
        pi.position.x = 375;
        pi.position.y = 375;
        pi.position.z = 0;
        char buf[10];
        sprintf( buf, "%d", newValue);
        pi.text = buf;

        if( cubes)
        {
            pi.color.x = 1.0f;
            pi.color.y = 1.0f;
            pi.color.z = 1.0f;
        }
        else
        {
            pi.color.x = 1.0f;
            pi.color.y = 0.0f;
            pi.color.z = 0.0f;
        }

        effects->newParticle( "ScoreHighlight", pi);
    }
#endif

#ifndef DEMO
    if (!_practiceMode) {
        _leaderBoard[_currentIndex].score += newValue;
        if (score == 1) {
            _leaderBoard[_currentIndex].points++;
        }
        _leaderBoard[_currentIndex].msPlayed = HeroS::instance()->Age();
        time(&_leaderBoard[_currentIndex].time);

        sortLeaderBoard();
    }
#endif
    //return the real value;
    return newValue;
}

int ScoreKeeper::getCurrentScore(void) {
    return getScore(_currentIndex);
}

bool ScoreKeeper::currentIsTopTen(void) {
    return _currentIndex < (LEADERBOARD_SIZE - 1);
}

void ScoreKeeper::setNameForCurrent(const std::string& name) {
    _leaderBoard[_currentIndex].name = name;
}

void ScoreKeeper::setLeaderBoard(const std::string& scoreboardName) {
    ScoreBoards::iterator i;
    for (i = _scoreBoards.begin(); i < _scoreBoards.end(); i++) {
        if (i->name == scoreboardName) {
            break;
        }
    }

    if (i != _scoreBoards.end()) {
        stringToLeaderBoard(i->data, _leaderBoard);
    } else {
        resetLeaderBoard(_leaderBoard);
        string data = leaderBoardToString(_leaderBoard, scoreboardName);
        addScoreBoard(scoreboardName, data);
    }

    _leaderBoardName = scoreboardName;
}

int ScoreKeeper::getScore(unsigned int index) {
    if (index < _leaderBoard.size()) {
        return _leaderBoard[index].score;
    }

    return 0;
}

int ScoreKeeper::getHighScore(void) {
    return getScore(0);
}

void ScoreKeeper::resetLeaderBoard(LeaderBoard& lb) {
    const int NUM_NAMES = 6;
    char* names[NUM_NAMES] = {
        "mooflu.com", "FB", "Larry", "Willy", "ff", "Minden",
    };

    int startIndex = _random.random();
    for (size_t i = 0; i < lb.size(); i++) {
        lb[i].score = 10 * i;
        lb[i].name = names[(i + startIndex) % NUM_NAMES];
        time(&lb[i].time);
        lb[i].points = 0;
        lb[i].msPlayed = 0;
    }

    sort(lb.begin(), lb.end());
}

void ScoreKeeper::sortLeaderBoard(void) {
    XTRACE();

    if (_currentIndex == 0) {
        return;  //we are leader, no update required
    }

    bool newPos = false;
    ScoreData tmpData = _leaderBoard[_currentIndex];
    while ((_currentIndex > 0) && (tmpData.score > _leaderBoard[_currentIndex - 1].score)) {
        _leaderBoard[_currentIndex] = _leaderBoard[_currentIndex - 1];
        _currentIndex--;
        newPos = true;
    }
    _leaderBoard[_currentIndex] = tmpData;

    //dumpLeaderBoard( _leaderBoard);
}

string ScoreKeeper::leaderBoardToString(LeaderBoard& lb, const string& leaderBoardName) {
    ostringstream ostr;
    ostr << leaderBoardName << "\001";

    for (unsigned int i = 0; i < lb.size(); i++) {
        if (lb[i].name == "") {
            lb[i].name = "Anonymous";
        }
        ostr << lb[i].name << "\002" << lb[i].score << "\002" << (long)lb[i].time << "\002" << lb[i].points << "\002"
             << lb[i].msPlayed << "\002";
    }

    LOG_INFO << "leaderBoardToString [" << ostr.str() << "]\n";

    return ostr.str();
}

void ScoreKeeper::stringToLeaderBoard(const string& lbString, LeaderBoard& lb) {
    Tokenizer t(lbString, false, "\001\002");

    string scoreboardName = t.next();
    if (scoreboardName != "") {
        lb.clear();

        bool done = false;
        while (!done) {
            ScoreData sData;
            sData.name = t.next();
            if (sData.name != "") {
                sData.score = atoi(t.next().c_str());
                sData.time = (time_t)atoi(t.next().c_str());

                string num = t.next();
                sData.points = 0;
                if (num.length()) {
                    sData.points = atoi(num.c_str());
                }

                num = t.next();
                sData.msPlayed = 0;
                if (num.length()) {
                    sData.msPlayed = atoi(num.c_str());
                }

                lb.push_back(sData);
            } else {
                done = true;
            }
        }
    }

    //should already be sorted, but better be safe...
    sort(lb.begin(), lb.end());

    dumpLeaderBoard(lb);
}

void ScoreKeeper::dumpLeaderBoard(const LeaderBoard& lb) {
    LOG_INFO << "------LeaderBoard-----" << endl;
    for (unsigned int i = 0; i < lb.size(); i++) {
        LOG_INFO.width(30);
        LOG_VOID.fill('.');
        LOG_VOID.unsetf(ios::right);
        LOG_VOID.setf(ios::left);
        LOG_VOID << lb[i].name.c_str();

        LOG_VOID.width(10);
        LOG_VOID.unsetf(ios::left);
        LOG_VOID.setf(ios::right);
        LOG_VOID << lb[i].score;

        LOG_VOID.width(10);
        LOG_VOID.unsetf(ios::left);
        LOG_VOID.setf(ios::right);
        LOG_VOID << lb[i].points;

        LOG_VOID.width(10);
        LOG_VOID.unsetf(ios::left);
        LOG_VOID.setf(ios::right);
        LOG_VOID << lb[i].msPlayed;

        char buf[128];
        strftime(buf, 127, "%a %d-%b-%Y %H:%M", localtime(&lb[i].time));
        LOG_VOID << " : " << buf << endl;
    }
}

const string ScoreKeeper::getInfoText(unsigned int index) {
    string info = "";
    _infoIndex = index;

    if (index < _currentScoreboardAsLeaderBoard.size()) {
        info = _currentScoreboardAsLeaderBoard[index].name;
        info += " played on ";

        char buf[128];
        strftime(buf, 127, "%a %d-%b-%Y %H:%M", localtime(&_currentScoreboardAsLeaderBoard[index].time));

        info += buf;
    }

    return info;
}

void ScoreKeeper::updateScoreBoardWithLeaderBoard(void) {
    ScoreBoards::iterator i;
    for (i = _scoreBoards.begin(); i < _scoreBoards.end(); i++) {
        if (i->name == _leaderBoardName) {
            i->data = leaderBoardToString(_leaderBoard, _leaderBoardName);
            break;
        }
    }
}

void ScoreKeeper::addScoreBoard(const string& scoreboardName, const string& data) {
    ScoreBoard newBoard;
    newBoard.name = scoreboardName;
    newBoard.data = data;
    _scoreBoards.push_back(newBoard);

    sort(_scoreBoards.begin(), _scoreBoards.end());

    for (ScoreBoards::iterator i = _scoreBoards.begin(); i < _scoreBoards.end(); i++) {
        if (i->name == scoreboardName) {
            _currentScoreboard = i;
            break;
        }
    }
}

void ScoreKeeper::setActive(string scoreboardName) {
    updateScoreBoardWithLeaderBoard();

    if (scoreboardName == "") {
        scoreboardName = _leaderBoardName;
    }
    if (_currentScoreboard->name != scoreboardName) {
        _currentScoreboard = _scoreBoards.end();
        for (ScoreBoards::iterator i = _scoreBoards.begin(); i < _scoreBoards.end(); i++) {
            if (i->name == scoreboardName) {
                _currentScoreboard = i;
                break;
            }
        }
    }

    if (_currentScoreboard != _scoreBoards.end()) {
        stringToLeaderBoard(_currentScoreboard->data, _currentScoreboardAsLeaderBoard);
    } else {
        LOG_WARNING << "Board [" << scoreboardName << "] does not exist! Creating...\n";
        resetLeaderBoard(_currentScoreboardAsLeaderBoard);
        string data = leaderBoardToString(_currentScoreboardAsLeaderBoard, scoreboardName);
        addScoreBoard(scoreboardName, data);
    }
}

void ScoreKeeper::nextBoard(void) {
    updateScoreBoardWithLeaderBoard();

    _currentScoreboard++;
    if (_currentScoreboard == _scoreBoards.end()) {
        _currentScoreboard = _scoreBoards.begin();
    }
    if (_currentScoreboard != _scoreBoards.end()) {
        stringToLeaderBoard(_currentScoreboard->data, _currentScoreboardAsLeaderBoard);
    }
}

void ScoreKeeper::prevBoard(void) {
    updateScoreBoardWithLeaderBoard();

    if (_currentScoreboard == _scoreBoards.begin()) {
        _currentScoreboard = _scoreBoards.end();
    }
    _currentScoreboard--;
    if (_currentScoreboard != _scoreBoards.end()) {
        stringToLeaderBoard(_currentScoreboard->data, _currentScoreboardAsLeaderBoard);
    }
}

void ScoreKeeper::load(void) {
#ifndef DEMO
    XTRACE();
    string scoreFilename = "leaderboard";
    LOG_INFO << "Loading hi-scores from " << scoreFilename << endl;

    int version = 1;
    ziStream infile(scoreFilename.c_str());
    if (infile.isOK())  //infile.good())
    {
        string line;
        while (!getline(infile, line).eof()) {
            LOG_INFO << "[" << line << "]" << endl;
            //explicitly skip comments
            if (line[0] == '#') {
                continue;
            }

            Tokenizer tv(line, false, " \t\n\r");
            string token = tv.next();
            if (token == "Version") {
                string versionString = tv.next();
                version = atoi(versionString.c_str());

                if (version != 4) {
                    LOG_ERROR << "Wrong version in score file!" << endl;
                    return;
                }
            } else {
                Tokenizer t(line, false, "\001\002");
                string scoreboardName = t.next();

                if (scoreboardName != "") {
                    addScoreBoard(scoreboardName, line);
                }
            }
        }
    }
#endif
}

void ScoreKeeper::save(void) {
#ifndef DEMO
    XTRACE();
    string scoreFilename = "leaderboard";
    LOG_INFO << "Saving hi-scores to " << scoreFilename << endl;

    //Save scores in a compressed file to make it a bit tougher to cheat...
    zoStream outfile(scoreFilename.c_str());
    if (outfile.good()) {
        outfile << "#------LeaderBoard-----#" << endl;
        outfile << "Version 4" << endl;

        updateScoreBoardWithLeaderBoard();

        ScoreBoards::const_iterator ci;
        for (ci = _scoreBoards.begin(); ci != _scoreBoards.end(); ci++) {
            LOG_INFO << "[" << ci->data << "]" << endl;
            outfile << ci->data << endl;
        }
    }
#endif
}

void ScoreKeeper::draw(const Point2Di& offset) {
    static GLBitmapFont* fontWhite = FontManagerS::instance()->getFont("bitmaps/menuWhite");
    static GLBitmapFont* fontShadow = FontManagerS::instance()->getFont("bitmaps/menuShadow");

    static GLBitmapCollection* icons = BitmapManagerS::instance()->getBitmap("bitmaps/atlas");  //"bitmaps/menuIcons");

    string scoreBoardHeader = "Skill: " + _currentScoreboard->name;
    //glColor4f(1.0, 1.0, 1.0, 1.0);
    fontShadow->setColor(1.0, 1.0, 1.0, 1.0);
    fontShadow->DrawString(scoreBoardHeader.c_str(), 110 + offset.x + 9, 480 + offset.y - 9, 1.5f, 1.5f);
    fontWhite->setColor(1.0, 1.0, 1.0, 1.0);
    fontWhite->DrawString(scoreBoardHeader.c_str(), 110 + offset.x, 480 + offset.y, 1.5f, 1.5f);

    if (numBoards() > 1) {
        //glEnable(GL_TEXTURE_2D);
        icons->bind();
        int slider = icons->getIndex("Slider");
        icons->setColor(1.0, 1.0, 1.0, 1.0);
        icons->Draw(slider, vec2i(50, 505), vec2i(150, 605));
        icons->Draw(slider, vec2i(680, 505), vec2i(580, 605));
    }

    size_t maxIndex = std::min((size_t)(LEADERBOARD_SIZE - 1), _currentScoreboardAsLeaderBoard.size());
    for (unsigned int i = 0; i < maxIndex; i++) {
        char buf[128];
        sprintf(buf, "%d", _currentScoreboardAsLeaderBoard[i].score);

        char buf2[100];
        sprintf(buf2, "%d.", i + 1);

        char buf3[100];
        int ms = _currentScoreboardAsLeaderBoard[i].msPlayed;
        sprintf(buf3, "%d.%02d sec", ms / 1000, (ms % 1000) / 10);
        float width3 = fontWhite->GetWidth(buf3, 1.0);

        float width = fontWhite->GetWidth(buf, 1.0);

        float top = 430;
        float left = 20.0;

        //glColor4f(1.0, 1.0, 1.0, 1.0);
        fontShadow->setColor(1.0, 1.0, 1.0, 1.0);
        fontShadow->DrawString(buf2, left + 70 + FONTSHADOW_OFFSET, top - (float)i * 36 - FONTSHADOW_OFFSET, 1.0f,
                               1.0f);
        fontShadow->DrawString(_currentScoreboardAsLeaderBoard[i].name.c_str(), left + 150 + FONTSHADOW_OFFSET,
                               top - (float)i * 36 - FONTSHADOW_OFFSET, 1.0f, 1.0f);
        fontShadow->DrawString(buf, left + 600 - width + FONTSHADOW_OFFSET, top - (float)i * 36 - FONTSHADOW_OFFSET,
                               1.0f, 1.0f);
        fontShadow->DrawString(buf3, left + 900 - width3 + FONTSHADOW_OFFSET, top - (float)i * 36 - FONTSHADOW_OFFSET,
                               1.0f, 1.0f);

        vec4f color;
        /*if( i == _infoIndex)
            color = vec4f(1.0, 0.1, 0.1, 1.0);
        else*/
        if (_leaderBoardName == _currentScoreboard->name && i == _currentIndex) {
            color = vec4f(1.0, 0.852f, 0.0, 1.0);
        } else {
            color = vec4f(1.0, 0.95, 0.7, 1.0);
        }

        fontWhite->setColor(color);
        fontWhite->DrawString(buf2, left + 70, top - (float)i * 36, 1.0f, 1.0f);
        fontWhite->DrawString(_currentScoreboardAsLeaderBoard[i].name.c_str(), left + 150, top - (float)i * 36, 1.0f,
                              1.0f);
        fontWhite->DrawString(buf, left + 600 - width, top - (float)i * 36, 1.0f, 1.0f);
        fontWhite->DrawString(buf3, left + 900 - width3, top - (float)i * 36, 1.0f, 1.0f);
    }
}
