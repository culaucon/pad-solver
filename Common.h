#include "CImg.h"

#define MAX_NO_SOLUTION 5000
#define SOLUTION_MAX_LENGTH 100
#define ITER 50

#define TPA_WEIGHT 3
#define ROW_WEIGHT 3
#define LKALI_ACTIVATE 100

#define IMG_NAME "screen.png"

#define TEAM_COMBOS 0
#define TEAM_GREEN_TPA 1
#define TEAM_ATHENA 2
#define TEAM_LKALI 3
#define TEAM_YAMATO 4

#define COMBO_TYPES "RGBLDH"

#define S_ALL (S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH)

using namespace cimg_library;

struct solution {
	int st_x, st_y;
	int len;
	int path[SOLUTION_MAX_LENGTH];
};

extern struct solution sol;

extern int team;

extern char ORB[6];
extern int dx[8];
extern int dy[8];
extern bool diag[8];


extern bool isPlus[5][6];
extern char initBoard[5][6];

extern CImg<unsigned char> img;
extern int h, w;
extern int top_x, top_y;
extern double sz;
extern bool isFirst;
