#include "scene.h"

int SceneManager::addBody(int type, glm::vec3 position, glm::vec3 size, int material) {
	if (body_count == MAX_BODIES - 1) return 0;

	Body body = { position, type, size, material };
	bodies[body_count] = body;
	return body_count++;
}

int SceneManager::addSphere(glm::vec3 position, float radius, int material) {
	return addBody(0, position, glm::vec3(radius), material);
}

int SceneManager::addBox(glm::vec3 position, glm::vec3 size, int material) {
	return addBody(1, position, size, material);
}

void SceneManager::deleteBody(int index) {
	if (body_count == 0 || index >= body_count || index < 0) return;
	int material_index = bodies[index].material;
	bodies[index] = bodies[body_count - 1];

	bool exists = false;
	for (int i = 0; i < body_count - 1; i++)
		if (bodies[i].material == material_index)
			exists = true;
	if (!exists) deleteMaterial(material_index);

	body_count--;
}

void SceneManager::deleteSelected() {
	int index = this->selected_index;
	deselectBody();
	deleteBody(index);
}

int SceneManager::addMaterial(glm::vec3 color, float emission, float roughness, float metallic) {
	if (material_count == MAX_BODIES - 1) return 0;

	Material new_material = { color, emission, roughness, metallic, 0, 0 };
	materials[material_count] = new_material;
	material_count++;
	return material_count - 1;
}

void SceneManager::deleteMaterial(int index) {
	if (material_count == 0 || index >= material_count || index < 1) return;
	materials[index] = materials[material_count - 1];

	for (int i = 0; i < body_count; i++) {
		int* material_index = &(bodies[i].material);

		if (*material_index == index)
			*material_index = 0;
		if (*material_index == material_count - 1)
			*material_index = index;
	}
	material_count--;
}

void SceneManager::cleanMaterials() {
	int current = material_count - 1;
	while (current > 0) {
		bool exists = false;
		for (int i = 0; i < body_count; i++) {
			if (bodies[i].material == current) {
				exists = true;
				break;
			}
		}

		if (!exists) 
			deleteMaterial(current);
		current--;
	}
}


void SceneManager::mousePlace(int type, GLFWwindow* window, Camera* camera) {
	deselectBody();
	Ray ray = rayCast(window, camera);
	if (!ray.collided) 
		ray.position = ray.origin + ray.direction * 2.0f;
	addBody(type, snapVec(ray.position + glm::vec3(0.0, 0.0, 1.0), snap_size), glm::vec3(1.0), addMaterial(glm::vec3(1.0), 0.0, 0.0, 0.0));
	selectBody(body_count - 1);
	selected_distance = length(ray.origin - bodies[body_count - 1].position);
}

void SceneManager::copySelected() {
	if (selected_index == -1) return;
	Body body = bodies[selected_index];
	Material material = materials[body.material];
	deselectBody();
	int new_body = addBody(body.type, body.position, body.size, addMaterial(material.color, material.emission, material.roughness, material.metallic));
	selectBody(new_body);
}


void* SceneManager::getSelectedBody() {
	if (selected_index == -1)
		return nullptr;
	else
		return (void*)&bodies[selected_index];
}

void* SceneManager::getSelectedMaterial() {
	if (selected_index == -1)
		return nullptr;
	else
		return (void*)&materials[bodies[selected_index].material];
}