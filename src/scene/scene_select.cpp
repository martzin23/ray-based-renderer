#include "scene.h"

SceneManager::Ray SceneManager::rayCast(GLFWwindow* window, Camera* camera) {
	Ray ray;
	ray.origin = camera->getPosition();
	ray.direction = (window == nullptr) ? camera->getDirection() : camera->getScreenDirection(window);
	ray.collided = false;
	ray.index = 0;

	float min_t = 99999;
	for (int i = 0; i < body_count; i++) {
		bool local_hit = (bodies[i].type == 0) ? sphereIntersection(ray, bodies[i]) : boxIntersection(ray, bodies[i]);
		ray.collided = ray.collided || local_hit;

		if (local_hit && ray.distance < min_t) {
			min_t = ray.distance;
			ray.index = i;
		}
	}

	ray.position = ray.origin + min_t * ray.direction;
	ray.normal = glm::normalize(ray.position - bodies[ray.index].position);
	return ray;
}

bool SceneManager::sphereIntersection(Ray& ray, Body sphere) {
	glm::vec3 oc = ray.origin - sphere.position;
	float a = glm::dot(ray.direction, ray.direction);
	float b = 2 * dot(oc, ray.direction);
	float c = glm::dot(oc, oc) - sphere.size.x * sphere.size.x;

	float discriminant = b * b - 4 * a * c;
	if (discriminant < 0) return false;

	float tN = (-b - sqrt(discriminant)) / (2 * a);
	ray.distance = tN;
	return tN > 0;
}

bool SceneManager::boxIntersection(Ray& ray, Body box) {
	glm::vec3 m = glm::vec3{ 1.0f / ray.direction.x , 1.0f / ray.direction.y , 1.0f / ray.direction.z };
	glm::vec3 n = m * (ray.origin - box.position);
	glm::vec3 k = abs(m) * box.size;
	glm::vec3 t1 = -n - k;
	glm::vec3 t2 = -n + k;
	float tN = std::max(std::max(t1.x, t1.y), t1.z);
	float tF = std::min(std::min(t2.x, t2.y), t2.z);
	if (tN > tF || tF < 0.0) return false;
	ray.distance = std::min(tN, tF);
	return true;
}



void SceneManager::selectBody(int index) {
	if (index >= body_count || index < 0) return;
	selected_index = index;
}

void SceneManager::deselectBody() {
	if (selected_index == -1) return;
	selected_index = -1;
	destroyGrabBodies();
}

void SceneManager::mouseSelect(GLFWwindow* window, Camera* camera) {
	Ray ray = rayCast(window, camera);
	if (!ray.collided || ray.index == selected_index) return;
	if (grab_ready && ray.index >= body_count - 6) return;
	deselectBody();
	selectBody(ray.index);
	selected_distance = length(ray.origin - bodies[ray.index].position);
}



void SceneManager::checkHover(GLFWwindow* window, Camera* camera) {
	Ray ray = rayCast(window, camera);
	if (ray.index != hovered_index && !grab_active) {
		onHover(ray.index);
		offHover(hovered_index);
		hovered_index = ray.index;
	}
}

void SceneManager::onHover(int index) {
	if (index >= body_count || index < 0) return;

	if (grab_ready and (index >= body_count - 6)) {
		glm::vec3 s = bodies[index].size;
		s *= 1.3f;
		bodies[index].size = s;
	}
}

void SceneManager::offHover(int index) {
	if (index >= body_count || index < 0) return;

	if (grab_ready and (index >= body_count - 6)) {
		glm::vec3 s = bodies[index].size;
		s /= 1.3f;
		bodies[index].size = s;
	}
}

