#define LOG_TAG "Square"

/**
 * Square.cpp
 *
 * Implements a standalone square Model with its own shader program and buffers.
 */

#include "Square.h"
#include "ShaderHelper.h"

static const GLfloat kPositions[] = {
     0.25f,  0.30f,
     0.75f,  0.30f,
     0.75f, -0.20f,
     0.25f, -0.20f
};

static const GLfloat kColors[] = {
    1.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 1.0f,
    1.0f, 0.0f, 1.0f,
    1.0f, 0.5f, 0.0f
};

#ifdef PLATFORM_ANDROID
Square::Square(AAssetManager* assetMgr)
    : mgr(assetMgr)
{
}
#else
Square::Square() = default;
#endif

Square::~Square()
{
    glDeleteBuffers(1, &vboColor);
    glDeleteBuffers(1, &vboPos);
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(programID);
}

void Square::InitModel()
{
    LOGI("Square::InitModel");

#ifdef PLATFORM_ANDROID
    programID = ShaderHelper::buildProgramFromAssets(
        mgr,
        "shader/SquareVertex.glsl",
        "shader/SquareFragment.glsl");
#else
    programID = ShaderHelper::buildProgramFromFile(
        "SquareVertex.glsl",
        "SquareFragment.glsl");
#endif

    if (!programID) {
        LOGE("Square: could not create program");
        return;
    }

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vboPos);
    glBindBuffer(GL_ARRAY_BUFFER, vboPos);
    glBufferData(GL_ARRAY_BUFFER, sizeof(kPositions), kPositions, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

    glGenBuffers(1, &vboColor);
    glBindBuffer(GL_ARRAY_BUFFER, vboColor);
    glBufferData(GL_ARRAY_BUFFER, sizeof(kColors), kColors, GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Square::Render()
{
    glUseProgram(programID);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindVertexArray(0);
}

void Square::Resize(int w, int h)
{
    glViewport(0, 0, w, h);
}
