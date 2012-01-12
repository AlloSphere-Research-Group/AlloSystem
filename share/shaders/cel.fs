varying vec3 normal, lightDir;
varying vec2 texCoord;
uniform sampler2D texture;

void main()
{
	float intensity;
	vec3 n;
	vec4 color = gl_Color;
	float factor;
	
	n = normalize(normal);
	intensity = dot(lightDir, n);
	
	if (intensity > 0.9) {
		color *= vec4(1.0,1.0,1.0,1.0);
	} else if (intensity > 0.5) {
		color *= vec4(0.8,0.8,0.8,1.0);
	} else if (intensity > 0.35) {
		color *= vec4(0.4,0.4,0.4,1.0);
	} else {
		color *= vec4(0.0,0.0,0.0,1.0);
	}
	if (intensity > 0.10) {
		factor = 0.25;
	} else {
		factor = 0.0;
	}
	//gl_FragColor = color * texture2D(texture, texCoord);
	//gl_FragColor = color * vec4(factor, factor, factor, 1);;
	gl_FragColor = color;
}
