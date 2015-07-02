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
	int num_combos;
	int combo_num_orbs[30];
	char combo_type[30];
	int weight;
	
	void init(int x, int y) {
		st_x = x;
		st_y = y;
		cur_x = x;
		cur_y = y;
		len = 0;
		memcpy(board, initBoard, sizeof(initBoard));
		calcCombos();
		calcWeight();
	}
	
	void calcCombos() {
		num_combos = 0;
		char current[5][6];
		memcpy(current, board, sizeof(board));
		bool is_matched[5][6], vst[5][6];
		while (true) {
			memset(is_matched, 0, sizeof(is_matched));
					
			// Mark matched combos
			for (int i = 0; i < 5; i++)
				for (int j = 0; j < 6; j++)
					if (current[i][j] != ' ' && current[i][j] != 'X') {
						for (int k = 3; k < 7; k++) { // only rightward and downward, prevent re-computation
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
									is_matched[i + dx[k] * h][j + dy[k] * h] = 1;
							}
						}
					}
			
			// Connect matched combos with same color
			bool is_extra_combo = 0;
			memset(vst, 0, sizeof(vst));
			for (int i = 0; i < 5; i++)
				for (int j = 0; j < 6; j++)
					if (is_matched[i][j] && !vst[i][j]) {
						is_extra_combo = 1;
						queue< pair<int, int> > q;
						q.push(make_pair(i, j));
						vst[i][j] = 1;
						int current_orb_cnt = 0;
						while (!q.empty()) {
							int cur_i = q.front().first, cur_j = q.front().second;
							q.pop();
							current_orb_cnt++;
							for (int k = 0; k < 8; k++) {
								if (diag[k]) continue;
								int pi = cur_i + dx[k], pj = cur_j + dy[k];
								while (true) {
									if (pi < 0 || pi >= 5 || pj < 0 || pj >= 6) break;
									if (current[pi][pj] != current[cur_i][cur_j] || !is_matched[pi][pj] || vst[pi][pj]) break;
									vst[pi][pj] = 1;
									q.push(make_pair(pi, pj));
								}
							}
						}
						
						combo_num_orbs[num_combos] = current_orb_cnt;
						combo_type[num_combos++] = current[i][j];
					}
			
			if (!is_extra_combo) break; // no other combos found
			
			// Cascade empty spaces
			for (int j = 0; j < 6; j++) // Column by column
				for (int i = 4; i >= 0; i--) // Bottom up
					if (is_matched[i][j]) {
						for (int h = i; h > 0; h--) {
							is_matched[h][j] = is_matched[h - 1][j];
							current[h][j] = current[h - 1][j];
						}
						current[0][j] = ' '; // mark the top most orb to be empty to cascade down
						is_matched[0][j] = 0; // clear is_matched flag for top most orb
						i++; // re-cascade this row
					}
		}
	}
	
	void calcWeight() {
		weight = num_combos;
		if (team == TEAM_COMBOS) {
		} else if (team == TEAM_GZL) {
			for (int i = 0; i < num_combos; i++)
				if (combo_num_orbs[i] == 4 && combo_type[i] == 'G') weight += TPA_WEIGHT;
		} else if (team == TEAM_ATHENA) {
			for (int i = 0; i < num_combos; i++)
				if (combo_num_orbs[i] == 4 && combo_type[i] == 'L') weight += TPA_WEIGHT;
		} else if (team == TEAM_LKALI) {
			bool is_L = 0, is_R = 0, is_B = 0, is_D = 0;
			for (int i = 0; i < num_combos; i++) {
				if (combo_type[i] == 'L') is_L = 1;
				if (combo_type[i] == 'R') is_R = 1;
				if (combo_type[i] == 'B') is_B = 1;
				if (combo_type[i] == 'D') is_D = 1;
				if (combo_num_orbs[i] == 4 && combo_type[i] == 'L') weight += TPA_WEIGHT;
			}
			if (is_L && is_R && is_B && is_D) weight += LKALI_ACTIVATE;
		} else if (team == TEAM_YAMATO) {
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
		calcWeight();
		return true;
	}
	
	bool operator<(const state& other) const {
		if (weight == other.weight) return len > other.len;
		return weight < other.weight;
	}
};

struct cmp_state {
	bool operator()(const state& lhs, const state& rhs) const {
		if (lhs.weight == rhs.weight) return lhs.len < rhs.len;
		return lhs.weight > rhs.weight;
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
		sort(v.begin(), v.end(), cmp_state());
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
	
	cout<<"Best solution: "<<v[0].num_combos<<" combos."<<endl;
	cout<<"Start at "<<v[0].st_x<<" "<<v[0].st_y<<" with length = "<<v[0].len<<endl;
	for (int j = 0; j < v[0].len; j++) cout<<v[0].path[j]<<" ";
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
