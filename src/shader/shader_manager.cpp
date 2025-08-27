#include "shader.h"

ShaderManager::ShaderManager(GLFWwindow* window, const std::string& vertex_filepath, const std::string& fragment_filepath, const std::string& compute_filepath) {
	pipeline = makePipelineShader(vertex_filepath, fragment_filepath);
	compute = makeComputeShader(compute_filepath);
	setupUniforms(window);
	setupScreenTris();
}

ShaderManager::~ShaderManager() {
	delete heightmap;

	glDeleteProgram(compute);
	glDeleteProgram(pipeline);

	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);

	glDeleteTextures(1, &color_buffer);
}

void ShaderManager::setupUniforms(GLFWwindow *window) {
	int window_height, window_width;
	glfwGetWindowSize(window, &window_height, &window_width);

	glGenTextures(1, &color_buffer);
	glBindTexture(GL_TEXTURE_2D, color_buffer);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, window_height, window_width);

	glGenBuffers(1, &uniform_buffer);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, uniform_buffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(GLfloat) * 44, NULL, GL_DYNAMIC_DRAW);

	glGenBuffers(1, &body_buffer);
	glBindBufferBase(GL_UNIFORM_BUFFER, 2, body_buffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(GLfloat) * 8 * MAX_BODIES, NULL, GL_DYNAMIC_DRAW);

	glGenBuffers(1, &material_buffer);
	glBindBufferBase(GL_UNIFORM_BUFFER, 3, material_buffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(GLfloat) * 8 * MAX_BODIES, NULL, GL_DYNAMIC_DRAW);

	heightmap = new Texture("res/textures/heightmap3.png", 1, GL_LINEAR);
}

void ShaderManager::updateUniforms(SceneManager* scene) {

	glUseProgram(compute);
	glBindImageTexture(0, color_buffer, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	heightmap->Bind();

	scene->packMaterials(material_buffer);
	scene->packBodies(body_buffer);
	packUniforms(uniform_buffer);

	glUseProgram(pipeline);
	glUniform1i(glGetUniformLocation(pipeline, "downsample_factor"), downsample_factor);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, color_buffer);
}

void ShaderManager::setupScreenTris() {
	std::vector<float> positions = {
		-1.0f,  3.0f, 0.0f,
		 3.0f, -1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f
	};

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(float), positions.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(VAO);
}

void ShaderManager::packUniforms(unsigned int uniform_buffer) const {

	glm::mat4 sun_matrix = rotate(glm::mat4(1.0), glm::radians(this->sun_rotation.x), glm::vec3{ 0.0, 0.0, -1.0 });
	sun_matrix = rotate(sun_matrix, glm::radians(-this->sun_rotation.y), glm::vec3{ 0.0, 1.0, 0.0 });
	glm::vec3 sun_direction = glm::vec3(sun_matrix * glm::vec4(1.0, 0.0, 0.0, 0.0));
	sun_direction = glm::normalize(sun_direction);

	struct PackedUniforms {
		int int1;
		int int2;
		int int3;
		int int4;

		glm::mat4 matrix1;

		glm::vec3 vec1;
		int int5;

		int int6;
		int int7;
		float float1;
		float float2;

		int int8;
		float float3;
		float float4;
		float float5;

		glm::vec3 vec2;
		int int9;

		float float6;
		float float7;
		float float8;
		float float9;

		glm::vec3 vec3;
		float float10;
	};

	PackedUniforms uniforms = {
		this->temporal_counter,
		this->downsample_factor,
		this->body_count,
		this->shader_mode,

		this->camera_rotation_matrix,

		this->camera_position,
		this->sdf_type,

		this->max_bounces,
		this->max_marches,
		this->fov,
		this->epsilon,

		this->custom_int,
		this->custom_float1,
		this->custom_float2,
		this->march_multiplier,

		sun_direction,
		this->detail,

		this->focus_distance,
		this->focus_blur,
		this->custom_float3,
		this->custom_float4,

		this->light_position,
		this->light_intensity
	};
	std::vector<PackedUniforms> data = { uniforms };

	glBindBuffer(GL_UNIFORM_BUFFER, uniform_buffer);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(PackedUniforms), data.data());
}

void ShaderManager::screenshot(const std::string& filepath, int window_width, int window_height) const {

	unsigned int texture;
	unsigned int render_buffer;
	unsigned int frame_buffer;

	//https://blog.42yeah.is/opengl/2023/05/27/framebuffer-export.html

	// Generate texture
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, window_width, window_height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

	// Generate RBO
	glGenRenderbuffers(1, &render_buffer);
	glBindRenderbuffer(GL_RENDERBUFFER, render_buffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, window_width, window_height);

	glGenFramebuffers(1, &frame_buffer);
	glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, render_buffer);

	glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);

	// Bind frame buffer
	glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
	glViewport(0, 0, window_width, window_height);

	// Draw
	glUseProgram(pipeline);
	glUniform1i(glGetUniformLocation(pipeline, "downsample_factor"), downsample_factor);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, color_buffer);
	glDrawArrays(GL_TRIANGLES, 0, 3);

	 // Read texture
	std::unique_ptr<unsigned char[]> data = std::make_unique<unsigned char[]>(sizeof(unsigned int) * window_width * window_height * 3);
	glBindTexture(GL_TEXTURE_2D, texture);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, data.get());

	// Save image
	stbi_flip_vertically_on_write(true);
	int ret = stbi_write_jpg(filepath.c_str(), window_width, window_height, 3, data.get(), 100);

	// Clean up
	glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
	glViewport(0, 0, window_width, window_height);

	glDeleteTextures(1, &texture);
	glDeleteRenderbuffers(1, &render_buffer);
	glDeleteFramebuffers(1, &frame_buffer);
}