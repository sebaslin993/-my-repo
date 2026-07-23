#pragma once

/**
 * Square.h
 *
 * Concrete Model that renders an independent colored square beside the triangle.
 */

#include "Model.h"
#include "Platform.h"

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

class Square : public Model {
public:
#ifdef PLATFORM_ANDROID
    explicit Square(AAssetManager* assetMgr);
#else
    Square();
#endif
    ~Square() override;

    void InitModel() override;
    void Render() override;
    void Resize(int w, int h) override;

private:
#ifdef PLATFORM_ANDROID
    AAssetManager* mgr = nullptr;
#endif

    GLuint programID = 0;
    GLuint vao = 0;
    GLuint vboPos = 0;
    GLuint vboColor = 0;
};
