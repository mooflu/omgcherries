// Description:
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
#include <string.h>  //memset
#include <Trace.hpp>

#include <Tracer.hpp>
#include <RandomKnuth.hpp>

static RandomKnuth _random;

Tracer::Tracer(PuckMaze* m) {
    //make a sentinel
    tail = new Node;
    tail->next = NULL;
    tail->info = NULL;

    head = tail;

    maze = m;

    //make a local map to keep track where we've been/traced
    map = new char[maze->Width() * maze->Height()];
}

Tracer::~Tracer() {
    //delete list
    while (head->next != NULL) {
        Node* tmp = head;
        head = head->next;
        delete tmp->info;
        delete tmp;
    }

    //delete sentinel
    delete tail;

    delete[] map;
}

void Tracer::Add(int x, int y, int dir, int dist) {
    //create a new sentinel
    Node* add = new Node;

    add->next = NULL;
    add->info = NULL;

    //fill old sentinel with info
    tail->next = add;
    tail->info = new Info;

    tail->info->x = x;
    tail->info->y = y;
    tail->info->direction = dir;
    tail->info->distance = dist;

    //point tail to new sentinel
    tail = add;
}

//do a recursive search. Slow but cute :)
int Tracer::Find(int x, int y, Uint32 element, int len) {
#define MAXLEN 6
    if (maze->isElement(x, y, element)) {
        return len;
    }

    if (len >= MAXLEN) {
        return MAXLEN + 1;
    }

    int min = MAXLEN + 1;
    int dir = 0;
    int mlen;

    if (!maze->isElement(x, y, WallUP)) {
        min = Find(x, y - 1, element, len + 1);
        dir = WallUP;
    }

    if (!maze->isElement(x, y, WallLT)) {
        mlen = Find(x - 1, y, element, len + 1);
        if (mlen < min) {
            min = mlen;
            dir = WallLT;
        }
    }
    if (!maze->isElement(x, y, WallDN)) {
        mlen = Find(x, y + 1, element, len + 1);
        if (mlen < min) {
            min = mlen;
            dir = WallDN;
        }
    }
    if (!maze->isElement(x, y, WallRT)) {
        mlen = Find(x + 1, y, element, len + 1);
        if (mlen < min) {
            min = mlen;
            dir = WallRT;
        }
    }

    if (len == 0) {
        if (min >= MAXLEN) {
            dir = 0;
        }
        return dir;
    } else {
        return min;
    }
}

//
// Breadth-First-Search.
//
// The maze is an undirected graph with up to 4 edges per node.
// (The representation of the maze is actually a directed graph but
// with edges going src->dst and dst->src). From the starting point
// we go to each adjacent node, marking them as checked as we go along.
// Checked nodes won't be revisited. We keep a list of the nodes we
// visited ordered by distance to the starting point. We also keep track
// of the first direction we took at the starting point on the path to
// the current location. If we encounter the element we were looking for
// we remember the "first direction" and keep processing the list, but
// only until the distance to a certain node is greater then the distance
// to the position of the first encounter. This will give us a selection
// of possible directions to take from the starting point (ie. we might
// find two (or more) distinct elements at the same distance from our
// starting point but in different directions. For the first we might
// have to go left, and for the second we might have to go right).
//
// Note that Find has to be called after each step we take, since it
// only returns a direction to take and not a path (see FindPath).
// It might return no direction (0) if there are no elements in the
// vicinity.
//
int NEWTracer::Find2(int x, int y, Uint32 element) {
#define NOTCHECKED 0
#define CHECKED 1
    //reset local map
    memset(map, NOTCHECKED, maze->Width() * maze->Height());

    // add possible trace directions
    if (!maze->isElement(x, y, WallUP)) {
        Add(x, y - 1, WallUP, 1);
    }
    if (!maze->isElement(x, y, WallLT)) {
        Add(x - 1, y, WallLT, 1);
    }
    if (!maze->isElement(x, y, WallDN)) {
        Add(x, y + 1, WallDN, 1);
    }
    if (!maze->isElement(x, y, WallRT)) {
        Add(x + 1, y, WallRT, 1);
    }
    //the distance from our starting point (x,y)
    int dist = 0;

    //the possible directions to take
    //to get from (x,y) to the nearest "element"
    int possibleDirs = 0;

    int maxDist = _maxDist;

    while (head->next != NULL && dist <= maxDist) {
        int xa = head->info->x;
        int ya = head->info->y;
        int oDir = head->info->direction;
        dist = head->info->distance + 1;
        Node* tmp = head;
        head = head->next;
        delete tmp->info;
        delete tmp;

        //mark the current location
        map[ya * maze->Width() + xa] = CHECKED;

        //check if there is an "element" at the current position
        if (maze->isElement(xa, ya, element)) {
            //Note, we could just return "oDir" here
            //but this would resuWallLT in an
            //WallUP-down, left-right (ie. the order
            //we do the maze check) movement.
            if (maxDist > dist) {
                maxDist = dist;
            }
            possibleDirs |= oDir;
            continue;
            //                      return( oDir);
        }

        // add if there is no wall and field not checked
        if (!maze->isElement(xa, ya, WallUP)) {
            if (map[(ya - 1) * maze->Width() + xa] != CHECKED) {
                Add(xa, ya - 1, oDir, dist);
            }
        }

        if (!maze->isElement(xa, ya, WallLT)) {
            if (map[ya * maze->Width() + xa - 1] != CHECKED) {
                Add(xa - 1, ya, oDir, dist);
            }
        }

        if (!maze->isElement(xa, ya, WallDN)) {
            if (map[(ya + 1) * maze->Width() + xa] != CHECKED) {
                Add(xa, ya + 1, oDir, dist);
            }
        }

        if (!maze->isElement(xa, ya, WallRT)) {
            if (map[ya * maze->Width() + xa + 1] != CHECKED) {
                Add(xa + 1, ya, oDir, dist);
            }
        }
    }

    // remove the remaining elements of the list
    while (head->next != NULL) {
        Node* tmp = head;
        head = head->next;
        delete tmp->info;
        delete tmp;
    }

    if (possibleDirs) {
        int select[4];
        int maxs = 0;

        //check the resuWallLTs
        if (possibleDirs & WallUP) {
            select[maxs++] = WallUP;
        }
        if (possibleDirs & WallDN) {
            select[maxs++] = WallDN;
        }
        if (possibleDirs & WallLT) {
            select[maxs++] = WallLT;
        }
        if (possibleDirs & WallRT) {
            select[maxs++] = WallRT;
        }

        if ((_maxDist > 16) && (maxDist < (_maxDist / 2))) {
            _maxDist = _maxDist / 2;
            LOG_INFO << "maxDist=" << _maxDist << "\n";
        }

        //select one of the possible directions at random
        return (select[_random.random() % maxs]);
    } else {
        /*
        if( _maxDist < 32)
        {
            _maxDist = _maxDist * 2;
            LOG_INFO << "maxDist=" << _maxDist << "\n";
        }
*/
        //no "element" in sight
        return (0);
    }
}

