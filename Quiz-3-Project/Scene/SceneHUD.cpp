#define LOG_TAG "Quiz3SceneHUD"
#include "SceneHUD.h"
#include "ShaderHelper.h"

#include <algorithm>
#include <cstddef>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace {

constexpr int kIncreaseSpeed = 0;
constexpr int kDecreaseSpeed = 1;
constexpr int kZoomIn = 2;
constexpr int kZoomOut = 3;
constexpr float kZoomStep = 0.4f;
constexpr int kBasePart = 0;
constexpr int kHubPart = 2;
constexpr int kFirstBladePart = 3;
constexpr int kHudBladeCount = 20;

const glm::vec3 kSpeedButtonColour(0.176f, 0.690f, 0.694f);
const glm::vec3 kZoomButtonColour(0.475f, 0.604f, 0.863f);
const glm::vec3 kActiveTint(0.16f, 0.16f, 0.16f);
const glm::vec3 kBarActive(0.251f, 0.851f, 0.298f);
const glm::vec3 kBarInactive(0.451f, 0.451f, 0.478f);
const glm::vec3 kPickInactive(0.18f, 0.19f, 0.22f);
const glm::vec3 kPickBase(0.58f, 0.60f, 0.64f);
const glm::vec3 kPickHub(0.95f, 0.65f, 0.08f);
const glm::vec3 kPickBlade(0.95f, 0.36f, 0.07f);
const glm::vec3 kZoomProgressActive(0.475f, 0.604f, 0.863f);
const glm::vec3 kZoomProgressInactive(0.25f, 0.26f, 0.30f);
const glm::vec3 kZoomProgressBorder(0.08f, 0.09f, 0.12f);

} // namespace

#ifdef PLATFORM_ANDROID
SceneHUD::SceneHUD(AAssetManager* manager) : assetManager(manager) {}
#else
SceneHUD::SceneHUD() {}
#endif

SceneHUD::~SceneHUD()
{
    if (vao) glDeleteVertexArrays(1, &vao);
    if (vbo) glDeleteBuffers(1, &vbo);
    if (program) glDeleteProgram(program);
}

void SceneHUD::InitModel()
{
#ifdef PLATFORM_ANDROID
    program = ShaderHelper::buildProgramFromAssets(
        assetManager, "shader/HUDVertex.glsl", "shader/HUDFragment.glsl");
#else
    program = ShaderHelper::buildProgramFromFile(
        "HUDVertex.glsl", "HUDFragment.glsl");
#endif
    if (!program) {
        LOGE("SceneHUD: failed to build shader program");
        return;
    }

    projectionLocation = glGetUniformLocation(program, "ORTHOPROJECTION");

    glGenBuffers(1, &vbo);
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0, 2, GL_FLOAT, GL_FALSE, sizeof(HudVertex),
        reinterpret_cast<void*>(offsetof(HudVertex, position)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1, 3, GL_FLOAT, GL_FALSE, sizeof(HudVertex),
        reinterpret_cast<void*>(offsetof(HudVertex, colour)));
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    UpdateLayout();
}

void SceneHUD::UpdateLayout()
{
    const float buttonWidth = 120.0f;
    const float buttonHeight = 50.0f;
    const float sideMargin = 30.0f;
    const float top = 68.0f;
    const float verticalGap = 20.0f;
    const float rightLeft =
        std::max(sideMargin, static_cast<float>(screenWidth) - sideMargin - buttonWidth);

    buttons[kIncreaseSpeed] =
        {sideMargin, top, sideMargin + buttonWidth, top + buttonHeight};
    buttons[kDecreaseSpeed] =
        {sideMargin, top + buttonHeight + verticalGap,
         sideMargin + buttonWidth, top + buttonHeight * 2.0f + verticalGap};
    buttons[kZoomIn] =
        {rightLeft, top, rightLeft + buttonWidth, top + buttonHeight};
    buttons[kZoomOut] =
        {rightLeft, top + buttonHeight + verticalGap,
         rightLeft + buttonWidth, top + buttonHeight * 2.0f + verticalGap};

    projectionMatrix = glm::ortho(
        0.0f, static_cast<float>(screenWidth),
        static_cast<float>(screenHeight), 0.0f,
        -1.0f, 1.0f);
}

