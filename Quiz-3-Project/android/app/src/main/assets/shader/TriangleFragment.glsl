#version 300 es

/**
 * TriangleFragment.glsl
 *
 * ES 3.0 fragment shader for the touch-events triangle demo.
 * Simply outputs the interpolated vertex colour as the fragment colour.
 */

precision mediump float;

in  vec3 VarColor;
out vec4 FragColor;

void main()
{
    FragColor = vec4(VarColor, 1.0);
}
