#version 300 es

/**
 * TriangleVertex.glsl
 *
 * ES 3.0 vertex shader for the touch-events triangle demo.
 * Attribute locations are bound via layout qualifiers so that
 * they match the constants in Triangle.cpp without needing
 * glBindAttribLocation calls.
 */

layout(location = 0) in vec2 VertexPosition;
layout(location = 1) in vec3 VertexColor;

out vec3 VarColor;

void main()
{
    gl_Position = vec4(VertexPosition, 0.0, 1.0);
    VarColor    = VertexColor;
}