// FindPath returns a complete path from the starting point to the
// element. It might return a zero length path if there are no
// elements in the vicinity.
//
// We keep track of a couple of things for each node we visit:
//  - where we came from (ie, WallUP WallDN WallLT WallRT)
//  - the distance to the starting position
//
// The two arrays simulate a tree, sortof. The from array is the
// actual information (where we came from) and the prev array contains
// an index to the "parent" ie. the previous node.
// Note that, in order to guarantee that a non-zero-length path is
// returned (assuming an element exists in the maze) MAXNODES has to
// be equal to maze->Width() * maze->Height().
// There are better ways to do this if performance per call isn't
// important...
//
char* Tracer::FindPath(int x, int y, Uint32 element) {
#define ADDLOCATION(f, p) (from[idx] = f, prev[idx] = p, idx++)

#define NOTCHECKED 0
#define CHECKED 1
#define ROOT 0
#define MAXEDGES 100

    // index into from and prev.
    int idx = 0;

    char* from = new char[MAXEDGES];
    char* prev = new char[MAXEDGES];

    // storage for path we'll return
    static char path[MAXEDGES];

    //init path and local map
    path[0] = 0;
    memset(map, NOTCHECKED, maze->Width() * maze->Height());

    ADDLOCATION(ROOT, 0);

    // add possible trace directions
    if (!maze->isElement(x, y, WallUP)) {
        Add(x, y - 1, idx, 1);
        ADDLOCATION(WallUP, 0);
    }
    if (!maze->isElement(x, y, WallDN)) {
        Add(x, y + 1, idx, 1);
        ADDLOCATION(WallDN, 0);
    }
    if (!maze->isElement(x, y, WallLT)) {
        Add(x - 1, y, idx, 1);
        ADDLOCATION(WallLT, 0);
    }
    if (!maze->isElement(x, y, WallRT)) {
        Add(x + 1, y, idx, 1);
        ADDLOCATION(WallRT, 0);
    }

    //we might add up to 4 elements to the array
    while (head->next != NULL && idx < (MAXEDGES - 4)) {
        int xa = head->info->x;
        int ya = head->info->y;
        int prevn = head->info->direction;
        int dist = head->info->distance + 1;

        Node* tmp = head;
        head = head->next;
        delete tmp->info;
        delete tmp;

        //mark the current location
        map[ya * maze->Width() + xa] = CHECKED;

        //check if there is an "element" at the current position
        if (maze->isElement(xa, ya, element)) {
            int pIdx = 0;
            while (from[prevn] != ROOT) {
                path[pIdx++] = from[prevn];
                prevn = prev[prevn];
            }
            path[pIdx] = 0;
            break;
        }

        // add if there is no wall and field not checked
        if (!maze->isElement(xa, ya, WallUP)) {
            if (map[(ya - 1) * maze->Width() + xa] != CHECKED) {
                Add(xa, ya - 1, idx, dist);
                ADDLOCATION(WallUP, prevn);
            }
        }

        if (!maze->isElement(xa, ya, WallDN)) {
            if (map[(ya + 1) * maze->Width() + xa] != CHECKED) {
                Add(xa, ya + 1, idx, dist);
                ADDLOCATION(WallDN, prevn);
            }
        }

        if (!maze->isElement(xa, ya, WallLT)) {
            if (map[ya * maze->Width() + xa - 1] != CHECKED) {
                Add(xa - 1, ya, idx, dist);
                ADDLOCATION(WallLT, prevn);
            }
        }

        if (!maze->isElement(xa, ya, WallRT)) {
            if (map[ya * maze->Width() + xa + 1] != CHECKED) {
                Add(xa + 1, ya, idx, dist);
                ADDLOCATION(WallRT, prevn);
            }
        }
    }

    // remove the remaining elements
    while (head->next != NULL) {
        Node* tmp = head;
        head = head->next;
        delete tmp->info;
        delete tmp;
    }

    delete[] from;
    delete[] prev;

    return (path);
}
