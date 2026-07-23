/**
 * Triangle.cpp
 *
 * Implementation of the Triangle model.
 * Uses a VAO + two VBOs (positions static, colours dynamic) to match
 * modern OpenGL ES 3.0 best-practices.
 *
 * Shader asset paths:
 *   shader/TriangleVertex.glsl
 *   shader/TriangleFragment.glsl
 */

#include "Triangle.h"
#include <android/log.h>

#define LOG_TAG "Triangle"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// Clip-space vertex positions (x, y) – same as the original cookbook values
static const GLfloat kPositions[] = {
     0.0f,  0.5f,   // top
    -0.5f, -0.5f,   // bottom-left
     0.5f, -0.5f    // bottom-right
};

// Attribute locations declared via layout qualifiers in the shader
static constexpr GLuint ATTRIB_POSITION = 0;
static constexpr GLuint ATTRIB_COLOR    = 1;

// ---------------------------------------------------------------------------

Triangle::Triangle(AAssetManager* assetMgr)
    : mgr(assetMgr)
{
    modelType = TriangleType;
    // Start blue so a touch-release transition is visible
    setAllColors(0.0f, 0.0f, 1.0f);
}

Triangle::~Triangle()
{
    if (vao)    { glDeleteVertexArrays(1, &vao);    vao    = 0; }
    if (vboPos) { glDeleteBuffers(1, &vboPos);      vboPos = 0; }
    if (vboCol) { glDeleteBuffers(1, &vboCol);      vboCol = 0; }
    if (program){ glDeleteProgram(program);          program = 0; }
}

// ---------------------------------------------------------------------------

void Triangle::InitModel()
{
    LOGI("InitModel");

    // --- Compile and link shader program ---
    program = ShaderHelper::buildProgramFromAssets(
        mgr,
        "shader/TriangleVertex.glsl",
        "shader/TriangleFragment.glsl");

    if (!program) {
        LOGE("Failed to build shader program");
        return;
    }

    // --- Create VAO ---
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // --- Position VBO (static) ---
    glGenBuffers(1, &vboPos);
    glBindBuffer(GL_ARRAY_BUFFER, vboPos);
    glBufferData(GL_ARRAY_BUFFER, sizeof(kPositions), kPositions, GL_STATIC_DRAW);
    glEnableVertexAttribArray(ATTRIB_POSITION);
    glVertexAttribPointer(ATTRIB_POSITION, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

    // --- Colour VBO (dynamic – updated on touch) ---
    glGenBuffers(1, &vboCol);
    glBindBuffer(GL_ARRAY_BUFFER, vboCol);
    glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(ATTRIB_COLOR);
    glVertexAttribPointer(ATTRIB_COLOR, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    LOGI("InitModel complete");
}

void Triangle::Render()
{
    if (!program || !vao) return;

    glUseProgram(program);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
}

// ---------------------------------------------------------------------------
// Touch handlers – change colour, upload to GPU
// ---------------------------------------------------------------------------

void Triangle::TouchEventDown(float /*x*/, float /*y*/)
{
    setAllColors(1.0f, 0.0f, 0.0f);   // Red
    uploadColors();
}

void Triangle::TouchEventMove(float /*x*/, float /*y*/)
{
    setAllColors(0.0f, 1.0f, 0.0f);   // Green
    uploadColors();
}

void Triangle::TouchEventRelease(float /*x*/, float /*y*/)
{
    setAllColors(0.0f, 0.0f, 1.0f);   // Blue
    uploadColors();
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

void Triangle::setAllColors(float r, float g, float b)
{
    for (int v = 0; v < 3; ++v) {
        colors[v * 3 + 0] = r;
        colors[v * 3 + 1] = g;
        colors[v * 3 + 2] = b;
    }
}

void Triangle::uploadColors()
{
    if (!vboCol) return;
    glBindBuffer(GL_ARRAY_BUFFER, vboCol);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(colors), colors);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
