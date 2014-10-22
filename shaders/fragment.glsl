#version 110

uniform sampler2D texture;
uniform sampler2D destTexture;

varying vec2 texcoord;
varying vec4 destination;

void main()
{
	vec4 color;
	vec4 destColor = texture2D(destTexture, texcoord);

	if (destination != vec4(0.0))
	{
		color = texture2D(texture, texcoord);

		if (color.a == 0.0)
		{
			discard;
		}
	}
	else
	{
		color = destColor;
	}

    gl_FragColor = color;
}
