#version 110

attribute vec2 position;
attribute float scaleX;
attribute float scaleY;

attribute float offsetX;
attribute float offsetY;

varying vec2 texcoord;

void main()
{
    gl_Position = vec4(position.x * scaleX - 1.0 + offsetX/0.5, -position.y * scaleY + 1.0 - offsetY/0.5, 0.0, 1.0);

    texcoord = position * vec2(0.5) + vec2(0.5);
}
