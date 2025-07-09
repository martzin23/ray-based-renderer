#include "scene.h"

SceneManager::SceneManager() {
	bodies.resize(MAX_BODIES);
	materials.resize(MAX_BODIES);
	body_count = 0;
	material_count = 0;
	edit_mode = 0;
	snap_size = 0.1f;

	selected_index = -1;
	selected_distance = 0;
	hovered_index = -1;

	grab_ready = false;
	grab_active = false;
	grab_start_direction = glm::vec3(0.0);
	grab_start_position = glm::vec3(0.0);
	grab_start_size = glm::vec3(0.0);
	grab_vector = glm::vec3(0.0);
}

int SceneManager::getBodyCount() const {
	return body_count;
}

int SceneManager::getMaterialCount() const {
	return material_count;
}

int SceneManager::getSelectedIndex() const {
	return selected_index;
}

void SceneManager::setEditMode(int mode) {
	edit_mode = mode;
	destroyGrabBodies();
	createGrabBodies();
}

int SceneManager::getEditMode() const {
	return this->edit_mode;
}

bool SceneManager::isGrabActive() const {
	return this->grab_active;
}

void SceneManager::packBodies(unsigned int body_buffer) {
	glBindBuffer(GL_UNIFORM_BUFFER, body_buffer);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Body)* MAX_BODIES, bodies.data());
}

void SceneManager::packMaterials(unsigned int material_buffer) {
	glBindBuffer(GL_UNIFORM_BUFFER, material_buffer);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Material) * MAX_BODIES, materials.data());
}

void SceneManager::printDebugData() {
	std::cout << "Body data:" << std::endl;
	for (int i = 0; i < body_count; i++) {
		Body b = bodies[i];
		std::cout << b.type << " " << b.position.x << "," << b.position.y << "," << b.position.z << " " << b.size.x << "," << b.size.y << "," << b.size.z << " " << b.material << std::endl;
	}
	std::cout << "\nMaterial data:" << std::endl;
	for (int i = 0; i < material_count; i++) {
		Material m = materials[i];
		std::cout << m.color.x << "," << m.color.y << "," << m.color.z << " " << m.emission << " " << m.roughness << " " << m.metallic << std::endl;
	}
}

