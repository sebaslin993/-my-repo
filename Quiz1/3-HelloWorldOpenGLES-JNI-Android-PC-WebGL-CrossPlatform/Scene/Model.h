#pragma once
class Model {
public:
    Model() {}
    virtual ~Model() {}

    virtual void InitModel() = 0;   // called once, before the first frame
    virtual void Render() = 0;   // called once per frame
    virtual void Resize(int w, int h) {}   // called whenever the surface/window resizes
};


