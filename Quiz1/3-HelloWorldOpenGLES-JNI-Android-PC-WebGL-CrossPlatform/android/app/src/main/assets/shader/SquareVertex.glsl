#version 300 es
layout(location = 0) in vec4 VertexPosition;
layout(location = 1) in vec3 VertexColor;

out vec4 SquareColor;

void main()
{
    gl_Position = VertexPosition;
    SquareColor = vec4(VertexColor, 1.0);
}
