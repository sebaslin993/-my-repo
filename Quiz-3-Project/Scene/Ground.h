#pragma once

/**
 * Ground.h
 *
 * Owns one large world-space quad. Its fragment shader creates the tiled
 * checkerboard procedurally and applies the same Phong light as the fan.
 */
#include "Model.h"
#include "Platform.h"
#include <glm/glm.hpp>

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

class Ground : public Model {
public:
#ifdef PLATFORM_ANDROID
    explicit Ground(AAssetManager* assetManager);
#else
    Ground();
#endif
    ~Ground() override;

    void InitModel() override;
    void Render() override;
    void Resize(int w, int h) override;
    void SetViewProjection(const glm::mat4& view, const glm::mat4& projection);

private:
#ifdef PLATFORM_ANDROID
    AAssetManager* assetManager = nullptr;
#endif

    GLuint program = 0;
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ibo = 0;

    GLint modelLocation = -1;
    GLint modelViewLocation = -1;
    GLint mvpLocation = -1;
    GLint normalLocation = -1;
    GLint lightPositionLocation = -1;

    glm::mat4 viewMatrix{1.0f};
    glm::mat4 projectionMatrix{1.0f};
};
