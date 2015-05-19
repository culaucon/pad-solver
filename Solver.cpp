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

#include "CImg.h"
#include "Common.h"
#include "Classifier.h"
#include "Optimizer.h"

using namespace cimg_library;

using namespace std;

char ORB[6] = {'R', 'G', 'B', 'D', 'L', 'H'};

int dx[8] = {-1, -1, -1, 0, 1, 1, 1, 0};
int dy[8] = {-1, 0, 1, 1, 1, 0, -1, -1};
bool diag[8] = {1, 0, 1, 0, 1, 0, 1, 0};

bool isPlus[5][6];
char initBoard[5][6];
int cnt[6];

CImg<unsigned char> img;
int h, w;
int top_x, top_y;
double sz;

// Event
__attribute__((aligned(1),packed)) struct input_event {
	uint32_t time_dummy_1;
	uint32_t time_dummy_2;
	uint16_t type;
	uint16_t code;
	__signed__ int value;
};

struct input_event event;
vector<uint16_t> type, code;
vector<__signed__ int> value;

struct solution sol;

char temp_files[SOLUTION_MAX_LENGTH][100];

void takeScreenshot() {
	char command[100];
	sprintf(command, "adb shell screencap -p /sdcard/%s", IMG_NAME);
	system(command);
	sprintf(command, "adb pull /sdcard/%s", IMG_NAME);
	system(command);
	sprintf(command, "adb shell rm /sdcard/%s", IMG_NAME);
	system(command);
}

void initImage() {
	img = CImg<unsigned char>(IMG_NAME);
	h = img.height();
	w = img.width();
}

void registerEvent(uint16_t t, uint16_t c, uint32_t v) {
	type.push_back(t);
	code.push_back(c);
	value.push_back(v);
}

void registerCoordinateEvent(int x, int y) {
	int cur_y = (top_y + (y + 0.5) * sz) * 2, cur_x = (top_x + (x + 0.5) * sz) * 2;
	registerEvent(3, 53, cur_y);
	registerEvent(3, 54, cur_x);
	registerEvent(0, 0, 0);
}

void registerFirstEvent() {
	registerEvent(3, 57, 0);
}

void registerFinalEvent() {
	registerEvent(3, 57, 4294967295U);
	registerEvent(0, 0, 0);
}

void writeEvents(int fd) {
	for (int i = 0; i < (int)type.size(); i++) {
		memset(&event, 0, sizeof(event));
		event.type = type[i];
		event.code = code[i];
		event.value = value[i];
		int len = write(fd, &event, sizeof(event));
		if(len < (int)sizeof(event)) {
			cout<<"write failed"<<endl;
			fprintf(stderr, "write event failed\n");
			return;
		}
	}
	type.clear();
	code.clear();
	value.clear();
}

void prepareEventsForSolution() {
	// Prepare temp files
	for (int i = 0; i < sol.len + 2; i++)
		sprintf(temp_files[i], "events/tmp%d.out", i);
	
	// Write first coordinates
	int cur_x = sol.st_x, cur_y = sol.st_y;
	registerFirstEvent();
	registerCoordinateEvent(cur_x, cur_y);
	
	int fd = open(temp_files[0], O_CREAT | O_WRONLY | O_TRUNC, S_ALL);
	if(fd < 0) {
		fprintf(stderr, "could not open %s\n", temp_files[0]);
		return;
	}
	writeEvents(fd);
	
	for (int i = 0; i < sol.len; i++) {
		fd = open(temp_files[i + 1], O_CREAT | O_WRONLY | O_TRUNC, S_ALL);
		cur_x += dx[sol.path[i]];
		cur_y += dy[sol.path[i]];

		registerCoordinateEvent(cur_x, cur_y);
		writeEvents(fd);
	}
	
	// Write final event
	fd = open(temp_files[sol.len + 1], O_CREAT | O_WRONLY | O_TRUNC, S_ALL);
	if(fd < 0) {
		fprintf(stderr, "could not open %s\n", temp_files[0]);
		return;
	}
	registerFinalEvent();
	writeEvents(fd);
}

void dispatchEvents() {
	prepareEventsForSolution();
	
	// Push all the temp files to sdcard
	system("adb push events /mnt/sdcard/");

	char command[10000];
	command[0] = '\0';
	
	// Adb sendevent commands
	int num = sol.len + 2;
	for (int i = 0; i < num - 1; i++) {
		sprintf(command, "%scat /mnt/sdcard/tmp%d.out > /dev/input/event2 && ", command, i);
		sprintf(command, "%ssleep 0.050 && ", command);
	}
	sprintf(command, "%scat /mnt/sdcard/tmp%d.out > /dev/input/event2", command, num - 1);
	printf("%s\n",command);
	
	char echo_command[100];
	sprintf(echo_command, "echo \"%s\" > longcommand.sh", command);
	system(echo_command);
	system("chmod +x longcommand.sh");
	system("adb push longcommand.sh /data/local/tmp");
	system("adb shell sh /data/local/tmp/longcommand.sh");
	
	// Remove temp files
	for (int i = 0; i < num; i++) {
		char rm_command[100];
		sprintf(rm_command, "adb shell \"rm /mnt/sdcard/tmp%d.out\"", i);
		system(rm_command);
		sprintf(rm_command, "rm events/tmp%d.out", i);
		system(rm_command);
	}
}

int main() {
	takeScreenshot();
	initImage();
	processImage();
	optimize();
	dispatchEvents();
}
