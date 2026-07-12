#version 300 es

precision mediump float;

in float vShade;

uniform vec3 PARTCOLOR;

out vec4 FragmentColor;

void main()
{
    FragmentColor = vec4(PARTCOLOR * vShade, 1.0);
}
