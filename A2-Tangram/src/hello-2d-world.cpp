////////////////////////////////////////////////////////////////////////////////
//
// Basic Triangle rendering
//
// This program demonstrates fundamental Modern OpenGL concepts by rendering
// two triangle instances directly in Clip Space. It serves as an introductory
// example for understanding the OpenGL graphics pipeline and basic shader
// programming.
//
// Key Concepts Demonstrated:
// - Vertex Array Objects (VAOs) and Vertex Buffer Objects (VBOs)
// - Shader program creation and management
// - Attribute and uniform handling
// - Basic transformation matrices
// - Clip space rendering without model/view/projection matrices
//
// Copyright (c) 2013-26 by Carlos Martinho
//
////////////////////////////////////////////////////////////////////////////////

#include <cstddef>
#include <memory>

#include "../mgl/mgl.hpp" // IWYU pragma: keep

////////////////////////////////////////////////////////////////////////// MYAPP

class MyApp : public mgl::App {
public:
  MyApp() = default;
  ~MyApp() override = default;

  void initCallback(GLFWwindow *win) override;
  void displayCallback(GLFWwindow *win, double elapsed) override;
  void windowCloseCallback(GLFWwindow *win) override;

private:
  const GLuint POSITION = 0, COLOR = 1;
  std::unique_ptr<mgl::ShaderProgram> Shaders = nullptr;
  GLint MatrixId;
  GLint PieceColorId;

  struct Mesh {
      GLuint vao = 0;
      GLuint buffers[2]{}; // vertex buffer, index buffer
  };
  Mesh triangle;
  Mesh parallelogram;
  Mesh squareProgram;

  void createShaderProgram();
  void createBufferObjects();
  void destroyBufferObjects();
  void drawScene();
  void drawObject(const Mesh& mesh, const glm::mat4& modelMatrix, GLsizei indexCount, const glm::vec4& color = { 1.0f, 0.0f, 0.0f, 1.0f });
};

//////////////////////////////////////////////////////////////////////// SHADERs

void MyApp::createShaderProgram() {
  Shaders = std::make_unique<mgl::ShaderProgram>();
  Shaders->addShader(GL_VERTEX_SHADER, "clip-vs.glsl");
  Shaders->addShader(GL_FRAGMENT_SHADER, "clip-fs.glsl");

  Shaders->addAttribute(mgl::POSITION_ATTRIBUTE, POSITION);
  Shaders->addAttribute(mgl::COLOR_ATTRIBUTE, COLOR);
  Shaders->addUniform("Matrix");
  Shaders->addUniform("PieceColor");

  Shaders->create();

  MatrixId = Shaders->Uniforms["Matrix"].index;
  PieceColorId = Shaders->Uniforms["PieceColor"].index;
}

//////////////////////////////////////////////////////////////////// VAOs & VBOs
struct Vertex {
  GLfloat XYZW[4];
  GLfloat RGBA[4];
};

const Vertex TriangleVertices[] = {
    {{0.0f, 0.0f, 0.0f, 1.0f},{1.0f, 0.0f, 0.0f, 1.0f}},
	{{0.5f, 0.0f, 0.0f, 1.0f},{1.0f, 0.0f, 0.0f, 1.0f}},
	{{0.0f, 0.5f, 0.0f, 1.0f},{1.0f, 0.0f, 0.0f, 1.0f}}
};
const GLubyte TriangleIndices[] = {0, 1, 2};

const Vertex ParallelogramVertices[] = {
    { { 0.0f, 0.0f, 0.0f, 1.0f },{ 0.0f, 1.0f, 0.0f, 1.0f } },
    { { 0.75f, 0.0f, 0.0f, 1.0f },{ 0.0f, 1.0f, 0.0f, 1.0f } },
	{ { 0.5f, 0.5f, 0.0f, 1.0f },{ 0.0f, 1.0f, 0.0f, 1.0f } },
    { { 1.25f, 0.5f, 0.0f, 1.0f },{ 0.0f, 1.0f, 0.0f, 1.0f } },
};
// Form parallelogram with triangles
const GLubyte ParallelogramIndices[] = { 
	0, 1, 2, // bot left, bot right, top left
	2, 1, 3  // top left, bot right, top right
};

const Vertex SquareVertices[] = {
    { { 0.0f, 0.0f, 0.0f, 1.0f },{ 0.0f, 0.0f, 1.0f, 1.0f } },
    { { 0.5f, 0.0f, 0.0f, 1.0f },{ 0.0f, 0.0f, 1.0f, 1.0f } },
    { { 0.0f, 0.5f, 0.0f, 1.0f },{ 0.0f, 0.0f, 1.0f, 1.0f } },
    { { 0.5f, 0.5f, 0.0f, 1.0f },{ 0.0f, 0.0f, 1.0f, 1.0f } },
};
const GLubyte SquareIndices[] = {
    0, 1, 2,
    2, 1, 3
};

