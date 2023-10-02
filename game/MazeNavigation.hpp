#include "Singleton.hpp"

#include <vmmlib/vector.hpp>
using namespace vmml;

#include <cmath>
using namespace std;
#include "Box2D/Box2D.h"

class MazeNavigation {
    friend class Singleton<MazeNavigation>;

public:
    MazeNavigation(void);

    ~MazeNavigation();

    vec2f getNextPosition(const vec2f& pos, const vec2f& velocity);

private:
    b2World* _world;
    b2Body* _item;
    b2Body* _hWalls[6];
    b2Body* _vWalls[6];
};

typedef Singleton<MazeNavigation> MazeNavigationS;