void SceneHUD::Resize(int w, int h)
{
    screenWidth = std::max(w, 1);
    screenHeight = std::max(h, 1);
    UpdateLayout();
}

void SceneHUD::AddRectangle(std::vector<HudVertex>& vertices,
                            const Rect& rect, const glm::vec3& colour) const
{
    const HudVertex topLeft{{rect.left, rect.top}, {colour.r, colour.g, colour.b}};
    const HudVertex topRight{{rect.right, rect.top}, {colour.r, colour.g, colour.b}};
    const HudVertex bottomRight{{rect.right, rect.bottom}, {colour.r, colour.g, colour.b}};
    const HudVertex bottomLeft{{rect.left, rect.bottom}, {colour.r, colour.g, colour.b}};

    vertices.push_back(topLeft);
    vertices.push_back(bottomLeft);
    vertices.push_back(bottomRight);
    vertices.push_back(topLeft);
    vertices.push_back(bottomRight);
    vertices.push_back(topRight);
}

void SceneHUD::Render()
{
    if (!program || !vao) return;

    const int heldButton = activeButton.load(std::memory_order_relaxed);
    const int currentSpeed = speed.load(std::memory_order_relaxed);
    const float currentCameraDistance =
        cameraDistance.load(std::memory_order_relaxed);

    std::vector<HudVertex> vertices;
    vertices.reserve((4 + 20 + kHudBladeCount + 2 + 3) * 6);

    for (int button = 0; button < 4; ++button) {
        glm::vec3 colour =
            button < 2 ? kSpeedButtonColour : kZoomButtonColour;
        if (button == heldButton) colour += kActiveTint;
        AddRectangle(vertices, buttons[button], glm::min(colour, glm::vec3(1.0f)));
    }

    // Exactly twenty 10 x 20 pixel blocks, centred near the bottom edge.
    const float blockWidth = 10.0f;
    const float blockHeight = 20.0f;
    const float barLeft =
        (static_cast<float>(screenWidth) - blockWidth * 20.0f) * 0.5f;
    const float barTop =
        std::max(0.0f, static_cast<float>(screenHeight) - 55.0f);

    for (int block = 0; block < 20; ++block) {
        const Rect rect{
            barLeft + block * blockWidth,
            barTop,
            barLeft + (block + 1) * blockWidth,
            barTop + blockHeight
        };
        AddRectangle(vertices, rect, block < currentSpeed ? kBarActive : kBarInactive);
    }

    // Bonus HUD: each of the twenty cells mirrors one blade highlight.
    // Separate side swatches report base and hub highlights.
    const float pickTop = std::max(0.0f, barTop - 34.0f);
    for (int block = 0; block < kHudBladeCount; ++block) {
        const unsigned int bladeBit =
            1u << (kFirstBladePart + block);
        const glm::vec3 colour =
            (highlightedPartMask & bladeBit) ? kPickBlade : kPickInactive;
        const Rect rect{
            barLeft + block * blockWidth,
            pickTop,
            barLeft + (block + 1) * blockWidth,
            pickTop + 16.0f
        };
        AddRectangle(vertices, rect, colour);
    }

    const bool baseHighlighted =
        (highlightedPartMask & (1u << kBasePart)) != 0;
    const bool hubHighlighted =
        (highlightedPartMask & (1u << kHubPart)) != 0;
    const Rect baseSwatch{
        barLeft - 24.0f, pickTop, barLeft - 8.0f, pickTop + 16.0f};
    const Rect hubSwatch{
        barLeft + blockWidth * kHudBladeCount + 8.0f, pickTop,
        barLeft + blockWidth * kHudBladeCount + 24.0f, pickTop + 16.0f};
    AddRectangle(vertices, baseSwatch,
                 baseHighlighted ? kPickBase : kPickInactive);
    AddRectangle(vertices, hubSwatch,
                 hubHighlighted ? kPickHub : kPickInactive);

    // A single continuous right-middle track mirrors the full zoom range.
    // Moving closer raises the blue fill; moving farther lowers it.
    const float zoomBarWidth = 20.0f;
    const float zoomBarHeight = 180.0f;
    const float zoomLeft = std::max(
        3.0f, static_cast<float>(screenWidth) - 50.0f);
    const float zoomTop = std::max(
        3.0f, (static_cast<float>(screenHeight) - zoomBarHeight) * 0.5f);
    const float zoomRatio = std::clamp(
        (9.0f - currentCameraDistance) / (9.0f - 3.0f), 0.0f, 1.0f);

    const Rect zoomBorder{
        zoomLeft - 3.0f, zoomTop - 3.0f,
        zoomLeft + zoomBarWidth + 3.0f, zoomTop + zoomBarHeight + 3.0f};
    const Rect zoomTrack{
        zoomLeft, zoomTop,
        zoomLeft + zoomBarWidth, zoomTop + zoomBarHeight};
    AddRectangle(vertices, zoomBorder, kZoomProgressBorder);
    AddRectangle(vertices, zoomTrack, kZoomProgressInactive);

    if (zoomRatio > 0.0f) {
        const float fillTop =
            zoomTop + zoomBarHeight * (1.0f - zoomRatio);
        const Rect zoomFill{
            zoomLeft, fillTop,
            zoomLeft + zoomBarWidth, zoomTop + zoomBarHeight};
        AddRectangle(vertices, zoomFill, kZoomProgressActive);
    }

    glUseProgram(program);
    glUniformMatrix4fv(
        projectionLocation, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(vertices.size() * sizeof(HudVertex)),
        vertices.data(),
        GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertices.size()));
    glBindVertexArray(0);
    glUseProgram(0);
}

