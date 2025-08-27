#include "gui.h"

GUI::GUI(GLFWwindow *window) {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 430");

	glfwGetWindowSize(window, &window_width, &window_height);
	ImGui::GetIO().IniFilename = NULL;
	ImGui::GetIO().LogFilename = NULL;

	ImGuiStyle& style = ImGui::GetStyle();
	style.ChildRounding = 
		style.FrameRounding = 
		style.GrabRounding = 
		style.PopupRounding = 
		style.ScrollbarRounding = 
		style.TreeLinesRounding = 
		style.WindowRounding = 6;
	style.TabRounding = 0;
	style.ChildBorderSize =
		style.FrameBorderSize =
		style.PopupBorderSize =
		style.TabBorderSize =
		style.WindowBorderSize = 0;
	style.TabBarBorderSize = 0;
	style.WindowPadding = ImVec2(10, 10);
	style.GrabMinSize = 20;

	this->gui_color = ImVec4(0.302f, 0.271f, 0.431f, 1.f);
	this->updatePallete(gui_color);
}

GUI::~GUI() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void GUI::setup() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void GUI::render() {
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GUI::build(ShaderManager* shader, SceneManager* scene, Camera* camera, FPSCounter* fps_counter) {
	if (!this->gui_active) return;

	//-------------------------------------------------------------------------------

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoCollapse || ImGuiWindowFlags_NoTitleBar;
	if (!this->gui_movable) window_flags |= ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize;
	if (!this->gui_background) window_flags |= ImGuiWindowFlags_NoBackground;
	if (ImGui::IsMouseDown(0) && this->shader_refresh)
		shader->temporal_counter = 0;

	//-------------------------------------------------------------------------------

	ImGui::Begin("Render Settings", 0, window_flags);
	if (!this->gui_movable) ImGui::SetWindowPos(ImVec2(0, 0));
	this->gui_hovered = ImGui::IsWindowHovered() ||
		ImGui::IsAnyItemHovered() || 
		ImGui::IsMouseDragging(0) || 
		ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopupLevel + ImGuiPopupFlags_AnyPopupId);
	ImGui::PushItemFlag(ImGuiItemFlags_NoTabStop, true);

	//-------------------------------------------------------------------------------

	ImGui::Text("%d", fps_counter->getFPS());
	ImGui::SameLine();
	if (ImGui::BeginTabBar("Tabs")) {

		if (ImGui::BeginTabItem("General")) {

			ImGui::SeparatorText("Display");
				ImGui::Text("Resolution");
				ImGui::SameLine();
				if (ImGui::ArrowButton("##+res", ImGuiDir_Up)) shader->downsample_factor = std::max(1, shader->downsample_factor - 1);
				ImGui::SameLine();
				if (ImGui::ArrowButton("##-res", ImGuiDir_Down)) shader->downsample_factor++;
				ImGui::Checkbox("Auto refresh", &shader_refresh);
				helpMarker("Reset screen when clicking so that the changes are immediately visible");
				if (ImGui::Button("Screenshot")) {
					shader->screenshot("res/screenshots/" + getDateTime() + ".jpg", window_width, window_height);
				};
				helpMarker("Save current frame to res/screenshots/.jpg");
			ImGui::SeparatorText("Interface");
				ImGui::Text("Scale");
				ImGui::SameLine();
				if (ImGui::ArrowButton("##+scale", ImGuiDir_Up)) {
					gui_scale *= 1.25f;
					ImGui::SetWindowFontScale(gui_scale);
				}
				ImGui::SameLine();
				if (ImGui::ArrowButton("##-scale", ImGuiDir_Down)) {
					gui_scale /= 1.25f;
					ImGui::SetWindowFontScale(gui_scale);
				}
				ImGui::SameLine();
				if (ImGui::Button("Reset")) {
					gui_scale = 1.f;
					ImGui::SetWindowFontScale(gui_scale);
				};
				ImGui::Checkbox("Movable", &gui_movable);
				ImGui::SameLine();
				ImGui::Checkbox("Background", &gui_background);
				if (ImGui::ColorEdit4("Color", &gui_color.x)) 
					updatePallete(gui_color);
				if (ImGui::SliderFloat("Transparency", &gui_color.w, 0.f, 1.f))
					updatePallete(gui_color);
			ImGui::SeparatorText("Camera");
				ImGui::DragFloat3("Position", &camera->position.x, 0.1f);
				ImGui::DragFloat2("Rotation", &camera->rotation.x, 1.f);
				ImGui::SliderFloat("Speed", &camera->speed, 0.f, 10.f, "%.6f", ImGuiSliderFlags_Logarithmic);
				ImGui::DragFloat("Field of view", &camera->fov, 0.01f, 0.f, FLT_MAX);
				ImGui::InputFloat("Sensitivity", &camera->sensitivity, 0.01f, 0.1f);
				ImGui::SliderFloat("Focus distance", &camera->focus_distance, 0.f, 100.f, "%.6f", ImGuiSliderFlags_Logarithmic);
				ImGui::SliderFloat("Focus blur", &camera->focus_blur, 0.f, 1.f, "%.6f", ImGuiSliderFlags_Logarithmic);
				if (ImGui::Button("Reset position")) camera->position = glm::vec3(0.f, 0.f, 0.f);
				helpMarker("Reset camera position to world origin");
				ImGui::Checkbox("Free movement", &camera->free_movement);
				helpMarker("Movement is unconstrained to world axis (WIP)");
			ImGui::SeparatorText("");

			ImGui::EndTabItem();
		}

		//-------------------------------------------------------------------------------

		if (ImGui::BeginTabItem("RayMarching")) {
			if ((shader->shader_mode != 2) && (shader->shader_mode != 3) && (shader->shader_mode != 4)) {
				shader->shader_mode = 2;
				shader->temporal_counter = 0;
				scene_visible = false;
			}

			ImGui::SeparatorText("View");
				ImGui::Text("Mode");
				ImGui::SameLine();
				if (ImGui::Button("Marches")) shader->shader_mode = 2;
				ImGui::SameLine();
				if (ImGui::Button("Phong")) shader->shader_mode = 3;
				ImGui::SameLine();
				if (ImGui::Button("Pathtraced")) shader->shader_mode = 4;
				helpMarker("Different ways od displaying bodies");
			ImGui::SeparatorText("Marching");
				ImGui::InputInt("Max marches", &shader->max_marches, 50, 200);
				if (shader->max_marches < 1) shader->max_marches = 1;
				helpMarker("Maximum number of steps a ray can take");
				ImGui::SliderFloat("March multiplier", &shader->march_multiplier, 0.f, 1.f);
				helpMarker("Multiplies the size of marches, reduces artifacts, slows performance");
				ImGui::SliderFloat("Epsilon", &shader->epsilon, 0.f, 1.f, "%.6f", ImGuiSliderFlags_Logarithmic);
				helpMarker("Drives precision of calculating normals, tweak to reduce artifacts");
				ImGui::InputInt("Detail", &shader->detail, 1, 5);
				helpMarker("Surface threshold of raymarching, level of detail for fractals");
			ImGui::SeparatorText("Signed Distance Function");
				ImGui::Text("Type");
				ImGui::SameLine();
				if (ImGui::Button("MandelBulb"))
				{
					shader->sdf_type = 1;
					scene_visible = false;
				}
				ImGui::SameLine();
				if (ImGui::Button("MandelBox"))
				{
					shader->sdf_type = 2;
					shader->custom_float2 = -2.f;
					scene_visible = false;
				}
				ImGui::SameLine();
				if (ImGui::Button("Scene"))
				{
					shader->sdf_type = 3;
					scene_visible = true;
				}
				ImGui::SameLine();
				if (ImGui::Button("Heightmap"))
				{
					shader->sdf_type = 4;
					scene_visible = false;
				}
				ImGui::SameLine();
				if (ImGui::Button("Custom"))
				{
					shader->sdf_type = 0;
					scene_visible = false;
				}
				helpMarker("Type of body that is rendered");
				warningMarker("If you can't see anything, try moving closer of further from the origin of the coordinate system!");
				switch (shader->sdf_type) {
				case 1:
					ImGui::DragFloat("Power", &shader->custom_float1, 0.1f);
					helpMarker("Changes the appearance of the fractal");
					break;
				case 2:
					ImGui::DragFloat("Fixed radius", &shader->custom_float1, 0.01f);
					ImGui::DragFloat("Scale", &shader->custom_float2, 0.01f);
					ImGui::DragFloat("Folding limit", &shader->custom_float3, 0.01f);
					ImGui::DragFloat("Min radius", &shader->custom_float4, 0.01f);
					break;
				case 3:
					break;
				case 4:
					ImGui::DragFloat("Height", &shader->custom_float1, 0.01f);
					ImGui::DragFloat("Size", &shader->custom_float2, 0.01f);
					break;
				case 0:
					ImGui::DragFloat("Custom Float", &shader->custom_float1, 0.01f);
					ImGui::DragFloat("Custom Float1", &shader->custom_float2, 0.01f);
					ImGui::DragFloat("Custom Float2", &shader->custom_float3, 0.01f);
					ImGui::DragFloat("Custom Float3", &shader->custom_float4, 0.01f);
					ImGui::InputInt("Custom Int", &shader->custom_int, 1, 5);
					break;
				}
			ImGui::SeparatorText("Lighting");
				ImGui::DragFloat2("Sun rotation", &shader->sun_rotation.x, 1.f, 0.f, 0.f, "%.0f deg");
				//if (ImGui::Button("Set light")) shader->light_position = camera->getPosition();
				//ImGui::DragFloat3("Light position", &shader->light_position.x, 0.001f);
				//ImGui::DragFloat("Light intensity", &shader->light_intensity, 0.01f, 0.f, FLT_MAX);
			ImGui::SeparatorText("");

			ImGui::EndTabItem();
		}

		//-------------------------------------------------------------------------------

		if (ImGui::BeginTabItem("RayTracing")) {
			if ((shader->shader_mode != 0) && (shader->shader_mode != 1)) {
				shader->shader_mode = 0;
				shader->temporal_counter = 0;
				scene_visible = true;
			}

			ImGui::SeparatorText("View");
				ImGui::Text("Mode");
				ImGui::SameLine();
				if (ImGui::Button("RayTraced")) shader->shader_mode = 0;
				ImGui::SameLine();
				if (ImGui::Button("PathTraced")) shader->shader_mode = 1;
			ImGui::SeparatorText("Sampling");
				ImGui::InputInt("Max Bounces", &shader->max_bounces, 1, 10);
				if (shader->max_bounces < 0) shader->max_bounces = 0;
				ImGui::SliderFloat("Epsilon", &shader->epsilon, 0.f, 1.f, "%.6f", ImGuiSliderFlags_Logarithmic);
				//if (shader->epsilon < 0.00001f) shader->epsilon = 0.00001f;
				helpMarker("Arbitrary small number, tweak to reduce artifacts");
			ImGui::SeparatorText("Sun");
				ImGui::DragFloat2("Sun rotation", &shader->sun_rotation.x, 1.f, 0.f, 0.f, "%.0f deg");
			ImGui::SeparatorText("");

			ImGui::EndTabItem();
		}

		//-------------------------------------------------------------------------------

		if (ImGui::BeginTabItem("Editor")) {

			ImGui::SeparatorText("Scene");
				ImGui::Checkbox("Auto save", &scene_autosave);
				helpMarker("Automatically save to res/scenes/last.txt on exit");
				static char load_filepath[128] = "res/scenes/last.txt";
				if (ImGui::Button("Load")) {
					scene_autosave = false;
					scene->reloadScene(load_filepath, camera);
				}
				ImGui::SameLine();
				ImGui::InputText("##load", load_filepath + 11, IM_ARRAYSIZE(load_filepath) - 11);
				helpMarker("Loads scene from res/scenes/...");
				static char save_filepath[128] = "res/scenes/last.txt";
				if (ImGui::Button("Save")) {
					scene->cleanMaterials();
					scene->saveScene(save_filepath, camera);
				}
				ImGui::SameLine();
				ImGui::InputText("##save", save_filepath + 11, IM_ARRAYSIZE(save_filepath) - 11);
				helpMarker("Saves scene to res/scenes/...");
			ImGui::SeparatorText("Body Actions");
				ImGui::Text("Edit mode");
				ImGui::SameLine();
				if (ImGui::Button("Repostition")) scene->setEditMode(0);
				ImGui::SameLine();
				if (ImGui::Button("Resize")) scene->setEditMode(1);
				ImGui::Text("Place");
				ImGui::SameLine();
				if (ImGui::Button("Sphere")) scene->mousePlace(0, nullptr, camera);
				ImGui::SameLine();
				if (ImGui::Button("Box")) scene->mousePlace(1, nullptr, camera);
				ImGui::Text("Selected");
				ImGui::SameLine();
				if (ImGui::Button("Delete")) scene->deleteSelected();
				ImGui::SameLine();
				if (ImGui::Button("Clone")) scene->copySelected();
				helpMarker("Clone the selected body at the same location, duplicates materials");
				ImGui::DragFloat("Snap size", &scene->snap_size, 0.01f, 0.001f, FLT_MAX);
				helpMarker("Snapping grid size of body position and scale");
			ImGui::SeparatorText("Body Settings");
				void* body = scene->getSelectedBody();
				void* material = scene->getSelectedMaterial();
				if (body != nullptr && material != nullptr) {
					if (ImGui::DragFloat3("Position", (float*)body, scene->snap_size)) scene->resetGrabBodies();
					if (ImGui::DragFloat3("Size", (float*)body + 4, scene->snap_size / 2, scene->snap_size / 2, FLT_MAX)) scene->resetGrabBodies();
					ImGui::DragInt("Material Index", (int*)body + 7, 0.1f, 0, scene->getMaterialCount() - 1);
					helpMarker("Index of material from material list, bodies can share material properties");
					ImGui::DragFloat("Emission", (float*)material + 3, 0.1f, 0.f, FLT_MAX);
					ImGui::SliderFloat("Metallic", (float*)material + 5, 0.f, 1.f);
					ImGui::SliderFloat("Roughness", (float*)material + 4, 0.f, 1.f);
					ImGui::ColorPicker3("Color", (float*)material, ImGuiColorEditFlags_PickerHueWheel);
				}
				else {
					ImGui::Text("Select a body with left click!");
				}
			ImGui::SeparatorText("");

			ImGui::EndTabItem();
		}

		//-------------------------------------------------------------------------------

		if (ImGui::BeginTabItem("Info")) {

			ImGui::SeparatorText("App");
			ImGui::BulletText("ESC - exit");
			ImGui::BulletText("ALT - toggle free mouse/disables edit");
			ImGui::BulletText("TAB - toggle GUI");
			ImGui::SeparatorText("Movement");
			ImGui::BulletText("WASDQE - movment");
			ImGui::BulletText("Mouse - look");
			ImGui::BulletText("SHIFT - speed*2");
			ImGui::BulletText("CTRL - speed/2");
			ImGui::SeparatorText("Edit");
			ImGui::BulletText("LMB - select/interact");
			ImGui::BulletText("RMB - deselect");
			ImGui::BulletText("MMB - grab selected");
			ImGui::BulletText("DEL - delete selected");
			ImGui::BulletText("1 - reposition mode");
			ImGui::BulletText("2 - resize mode");
			ImGui::BulletText("C - clone selected");
			ImGui::BulletText("B - place box");
			ImGui::BulletText("O - place sphere");
			ImGui::SeparatorText("");

			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}

	ImGui::PopItemFlag();
	ImGui::End();
}

void GUI::updatePallete(ImVec4 color) {
	ImVec4 primary_normal = color;
	ImVec4 primary_highlight = tweakColor(primary_normal, 1.f, 1.f, 1.5f, 1.f);
	ImVec4 primary_muted = tweakColor(primary_normal, 1.f, 1.f, 1.f, 0.5f);

	ImVec4 secondary_normal = tweakColor(color, 1.f, 0.5f, 1.f, 0.3f);
	ImVec4 secondary_highlight = tweakColor(secondary_normal, 1.f, 1.f, 1.5f, 1.f);
	ImVec4 secondary_muted = tweakColor(secondary_normal, 1.f, 1.f, 1.f, 0.5f);

	ImVec4 background_normal = tweakColor(color, 1.f, 0.25f, 0.25f, 1.f);
	ImVec4 blank = ImVec4();
	ImVec4 text = ImVec4(1.f, 1.f, 1.f, 1.f);

	ImGuiStyle& style = ImGui::GetStyle();
	style.Colors[ImGuiCol_Text] = text;
	style.Colors[ImGuiCol_TextDisabled] = ImVec4(text.x, text.y, text.z, 0.5f * text.w);
	style.Colors[ImGuiCol_WindowBg] = background_normal;
	style.Colors[ImGuiCol_ChildBg] = blank;
	style.Colors[ImGuiCol_PopupBg] = background_normal;
	style.Colors[ImGuiCol_Border] = secondary_muted;
	style.Colors[ImGuiCol_BorderShadow] = secondary_muted;
	style.Colors[ImGuiCol_FrameBg] = secondary_normal;
	style.Colors[ImGuiCol_FrameBgHovered] = secondary_highlight;
	style.Colors[ImGuiCol_FrameBgActive] = secondary_highlight;
	style.Colors[ImGuiCol_TitleBg] = secondary_normal;
	style.Colors[ImGuiCol_TitleBgCollapsed] = secondary_normal;
	style.Colors[ImGuiCol_TitleBgActive] = secondary_highlight;
	style.Colors[ImGuiCol_MenuBarBg] = secondary_muted;
	style.Colors[ImGuiCol_ScrollbarBg] = secondary_muted;
	style.Colors[ImGuiCol_ScrollbarGrab] = secondary_normal;
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = secondary_highlight;
	style.Colors[ImGuiCol_ScrollbarGrabActive] = secondary_highlight;
	style.Colors[ImGuiCol_CheckMark] = primary_normal;
	style.Colors[ImGuiCol_SliderGrab] = primary_normal;
	style.Colors[ImGuiCol_SliderGrabActive] = primary_highlight;
	style.Colors[ImGuiCol_Button] = primary_normal;
	style.Colors[ImGuiCol_ButtonHovered] = primary_highlight;
	style.Colors[ImGuiCol_ButtonActive] = primary_highlight;
	style.Colors[ImGuiCol_Header] = secondary_normal;
	style.Colors[ImGuiCol_HeaderHovered] = secondary_highlight;
	style.Colors[ImGuiCol_HeaderActive] = secondary_highlight;
	style.Colors[ImGuiCol_Separator] = secondary_normal;
	style.Colors[ImGuiCol_SeparatorHovered] = secondary_highlight;
	style.Colors[ImGuiCol_SeparatorActive] = secondary_highlight;
	style.Colors[ImGuiCol_ResizeGrip] = secondary_normal;
	style.Colors[ImGuiCol_ResizeGripHovered] = secondary_highlight;
	style.Colors[ImGuiCol_ResizeGripActive] = secondary_highlight;
	style.Colors[ImGuiCol_Tab] = secondary_normal;
	style.Colors[ImGuiCol_TabHovered] = primary_highlight;
	style.Colors[ImGuiCol_TabSelected] = primary_normal;
	style.Colors[ImGuiCol_TabSelectedOverline] = blank;
	style.Colors[ImGuiCol_TabDimmed] = secondary_muted;
	style.Colors[ImGuiCol_TabDimmedSelected] = secondary_highlight;
	style.Colors[ImGuiCol_TabDimmedSelectedOverline] = blank;
	style.Colors[ImGuiCol_PlotLines] = primary_normal;
	style.Colors[ImGuiCol_PlotLinesHovered] = primary_highlight;
	style.Colors[ImGuiCol_PlotHistogram] = primary_normal;
	style.Colors[ImGuiCol_PlotHistogramHovered] = primary_highlight;
	style.Colors[ImGuiCol_TableHeaderBg] = secondary_highlight;
	style.Colors[ImGuiCol_TableBorderStrong] = blank;
	style.Colors[ImGuiCol_TableBorderLight] = blank;
	style.Colors[ImGuiCol_TableRowBg] = secondary_normal;
	style.Colors[ImGuiCol_TableRowBgAlt] = secondary_muted;
	style.Colors[ImGuiCol_TextLink] = primary_normal;
	style.Colors[ImGuiCol_TextSelectedBg] = secondary_highlight;
	style.Colors[ImGuiCol_DragDropTarget] = primary_normal;
	style.Colors[ImGuiCol_NavCursor] = primary_normal;
	style.Colors[ImGuiCol_NavWindowingDimBg] = secondary_muted;
	style.Colors[ImGuiCol_NavWindowingHighlight] = secondary_highlight;
	style.Colors[ImGuiCol_ModalWindowDimBg] = secondary_muted;
}

void GUI::helpMarker(const char* text) {
	ImGui::SameLine();
	ImGui::TextDisabled("(?)");
	ImGui::SetItemTooltip(text);
}

void GUI::warningMarker(const char* text) {
	ImGui::SameLine();
	ImGui::TextDisabled("(!)");
	ImGui::SetItemTooltip(text);
}

void GUI::setCursorVisibility(bool show) {
	if (show)
		ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
	else
		ImGui::SetMouseCursor(ImGuiMouseCursor_None);
}

ImVec4 GUI::rgbToHsv(ImVec4 rgb) {
	float r = rgb.x;
	float g = rgb.y;
	float b = rgb.z;

	float maxVal = std::max({ r, g, b });
	float minVal = std::min({ r, g, b });
	float delta = maxVal - minVal;

	float h = 0.0f;
	float s = 0.0f;
	float v = maxVal;

	if (delta > 0.0f) {
		s = delta / maxVal;

		if (maxVal == r)
			h = (g - b) / delta;
		else if (maxVal == g)
			h = 2.0f + (b - r) / delta;
		else
			h = 4.0f + (r - g) / delta;

		h /= 6.0f;
		if (h < 0.0f)
			h += 1.0f;
	}
	
	return ImVec4(h, s, v, rgb.w);
}

ImVec4 GUI::hsvToRgb(ImVec4 hsv) {
	float h = hsv.x;
	float s = hsv.y;
	float v = hsv.z;

	if (s <= 0.0f)
		return ImVec4(v, v, v, hsv.w);

	h = std::fmod(h, 1.0f);
	if (h < 0.0f) 
		h += 1.0f;

	h *= 6.0f;
	int sector = static_cast<int>(h);
	float fractional = h - sector;

	float p = v * (1.0f - s);
	float q = v * (1.0f - s * fractional);
	float t = v * (1.0f - s * (1.0f - fractional));

	switch (sector) {
		case 0: return ImVec4(v, t, p, hsv.w);
		case 1: return ImVec4(q, v, p, hsv.w);
		case 2: return ImVec4(p, v, t, hsv.w);
		case 3: return ImVec4(p, q, v, hsv.w);
		case 4: return ImVec4(t, p, v, hsv.w);
		case 5: return ImVec4(v, p, q, hsv.w);
		default: return ImVec4(v, p, q, hsv.w);
	}
}

ImVec4 GUI::tweakColor(ImVec4 color, float hue_multiplier, float sat_multiplier, float val_multiplier, float alpha_multiplier) {
	ImVec4 hsv = rgbToHsv(color);
	ImVec4 multiplied_hsv = ImVec4(hsv.x * hue_multiplier, hsv.y * sat_multiplier, hsv.z * val_multiplier, hsv.w * alpha_multiplier);
	return hsvToRgb(multiplied_hsv);
}