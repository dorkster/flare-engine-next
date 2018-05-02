#version 110

attribute vec2 position;
attribute float texScaleX;
attribute float texScaleY;

varying vec2 texcoord;

void main()
{
    gl_Position = vec4(position.x, -position.y, 0.0, 1.0);

	float xCoord = position.x / texScaleX * 0.5 + 0.5;
	float yCoord = position.y / texScaleY * 0.5 + 0.5;

    texcoord = vec2(xCoord, yCoord);
}
