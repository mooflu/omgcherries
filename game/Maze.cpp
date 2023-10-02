//I got the original maze algorithm from someones email-sig. I don't know
//who wrote it originally and since there was no copyright notice...
//I modified it to fit my needs...
//
//Here is the original:
//
// char*M,A,Z,E=40,J[40],T[40];main(C){for(*J=A=scanf(M="%d",&C);
// --            E;             J[              E]             =T
// [E   ]=  E)   printf("._");  for(;(A-=Z=!Z)  ||  (printf("\n|"
// )    ,   A    =              39              ,C             --
// )    ;   Z    ||    printf   (M   ))M[Z]=Z[A-(E   =A[J-Z])&&!C
// &    A   ==             T[                                  A]
// |6<<11<rand()||!C&!Z?J[T[E]=T[A]]=E,J[T[A]=A-Z]=A,"_.":" |"];}
//
#include <Maze.hpp>
#include <RandomKnuth.hpp>

static RandomKnuth _random;

//make maze
Maze::Maze() :
    map(0),
    y2off(0) {}

//destroy maze
Maze::~Maze() {
    delete[] map;
    delete[] y2off;
}

void Maze::init(int w, int h) {
    width = w;
    height = h;

    map = new Uint32[width * height];
    y2off = new int[height];

    for (int i = 0; i < height; i++) {
        y2off[i] = i * width;
    }
}

void Maze::reset(void) {
    Create();
    Simplify();
}

//create a maze. Just adds a bunch of walls
void Maze::Create(void) {
    char t[3] = {0, 0, 0};
    char h = 0;
    int x = 1;
    int y = height;
    int i;

    int* j = new int[width + 1];
    int* k = new int[width + 1];

    for (i = 0; i < width * height; i++) {
        map[i] = 0;
    }

    int pos = 0;

    for (i = 1; i <= width; i++) {
        j[i] = i;
        k[i] = i;
    }

    j[0] = 1;
    k[0] = 0;

    while (1) {
        h = !h;
        x -= h;
        if (x == 0) {
            x = width;
            y--;
            if (y == -1) {
                break;
            }
            //                      printf( "\n|");
        }

        if (x - (i = j[x - h]) && (!y & x == k[x] | (_random.random() % 8192) > 3192) || !y & !h) {
            k[i] = k[x];
            j[k[i]] = i;

            k[x] = x - h;
            j[k[x]] = x;

            t[h] = h["_ "];

            if (!y & h) {
                t[h] = '_';
            }
        } else {
            t[h] = h[" |"];
        }

        if (h == 0) {
            if (t[0] == '_') {
                map[pos] |= WallDN;
                if ((pos + width) < (width * height)) {
                    map[pos + width] |= WallUP;
                }
            }
            if (t[1] == '|') {
                map[pos] |= WallRT;
                if (((pos + 1) % width) != 0) {
                    map[pos + 1] |= WallLT;
                }
            }

            if (pos < width) {
                map[pos] |= WallUP;
            }

            if ((pos % width) == 0) {
                map[pos] |= WallLT;
            }

            pos++;
            //                      printf( t);
        }
    }

    delete[] j;
    delete[] k;
}

//simplyfy maze. It removes dead ends.
//the magic numbers in the case statements are the bit representation
//of the walls.
void Maze::Simplify(void) {
    int pos = 0;
    int x, y;
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            switch (map[pos] & 0xf) {
                case 0x0e:  //DN LT RT
                    if (y != height - 1) {
                        map[pos] &= ~WallDN;
                        map[pos + width] &= ~WallUP;
                    } else {
                        if (x != width - 1) {
                            map[pos] &= ~WallRT;
                            map[pos + 1] &= ~WallLT;
                        }
                    }
                    break;
                case 0x0d:  //UP LT RT
                    if (y != 0) {
                        map[pos] &= ~WallUP;
                        map[pos - width] &= ~WallDN;
                    } else {
                        if (x != 0) {
                            map[pos] &= ~WallLT;
                            map[pos - 1] &= ~WallRT;
                        }
                    }
                    break;
                case 0x0b:  //UP DN RT
                    if (x != width - 1) {
                        map[pos] &= ~WallRT;
                        map[pos + 1] &= ~WallLT;
                    } else {
                        if (y != height - 1) {
                            map[pos] &= ~WallDN;
                            map[pos + width] &= ~WallUP;
                        }
                    }
                    break;
                case 0x0f:  //UP DN LT RT
                case 0x07:  //UP DN LT
                    if (x != 0) {
                        map[pos] &= ~WallLT;
                        map[pos - 1] &= ~WallRT;
                    } else {
                        if (y != 0) {
                            map[pos] &= ~WallUP;
                            map[pos - width] &= ~WallDN;
                        }
                    }
                    break;
                default:
                    break;
            }
            pos++;
        }
    }
}
