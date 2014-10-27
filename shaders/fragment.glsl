#version 110

uniform sampler2D texture;
//uniform sampler2D destTexture;
uniform vec4 offset;

varying vec2 texcoord;

void main()
{
	vec4 color;
	/*vec4 destColor = texture2D(destTexture, texcoord);

	if (offset != vec4(0.0))
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
	}*/

	color = texture2D(texture, texcoord);

    gl_FragColor = color;
}
