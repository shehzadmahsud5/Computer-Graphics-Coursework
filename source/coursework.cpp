#include <iostream>
#include <vector>
#include <cmath>
#include <string>

#ifdef _WIN32
#include <direct.h> // For _getcwd
#define GetCurrentDir _getcwd
#else
#include <unistd.h> // For getcwd
#define GetCurrentDir getcwd
#endif

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/maths.hpp> 
#include <common/camera.hpp>
#include <common/model.hpp>
#include <common/light.hpp> 

// Function prototypes
void keyboardInput(GLFWwindow* window);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

// Timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Light Properties (Global)
glm::vec3 lightPosition(1.2f, 1.5f, 3.0f); // Adjusted light position slightly
glm::vec3 LColor(1.0f, 1.0f, 0.95f);   // Slightly warm white light
const float lightMoveSpeed = 2.5f;

// Shader Program
GLuint shaderProgram;

// Uniform Locations
GLuint modelMatrixID;
GLuint viewMatrixID;
GLuint projectionMatrixID;
GLuint lightPosID;
GLuint lightColorID;
GLuint viewPosID;

// Scene Management
struct SceneObject {
    Model* model_ptr;
    glm::mat4 transform;
};
std::vector<Model*> scene_loaded_models;
std::vector<SceneObject> scene_instances;

// Window Dimensions
const unsigned int SCR_WIDTH = 1024;
const unsigned int SCR_HEIGHT = 768;

// Camera
Camera camera(glm::vec3(0.0f, 1.0f, 7.0f)); // Pulled camera back a bit more to see both objects
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;


