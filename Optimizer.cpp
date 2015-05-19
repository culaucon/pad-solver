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
#include <queue>

#include "Common.h"
#include "Classifier.h"
#include "Optimizer.h"

using namespace std;

struct state {
	char board[5][6];
	int st_x, st_y;
	int cur_x, cur_y;
	int path[SOLUTION_MAX_LENGTH];
	int len;
	int combos;
	
	void init(int x, int y) {
		st_x = x;
		st_y = y;
		cur_x = x;
		cur_y = y;
		len = 0;
		memcpy(board, initBoard, sizeof(initBoard));
		calcCombos();
	}
	
	void calcCombos() {
		combos = 0;
		char current[5][6], tmp[5][6];
		memcpy(current, board, sizeof(board));
		bool vst[5][6];
		while (true) {
			memcpy(tmp, current, sizeof(current));
			
			// Find matched regions
			for (int i = 0; i < 5; i++)
				for (int j = 0; j < 6; j++)
					if (current[i][j] != ' ' && current[i][j] != 'X') {
						for (int k = 3; k < 7; k++) {
							if (diag[k]) continue;
							int cnt = 1;
							int pi = i, pj = j;
							while (true) {
								pi += dx[k];
								pj += dy[k];
								if (pi < 0 || pi >= 5 || pj < 0 || pj >= 6 || current[pi][pj] != current[i][j]) break;
								cnt++;
							}
							if (cnt >= 3) {
								for (int h = 0; h < cnt; h++)
									tmp[i + dx[k] * h][j + dy[k] * h] = 'X';
							}
						}
					}
			
			// Calculate extra combos
			int extra = 0;
			memset(vst, 0, sizeof(vst));
			for (int i = 0; i < 5; i++)
				for (int j = 0; j < 6; j++)
					if (tmp[i][j] == 'X' && !vst[i][j]) {
						extra++;
						queue< pair<int, int> > q;
						q.push(make_pair(i, j));
						vst[i][j] = 1;
						while (!q.empty()) {
							int cur_i = q.front().first, cur_j = q.front().second;
							q.pop();
							for (int k = 0; k < 8; k++) {
								if (diag[k]) continue;
								int pi = cur_i + dx[k], pj = cur_j + dy[k];
								while (true) {
									if (pi < 0 || pi >= 5 || pj < 0 || pj >= 6) break;
									if (current[pi][pj] != current[cur_i][cur_j] || tmp[pi][pj] != 'X' || vst[pi][pj]) break;
									vst[pi][pj] = 1;
									q.push(make_pair(pi, pj));
								}
							}
						}
					}
			if (extra == 0) break;
			combos += extra;
			
			// Cascade empty spaces
			for (int j = 0; j < 6; j++)
				for (int i = 4; i >= 0; i--)
					if (tmp[i][j] == 'X') {
						for (int h = i; h > 0; h--) tmp[h][j] = tmp[h-1][j];
						tmp[0][j] = ' ';
						i++;
					}
			memcpy(current, tmp, sizeof(tmp));
		}
	}
	
	bool transform(int k) {
		if (len == SOLUTION_MAX_LENGTH - 1) return false;
		if (len > 0 && (path[len - 1] + 4) % 8 == k) return false;
		int new_cur_x = cur_x + dx[k];
		int new_cur_y = cur_y + dy[k];
		if (new_cur_x < 0 || new_cur_x >= 5 || new_cur_y < 0 || new_cur_y >= 6) return false;
		swap(board[cur_x][cur_y], board[new_cur_x][new_cur_y]);
		cur_x = new_cur_x;
		cur_y = new_cur_y;
		path[len++] = k;
		calcCombos();
		return true;
	}
	
	bool operator<(const state& other) const {
		if (combos == other.combos) return len > other.len;
		return combos < other.combos;
	}
};

struct cmp_state {
	bool operator()(const state& lhs, const state& rhs) const {
		if (lhs.combos == rhs.combos) return lhs.len < rhs.len;
		return lhs.combos > rhs.combos;
	}
};

priority_queue<state> list;
priority_queue<state, vector<state>, cmp_state > best;

void initOptimizer() {
	state s;
	for (int i = 0; i < 5; i++)
		for (int j = 0; j < 6; j++) {
			s.init(i, j);
			list.push(s);
			best.push(s);
		}
}

void find() {
	for (int i = 0; i < ITER; i++) {
		vector<state> v;
		while (!list.empty()) {
			state s = list.top();
			list.pop();
			for (int k = 0; k < 8; k++) {
				state tmp = s;
				if (!tmp.transform(k)) continue;
				v.push_back(tmp);
			}
		}
		sort(v.begin(), v.end());
		reverse(v.begin(), v.end());
		for (int j = 0; j < min(MAX_NO_SOLUTION, (int)v.size()); j++) {
			list.push(v[j]);
			best.push(v[j]);
			if (best.size() > MAX_NO_SOLUTION) best.pop();
		}
	}
}

void optimize() {
	initOptimizer();
	find();
	vector<state> v;
	while (!best.empty()) {
		v.push_back(best.top());
		best.pop();
	}
	reverse(v.begin(), v.end());
	for (int i = 0; i < 5; i++) {
		cout<<"Solution "<<i + 1<<": "<<v[i].combos<<" combos."<<endl;
		cout<<"Start at "<<v[i].st_x<<" "<<v[i].st_y<<" with length = "<<v[i].len<<endl;
		for (int j = 0; j < v[i].len; j++) cout<<v[i].path[j]<<" ";
		cout<<endl;
	}
	cout<<endl;
	
	sol.st_x = v[0].st_x;
	sol.st_y = v[0].st_y;
	sol.len = v[0].len;
	for (int i = 0; i < v[0].len; i++) sol.path[i] = v[0].path[i];


	state s;
	s.init(v[0].st_x, v[0].st_y);
	for (int i = 0; i < v[0].len; i++)
		s.transform(v[0].path[i]);
	cout<<"Final layout: "<<endl;
	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 6; j++)
			cout<<s.board[i][j];
		cout<<endl;
	}
	cout<<endl;
}
