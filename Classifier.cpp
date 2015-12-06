#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <errno.h>
#include <sstream>
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <vector>
#include <algorithm>

#include "CImg.h"
#include "Common.h"

using namespace cimg_library;

using namespace std;

double VR[6] = {885748, 293783, 300391, 607459, 835587, 868031};
double VG[6] = {377968, 651749, 624135, 309999, 775478 ,296616};
double VB[6] = {308418, 417185, 860149, 658677, 320619, 631009};

double VR_P[6] = {895791, 591046, 313007, 758484, 928395, 961682};
double VG_P[6] = {425218, 846944, 631362, 484856, 897501, 460849};
double VB_P[6] = {352430, 703509, 847330, 805604, 569245, 785462};

// Finding the top left coordinates of the board
void locateTopLeft() {
	int roughSize = h / 2 / 5;
	for (top_x = h - roughSize; top_x >= 0; top_x--) {
		int v = 0;
		for (int j = 0; j < w; j++)
			for (int c = 0; c < 3; c++) v += img(j, top_x, 0, c);
		if (v < 500) break;
	}
	top_x++;
	
	for (top_y = 0; top_y < w; top_y++) {
		int v = 0;
		for (int c = 0; c < 3; c++) v += img(top_y, top_x, 0, c);
		if (v >= 10) break;
	}
	
	for (int y1 = w - 1; y1 >= 0; y1--) {
		int v=0;
		for (int c = 0; c < 3; c++) v += img(y1, top_x, 0, c);
		if (v >= 10) {
			sz = (y1 - top_y + 1) / 6.;
			break;
		}
	}
	cout<<"Image info:"<<endl;
	cout<<"Size: "<<h<<"x"<<w<<endl;
	cout<<"Top left coordinates: ("<<top_x<<", "<<top_y<<")"<<endl;
	cout<<"Orb size: "<<sz<<endl;
	cout<<endl;
}

void classifyBoard() {
	// Check for enhanced orbs
	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 6; j++) {
			int x = top_x + (i + 0.75) * sz, y = top_y + (j + 0.75) * sz;
			int r = 0, g = 0, b = 0;
			for (int dx = 0; dx < 0.05 * sz; dx++)
				for (int dy = 0; dy < 0.05 * sz; dy++) {
					r += img(y + dy, x + dx, 0, 0);
					g += img(y + dy, x + dx, 0, 1);
					b += img(y + dy, x + dx, 0, 2);
				}
			if (b * 1. / (r + g + b) < 0.05) isPlus[i][j] = true;
			else isPlus[i][j] = false;
		}
	}
	
	// Identify orbs color
	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 6; j++) {
			int x = top_x + (i + 0.25) * sz, y = top_y + (j + 0.25) * sz;
			int r = 0, g = 0, b = 0;
			
			for (int dx = 0; dx < 0.5 * sz; dx++)
				for (int dy = 0; dy < 0.5 * sz; dy++) {
					r += img(y + dy, x + dx, 0, 0);
					g += img(y + dy, x + dx, 0, 1);
					b += img(y + dy, x + dx, 0, 2);
				}

			int best_k = -1, best_score = -1;

			if (!isPlus[i][j]) {
				for (int k = 0; k < 6; k++) {
					int score = abs(r - VR[k]) + abs(g - VG[k]) + abs(b - VB[k]);
					if (best_score == -1 || score < best_score) {
						best_score = score;
						best_k = k;
					}
				}
			} else {
				for (int k = 0; k < 6; k++) {
					int t0 = min(min(r, g), b), t1 = min(min(VR_P[k], VG_P[k]), VB_P[k]);
					int score = abs((r - t0) - (VR_P[k] - t1));
					score += abs((g - t0) - (VG_P[k] - t1));
					score += abs((b - t0) - (VB_P[k] - t1));
					if (best_score == -1 || score < best_score) {
						best_score = score;
						best_k = k;
					}
				}
			}
			//cout<<"("<<r<<","<<g<<","<<b<<") ";
			initBoard[i][j] = ORB[best_k];
		}
		//cout<<endl;
	}

	cout<<"Identified board:"<<endl;
	for (int i=0;i<5;i++) {
		for (int j=0;j<6;j++) cout<<initBoard[i][j];
		cout<<endl;
	}
	cout<<endl;
}

void processImage() {
	locateTopLeft();
	classifyBoard();
}
