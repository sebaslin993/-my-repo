#version 300 es

precision mediump float;

in vec3 vColour;
out vec4 FragmentColor;

void main()
{
    FragmentColor = vec4(vColour, 1.0);
}
