#pragma once
#include "../utility/includes.h"

class FPSCounter {
private:
	double last_time;
	int num_frames;
	double frame_time;
	int frame_rate;

public:
	FPSCounter();
	void update();
	int getFPS();
	float getMS();
	void setWindow(GLFWwindow* window);
};