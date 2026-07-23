#pragma once

/**
 * Model.h
 *
 * Abstract base class for all renderable objects.
 * Ported from the original Model.h in the OpenGL ES 3.0 Cookbook;
 * dependency on the old GLPIFramework removed – now self-contained.
 */

enum ModelType {
    TriangleType = 0,
    CubeType     = 1
};

class Model {
public:
    Model() {}
    virtual ~Model() {}

    /** Load shaders, create GPU buffers, etc. */
    virtual void InitModel() = 0;

    /** Called every frame to draw this model. */
    virtual void Render() = 0;

    /** Optional per-frame update (animation, physics, etc.). */
    virtual void Update(float dt) {}

    /** Called when the viewport dimensions change. */
    virtual void Resize(int w, int h) {}

    /** Returns the model's type enum. */
    virtual ModelType GetModelType() { return modelType; }

    // ------------------------------------------------------------------
    // Touch event interface
    // ------------------------------------------------------------------
    virtual void TouchEventDown(float x, float y)    {}
    virtual void TouchEventMove(float x, float y)    {}
    virtual void TouchEventRelease(float x, float y) {}

protected:
    ModelType modelType = TriangleType;
};
