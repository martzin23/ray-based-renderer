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
	int getFPS() const;
	float getMS() const;
	void setWindow(GLFWwindow* window) const;
};