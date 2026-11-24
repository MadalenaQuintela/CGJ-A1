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
    GLint PieceColorId = -1;

    struct CameraState {
        glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        glm::vec3 center = glm::vec3(0.0f);
        float distance = 8.0f;
        bool perspective = true; // switch between orthographic and perspective projection for that camera
    };

    struct SceneNode {
        mgl::Mesh *mesh = nullptr;
        mgl::ShaderProgram *shader = nullptr;
        glm::mat4 modelMatrix{1.0f}; // local transform
        glm::vec3 color{1.0f};
        std::vector<std::unique_ptr<SceneNode>> children;
    };

    std::unique_ptr<mgl::ShaderProgram> Shaders;
    std::unique_ptr<mgl::Camera> Camera; // one camera, just switch states
    int activeCamera = 0;
    CameraState cameraStates[2];
    std::unique_ptr<SceneNode> sceneRoot;

    bool rotating = false; // use to calculate rotation and stuff later
    double lastMouseX = 0.0;
    double lastMouseY = 0.0;

    GLint ModelMatrixId = -1;
    std::unique_ptr<mgl::Mesh> triangleMesh;
    std::unique_ptr<mgl::Mesh> squareMesh;
    std::unique_ptr<mgl::Mesh> parallelogramMesh;

    void createMeshes();
    void createShaderPrograms();
    void createCamera();
    void drawScene();
    void createSceneGraph();
    void drawNode(const SceneNode &node, const glm::mat4 &parentMatrix);
    void updateCamera();
    void updateProjection(GLFWwindow *win);
    void keyCallback(GLFWwindow *win, int key, int scancode, int action, int mods) override;
    void mouseButtonCallback(GLFWwindow *win, int button, int action, int mods) override;
    void cursorCallback(GLFWwindow *win, double xpos, double ypos) override;
    void scrollCallback(GLFWwindow *win, double xoffset, double yoffset) override;
    void windowSizeCallback(GLFWwindow *win, int width, int height) override;
};

///////////////////////////////////////////////////////////////////////// MESHES

std::unique_ptr<mgl::Mesh> loadMesh(const std::string &string) {
    const std::string mesh_dir = "../../assets/models/";
    auto mesh = std::make_unique<mgl::Mesh>();
    mesh->joinIdenticalVertices();
    mesh->create(mesh_dir + string);
    return mesh;
}

void MyApp::createMeshes() {
    triangleMesh = loadMesh("triangle.obj");
    squareMesh = loadMesh("square.obj");
    parallelogramMesh = loadMesh("parallelogram.obj");
}

///////////////////////////////////////////////////////////////////////// SHADER

void MyApp::createShaderPrograms() {
    Shaders = std::make_unique<mgl::ShaderProgram>();
    Shaders->addShader(GL_VERTEX_SHADER, "cube-vs.glsl");
    Shaders->addShader(GL_FRAGMENT_SHADER, "cube-fs.glsl");

    Shaders->addAttribute(mgl::POSITION_ATTRIBUTE, mgl::Mesh::POSITION);
    Shaders->addAttribute(mgl::NORMAL_ATTRIBUTE, mgl::Mesh::NORMAL);
    /*if (Mesh->hasNormals()) {
        Shaders->addAttribute(mgl::NORMAL_ATTRIBUTE, mgl::Mesh::NORMAL);
    }
    if (Mesh->hasTexcoords()) {
        Shaders->addAttribute(mgl::TEXCOORD_ATTRIBUTE, mgl::Mesh::TEXCOORD);
    }*/

    Shaders->addUniform(mgl::MODEL_MATRIX);
    Shaders->addUniform("PieceColor");
    Shaders->addUniformBlock(mgl::CAMERA_BLOCK, UBO_BP);
    Shaders->create();

    ModelMatrixId = Shaders->Uniforms[mgl::MODEL_MATRIX].index;
    PieceColorId = Shaders->Uniforms["PieceColor"].index;
}

///////////////////////////////////////////////////////////////////////// SCENE GRAPH

glm::mat4 makeModelMatrix(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale) {
    glm::mat4 model(1.0f);
    model = glm::translate(model, position);
    model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0, 0, 1));
    model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0, 1, 0));
    model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1, 0, 0));
    model = glm::scale(model, scale);
    model = glm::translate(model, glm::vec3(0.0f, -0.25f, 0.0f));
    return model;
}

