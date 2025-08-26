#include "fps_counter.h"

FPSCounter::FPSCounter() {
	last_time = glfwGetTime();
	num_frames = 0;
	frame_time = 0.0;
	frame_rate = 0;
}

void FPSCounter::update() {
	double current_time = glfwGetTime();
	double delta = current_time - last_time;

	if (delta >= 1.0) {
		frame_rate = std::max(1, int(num_frames / delta));
		last_time = current_time;
		num_frames = -1;
		frame_time = float(1000.0 / frame_rate);
	}
	++num_frames;
}

int FPSCounter::getFPS() const {
	return frame_rate;
}

float FPSCounter::getMS() const {
	return (float)frame_time;
}

void FPSCounter::setWindow(GLFWwindow* window) const {
	std::stringstream title;
	title << frame_rate << " fps / " << frame_time << " ms";
	glfwSetWindowTitle(window, title.str().c_str());
}