#define LOG_TAG "Quiz3Scene3D"
#include "Scene3D.h"

#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>

#ifdef PLATFORM_ANDROID
Scene3D::Scene3D(AAssetManager* manager) : assetManager(manager) {}
#else
Scene3D::Scene3D() {}
#endif

Scene3D::~Scene3D()
{
    delete fan;
    delete ground;
}

void Scene3D::InitModel()
{
#ifdef PLATFORM_ANDROID
    ground = new Ground(assetManager);
    fan = new Fan(assetManager);
#else
    ground = new Ground();
    fan = new Fan();
#endif

    ground->InitModel();
    fan->InitModel();
    UpdateCamera();
}

void Scene3D::UpdateCamera()
{
    // Keep the fixed vertical support at the visual centre while the complete fan
    // turns around it. This prevents the model drifting left/right as it yaws.
    const glm::vec3 eye(0.42f, -0.10f, cameraDistance);
    const glm::vec3 target(0.42f, -0.30f, 0.0f);
    const glm::vec3 up(0.0f, 1.0f, 0.0f);
    viewMatrix = glm::lookAt(eye, target, up);

    const float aspect =
        static_cast<float>(screenWidth) / static_cast<float>(screenHeight);
    projectionMatrix = glm::perspective(
        glm::radians(55.0f), aspect, 0.10f, 100.0f);

    if (ground) ground->SetViewProjection(viewMatrix, projectionMatrix);
    if (fan) fan->SetViewProjection(viewMatrix, projectionMatrix);
}

void Scene3D::Render()
{
    if (!fan || !ground) return;

    UpdateCamera();

    // Draw the opaque floor first, then the fan. Depth testing resolves overlap.
    ground->Render();
    fan->Render();
}

void Scene3D::Resize(int w, int h)
{
    screenWidth = std::max(w, 1);
    screenHeight = std::max(h, 1);
    UpdateCamera();

    if (ground) ground->Resize(screenWidth, screenHeight);
    if (fan) fan->Resize(screenWidth, screenHeight);
}

void Scene3D::SetCameraDistance(float distance)
{
    cameraDistance = std::clamp(distance, 3.0f, 9.0f);
    UpdateCamera();
}

void Scene3D::SetFanSpeed(float speed)
{
    if (fan) fan->SetSpeed(speed);
}

bool Scene3D::PickAt(float screenX, float screenY)
{
    if (!fan || screenWidth <= 0 || screenHeight <= 0) return false;

    // TransformUnproject expects bottom-left window coordinates; touch/mouse input
    // and the HUD use a top-left origin.
    const float windowY = static_cast<float>(screenHeight) - screenY;
    int viewport[4] = {0, 0, screenWidth, screenHeight};

    float nearX = 0.0f, nearY = 0.0f, nearZ = 0.0f;
    float farX = 0.0f, farY = 0.0f, farZ = 0.0f;

    if (!unprojectTransform.TransformUnproject(
            screenX, windowY, 0.0f, &viewMatrix, &projectionMatrix, viewport,
            &nearX, &nearY, &nearZ) ||
        !unprojectTransform.TransformUnproject(
            screenX, windowY, 1.0f, &viewMatrix, &projectionMatrix, viewport,
            &farX, &farY, &farZ)) {
        LOGE("Scene3D: unable to unproject pick ray");
        return false;
    }

    const glm::vec3 rayOrigin(nearX, nearY, nearZ);
    const glm::vec3 rayDirection =
        glm::normalize(glm::vec3(farX, farY, farZ) - rayOrigin);
    return fan->Pick(rayOrigin, rayDirection);
}
