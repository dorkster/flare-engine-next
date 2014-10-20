#version 110

attribute vec2 position;
attribute float scaleX;
attribute float scaleY;

attribute float offsetX;
attribute float offsetY;

varying vec2 texcoord;

void main()
{
	float leftCornerX = position.x * scaleX - 1.0 + scaleX;
	float leftCornerY =  - (position.y * scaleY - 1.0 + scaleY);

    gl_Position = vec4(leftCornerX + offsetX, leftCornerY - offsetY, 0.0, 1.0);

    texcoord = position * vec2(0.5) + vec2(0.5);
}
