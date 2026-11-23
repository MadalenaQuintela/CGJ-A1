////////////////////////////////////////////////////////////////////////////////
//
//  Loading meshes from external files
//
// Copyright (c) 2023-26 by Carlos Martinho
//
// INTRODUCES:
// MODEL DATA, ASSIMP, mglMesh.hpp
//
////////////////////////////////////////////////////////////////////////////////

#include "../mgl/mgl.hpp" // IWYU pragma: keep

#include <memory>

////////////////////////////////////////////////////////////////////////// MYAPP

class MyApp : public mgl::App {
  public:
    void initCallback(GLFWwindow *win) override;
    void displayCallback(GLFWwindow *win, double elapsed) override;

  private:
    const GLuint UBO_BP = 0;

    struct CameraState {
        glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        glm::vec3 center = glm::vec3(0.0f);
        float distance = 8.0f;
        bool perspective = true; // switch between orthographic and perspective projection for that camera
    };

    std::unique_ptr<mgl::ShaderProgram> Shaders;
    std::unique_ptr<mgl::Camera> Camera; // one camera, just switch states
    int activeCamera = 0;
    CameraState cameraStates[2];

    bool rotating = false; // use to calculate rotation and stuff later
    double lastMouseX = 0.0;
    double lastMouseY = 0.0;

    GLint ModelMatrixId = -1;
    std::unique_ptr<mgl::Mesh> Mesh;

    void createMeshes();
    void createShaderPrograms();
    void createCamera();
    void drawScene();
    void updateCamera();
    void updateProjection(GLFWwindow *win);
    void keyCallback(GLFWwindow *win, int key, int scancode, int action, int mods) override;
    void mouseButtonCallback(GLFWwindow *win, int button, int action, int mods) override;
    void cursorCallback(GLFWwindow *win, double xpos, double ypos) override;
    void scrollCallback(GLFWwindow *win, double xoffset, double yoffset) override;
    void windowSizeCallback(GLFWwindow *win, int width, int height) override;
};

///////////////////////////////////////////////////////////////////////// MESHES

void MyApp::createMeshes() {
    std::string mesh_dir = "../../assets/models/";
    // std::string mesh_file = "cube-v.obj";
    // std::string mesh_file = "cube-vn-flat.obj";
    // std::string mesh_file = "cube-vn-smooth.obj";
    // std::string mesh_file = "cube-vt.obj";
    // std::string mesh_file = "cube-vt2.obj";
    // std::string mesh_file = "torus-vtn-flat.obj";
    // std::string mesh_file = "torus-vtn-smooth.obj";
    // std::string mesh_file = "suzanne-vtn-flat.obj";
    // std::string mesh_file = "suzanne-vtn-smooth.obj";
    // std::string mesh_file = "teapot-vn-flat.obj";
    // std::string mesh_file = "teapot-vn-smooth.obj";
    // std::string mesh_file = "bunny-vn-flat.obj";
    // std::string mesh_file = "bunny-vn-smooth.obj";
    // std::string mesh_file = "monkey-torus-vtn-flat.obj";
    std::string mesh_file = "cube-vtn-flat.obj";
    std::string mesh_fullname = mesh_dir + mesh_file;

    Mesh = std::make_unique<mgl::Mesh>();
    Mesh->joinIdenticalVertices();
    Mesh->create(mesh_fullname);
}

///////////////////////////////////////////////////////////////////////// SHADER

void MyApp::createShaderPrograms() {
    Shaders = std::make_unique<mgl::ShaderProgram>();
    Shaders->addShader(GL_VERTEX_SHADER, "cube-vs.glsl");
    Shaders->addShader(GL_FRAGMENT_SHADER, "cube-fs.glsl");

    Shaders->addAttribute(mgl::POSITION_ATTRIBUTE, mgl::Mesh::POSITION);
    if (Mesh->hasNormals()) {
        Shaders->addAttribute(mgl::NORMAL_ATTRIBUTE, mgl::Mesh::NORMAL);
    }
    if (Mesh->hasTexcoords()) {
        Shaders->addAttribute(mgl::TEXCOORD_ATTRIBUTE, mgl::Mesh::TEXCOORD);
    }

    Shaders->addUniform(mgl::MODEL_MATRIX);
    Shaders->addUniformBlock(mgl::CAMERA_BLOCK, UBO_BP);
    Shaders->create();

    ModelMatrixId = Shaders->Uniforms[mgl::MODEL_MATRIX].index;
}

///////////////////////////////////////////////////////////////////////// CAMERA

void MyApp::createCamera() {
    Camera = std::make_unique<mgl::Camera>(UBO_BP);
    glm::vec3 center(0.0f);
    glm::vec3 up(0.0f, 1.0f, 0.0f);
    // TODO default positions should be suitable for both closed tangram and standing mallard
    // these values work for now since we just have a cube and not the mallard object from blender yet
    glm::vec3 eye1(5.0f, 5.0f, 5.0f); 
    glm::vec3 eye2(-5.0f, -5.0f, -5.0f);

    CameraState& c1 = cameraStates[0]; // make sure its not a copy lol
    c1.center = center;
    c1.distance = glm::length(eye1 - center);
    // view matrix, extract rotation with mat3 (exclude translation), convert to quat, conjugate
    // (reverse rotation so camera orients relative to world)
    c1.rotation = glm::conjugate(glm::quat_cast(glm::mat3(glm::lookAt(eye1, center, up))));

    CameraState &c2 = cameraStates[1];
    c2.center = center;
    c2.distance = glm::length(eye2 - center);
    c2.rotation = glm::conjugate(glm::quat_cast(glm::mat3(glm::lookAt(eye2, center, up))));
}

