#pragma once
#include "../utility/includes.h"
#include "../utility/fps_counter.h"
#include "../input/controls.h"
#include "../input/gui.h"
#include "../scene/camera.h"
#include "../scene/scene.h"
#include "../shader/shader.h"

class Engine {
private:
	// Components
	GLFWwindow* window;
	InputController* controls;
	ShaderManager *shaders;
	Camera *camera;
	FPSCounter *fps_counter;
	SceneManager *scene;
	GUI* gui;

	// Variables
private:
	int window_width;
	int window_height;
public:
	bool shutdown = false;
	bool save = true;

public:
	Engine();
	~Engine();
	bool isRunning();
	void update();
	void render();
private:
	void setup();
};