#pragma once

/**
 * SceneHUD.h
 *
 * Owns the four screen-space controls, speed and zoom progress bars, and
 * colour-coded picked-part indicator. Renderer synchronizes Scene3D state.
 */
#include "Model.h"
#include "Platform.h"
#include <array>
#include <atomic>
#include <vector>
#include <glm/glm.hpp>

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

class SceneHUD : public Model {
public:
#ifdef PLATFORM_ANDROID
    explicit SceneHUD(AAssetManager* assetManager);
#else
    SceneHUD();
#endif
    ~SceneHUD() override;

    void InitModel() override;
    void Render() override;
    void Resize(int w, int h) override;

    bool TouchDown(float x, float y);
    void TouchRelease();
    void SetHighlightedPartMask(unsigned int mask) {
        highlightedPartMask = mask;
    }

    float CurrentSpeed() const {
        return static_cast<float>(speed.load(std::memory_order_relaxed));
    }
    float CurrentCameraDistance() const {
        return cameraDistance.load(std::memory_order_relaxed);
    }

private:
    struct Rect {
        float left = 0.0f;
        float top = 0.0f;
        float right = 0.0f;
        float bottom = 0.0f;

        bool Contains(float x, float y) const {
            return x >= left && x <= right && y >= top && y <= bottom;
        }
    };

    struct HudVertex {
        GLfloat position[2];
        GLfloat colour[3];
    };

    void UpdateLayout();
    void ApplyButtonAction(int button);
    void AddRectangle(std::vector<HudVertex>& vertices,
                      const Rect& rect, const glm::vec3& colour) const;

#ifdef PLATFORM_ANDROID
    AAssetManager* assetManager = nullptr;
#endif

    GLuint program = 0;
    GLuint vao = 0;
    GLuint vbo = 0;
    GLint projectionLocation = -1;

    glm::mat4 projectionMatrix{1.0f};
    std::array<Rect, 4> buttons{};
    int screenWidth = 1;
    int screenHeight = 1;
    std::atomic<int> speed{8};
    std::atomic<float> cameraDistance{8.0f};
    std::atomic<int> activeButton{-1};
    unsigned int highlightedPartMask = 0;
};
