#version 300 es

precision highp float;

layout(location = 0) in vec2 Position;
layout(location = 1) in vec3 Colour;

uniform mat4 ORTHOPROJECTION;

out vec3 vColour;

void main()
{
    vColour = Colour;
    gl_Position = ORTHOPROJECTION * vec4(Position, 0.0, 1.0);
}
