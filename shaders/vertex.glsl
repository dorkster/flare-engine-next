#version 110

uniform float offsetX;
uniform float offsetY;

attribute vec2 position;

varying vec2 texcoord;

void main()
{
    gl_Position = vec4(offsetX, offsetY, 0.0, 1.0);
    texcoord = position * vec2(0.5) + vec2(0.5);
}
