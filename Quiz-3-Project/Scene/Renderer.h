#pragma once

/**
 * Renderer.h
 *
 * Owns exactly two scenes for Quiz 3: the perspective Scene3D and the
 * orthographic SceneHUD. Renderer alone controls their fixed draw/input order.
 */
#include "Platform.h"

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

class Scene3D;
class SceneHUD;

class Renderer {
public:
    static Renderer& Instance() {
        static Renderer instance;
        return instance;
    }

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

#ifdef PLATFORM_ANDROID
    void setAssetManager(AAssetManager* mgr);
#endif

    bool initializeRenderer();
    void resize(int w, int h);
    void render();

    void TouchEventDown(float x, float y);
    void TouchEventMove(float x, float y);
    void TouchEventRelease(float x, float y);

private:
    Renderer() = default;
    ~Renderer();

    void createModels();
    void initializeModels();
    void clearModels();
    void ApplyHUDState();

#ifdef PLATFORM_ANDROID
    AAssetManager* assetMgr = nullptr;
#endif

    Scene3D* scene3D = nullptr;
    SceneHUD* sceneHUD = nullptr;
    int screenWidth = 0;
    int screenHeight = 0;
};
