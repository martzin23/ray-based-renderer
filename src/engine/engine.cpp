#include "engine.h"

Engine::Engine() {
	this->setup();

	this->shaders = new ShaderManager(window, "res/shaders/vertex.glsl", "res/shaders/fragment.glsl", "res/shaders/compute.glsl");
	this->camera = new Camera();
	this->scene = new SceneManager();
	this->controls = new InputController(window);
	this->gui = new GUI(window);
	this->fps_counter = new FPSCounter();

	glfwSetCursorPos(window, window_width / 2.0, window_height / 2.0);
	scene->loadScene("res/scenes/last.txt", camera);
}

void Engine::setup() {

	if (!glfwInit()) {
		std::cout << "Failed to initialise GLFW!" << std::endl;
		this->shutdown = true;
		return;
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	glfwWindowHint(GLFW_RED_BITS, mode->redBits);
	glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
	glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
	glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
	this->window = glfwCreateWindow(mode->width, mode->height, "Renderer", glfwGetPrimaryMonitor(), NULL);
	glfwGetWindowSize(window, &window_width, &window_height);

	if (window == NULL) {
		std::cout << "Failed to create GLFW window!" << std::endl;
		glfwTerminate();
		this->shutdown = true;
		return;
	}

	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to load opengl!" << std::endl;
		glfwTerminate();
		this->shutdown = true;
		return;
	}
}

bool Engine::isRunning() {
	return !glfwWindowShouldClose(window) 
		and !(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) 
		and !shutdown;
}

void Engine::update() {
	glfwPollEvents();
	fps_counter->update();

	controls->updateGUI(gui);
	controls->updateShaderVariables(shaders);
	controls->updateCameraMovement(camera);
	controls->updateEditor(scene, camera);

	scene->checkHover(window, camera);
	scene->updateGrabEdit(window, camera);

	if (controls->triggered()) shaders->temporal_counter = 0;
	shaders->temporal_counter++;
}

void Engine::render() {
	glClearColor(1.f, 0.f, 1.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT);

	shaders->camera_position = camera->getPosition();
	shaders->camera_rotation_matrix = camera->getRotationMatrix();
	shaders->body_count = scene->getBodyCount();
	shaders->fov = camera->fov;
	shaders->focus_distance = camera->focus_distance;
	shaders->focus_blur = camera->focus_blur;
	shaders->updateUniforms(scene);

	const unsigned int local_size = 32;
	const unsigned int workgroup_size_x = (window_width + local_size * shaders->downsample_factor - 1) / (local_size * shaders->downsample_factor);
	const unsigned int workgroup_size_y = (window_height + local_size * shaders->downsample_factor - 1) / (local_size * shaders->downsample_factor);

	glUseProgram(shaders->compute);
	glDispatchCompute(workgroup_size_x, workgroup_size_y, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	glUseProgram(shaders->pipeline);
	glDrawArrays(GL_TRIANGLES, 0, 3);

	gui->setup();
	gui->build(shaders, scene, camera, fps_counter);
	gui->render();

	glfwSwapBuffers(window);
}

Engine::~Engine() {
	scene->deselectBody();
	if (gui->scene_autosave) {
		scene->cleanMaterials();
		scene->saveScene("res/scenes/last.txt", camera);
	}
	//scene->printDebugData();

	delete controls;
	delete shaders;
	delete camera;
	delete fps_counter;
	delete scene;
	delete gui;

	glfwDestroyWindow(window);
	glfwTerminate();
}
