#include "scene.h"

void SceneManager::mouseGrab(GLFWwindow* window, Camera* camera) {
	if (selected_index == -1) return;
	glm::vec3 new_pos = camera->getPosition() + camera->getScreenDirection(window) * selected_distance;
	bodies[selected_index].position = snapVec(new_pos, snap_size);

	Body body = bodies[selected_index];
	body.size = snapVec(body.size, snap_size / 2);
	if (body.size.x == 0.f) body.size.x = snap_size / 2;
	if (body.size.y == 0.f) body.size.y = snap_size / 2;
	if (body.size.z == 0.f) body.size.z = snap_size / 2;
	bodies[selected_index].size = body.size;
}


void SceneManager::createGrabBodies() {
	if (selected_index == -1 || grab_ready) return;
	grab_ready = true;

	Body body = bodies[selected_index];
	float size = length(body.size) * 0.1f;
	glm::vec3 factor = body.size * 1.2f;
	glm::vec3 pos_dx = glm::vec3{ factor.x, 0.0f, 0.0f };
	glm::vec3 pos_dy = glm::vec3{ 0.0f, factor.y, 0.0f };
	glm::vec3 pos_dz = glm::vec3{ 0.0f, 0.0f, factor.z };

	int material = addMaterial(glm::vec3(1.0), 1.0, 0.0, 0.0); 
	if (edit_mode == 0) {
		addBox(body.position + pos_dx, glm::vec3(size), material);
		addBox(body.position - pos_dx, glm::vec3(size), material);
		addBox(body.position + pos_dy, glm::vec3(size), material);
		addBox(body.position - pos_dy, glm::vec3(size), material);
		addBox(body.position + pos_dz, glm::vec3(size), material);
		addBox(body.position - pos_dz, glm::vec3(size), material);
	}
	else if (edit_mode == 1) {
		addSphere(body.position + pos_dx, size, material);
		addSphere(body.position - pos_dx, size, material);
		addSphere(body.position + pos_dy, size, material);
		addSphere(body.position - pos_dy, size, material);
		addSphere(body.position + pos_dz, size, material);
		addSphere(body.position - pos_dz, size, material);
	}
}

void SceneManager::resetGrabBodies() {
	if (!grab_ready) return;

	Body body = bodies[selected_index];

	glm::vec3 point_displace = body.size * 1.2f;
	bodies[body_count - 6].position = body.position + glm::vec3(point_displace.x, 0.f, 0.f);
	bodies[body_count - 5].position = body.position - glm::vec3(point_displace.x, 0.f, 0.f);
	bodies[body_count - 4].position = body.position + glm::vec3(0.f, point_displace.y, 0.f);
	bodies[body_count - 3].position = body.position - glm::vec3(0.f, point_displace.y, 0.f);
	bodies[body_count - 2].position = body.position + glm::vec3(0.f, 0.f, point_displace.z);
	bodies[body_count - 1].position = body.position - glm::vec3(0.f, 0.f, point_displace.z);

	float size = length(body.size) * 0.1f;
	bodies[body_count - 6].size = glm::vec3(size);
	bodies[body_count - 5].size = glm::vec3(size);
	bodies[body_count - 4].size = glm::vec3(size);
	bodies[body_count - 3].size = glm::vec3(size);
	bodies[body_count - 2].size = glm::vec3(size);
	bodies[body_count - 1].size = glm::vec3(size);
}

void SceneManager::destroyGrabBodies() {
	if (!grab_ready) return;
	grab_ready = false;

	deleteBody(body_count - 1);
	deleteBody(body_count - 1);
	deleteBody(body_count - 1);
	deleteBody(body_count - 1);
	deleteBody(body_count - 1);
	deleteBody(body_count - 1);
}

void SceneManager::startGrabEdit(GLFWwindow* window, Camera* camera) {
	if (selected_index == -1 || !grab_ready || hovered_index < body_count - 6) return;
	grab_active = true;

	grab_start_direction = camera->getScreenDirection(window);
	grab_start_position = bodies[selected_index].position;
	grab_start_size = bodies[selected_index].size;

	if (hovered_index == body_count - 6) grab_vector = glm::vec3(1.f, 0.f, 0.f);
	if (hovered_index == body_count - 5) grab_vector = glm::vec3(-1.f, 0.f, 0.f);
	if (hovered_index == body_count - 4) grab_vector = glm::vec3(0.f, 1.f, 0.f);
	if (hovered_index == body_count - 3) grab_vector = glm::vec3(0.f, -1.f, 0.f);
	if (hovered_index == body_count - 2) grab_vector = glm::vec3(0.f, 0.f, 1.f);
	if (hovered_index == body_count - 1) grab_vector = glm::vec3(0.f, 0.f, -1.f);
}

void SceneManager::updateGrabEdit(GLFWwindow* window, Camera* camera) {
	if (!grab_active) return;

	glm::vec3 displace = selected_distance * grab_vector * dot(camera->getScreenDirection(window) - grab_start_direction, grab_vector);

	if (edit_mode == 0) {
		displace = snapVec(displace, snap_size);
		bodies[selected_index].position = grab_start_position + displace;
	}
	else if (edit_mode == 1) {
		displace = snapVec(displace, snap_size / 2.f);

		if (bodies[selected_index].type == 0) {
			float radius = grab_start_size.x + glm::length(displace) * glm::sign(dot(displace, glm::vec3(-1.0f))) * glm::sign(dot(grab_vector, glm::vec3(-1.0f)));

			if (radius < snap_size / 2) return;
			bodies[selected_index].size = glm::vec3(radius, radius, radius);
		}
		else {
			float temp_factor = dot(grab_vector, glm::vec3(-1.0f));
			temp_factor = (temp_factor < 0.0f) ? 1.0f : -1.0f;
			glm::vec3 temp_size = grab_start_size + displace * temp_factor;

			if (temp_size.x < snap_size / 2 || temp_size.y < snap_size / 2 || temp_size.z < snap_size / 2) return;
			bodies[selected_index].size = temp_size;
			bodies[selected_index].position = grab_start_position + displace;
		}
	}
	resetGrabBodies();
}

void SceneManager::endGrabEdit() {
	grab_active = false;
}


glm::vec3 SceneManager::snapVec(glm::vec3 vec, float snap) {
	vec.x = floor(vec.x / snap) * snap;
	vec.y = floor(vec.y / snap) * snap;
	vec.z = floor(vec.z / snap) * snap;
	return vec;
}

