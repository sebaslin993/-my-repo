#define LOG_TAG "Quiz3Renderer"
#include "Renderer.h"
#include "Scene3D.h"
#include "SceneHUD.h"

Renderer::~Renderer()
{
    clearModels();
}

#ifdef PLATFORM_ANDROID
void Renderer::setAssetManager(AAssetManager* mgr)
{
    assetMgr = mgr;
}
#endif

bool Renderer::initializeRenderer()
{
    LOGI("Renderer::initializeRenderer");
    createModels();
    initializeModels();
    return scene3D != nullptr && sceneHUD != nullptr;
}

void Renderer::createModels()
{
    clearModels();
#ifdef PLATFORM_ANDROID
    scene3D = new Scene3D(assetMgr);
    sceneHUD = new SceneHUD(assetMgr);
#else
    scene3D = new Scene3D();
    sceneHUD = new SceneHUD();
#endif
    LOGI("Renderer: Scene3D and SceneHUD created");
}

void Renderer::initializeModels()
{
    if (scene3D) scene3D->InitModel();
    if (sceneHUD) sceneHUD->InitModel();
    ApplyHUDState();
}

void Renderer::clearModels()
{
    delete scene3D;
    scene3D = nullptr;
    delete sceneHUD;
    sceneHUD = nullptr;
}

void Renderer::ApplyHUDState()
{
    if (!scene3D || !sceneHUD) return;
    scene3D->SetFanSpeed(sceneHUD->CurrentSpeed());
    scene3D->SetCameraDistance(sceneHUD->CurrentCameraDistance());
    sceneHUD->SetHighlightedPartMask(scene3D->HighlightedPartMask());
}

void Renderer::resize(int w, int h)
{
    screenWidth = w;
    screenHeight = h;
    glViewport(0, 0, w, h);

    if (scene3D) scene3D->Resize(w, h);
    if (sceneHUD) sceneHUD->Resize(w, h);

    LOGI("Renderer::resize %d x %d", w, h);
}

void Renderer::render()
{
    ApplyHUDState();

    glClearColor(0.035f, 0.045f, 0.065f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Required frame order: 3D with depth, then the screen-space HUD on top.
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    if (scene3D) scene3D->Render();

    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    if (sceneHUD) sceneHUD->Render();

    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
}

void Renderer::TouchEventDown(float x, float y)
{
    // HUD hit-testing always gets first refusal and consumes successful hits.
    if (sceneHUD && sceneHUD->TouchDown(x, y)) {
        ApplyHUDState();
        return;
    }

    if (scene3D) scene3D->PickAt(x, y);
}

void Renderer::TouchEventMove(float, float)
{
    // Quiz 2 drag-to-boost is intentionally retired.
}

void Renderer::TouchEventRelease(float, float)
{
    if (sceneHUD) sceneHUD->TouchRelease();
}
