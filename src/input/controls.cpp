#include "controls.h"

InputController::InputController(GLFWwindow* window) {
	this->window = window;
    trigger = false;
    UP = DOWN = LEFT = RIGHT = false;
    LMB = RMB = MMB = false;
    TAB = DEL = ALT = CTRL = SHIFT = false;
    O = B = C = false;
    K1 = K2 = false;
    camera_movable = false;
    scene_editable = true;
}

bool InputController::triggered() {
    if (trigger) {
        trigger = !trigger;
        return true;
    }
    return false;
}

void InputController::updateCameraMovement(Camera* camera) {

    if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS && !ALT) {
        ALT = true;
        camera_movable = !camera_movable;
        if (camera_movable) {
            int window_height, window_width;
            glfwGetWindowSize(window, &window_width, &window_height);
            glfwSetCursorPos(window, window_width / 2.0, window_height / 2.0);
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
        }
        else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }
    else if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_RELEASE) ALT = false;
    if (!camera_movable) return;

    //-------------------------------------------------------------------------------

    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS && !CTRL) {
        CTRL = true;
        camera->speed *= 0.5f;
        trigger = true;
    }
    else if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_RELEASE) CTRL = false;

    //-------------------------------------------------------------------------------

    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS && !SHIFT) {
        SHIFT = true;
        trigger = true;
        camera->speed *= 2.f;
    }
    else if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE) SHIFT = false;

    //-------------------------------------------------------------------------------

    glm::vec3 local_direction = { 0.0f, 0.0f, 0.0f };

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) local_direction.x += 1.0f;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) local_direction.x -= 1.0f;

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) local_direction.y += 1.0f;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) local_direction.y -= 1.0f;

    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) local_direction.z += 1.0f;
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) local_direction.z -= 1.0f;

    if (length(local_direction) != 0.0f) {
        local_direction = normalize(local_direction);
        trigger = true;
    }
    camera->movePosition(local_direction);

    //-------------------------------------------------------------------------------

    int window_height, window_width;
    glfwGetWindowSize(window, &window_width, &window_height);

    double mouse_x, mouse_y;
    glfwGetCursorPos(window, &mouse_x, &mouse_y);
    glfwSetCursorPos(window, window_width / 2.0, window_height / 2.0);

    float dh = ((float)mouse_x - (window_width / 2.0f));
    float dv = ((float)mouse_y - (window_height / 2.0f));
    if (abs(dh) != 0.0f || abs(dv) != 0.0f) 
        trigger = true;

    camera->moveRotation(dh, dv);
}

void InputController::updateShaderVariables(ShaderManager* shader) {

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS && !UP) {
        UP = true;
        shader->downsample_factor = std::max(shader->downsample_factor - 1, 1);
        trigger = true;
    }
    else if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_RELEASE) UP = false;

    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS && !DOWN) {
        DOWN = true;
        shader->downsample_factor = std::min(shader->downsample_factor + 1, 16);
        trigger = true;
    }
    else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_RELEASE) DOWN = false;
}

void InputController::updateEditor(SceneManager* scene, Camera* camera) {
    if (!scene_editable) return;

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && !LMB) {
        LMB = true;
        scene->mouseSelect(window, camera);
        scene->createGrabBodies();
        scene->startGrabEdit(window, camera);
        trigger = true;
    }
    else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        if (scene->isGrabActive()) trigger = true;
    }
    else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
        LMB = false;
        scene->endGrabEdit();
    }

    //-------------------------------------------------------------------------------

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS && !RMB) {
        RMB = true;
        scene->destroyGrabBodies();
        scene->deselectBody();
        scene->endGrabEdit();
        trigger = true;
    }
    else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE) RMB = false;

    //-------------------------------------------------------------------------------

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS && !MMB) {
        MMB = true;
        scene->destroyGrabBodies();
    }
    else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS) {
        scene->mouseGrab(window, camera);
        trigger = true;
    }
    else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_RELEASE) {
        MMB = false;
        scene->createGrabBodies();
    }

    //-------------------------------------------------------------------------------

    if (glfwGetKey(window, GLFW_KEY_BACKSPACE) == GLFW_PRESS && !DEL) {
        DEL = true;
        int index = scene->getSelectedIndex();
        scene->deselectBody();
        scene->deleteBody(index);
        trigger = true;
    }
    else if (glfwGetKey(window, GLFW_KEY_BACKSPACE) == GLFW_RELEASE) DEL = false;

    //-------------------------------------------------------------------------------

    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS && !K1) {
        K1 = true;
        scene->setEditMode(0);
        trigger = true;
    }
    else if (glfwGetKey(window, GLFW_KEY_1) == GLFW_RELEASE) K1 = false;

    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS && !K2) {
        K2 = true;
        scene->setEditMode(1);
        trigger = true;
    }
    else if (glfwGetKey(window, GLFW_KEY_2) == GLFW_RELEASE) K2 = false;

    //-------------------------------------------------------------------------------

    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS && !C) {
        C = true;
        scene->copySelected();
        trigger = true;
    }
    else if (glfwGetKey(window, GLFW_KEY_C) == GLFW_RELEASE) C = false;

    //-------------------------------------------------------------------------------

    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS && !O) {
        O = true;
        scene->mousePlace(0, window, camera);
        trigger = true;
    }
    else if (glfwGetKey(window, GLFW_KEY_O) == GLFW_RELEASE) O = false;

    //-------------------------------------------------------------------------------

    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS && !B) {
        B = true;
        scene->mousePlace(1, window, camera);
        trigger = true;
    }
    else if (glfwGetKey(window, GLFW_KEY_B) == GLFW_RELEASE) B = false;

    //-------------------------------------------------------------------------------
}

void InputController::updateGUI(GUI* gui) {

    if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS && !TAB) {
        TAB = true;
        gui->gui_active = !gui->gui_active;
    }
    else if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_RELEASE) TAB = false;
    
    scene_editable = gui->scene_visible && !gui->gui_hovered;
    gui->showCursor(!camera_movable);
}