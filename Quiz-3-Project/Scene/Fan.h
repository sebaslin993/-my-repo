#pragma once

/**
 * Fan.h
 *
 * Owns the Quiz 2 fan hierarchy, refined to match the supplied Quiz 3 visual
 * reference. Quiz 3 supplies camera matrices and speed from Scene3D/SceneHUD,
 * and this class caches every pickable part transform for world-ray picking.
 */
#include "Model.h"
#include "Platform.h"
#include "Transform.h"
#include <array>
#include <atomic>
#include <vector>
#include <glm/glm.hpp>

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

class Fan : public Model {
public:
    static constexpr int kBladeCount = 20;
    static constexpr int kPartCount = 3 + kBladeCount;

#ifdef PLATFORM_ANDROID
    explicit Fan(AAssetManager* assetManager);
#else
    Fan();
#endif
    ~Fan() override;

    void InitModel() override;
    void Render() override;
    void Resize(int w, int h) override;

    void SetViewProjection(const glm::mat4& view, const glm::mat4& projection);
    void SetSpeed(float newSpeed);
    bool Pick(const glm::vec3& rayOrigin, const glm::vec3& rayDirection);
    unsigned int HighlightedPartMask() const {
        return highlightedPartMask.load(std::memory_order_relaxed);
    }

private:
    struct Mesh {
        GLuint vao = 0;
        GLuint vbo = 0;
        GLuint ibo = 0;
        GLsizei indexCount = 0;
    };

    bool LoadObjMesh(const char* fileName, Mesh& mesh);
    void UploadMesh(Mesh& mesh, const std::vector<GLfloat>& vertices,
                    const std::vector<GLushort>& indices);
    void CreateCubeMesh();
    void CreateCylinderMesh();
    void CreateSphereMesh();
    void DeleteMesh(Mesh& mesh);
    void DrawMesh(const Mesh& mesh, const glm::vec3& colour, int partId);
    bool RayHitsPart(const glm::vec3& origin, const glm::vec3& direction,
                     const glm::mat4& partMatrix, float& distance) const;

#ifdef PLATFORM_ANDROID
    AAssetManager* assetManager = nullptr;
#endif

    Transform transform;
    GLuint program = 0;
    Mesh cube;
    Mesh cylinder;
    Mesh sphere;

    GLint mvpLocation = -1;
    GLint modelViewLocation = -1;
    GLint normalLocation = -1;
    GLint partColorLocation = -1;
    GLint lightPositionLocation = -1;

    glm::mat4 viewMatrix{1.0f};
    glm::mat4 projectionMatrix{1.0f};
    std::array<glm::mat4, kPartCount> partMatrices{};
    std::array<bool, kPartCount> partMatrixValid{};

    float spinAngle = 0.0f;
    float assemblyAngle = 0.0f;
    float speed = 8.0f;
    std::atomic<unsigned int> highlightedPartMask{0u};
};