void MyApp::updateCamera() {
    CameraState &cam = cameraStates[activeCamera];

    // Rotate offset and up vector around center
    glm::vec3 offset = cam.rotation * glm::vec3(0.0f, 0.0f, cam.distance);
    glm::vec3 eye = cam.center + offset;
    glm::vec3 up = cam.rotation * glm::vec3(0.0f, 1.0f, 0.0f);
    Camera->setViewMatrix(glm::lookAt(eye, cam.center, up));
}

void MyApp::updateProjection(GLFWwindow *win) {
    int width, height;
    glfwGetFramebufferSize(win, &width, &height);

    if (width == 0 || height == 0) {
        return;
    }

    float aspect = static_cast<float>(width) / height;
    float fov = glm::radians(30.0f);
    CameraState& cam = cameraStates[activeCamera];
    if (cam.perspective) {
        Camera->setProjectionMatrix(glm::perspective(fov, aspect, 0.1f, 100.0f));
    } else {
        float halfHeight = cam.distance * std::tan(fov / 2.0f);
        float halfWidth = halfHeight * aspect;
        Camera->setProjectionMatrix(glm::ortho(-halfWidth, halfWidth, -halfHeight, halfHeight, 0.1f, 100.0f));
    }
}

/////////////////////////////////////////////////////////////////////////// DRAW

glm::mat4 ModelMatrix(1.0f);

void MyApp::drawScene() {
    Shaders->bind();
    glUniformMatrix4fv(ModelMatrixId, 1, GL_FALSE, glm::value_ptr(ModelMatrix));
    Mesh->draw();
    Shaders->unbind();
}

////////////////////////////////////////////////////////////////////// CALLBACKS

void MyApp::initCallback(GLFWwindow *win [[maybe_unused]]) {
    createMeshes();
    createShaderPrograms(); // after mesh;
    createCamera();
    updateCamera();
    updateProjection(win);
}

void MyApp::displayCallback(GLFWwindow *win [[maybe_unused]],
                            double elapsed [[maybe_unused]]) {
    drawScene();
}

void MyApp::keyCallback(GLFWwindow *win, int key, int scancode, int action, int mods) {
    if (action != GLFW_PRESS) {
        return;
    }
    if (key == GLFW_KEY_C) {
        // Switch between the two stored camera states
        activeCamera = 1 - activeCamera;
        updateCamera();
        updateProjection(win);
    }
    if (key == GLFW_KEY_P) {
        // Switch between perspective and orthographic projection
        CameraState &cam = cameraStates[activeCamera];
        cam.perspective = !cam.perspective;
        updateProjection(win);
    }
}

void MyApp::mouseButtonCallback(GLFWwindow *win, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        rotating = (action == GLFW_PRESS); // save mouse positions to be used in cursorCallback
        if (rotating) {
            glfwGetCursorPos(win, &lastMouseX, &lastMouseY);
        }
    }
}

void MyApp::cursorCallback(GLFWwindow *win, double xpos, double ypos) {
    if (!rotating) {
        return;
    }
    float sensitivity = 0.005f;
    // How many pixels the mouse moved
    float dx = static_cast<float>(xpos - lastMouseX);
    float dy = static_cast<float>(ypos - lastMouseY);
    lastMouseX = xpos;
    lastMouseY = ypos;

    CameraState& cam = cameraStates[activeCamera];
    // Horizontal rotation around worlds Y-axis
    glm::quat yaw = glm::angleAxis(-dx * sensitivity, glm::vec3(0.0f, 1.0f, 0.0f)); 
    // angleAxis makes quaternion representing rotation around the axis

    // Cameras current right direction and vertical rotation around it
    glm::vec3 right = cam.rotation * glm::vec3(1.0f, 0.0f, 0.0f);
    glm::quat pitch = glm::angleAxis(-dy * sensitivity, right);

    // Combine rotations and normalize
    cam.rotation = glm::normalize(yaw * pitch * cam.rotation);
    updateCamera();
}

void MyApp::scrollCallback(GLFWwindow *win, double xoffset, double yoffset) {
    CameraState &cam = cameraStates[activeCamera];
    cam.distance *= std::pow(0.9f, static_cast<float>(yoffset));
    // clamp prevents us from being too close or far away
    cam.distance = glm::clamp(cam.distance, 1.0f, 30.0f);
    updateCamera();
    updateProjection(win);
}

void MyApp::windowSizeCallback(GLFWwindow *win, int width, int height) {
    int framebufferWidth, framebufferHeight;
    glfwGetFramebufferSize(win, &framebufferWidth, &framebufferHeight);
    if (framebufferWidth == 0 || framebufferHeight == 0) {
        return;
    }
    glViewport(0, 0, framebufferWidth, framebufferHeight);
    updateProjection(win);
}

/////////////////////////////////////////////////////////////////////////// MAIN

int main() {
    mgl::Engine &engine = mgl::Engine::getInstance();
    engine.setApp(std::make_unique<MyApp>());
    engine.setOpenGL(4, 6);
    engine.setWindow(800, 600, "Mesh Loader", 0, 1);
    engine.init();
    engine.run();
    return EXIT_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
