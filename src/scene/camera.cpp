#include "camera.h"

Camera::Camera(float speed, float sensitivity) {
    this->position = glm::vec3{ 0.0, 0.0, 0.0 };
    this->rotation = glm::vec3{ 0.0, 0.0, 0.0 };
    this->speed = speed;
    this->sensitivity = sensitivity;
    this->fov = 1.f;
    this->focus_distance = 1.f;
    this->focus_blur = 0.f;
    this->free_movement = false;
}

Camera::Camera() {
    this->position = glm::vec3{ 0.0, 0.0, 0.0 };
    this->rotation = glm::vec3{ 0.0, 0.0, 0.0 };
    this->speed = 0.f;
    this->sensitivity = 0.f;
    this->fov = 1.f;
    this->focus_distance = 1.f;
    this->focus_blur = 0.f;
    this->free_movement = false;
}

glm::vec3 Camera::getPosition() const {
    return position;
}

glm::mat4 Camera::getRotationMatrix() const {
    glm::mat4 output = rotate(glm::mat4(1.0), glm::radians(rotation.x), glm::vec3{0.0, 0.0, -1.0});
    output = rotate(output, glm::radians(rotation.y), glm::vec3{0.0, 1.0, 0.0});
    return output;
}

glm::vec3 Camera::getDirection() const {
    return glm::vec3(this->getRotationMatrix() * glm::vec4(1.0, 0.0, 0.0, 0.0));
}

glm::vec3 Camera::getScreenDirection(GLFWwindow* window) const {
    int window_height, window_width;
    glfwGetWindowSize(window, &window_width, &window_height);

    double mouse_x, mouse_y;
    glfwGetCursorPos(window, &mouse_x, &mouse_y);

    float aspect_ratio = 1.0f * window_width / window_height;
    glm::vec2 uv = glm::vec2{ (mouse_x / window_width) * 2 - 1, (mouse_y / window_height) * 2 - 1 };
    return normalize(glm::vec3(getRotationMatrix() * glm::vec4(1.0f, -uv.x, -uv.y / aspect_ratio, 1.0f)));
}

void Camera::movePosition(glm::vec3 local_dir) {
    glm::mat4 temp = rotate(glm::mat4(1.0), glm::radians(rotation.x), glm::vec3{ 0.0, 0.0, -1.0 });
    if (free_movement) temp = rotate(temp, glm::radians(rotation.y), glm::vec3{ 0.0, 1.0, 0.0 });

    glm::vec3 forward = glm::vec3(temp * glm::vec4{ 1.0, 0.0, 0.0, 0.0 });
    glm::vec3 up = glm::vec3{ 0.0, 0.0, 1.0 };
    if (free_movement) up = glm::vec3(temp * glm::vec4{ 0.0, 0.0, 1.0, 0.0 });
    glm::vec3 right = cross(forward, up);

    position += ((forward * local_dir.x) + (right * local_dir.y) + (up * local_dir.z)) * speed;
}

void Camera::moveRotation(float dh, float dv) {
    if (free_movement)
        if (abs((int)(rotation.y + 90) % 360) > 180) 
            dh *= -1;
    rotation.x += dh * sensitivity;
    rotation.y += dv * sensitivity;

    if (rotation.x > 360.0f || rotation.x < -360.0f) rotation.x = 0;
    if (rotation.y > 360.0f || rotation.y < -360.0f) rotation.y = 0;
    if (!free_movement)
        rotation.y = std::max(std::min(rotation.y, 90.0f), -90.0f);
}

