#pragma once
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "../utility/includes.h"
#include "../scene/scene.h"
#include "../scene/camera.h"
#include "../shader/shader.h"
#include "../utility/fps_counter.h"
#include "../utility/date_time.h"

class GUI {
public:
	GUI(GLFWwindow* window);
	~GUI();
	void setup();
	void render();
	void build(ShaderManager *shaders, SceneManager *scene, Camera *camera, FPSCounter *fps_counter);

	void updatePallete(ImVec4 color);
	void helpMarker(const char* text);
	void setCursorVisibility(bool show);
//private:
	ImVec4 rgbToHsv(ImVec4 rgb);
	ImVec4 hsvToRgb(ImVec4 hsv);
	ImVec4 tweakColor(ImVec4 color, float hue_multiplier, float sat_multiplier, float val_multiplier, float alpha_multiplier);

public:
	bool gui_active = false;
	bool gui_hovered = false;
	bool scene_autosave = true;
	bool scene_visible = true;
private:
	ImVec4 gui_color;
	bool gui_movable = false;
	bool gui_background = true;
	float gui_alpha = 1.0f;
	float gui_scale = 1.0f;
	int window_width;
	int window_height;
	bool shader_refresh = true;
};