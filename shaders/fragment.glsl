#version 110

uniform sampler2D texture;
uniform sampler2D normals;
uniform bool lightEnabled;

varying vec2 texcoord;

void main()
{
	if (lightEnabled)
	{
		// need to setup these
		vec4 v_color = vec4(1.0);

		vec3 light = vec3(0.5);
		vec3 ambientColor = vec3(0.8);
		float ambientIntensity = 0.1;
		vec2 resolution = vec2(640, 480);
		vec3 lightColor = vec3(0.7);
		bool useNormals = true;
		bool useShadow = true;
		vec3 attenuation = vec3(0.001);
		// end of setup

		vec4 color = texture2D(texture, texcoord);
		vec3 normal = normalize(texture2D(normals, texcoord).rgb * 2.0 - 1.0);

		vec3 light_pos = normalize(light);
		float lambert = useNormals ? max(dot(normal, light_pos), 0.0) : 1.0;

		float d = distance(gl_FragCoord.xy, light.xy * resolution);
		d *= light.z;
       
		float att = useShadow ? 1.0 / ( attenuation.x + (attenuation.y*d) + (attenuation.z*d*d) ) : 1.0;
       
		vec3 result = (ambientColor * ambientIntensity) + (lightColor.rgb * lambert) * att;
		result *= color.rgb;
       
		gl_FragColor = v_color * vec4(result, color.a);
	}
	else
	{
        gl_FragColor = texture2D(texture, texcoord);
	}
}
