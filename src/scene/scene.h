#pragma once
#include "../utility/includes.h"
#include "../scene/camera.h"
#define MAX_BODIES 128

class SceneManager {
private:
    struct Body {
        glm::vec3 position;
        int type;
        glm::vec3 size;
        int material;
    };
    struct Material {
        glm::vec3 color;
        float emission;
        float roughness;
        float metallic;
        int padding1;
        int padding2;
    };
    struct Ray {
        glm::vec3 origin;
        glm::vec3 direction;
        glm::vec3 position;
        glm::vec3 normal;
        bool collided;
        int index;
        float distance;
    };
    std::vector<Body> bodies = {};
    std::vector<Material> materials = {};
    
    int body_count;
    int material_count;
    int edit_mode;

    int selected_index;
    int hovered_index;

    bool grab_ready;
    bool grab_active;
    glm::vec3 grab_vector;
    glm::vec3 grab_start_direction;
    glm::vec3 grab_start_position;
    glm::vec3 grab_start_size;

public:
    float snap_size;
    float selected_distance;

    // scene_manager
public:
    SceneManager();
    int getBodyCount() const;
    int getMaterialCount() const;
    int getSelectedIndex() const;
    void setEditMode(int mode);
    int getEditMode() const;
    bool isGrabActive() const;

    void packBodies(unsigned int body_buffer);
    void packMaterials(unsigned int material_buffer);
    void printDebugData();

    // scene_select
public:
    void selectBody(int index);
    void deselectBody();
    void mouseSelect(GLFWwindow* window, Camera* camera);

    void checkHover(GLFWwindow* window, Camera* camera);
    void onHover(int index);
    void offHover(int index);
private:
    Ray rayCast(GLFWwindow* window, Camera* camera);
    bool sphereIntersection(Ray& ray, Body sphere);
    bool boxIntersection(Ray& ray, Body box);

    // scene_edit
public:
    int addBody(int type, glm::vec3 position, glm::vec3 size, int material);
    int addSphere(glm::vec3 position, float radius, int material);
    int addBox(glm::vec3 position, glm::vec3 size, int material);
    void deleteBody(int index);
    void deleteSelected();
    int addMaterial(glm::vec3 color, float emission, float roughness, float metallic);
    void deleteMaterial(int index);
    void cleanMaterials();

    void mousePlace(int type, GLFWwindow* window, Camera* camera);
    void copySelected();

    void* getSelectedBody();
    void* getSelectedMaterial();

    // scene_grab
public:
    void mouseGrab(GLFWwindow* window, Camera* camera);
    void createGrabBodies();
    void resetGrabBodies();
    void destroyGrabBodies();
    void startGrabEdit(GLFWwindow* window, Camera* camera);
    void updateGrabEdit(GLFWwindow* window, Camera* camera);
    void endGrabEdit();
    glm::vec3 snapVec(glm::vec3 vec, float snap);

    // scene_loader
public:
    void loadScene(const std::string &filepath, Camera* camera);
    void saveScene(const std::string &filepath, Camera* camera);
    void reloadScene(const std::string& filepath, Camera* camera);
    glm::vec3 readVec3(const std::string &string);
    glm::vec2 readVec2(const std::string &string);
};