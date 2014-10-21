#version 110

uniform sampler2D texture;

varying vec2 texcoord;

void main()
{
	vec4 color = texture2D(texture, texcoord);

	// mix colors in brameBuffer instead of discarding transparent pixels
	if (color.rgb == vec3(0.0,0.0,0.0))
		  discard;

    gl_FragColor = color;
}