void MyApp::createSceneGraph() {
    sceneRoot = std::make_unique<SceneNode>();
    auto table = std::make_unique<SceneNode>(); // tabletop node (like main)
    auto pickagram = std::make_unique<SceneNode>(); // pickagram node, child of table

    auto addPiece = [&](mgl::Mesh *mesh, glm::vec3 color, const glm::mat4 &modelMatrix) {
        auto piece = std::make_unique<SceneNode>();

        piece->mesh = mesh;
        piece->shader = Shaders.get();
        piece->color = color;
        piece->modelMatrix = modelMatrix;

        pickagram->children.push_back(std::move(piece));
    };
   
    // #44 Mallard 3D piece transforms
    glm::vec3 scaleTriangleLarge(1.0f);
    glm::vec3 scaleTriangleMed(0.707f, 1.0f, 0.707f);
    glm::vec3 scaleTriangleSmall(0.5f, 1.0f, 0.5f);
    glm::vec3 scaleSquare(0.5f, 1.0f, 0.5f);
    glm::vec3 scaleParallelogram(0.353f, 1.0f, 0.353f);

    // Large red triangle
    addPiece(triangleMesh.get(), {1, 0, 0}, makeModelMatrix(
        {-1.0f, -1.5f, 0.0f},
        {-90.0f, 0.0f, 45.0f}, 
        scaleTriangleLarge
    ));

    // Large blue triangle
    addPiece(triangleMesh.get(), {0, 0, 1}, makeModelMatrix(
        {0.413f, -0.087f, 0.0f}, 
        {-90.0f, 0.0f, 225.0f}, 
        scaleTriangleLarge
    ));

    // Medium Magenta triangle
    addPiece(triangleMesh.get(), {1, 0, 1}, makeModelMatrix(
        {1.413f, -1.5f, 0.0f},
        {-90.0f, 0.0f, 45.0f}, 
        scaleTriangleMed
    ));

    // Small Green triangle
    addPiece(triangleMesh.get(), {0, 1, 0}, makeModelMatrix(
        {-1.913f, 1.0f, 0.0f}, 
        {-90.0f, 0.0f, 270.0f}, 
        scaleTriangleSmall
    ));

    // Small Yellow triangle
    addPiece(triangleMesh.get(), {1, 1, 0}, makeModelMatrix(
        {-0.913f, 0.0f, 0.0f},
        {-90.0f, 0.0f, 90.0f}, 
        scaleTriangleSmall
    ));

    // Cyan square
    addPiece(squareMesh.get(), {0, 1, 1}, makeModelMatrix(
        {-0.913f, 1.0f, 0.0f}, 
        {-90.0f, 0.0f, 0.0f},
        scaleSquare
    ));

    // Orange parallelogram
    addPiece(parallelogramMesh.get(), {1, 0.5f, 0}, makeModelMatrix(
        {-1.913f, -0.5f, 0.0f}, 
        {-90.0f, 180.0f, -135.0f}, 
        scaleParallelogram
    ));

    // Build the hierarchy of nodes, root -> table -> pickagram -> pieces
    table->children.push_back(std::move(pickagram));
    sceneRoot->children.push_back(std::move(table));
}


///////////////////////////////////////////////////////////////////////// CAMERA

void MyApp::createCamera() {
    Camera = std::make_unique<mgl::Camera>(UBO_BP);
    glm::vec3 center(0.0f);
    glm::vec3 up(0.0f, 1.0f, 0.0f);

    glm::vec3 eye1(0.0f, 0.0f, 14.0f); // front view (XY plane)
    glm::vec3 eye2(0.0f, 12.0f, 5.0f); // view from above (XZ plane) TODO adjust this later

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

void MyApp::drawScene() {
    drawNode(*sceneRoot, glm::mat4(1.0f));
}

void MyApp::drawNode(const SceneNode &node, const glm::mat4 &parentMatrix) {
    glm::mat4 worldMatrix = parentMatrix * node.modelMatrix; // inherit parent transform
    if (node.mesh && node.shader) {
        node.shader->bind();
        glUniformMatrix4fv(ModelMatrixId, 1, GL_FALSE, glm::value_ptr(worldMatrix));
        glUniform3fv(PieceColorId, 1, glm::value_ptr(node.color));
        node.mesh->draw();
        node.shader->unbind();
    }
    for (const auto &child : node.children) {
        drawNode(*child, worldMatrix);
    }
}

////////////////////////////////////////////////////////////////////// CALLBACKS

void MyApp::initCallback(GLFWwindow *win [[maybe_unused]]) {
    createMeshes();
    createShaderPrograms(); // after mesh;
    createSceneGraph();

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
