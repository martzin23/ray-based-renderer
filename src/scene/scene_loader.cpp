#include "scene.h"

void SceneManager::loadScene(const std::string& filepath, Camera *camera) {
	std::ifstream file;
	std::string line;

	file.open(filepath);
	while (getline(file, line)) {
	
		char line_type = line[0];
		if (line_type == 'c') {
			std::stringstream split_line(line);
			std::string tempstr;

			glm::vec3 pos;
			glm::vec2 rot;
			float speed, sens;

			getline(split_line, tempstr, ' ');
			getline(split_line, tempstr, ' ');
			pos = readVec3(tempstr);
			getline(split_line, tempstr, ' ');
			rot = readVec2(tempstr);
			getline(split_line, tempstr, ' ');
			speed = stof(tempstr);
			getline(split_line, tempstr, ' ');
			sens = stof(tempstr);

			camera->position = pos;
			camera->rotation = rot;
			camera->speed = speed;
			camera->sensitivity = sens;
		}

		//------------------------------------

		else if (line_type == 'b') {
			std::stringstream split_line(line);
			std::string tempstr;

			glm::vec3 pos;
			glm::vec3 size;
			int mat;

			getline(split_line, tempstr, ' ');
			getline(split_line, tempstr, ' ');
			pos = readVec3(tempstr);
			getline(split_line, tempstr, ' ');
			size = readVec3(tempstr);
			getline(split_line, tempstr, ' ');
			mat = stoi(tempstr);

			addBox(pos, size, mat);
		}

		//------------------------------------

		else if (line_type == 's') {
			std::stringstream split_line(line);
			std::string tempstr;

			glm::vec3 pos;
			float size;
			int mat;

			getline(split_line, tempstr, ' ');
			getline(split_line, tempstr, ' ');
			pos = readVec3(tempstr);
			getline(split_line, tempstr, ' ');
			size = stof(tempstr);
			getline(split_line, tempstr, ' ');
			mat = stoi(tempstr);

			addSphere(pos, size, mat);
		}

		//------------------------------------

		else if (line_type == 'm') {
			std::stringstream split_line(line);
			std::string tempstr;

			glm::vec3 col;
			float emi, rou, met;

			getline(split_line, tempstr, ' ');
			getline(split_line, tempstr, ' ');
			col = readVec3(tempstr);
			getline(split_line, tempstr, ' ');
			emi = stof(tempstr);
			getline(split_line, tempstr, ' ');
			rou = stof(tempstr);
			getline(split_line, tempstr, ' ');
			met = stof(tempstr);

			addMaterial(col, emi, rou, met);
		}

	}
	file.close();
}

void SceneManager::saveScene(const std::string& filepath, Camera* camera) {
	std::ofstream file;
	std::stringstream line;
	file.open(filepath);

	//------------------------------------

	glm::vec3 pos = camera->position;
	glm::vec2 rot = camera->rotation;
	float speed = camera->speed;
	float sens = camera->sensitivity;
	line << "c " << pos.x << "/" << pos.y << "/" << pos.z << " " << rot.x << "/" << rot.y << " " << speed << " " << sens;

	file << line.str() << "\n";
	line.str("");
	file << "\n";

	//------------------------------------

	for (int i = 0; i < body_count; i++) {
		Body b = bodies[i];

		if (b.type == 0) {
			line << "s ";
			line << b.position.x << "/" << b.position.y << "/" << b.position.z << " " << b.size.x << " " << b.material;
		}
		else if (b.type == 1) {
			line << "b ";
			line << b.position.x << "/" << b.position.y << "/" << b.position.z << " " << b.size.x << "/" << b.size.y << "/" << b.size.z << " " << b.material;
		}

		file << line.str() << "\n";
		line.str("");
	}
	file << "\n";

	//------------------------------------

	for (int i = 0; i < material_count; i++) {
		Material m = materials[i];

		line << "m " << m.color.x << "/" << m.color.y << "/" << m.color.z << " " << m.emission << " " << m.roughness << " " << m.metallic;
		file << line.str() << "\n";
		line.str("");
	}

	//------------------------------------

	file.close();
}

void SceneManager::reloadScene(const std::string& filepath, Camera* camera) {
	deselectBody();
	this->body_count = 0;
	this->material_count = 0;
	loadScene(filepath, camera);
}

glm::vec3 SceneManager::readVec3(const std::string& string) {
	std::stringstream split_str(string);
	std::string tempstr;

	float x, y, z;
	getline(split_str, tempstr, '/');
	x = stof(tempstr);
	getline(split_str, tempstr, '/');
	y = stof(tempstr);
	getline(split_str, tempstr, '/');
	z = stof(tempstr);
	return glm::vec3{ x, y, z };
}

glm::vec2 SceneManager::readVec2(const std::string& string) {
	std::stringstream split_str(string);
	std::string tempstr;

	float x, y;
	getline(split_str, tempstr, '/');
	x = stof(tempstr);
	getline(split_str, tempstr, '/');
	y = stof(tempstr);
	return glm::vec2{ x, y };
}

