#pragma once

/**
 * Fan.h
 *
 * Hierarchy: one shared cube is drawn as a base, pole, hub, and four blades.
 * Every part starts from the common world transform using Transform push/pop;
 * only the blades inherit the animated Z-axis rotation around the hub.
 * A tap toggles power, while drag velocity temporarily boosts blade speed.
 */
#include "Model.h"
#include "Platform.h"
#include "Transform.h"
#include <chrono>

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

class Fan : public Model {
public:
#ifdef PLATFORM_ANDROID
    explicit Fan(AAssetManager* assetManager);
#else
    Fan();
#endif
    ~Fan() override;
    void InitModel() override;
    void Render() override;
    void Resize(int w, int h) override;
    void TouchEventDown(float x, float y) override;
    void TouchEventMove(float x, float y) override;
    void TouchEventRelease(float x, float y) override;

private:
    void DrawCube(const glm::vec3& colour);

#ifdef PLATFORM_ANDROID
    AAssetManager* assetManager = nullptr;
#endif
    Transform transform;
    GLuint program = 0;
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ibo = 0;
    GLint mvpLocation = -1;
    GLint partColorLocation = -1;

    float spinAngle = 0.0f;
    bool fanOn = true;
    float dragBoost = 0.0f;
    static constexpr float kBaseSpeed = 1.5f;
    static constexpr float kMaxBoost = 20.0f;
    static constexpr float kBoostScale = 8.0f;
    static constexpr float kTapThreshold = 12.0f;
    float lastX = 0.0f;
    float lastY = 0.0f;
    float movedDistance = 0.0f;
    std::chrono::steady_clock::time_point lastMoveTime{};
};
