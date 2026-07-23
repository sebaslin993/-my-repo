#pragma once

/**
 * Scene3D.h
 *
 * Owns the Fan and Ground, their shared perspective camera and light, and the
 * world-space picking path. It has no knowledge of HUD geometry.
 */
#include "Model.h"
#include "Fan.h"
#include "Ground.h"
#include "Transform.h"
#include <glm/glm.hpp>

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

class Scene3D : public Model {
public:
#ifdef PLATFORM_ANDROID
    explicit Scene3D(AAssetManager* assetManager);
#else
    Scene3D();
#endif
    ~Scene3D() override;

    void InitModel() override;
    void Render() override;
    void Resize(int w, int h) override;

    void SetCameraDistance(float distance);
    void SetFanSpeed(float speed);
    bool PickAt(float screenX, float screenY);
    unsigned int HighlightedPartMask() const {
        return fan ? fan->HighlightedPartMask() : 0u;
    }

private:
    void UpdateCamera();

#ifdef PLATFORM_ANDROID
    AAssetManager* assetManager = nullptr;
#endif

    Fan* fan = nullptr;
    Ground* ground = nullptr;
    Transform unprojectTransform;

    glm::mat4 viewMatrix{1.0f};
    glm::mat4 projectionMatrix{1.0f};
    float cameraDistance = 8.0f;
    int screenWidth = 1;
    int screenHeight = 1;
};
