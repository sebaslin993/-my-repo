#version 300 es

precision highp float;

layout(location = 0) in vec3 Position;
layout(location = 1) in float Shade;

uniform mat4 MODELVIEWPROJECTIONMATRIX;

out float vShade;

void main()
{
    vShade = Shade;
    gl_Position = MODELVIEWPROJECTIONMATRIX * vec4(Position, 1.0);
}
