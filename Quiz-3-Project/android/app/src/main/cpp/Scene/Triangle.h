#pragma once

/**
 * Triangle.h
 *
 * Renders a simple 2D triangle using OpenGL ES 3.0 VAO/VBO.
 * Touch events change the triangle's vertex colours:
 *   - TouchEventDown    → all red
 *   - TouchEventMove    → all green
 *   - TouchEventRelease → all blue   (initial state)
 *
 * Ported from the OpenGL ES 3.0 Cookbook Chapter 2.
 */

#include "Model.h"
#include "ShaderHelper.h"
#include <android/asset_manager.h>

class Triangle : public Model {
public:
    explicit Triangle(AAssetManager* assetMgr);
    ~Triangle() override;

    void InitModel()                         override;
    void Render()                            override;

    void TouchEventDown(float x, float y)    override;
    void TouchEventMove(float x, float y)    override;
    void TouchEventRelease(float x, float y) override;

private:
    AAssetManager* mgr = nullptr;

    GLuint program = 0;
    GLuint vao     = 0;   // Vertex Array Object  (ES 3.0)
    GLuint vboPos  = 0;   // positions
    GLuint vboCol  = 0;   // colours

    // 3 vertices × 3 components (R,G,B) – updated on touch
    GLfloat colors[9] = {
        0.0f, 0.0f, 1.0f,   // vertex 0 – blue (initial)
        0.0f, 0.0f, 1.0f,   // vertex 1
        0.0f, 0.0f, 1.0f    // vertex 2
    };

    void setAllColors(float r, float g, float b);
    void uploadColors();
};
