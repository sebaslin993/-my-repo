#define LOG_TAG "Quiz3Ground"
#include "Ground.h"
#include "ShaderHelper.h"

#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace {

struct Vertex {
    GLfloat position[3];
    GLfloat normal[3];
};

const Vertex kGroundVertices[] = {
    {{-30.0f, -1.93f, -30.0f}, {0.0f, 1.0f, 0.0f}},
    {{-30.0f, -1.93f,  30.0f}, {0.0f, 1.0f, 0.0f}},
    {{ 30.0f, -1.93f,  30.0f}, {0.0f, 1.0f, 0.0f}},
    {{ 30.0f, -1.93f, -30.0f}, {0.0f, 1.0f, 0.0f}}
};

const GLushort kGroundIndices[] = {0, 1, 2, 0, 2, 3};

} // namespace

#ifdef PLATFORM_ANDROID
Ground::Ground(AAssetManager* manager) : assetManager(manager) {}
#else
Ground::Ground() {}
#endif

Ground::~Ground()
{
    if (vao) glDeleteVertexArrays(1, &vao);
    if (vbo) glDeleteBuffers(1, &vbo);
    if (ibo) glDeleteBuffers(1, &ibo);
    if (program) glDeleteProgram(program);
}

void Ground::InitModel()
{
#ifdef PLATFORM_ANDROID
    program = ShaderHelper::buildProgramFromAssets(
        assetManager, "shader/GroundVertex.glsl", "shader/GroundFragment.glsl");
#else
    program = ShaderHelper::buildProgramFromFile(
        "GroundVertex.glsl", "GroundFragment.glsl");
#endif
    if (!program) {
        LOGE("Ground: failed to build shader program");
        return;
    }

    modelLocation = glGetUniformLocation(program, "MODELMATRIX");
    modelViewLocation = glGetUniformLocation(program, "MODELVIEWMATRIX");
    mvpLocation = glGetUniformLocation(program, "MODELVIEWPROJECTIONMATRIX");
    normalLocation = glGetUniformLocation(program, "NORMALMATRIX");
    lightPositionLocation = glGetUniformLocation(program, "LIGHTPOSITIONVIEW");

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(kGroundVertices),
                 kGroundVertices, GL_STATIC_DRAW);

    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(kGroundIndices),
                 kGroundIndices, GL_STATIC_DRAW);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          sizeof(Vertex), reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                          sizeof(Vertex), reinterpret_cast<void*>(3 * sizeof(GLfloat)));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBindVertexArray(0);
}

void Ground::SetViewProjection(const glm::mat4& view, const glm::mat4& projection)
{
    viewMatrix = view;
    projectionMatrix = projection;
}

void Ground::Render()
{
    if (!program || !vao) return;

    const glm::mat4 model(1.0f);
    const glm::mat4 modelView = viewMatrix * model;
    const glm::mat4 mvp = projectionMatrix * modelView;
    const glm::mat3 normalMatrix =
        glm::transpose(glm::inverse(glm::mat3(modelView)));
    const glm::vec3 lightWorld(4.0f, 7.0f, 6.0f);
    const glm::vec3 lightView =
        glm::vec3(viewMatrix * glm::vec4(lightWorld, 1.0f));

    glUseProgram(program);
    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(modelViewLocation, 1, GL_FALSE, glm::value_ptr(modelView));
    glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, glm::value_ptr(mvp));
    glUniformMatrix3fv(normalLocation, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    glUniform3fv(lightPositionLocation, 1, glm::value_ptr(lightView));

    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
    glBindVertexArray(0);
    glUseProgram(0);
}

void Ground::Resize(int, int)
{
    // Scene3D owns the shared perspective projection.
}
