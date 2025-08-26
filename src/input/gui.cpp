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

	this->primary_color = ImVec4(0.302f, 0.271f, 0.431f, 1.f);
	this->secondary_color = ImVec4(0.231f, 0.259f, 0.322f, 0.5f);
	this->background_color = ImVec4(secondary_color.x * 0.5f, secondary_color.y * 0.5f, secondary_color.z * 0.5f, 1.f);
	this->updatePallete(primary_color, secondary_color, background_color);
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

void GUI::build(ShaderManager* shader, SceneManager* scene, Camera* camera, FPSCounter* fps_counter, GLFWwindow* window) {
	if (!this->gui_active) return;

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
					shader->screenshot("res/screenshots/" + getDateTime() + ".jpg", window);
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
				if (ImGui::ColorEdit4("Primary color", &primary_color.x)) 
					updatePallete(primary_color, secondary_color, background_color);
				if (ImGui::ColorEdit4("Secondary color", &secondary_color.x)) 
					updatePallete(primary_color, secondary_color, background_color);
				if (ImGui::ColorEdit4("Background color", &background_color.x)) 
					updatePallete(primary_color, secondary_color, background_color);
			ImGui::SeparatorText("Camera");
				ImGui::Checkbox("Free movement", &camera->free_movement);
				helpMarker("Movement is unconstrained to world axis (WIP)");
				if (ImGui::Button("Reset position")) camera->position = glm::vec3(0.f, 0.f, 0.f);
				helpMarker("Reset camera position to world origin");
				ImGui::DragFloat3("Position", &camera->position.x, 0.1f);
				ImGui::DragFloat2("Rotation", &camera->rotation.x, 1.f);
				ImGui::SliderFloat("Speed", &camera->speed, 0.f, 10.f, "%.6f", ImGuiSliderFlags_Logarithmic);
				ImGui::DragFloat("Field of view", &camera->fov, 0.01f, 0.f, FLT_MAX);
				ImGui::InputFloat("Sensitivity", &camera->sensitivity, 0.01f, 0.1f);
				ImGui::DragFloat("Focus distance", &camera->focus_distance, 0.001f, 0.f, FLT_MAX);
				//ImGui::DragFloat("Focus blur", &camera->focus_blur, 0.00005f, 0.f, FLT_MAX);
				ImGui::SliderFloat("Focus blur", &camera->focus_blur, 0.f, 1.f, "%.6f", ImGuiSliderFlags_Logarithmic);
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
				if (ImGui::Button("PBR")) shader->shader_mode = 4;
				helpMarker("Different ways od displaying bodies");
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
					scene_visible = false;
				}
				ImGui::SameLine();
				if (ImGui::Button("Scene")) 
				{
					shader->sdf_type = 3;
					scene_visible = true;
				}
				ImGui::SameLine();
				if (ImGui::Button("Custom")) 
				{
					shader->sdf_type = 0;
					scene_visible = false;
				}
				helpMarker("Type of body that is rendered");
			ImGui::SeparatorText("Marching");
				ImGui::InputInt("Max marches", &shader->max_marches, 10, 50);
				if (shader->max_marches < 1) shader->max_marches = 1;
				helpMarker("Maximum number of steps a ray takes before it gives up");
				ImGui::SliderFloat("March multiplier", &shader->march_multiplier, 0.f, 1.f);
				helpMarker("Multiplies the size of marches, reduces artifacts");
				ImGui::InputInt("Detail", &shader->custom_int2, 1, 5);
				helpMarker("Surface threshold of raymarching, level of detail for fractals");
				ImGui::InputInt("Iterations", &shader->custom_int, 1, 5);
				helpMarker("Number of iterations for calculating fractals, reduces artifacts");
				ImGui::SliderFloat("Variant", &shader->custom_normalized, 0.f, 1.f);
				helpMarker("Changes the appearance of the fractal");
			ImGui::SeparatorText("Custom variables");
				ImGui::DragFloat("Custom Float1", &shader->custom_float, 0.01f);
				ImGui::DragFloat("Custom Float2", &shader->custom_float2, 0.01f);
				ImGui::DragFloat("Custom Float3", &shader->custom_float3, 0.01f);
			ImGui::SeparatorText("Lighting");
				ImGui::DragFloat2("Sun rotation", &shader->sun_rotation.x, 1.f, 0.f, 0.f, "%.0f deg");
				if (ImGui::Button("Set light")) shader->light_position = camera->getPosition();
				ImGui::DragFloat3("Light position", &shader->light_position.x, 0.001f);
				ImGui::DragFloat("Light intensity", &shader->light_intensity, 0.01f, 0.f, FLT_MAX);
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
				if (ImGui::Button("PathTraced")) shader->shader_mode = 1;
				ImGui::SameLine();
				if (ImGui::Button("RayTraced")) shader->shader_mode = 0;
			ImGui::SeparatorText("Sampling");
				ImGui::InputInt("Max Bounces", &shader->max_bounces, 1, 10);
				if (shader->max_bounces < 0) shader->max_bounces = 0;
				//ImGui::InputFloat("Epsilon", &shader->epsilon, 0.001f, 0.01f);
				ImGui::SliderFloat("Epsilon", &shader->epsilon, 0.f, 1.f, "%.6f", ImGuiSliderFlags_Logarithmic);
				//if (shader->epsilon < 0.00001f) shader->epsilon = 0.00001f;
				helpMarker("Used when a small number is needed but isn't zero");
			ImGui::SeparatorText("Custom variables");
				ImGui::InputInt("Custom Int", &shader->custom_int, 1, 10);
				ImGui::InputInt("Custom Int2", &shader->custom_int2, 1, 10);
				ImGui::SliderFloat("Custom Norm", &shader->custom_normalized, 0.f, 1.f);
				ImGui::DragFloat("Custom Float1", &shader->custom_float, 0.01f);
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


void GUI::updatePallete(ImVec4 primary, ImVec4 secondary, ImVec4 background) {

	ImVec4 blank = ImVec4();

	ImVec4 primary_normal = primary;
	ImVec4 primary_highlight = ImVec4(primary_normal.x * 1.5f, primary_normal.y * 1.5f, primary_normal.z * 1.5f, primary_normal.w);
	ImVec4 primary_muted = ImVec4(primary_normal.x, primary_normal.y, primary_normal.z, primary_normal.w * 0.5f);

	//ImVec4 secondary_normal = ImVec4(primary.x, primary.y, primary.z, primary.w * 0.3f);
	ImVec4 secondary_normal = secondary;
	ImVec4 secondary_highlight = ImVec4(secondary_normal.x * 1.5f, secondary_normal.y * 1.5f, secondary_normal.z * 1.5f, secondary_normal.w);
	ImVec4 secondary_muted = ImVec4(secondary_normal.x, secondary_normal.y, secondary_normal.z, secondary_normal.w * 0.5f);

	//ImVec4 background_normal = ImVec4(primary.x * 0.25f, primary.y * 0.25f, primary.z * 0.25f, primary.w);
	ImVec4 background_normal = background;

	ImGuiStyle& style = ImGui::GetStyle();
	style.Colors[ImGuiCol_Text] = ImVec4(1.f, 1.f, 1.f, 1.f);
	style.Colors[ImGuiCol_TextDisabled] = ImVec4(1.f, 1.f, 1.f, 0.5f);
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

void GUI::showCursor(bool show) {
	if (show)
		ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
	else
		ImGui::SetMouseCursor(ImGuiMouseCursor_None);
}