void SceneHUD::ApplyButtonAction(int button)
{
    switch (button) {
        case kIncreaseSpeed: {
            int current = speed.load(std::memory_order_relaxed);
            while (current < 20 && !speed.compare_exchange_weak(
                       current, std::min(current + 1, 20),
                       std::memory_order_relaxed)) {}
            LOGI("SceneHUD: +SPD pressed");
            break;
        }
        case kDecreaseSpeed: {
            int current = speed.load(std::memory_order_relaxed);
            while (current > 0 && !speed.compare_exchange_weak(
                       current, std::max(current - 1, 0),
                       std::memory_order_relaxed)) {}
            LOGI("SceneHUD: -SPD pressed");
            break;
        }
        case kZoomIn: {
            float current = cameraDistance.load(std::memory_order_relaxed);
            while (current > 3.0f && !cameraDistance.compare_exchange_weak(
                       current, std::max(current - kZoomStep, 3.0f),
                       std::memory_order_relaxed)) {}
            LOGI("SceneHUD: +ZOOM pressed");
            break;
        }
        case kZoomOut: {
            float current = cameraDistance.load(std::memory_order_relaxed);
            while (current < 9.0f && !cameraDistance.compare_exchange_weak(
                       current, std::min(current + kZoomStep, 9.0f),
                       std::memory_order_relaxed)) {}
            LOGI("SceneHUD: -ZOOM pressed");
            break;
        }
        default:
            break;
    }
}

bool SceneHUD::TouchDown(float x, float y)
{
    int pressedButton = -1;
    for (int button = 0; button < 4; ++button) {
        if (buttons[button].Contains(x, y)) {
            pressedButton = button;
            break;
        }
    }

    activeButton.store(pressedButton, std::memory_order_relaxed);
    if (pressedButton < 0) return false;

    ApplyButtonAction(pressedButton);
    return true;
}

void SceneHUD::TouchRelease()
{
    activeButton.store(-1, std::memory_order_relaxed);
}
