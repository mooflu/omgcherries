#pragma once
// Description:
//   Score keeper.
//
// Copyright (C) 2004 Frank Becker
//

#include <string>
#include <vector>

#include <time.h>

#include "Point.hpp"
#include "Singleton.hpp"
#include "Skill.hpp"

#include "hashMap.hpp"
#include "HashString.hpp"

using namespace std;

struct ScoreData {
    int score;
    std::string name;
    time_t time;
    int points;
    int msPlayed;
};

inline bool operator<(const ScoreData& s1, const ScoreData& s2) {
    // ">" since we want highscore first
    return s1.score > s2.score;
}

struct ScoreBoard {
    std::string name;
    std::string data;  // top-10 as string data
};

inline bool operator<(const ScoreBoard& s1, const ScoreBoard& s2) {
    int skill1 = Skill::getSkill(s1.name);
    int skill2 = Skill::getSkill(s2.name);
    return skill1 < skill2;
}

class ScoreKeeper {
    friend class Singleton<ScoreKeeper>;

public:
    ~ScoreKeeper();
    ScoreKeeper(void);

    //ongoing game (leader board)
    void resetCurrentScore(void);
    int addToCurrentScore(int score);
    int getCurrentScore(void);
    bool currentIsTopTen(void);
    void setNameForCurrent(const std::string& name);
    void setLeaderBoard(const std::string& scoreboardName);
    void updateScoreBoardWithLeaderBoard(void);

    std::string getCurrentScoreBoardName() { return _leaderBoardName; }

    ScoreData& getCurrentScoreData() { return _leaderBoard[_currentIndex]; }

    int getScore(unsigned int index);
    int getHighScore(void);

    // load/save all score boards
    void load(void);
    void save(void);

    // draw current score board
    void draw(const Point2Di& offset);

    // get info text for current score board
    const std::string getInfoText(unsigned int index);

    //set active score board (if empty string, uses current leader board)
    void setActive(std::string scoreboardName);

    int numBoards() { return _scoreBoards.size(); }

    //advance to next/previous score board
    void nextBoard(void);
    void prevBoard(void);

    void setPracticeMode(bool practiceMode) { _practiceMode = practiceMode; }

private:
    typedef std::vector<ScoreData> LeaderBoard;
    typedef std::vector<ScoreBoard> ScoreBoards;

    ScoreKeeper(const ScoreKeeper&);
    ScoreKeeper& operator=(const ScoreKeeper&);

    void resetLeaderBoard(LeaderBoard& lb);
    void sortLeaderBoard(void);
    string leaderBoardToString(LeaderBoard& lb, const string& leaderBoardName);
    void stringToLeaderBoard(const std::string& lbString, LeaderBoard& lb);
    void dumpLeaderBoard(const LeaderBoard& lb);

    void addScoreBoard(const string& scoreboardName, const string& data);

    unsigned int _currentIndex;
    unsigned int _infoIndex;
    std::string _leaderBoardName;
    LeaderBoard _leaderBoard;

    ScoreBoards::iterator _currentScoreboard;
    LeaderBoard _currentScoreboardAsLeaderBoard;
    ScoreBoards _scoreBoards;
    bool _practiceMode;

    hash_map<const std::string, time_t, hash<const std::string>, equal_to<const std::string>> _lastOnlineRequestTime;
};

typedef Singleton<ScoreKeeper> ScoreKeeperS;