void MyApp::createBufferObjects() {
    auto upload = [&](Mesh& mesh, const auto& vertices, const auto& indices) {
        glGenVertexArrays(1, &mesh.vao);
        glBindVertexArray(mesh.vao);
        glGenBuffers(2, mesh.buffers);

        glBindBuffer(GL_ARRAY_BUFFER, mesh.buffers[0]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(POSITION);
        glVertexAttribPointer(POSITION, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), 
        	reinterpret_cast<GLvoid*>(0));

        glEnableVertexAttribArray(COLOR);
        glVertexAttribPointer(COLOR, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex),
            reinterpret_cast<const void*>(offsetof(Vertex, RGBA)));

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.buffers[1]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    };

    upload(triangle, TriangleVertices, TriangleIndices);
    upload(parallelogram, ParallelogramVertices, ParallelogramIndices);
    upload(squareProgram, SquareVertices, SquareIndices);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void MyApp::destroyBufferObjects() {
    glDeleteVertexArrays(1, &triangle.vao);
    glDeleteBuffers(2, triangle.buffers);
    glDeleteVertexArrays(1, &parallelogram.vao);
    glDeleteBuffers(2, parallelogram.buffers);
}

////////////////////////////////////////////////////////////////////////// SCENE

const glm::mat4 I(1.0f);

glm::mat4 makeModelMatrix(glm::vec2 offset, float degrees, glm::vec3 size) {
    const glm::mat4 I(1.0f);
    const auto move = glm::translate(I, glm::vec3(offset, 0.0f));
    const auto rot = glm::rotate(I, glm::radians(degrees), glm::vec3(0.0f, 0.0f, 1.0f));
    const auto scale = glm::scale(I, size);
    return move * rot * scale;
}

glm::mat4 makeModelMatrix(glm::vec2 offset, float degrees, float size = 1.0f) {
    return makeModelMatrix(offset, degrees, glm::vec3(size, size, 1.0f));
}

// #9 Bird
// Matrices for pieces 1 to 7
const glm::mat4 M1 = makeModelMatrix({ -0.175f, -0.324f }, -45.0f);
const glm::mat4 M2 = makeModelMatrix({ 0.179f, -0.469f }, 0);
const glm::mat4 M3 = makeModelMatrix({ 0.8f, -0.678f }, 0.0f, {-0.42f, 0.42f, 1.0f});
const glm::mat4 M4 = makeModelMatrix({ 0.0f, -0.5f }, 225.0f, 0.5f);
const glm::mat4 M5 = makeModelMatrix({ -0.18f, 0.03f }, -90.0f, 0.72f);
const glm::mat4 M6 = makeModelMatrix({ -0.43f, -0.22f }, 0.0f, 0.5f);
const glm::mat4 M7 = makeModelMatrix({ -0.43f, -0.22f }, 90.0f, 0.5f);

void MyApp::drawScene() {
	// Drawing directly in clip space
	Shaders->bind();

    drawObject(triangle, M1, 3);
    drawObject(triangle, M2, 3, { 0.0f, 0.0f, 1.0f, 1.0f });
    // flipping the object from earlier shows back of it so we flip the front face winding order to draw it correctly
	glFrontFace(GL_CW);
    drawObject(parallelogram, M3, 6, { 1.0f, 0.5f, 0.0f, 1.0f });
    glFrontFace(GL_CCW);
    drawObject(triangle, M4, 3, { 0.0f, 1.0f, 0.0f, 1.0f });
    drawObject(triangle, M5, 3, { 1.0f, 0.0f, 1.0f, 1.0f });
    drawObject(squareProgram, M6, 6, { 0.0f, 1.0f, 1.0f, 1.0f });
    drawObject(triangle, M7, 3, { 1.0f, 1.0f, 0.0f, 1.0f });

	Shaders->unbind();
	glBindVertexArray(0);
}

void MyApp::drawObject(const Mesh& mesh, const glm::mat4& modelMatrix, GLsizei indexCount, const glm::vec4& color) {
    glBindVertexArray(mesh.vao);
    glUniformMatrix4fv(MatrixId, 1, GL_FALSE, glm::value_ptr(modelMatrix));
    glUniform4fv(PieceColorId, 1, glm::value_ptr(color));
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_BYTE, reinterpret_cast<GLvoid*>(0));
}

////////////////////////////////////////////////////////////////////// CALLBACKS

void MyApp::initCallback(GLFWwindow *win [[maybe_unused]]) {
  createBufferObjects();
  createShaderProgram();
}

void MyApp::windowCloseCallback(GLFWwindow *win [[maybe_unused]]) {
  destroyBufferObjects();
}

void MyApp::displayCallback(GLFWwindow *win [[maybe_unused]],
                            double elapsed [[maybe_unused]]) {
  drawScene();
}

/////////////////////////////////////////////////////////////////////////// MAIN

int main() {
  mgl::Engine &engine = mgl::Engine::getInstance();
  engine.setApp(std::make_unique<MyApp>());
  engine.setOpenGL(4, 6);
  engine.setWindow(600, 600, "Hello Modern 2D World", 0, 1);
  engine.init();
  engine.run();
  return EXIT_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////// END
