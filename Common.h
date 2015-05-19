#include "CImg.h"

#define MAX_NO_SOLUTION 1000
#define SOLUTION_MAX_LENGTH 80
#define ITER 70
#define IMG_NAME "screen.png"

#define S_ALL (S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH)

using namespace cimg_library;

struct solution {
	int st_x, st_y;
	int len;
	int path[SOLUTION_MAX_LENGTH];
};

extern struct solution sol;

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
