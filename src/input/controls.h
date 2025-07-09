#pragma once
#include "../utility/includes.h"
#include "../shader/shader.h"
#include "../scene/scene.h"
#include "../scene/camera.h"
#include "../input/gui.h"

class InputController {
private:
	GLFWwindow* window;
	bool trigger;
	bool camera_movable;
	bool scene_editable;

	bool UP, DOWN, LEFT, RIGHT;
	bool LMB, RMB, MMB;
	bool TAB, DEL, ALT, CTRL, SHIFT;
	bool O, B, C;
	bool K1, K2;

public:
	InputController(GLFWwindow* window);
	bool triggered();

	void updateCameraMovement(Camera* camera);
	void updateEditor(SceneManager* scene, Camera* camera);
	void updateShaderVariables(ShaderManager* shader);
	void updateGUI(GUI* gui);
};