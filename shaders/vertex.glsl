#version 110

attribute vec2 position;
attribute vec2 texScale;

varying vec2 texcoord;

void main()
{
    gl_Position = vec4(position.x, -position.y, 0.0, 1.0);

	float xCoord = position.x * 0.5 + 0.5 * texScale.x;
	float yCoord = position.y * 0.5 + 0.5 * texScale.y;

    texcoord = vec2(xCoord, yCoord);
}
