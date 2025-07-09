#pragma once
#include "../utility/includes.h"
#include "../scene/scene.h"
#include "../shader/texture.h"
//#include "../utility/stb_image.h"
//#include "../utility/stb_image_write.h"
#define MAX_BODIES 128

class ShaderManager {
public:
	ShaderManager(GLFWwindow* window, const std::string& vertex_filepath, const std::string& fragment_filepath, const std::string& compute_filepath);
	~ShaderManager();

	void updateUniforms(SceneManager* scene);
	void packUniforms(unsigned int uniform_buffer) const;

private:
	void setupUniforms(GLFWwindow* window);
	void setupScreenTris();
	unsigned int make_module(const std::string& filepath, unsigned int module_type);
	unsigned int make_pipeline_shader(const std::string& vertex_filepath, const std::string& fragment_filepath);
	unsigned int make_compute_shader(const std::string& compute_filepath);
	//void screenshot(const std::string& filepath, GLFWwindow *window);

public:
	// Program IDs
	unsigned int compute;
	unsigned int pipeline;

	// Uniform values
	glm::vec3 camera_position;
	glm::mat4 camera_rotation_matrix;
	glm::vec2 sun_rotation = glm::vec2(30.f, 45.f);
	glm::vec3 light_position = glm::vec3(100.f, 100.f, 100.f);

	int temporal_counter = 1;
	int downsample_factor = 1;
	int shader_mode = 0;
	int body_count;
	int max_bounces = 5;
	int sdf_type = 0;
	int max_marches = 50;
	float fov;
	float epsilon = 0.001f;
	float focus_distance;
	float focus_blur;
	float light_intensity = 1.0;

	int custom_int = 10;
	int custom_int2 = 5;
	float custom_float = -2.0f;
	float custom_float2 = 1.0f;
	float custom_float3 = 1.0f;
	float march_multiplier = 1.0f;
	float custom_normalized = 0.0f;


private:
	// Buffer IDs
	unsigned int color_buffer;
	unsigned int VBO;
	unsigned int VAO;
	unsigned int uniform_buffer;
	unsigned int body_buffer;
	unsigned int material_buffer;
	// Textures
	Texture *heightmap;
};

