#pragma once
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "../utility/includes.h"
#include "../scene/scene.h"
#include "../scene/camera.h"
#include "../shader/shader.h"
#include "../utility/fps_counter.h"

class GUI {
public:
	GUI(GLFWwindow* window);
	~GUI();
	void setup();
	void render();
	void build(ShaderManager *shaders, SceneManager *scene, Camera *camera, FPSCounter *fps_counter);

	void updatePallete(ImVec4 primary, ImVec4 secondary, ImVec4 background);
	void helpMarker(const char* text);
	void showCursor(bool show);

private:
	ImVec4 primary_color;
	ImVec4 secondary_color;
	ImVec4 background_color;
	int window_width;
	int window_height;
	bool gui_movable = false;
	bool gui_background = false;
	float gui_scale = 1.0f;
	bool shader_refresh = true;
public:
	bool gui_active = false;
	bool gui_hovered = false;
	bool scene_autosave = true;
	bool scene_visible = true;
};