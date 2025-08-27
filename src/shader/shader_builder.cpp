#include "shader.h"

unsigned int ShaderManager::makeModule(const std::string& filepath, unsigned int module_type) {
	std::ifstream file;
	std::string line;
	std::stringstream buffered_lines;

	file.open(filepath);
	while (getline(file, line)) {
		buffered_lines << line << "\n";
	}

	std::string shader_source_string = buffered_lines.str();
	const char* shader_source = shader_source_string.c_str();
	buffered_lines.str("");
	file.close();

	unsigned int shader_module = glCreateShader(module_type);
	glShaderSource(shader_module, 1, &shader_source, NULL);
	glCompileShader(shader_module);

	int success;
	glGetShaderiv(shader_module, GL_COMPILE_STATUS, &success);
	if (!success) {
		char errorLog[1024];
		glGetShaderInfoLog(shader_module, 1024, NULL, errorLog);
		std::cout << "Shader Module compilation error:\n" << errorLog << std::endl;
	}

	return shader_module;
}

unsigned int ShaderManager::makePipelineShader(const std::string& vertex_filepath, const std::string& fragment_filepath) {
	std::vector<unsigned int> modules;
	modules.push_back(makeModule(vertex_filepath, GL_VERTEX_SHADER));
	modules.push_back(makeModule(fragment_filepath, GL_FRAGMENT_SHADER));

	unsigned int shader_program = glCreateProgram();
	for (unsigned int shader_module : modules) {
		glAttachShader(shader_program, shader_module);
	}
	glLinkProgram(shader_program);

	int success;
	glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
	if (!success) {
		char errorLog[1024];
		glGetProgramInfoLog(shader_program, 1024, NULL, errorLog);
		std::cout << "Shader Linking error:\n" << errorLog << std::endl;
	}

	for (unsigned int shader_module : modules) {
		glDeleteShader(shader_module);
	}

	return shader_program;
}

unsigned int ShaderManager::makeComputeShader(const std::string& compute_filepath) {
	unsigned int shader_module = makeModule(compute_filepath, GL_COMPUTE_SHADER);

	unsigned int shader_program = glCreateProgram();
	glAttachShader(shader_program, shader_module);
	glLinkProgram(shader_program);

	int success;
	glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
	if (!success) {
		char errorLog[1024];
		glGetProgramInfoLog(shader_program, 1024, NULL, errorLog);
		std::cout << "Shader Linking error:\n" << errorLog << std::endl;
	}

	glDeleteShader(shader_module);
	return shader_program;
}