int main(void)
{
    char cCurrentPath[FILENAME_MAX];
    if (!GetCurrentDir(cCurrentPath, sizeof(cCurrentPath))) {
        std::cerr << "Error getting current working directory" << std::endl;
    }
    cCurrentPath[sizeof(cCurrentPath) - 1] = '\0';
    std::cout << "Current working directory is: " << cCurrentPath << std::endl;

    // --- GLFW and GLEW Initialization (Same as your last working version) ---
    if (!glfwInit()) { fprintf(stderr, "CRITICAL: glfwInit() failed!\n"); return -1; }
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Computer Graphics Coursework", NULL, NULL);
    if (!window) { fprintf(stderr, "CRITICAL: glfwCreateWindow() failed!\n"); glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    glewExperimental = true;
    GLenum glewError = glewInit();
    if (glewError != GLEW_OK) { fprintf(stderr, "CRITICAL: glewInit() failed! Error: %s\n", glewGetErrorString(glewError)); glfwTerminate(); return -1; }
    fprintf(stdout, "GLEW Initialized. OpenGL version: %s\n", glGetString(GL_VERSION));
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) { fprintf(stderr, "OpenGL error immediately after glewInit: %u\n", err); }
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // --- Load Shaders and Get Uniform Locations (Same as your last working version) ---
    shaderProgram = LoadShaders("vertexShader.glsl", "fragmentShader.glsl");
    if (shaderProgram == 0) { fprintf(stderr, "Failed to load shaders.\n"); glfwTerminate(); return -1; }
    modelMatrixID = glGetUniformLocation(shaderProgram, "model");
    viewMatrixID = glGetUniformLocation(shaderProgram, "view");
    projectionMatrixID = glGetUniformLocation(shaderProgram, "projection");
    lightPosID = glGetUniformLocation(shaderProgram, "lightPos");
    lightColorID = glGetUniformLocation(shaderProgram, "lightColor");
    viewPosID = glGetUniformLocation(shaderProgram, "viewPos");
    glUseProgram(shaderProgram);
    glVertexAttrib2f(1, 0.0f, 0.0f);
    glVertexAttrib3f(2, 0.0f, 0.0f, 1.0f);

    // --- Load Models and Create Scene Instances ---
    const std::string assetRelativePath = "../assets/";

    // --- Model 1: Crate ---
    Model* crateModelAsset = new Model((assetRelativePath + "caixote.obj").c_str());
    if (crateModelAsset && !crateModelAsset->vertices.empty()) {
        crateModelAsset->addTexture((assetRelativePath + "caixote_uv_pronto.png").c_str(), "diffuse");
        scene_loaded_models.push_back(crateModelAsset);

        SceneObject crateInstance;
        crateInstance.model_ptr = crateModelAsset;
        crateInstance.transform = glm::mat4(1.0f);
        // Position it to the left
        crateInstance.transform = glm::translate(crateInstance.transform, glm::vec3(-2.0f, 0.0f, 0.0f));
        // Optional: If the crate itself is too big/small, apply a uniform scale.
        // crateInstance.transform = glm::scale(crateInstance.transform, glm::vec3(1.0f)); // Assuming crate is reasonably sized
        scene_instances.push_back(crateInstance);
    }
    else {
        std::cerr << "Failed to load/initialize crate model asset (" << (assetRelativePath + "caixote.obj") << ")." << std::endl;
        if (crateModelAsset) delete crateModelAsset;
    }

    // --- Model 2: Your Second Model (e.g., pilot.obj) ---
    std::string secondModelFilename = "pilot.obj"; // Or "cottage.obj", or whatever you are using
    std::string secondModelTextureFilename = "pilot.png"; // Or "cottage_diffuse.png", etc.

    Model* secondModelAsset = new Model((assetRelativePath + secondModelFilename).c_str());
    if (secondModelAsset && !secondModelAsset->vertices.empty()) {
        secondModelAsset->addTexture((assetRelativePath + secondModelTextureFilename).c_str(), "diffuse");
        scene_loaded_models.push_back(secondModelAsset);

        SceneObject secondInstance;
        secondInstance.model_ptr = secondModelAsset;
        secondInstance.transform = glm::mat4(1.0f);
        // Position it to the right, side-by-side with the crate
        secondInstance.transform = glm::translate(secondInstance.transform, glm::vec3(1.0f, -1.5f, 0.0f));

        float scaleFactorForSecondModel = 1.0f; // <<<< START WITH A SMALL VALUE AND TWEAK THIS!!!!
 
        secondInstance.transform = glm::scale(secondInstance.transform, glm::vec3(scaleFactorForSecondModel));

        // Optional: Rotate if needed for correct orientation
        // secondInstance.transform = glm::rotate(secondInstance.transform, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        scene_instances.push_back(secondInstance);
    }
    else {
        std::cerr << "Failed to load/initialize second model asset (" << (assetRelativePath + secondModelFilename) << ")." << std::endl;
        if (secondModelAsset) delete secondModelAsset;
    }

    // --- Render Loop (Same as your last working version but uses scene_instances) ---
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        keyboardInput(window);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);

        glm::mat4 viewMat = camera.GetViewMatrix();
        glUniformMatrix4fv(viewMatrixID, 1, GL_FALSE, glm::value_ptr(viewMat));

        int currentWidth, currentHeight;
        glfwGetFramebufferSize(window, &currentWidth, &currentHeight);
        glm::mat4 projectionMat = glm::perspective(glm::radians(camera.Zoom),
            (currentHeight == 0) ? 1.0f : (float)currentWidth / (float)currentHeight,
            0.1f, 100.0f);
        glUniformMatrix4fv(projectionMatrixID, 1, GL_FALSE, glm::value_ptr(projectionMat));

        glUniform3fv(lightPosID, 1, &lightPosition[0]);
        glUniform3fv(lightColorID, 1, &LColor[0]);
        glUniform3fv(viewPosID, 1, &camera.Position[0]);

        for (const auto& instance : scene_instances) {
            if (instance.model_ptr) {
                glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, glm::value_ptr(instance.transform));
                instance.model_ptr->draw(shaderProgram);
            }
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // --- Cleanup (Same as your last working version) ---
    for (Model* loaded_model : scene_loaded_models) {
        if (loaded_model) {
            loaded_model->deleteBuffers();
            delete loaded_model;
        }
    }
    scene_loaded_models.clear();
    scene_instances.clear();
    glDeleteProgram(shaderProgram);
    glfwTerminate();
    return 0;
}

// --- Callback Functions (keyboardInput, mouse_callback, scroll_callback, framebuffer_size_callback - Same) ---
void keyboardInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) camera.ProcessKeyboard(UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) camera.ProcessKeyboard(DOWN, deltaTime);

    float lightVelocity = lightMoveSpeed * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) lightPosition.z -= lightVelocity;
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) lightPosition.z += lightVelocity;
    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) lightPosition.x -= lightVelocity;
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) lightPosition.x += lightVelocity;
    if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS) lightPosition.y += lightVelocity;
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) lightPosition.y -= lightVelocity;
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);
    if (firstMouse) { lastX = xpos; lastY = ypos; firstMouse = false; }
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos; lastY = ypos;
    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}