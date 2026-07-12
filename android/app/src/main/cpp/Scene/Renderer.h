#pragma once

/**
 * Renderer.h
 *
 * Singleton renderer that owns and drives all Model instances.
 * Ported from the original OpenGL ES 3.0 Cookbook Renderer with the
 * old GLPIFramework (ProgramManager, Transform, etc.) removed;
 * now self-contained and compatible with Android API 34+.
 */

#include "Model.h"
#include <vector>
#include <android/asset_manager.h>

class Renderer {
public:
    // Singleton access
    static Renderer& Instance() {
        static Renderer instance;
        return instance;
    }

    // Non-copyable
    Renderer(const Renderer&)            = delete;
    Renderer& operator=(const Renderer&) = delete;

    // -----------------------------------------------------------------------
    // Called from NativeTemplate.cpp / JNI
    // -----------------------------------------------------------------------
    void setAssetManager(AAssetManager* mgr);
    bool initializeRenderer();
    void resize(int w, int h);
    void render();

    // Touch delegation
    void TouchEventDown(float x, float y);
    void TouchEventMove(float x, float y);
    void TouchEventRelease(float x, float y);

private:
    Renderer() = default;
    ~Renderer();

    void createModels();
    void initializeModels();
    void clearModels();

    AAssetManager*         assetMgr = nullptr;
    std::vector<Model*>    models;
    int screenWidth  = 0;
    int screenHeight = 0;
};
