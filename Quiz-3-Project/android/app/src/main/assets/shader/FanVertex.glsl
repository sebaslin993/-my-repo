#version 300 es

precision highp float;

layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Normal;

uniform mat4 MODELVIEWPROJECTIONMATRIX;
uniform mat4 MODELVIEWMATRIX;
uniform mat3 NORMALMATRIX;

out vec3 vPositionView;
out vec3 vNormalView;

void main()
{
    vec4 positionView = MODELVIEWMATRIX * vec4(Position, 1.0);
    vPositionView = positionView.xyz;
    vNormalView = normalize(NORMALMATRIX * Normal);
    gl_Position = MODELVIEWPROJECTIONMATRIX * vec4(Position, 1.0);
}
