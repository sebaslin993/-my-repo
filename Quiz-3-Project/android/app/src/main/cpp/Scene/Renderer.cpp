/**
 * Renderer.cpp
 *
 * Singleton renderer implementation.
 * Creates and owns all model objects, drives the render loop,
 * and forwards touch events to every model.
 */

#include "Renderer.h"
#include "Triangle.h"
#include <GLES3/gl3.h>
#include <android/log.h>

#define LOG_TAG "Renderer"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// ---------------------------------------------------------------------------

Renderer::~Renderer()
{
    clearModels();
}

void Renderer::setAssetManager(AAssetManager* mgr)
{
    assetMgr = mgr;
}

// ---------------------------------------------------------------------------

bool Renderer::initializeRenderer()
{
    LOGI("initializeRenderer");
    createModels();
    initializeModels();
    return true;
}

void Renderer::createModels()
{
    clearModels();
    models.push_back(new Triangle(assetMgr));
    LOGI("createModels: %zu model(s) created", models.size());
}

void Renderer::initializeModels()
{
    for (Model* m : models) {
        m->InitModel();
    }
}

void Renderer::clearModels()
{
    for (Model* m : models) {
        delete m;
    }
    models.clear();
}

// ---------------------------------------------------------------------------

void Renderer::resize(int w, int h)
{
    screenWidth  = w;
    screenHeight = h;
    glViewport(0, 0, w, h);
    LOGI("resize: %d x %d", w, h);
}

void Renderer::render()
{
    // Clear to dark grey background so the triangle is clearly visible
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (Model* m : models) {
        m->Render();
    }
}

// ---------------------------------------------------------------------------
// Touch delegation
// ---------------------------------------------------------------------------

void Renderer::TouchEventDown(float x, float y)
{
    for (Model* m : models) {
        m->TouchEventDown(x, y);
    }
}

void Renderer::TouchEventMove(float x, float y)
{
    for (Model* m : models) {
        m->TouchEventMove(x, y);
    }
}

void Renderer::TouchEventRelease(float x, float y)
{
    for (Model* m : models) {
        m->TouchEventRelease(x, y);
    }
}
