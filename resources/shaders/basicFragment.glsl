
uniform sampler2D tex0;

varying vec4 diffuse,ambient;
varying vec3 normal,lightDir,halfVector;
varying vec2 texcoord0;

void main()
{
	vec3 n,halfV,viewV,ldir;
	float NdotL,NdotHV;
	
	vec4 color = ambient;
	//color = vec4(0.2, 0.2, 0, 1);
	//color = ambient;
	//color = vec4(0.5, texcoord0, 1); 
	
	/* a fragment shader can't write a verying variable, hence we need
	a new variable to store the normalized interpolated normal */
	n = normalize(normal);
	
	/* compute the dot product between normal and ldir */
	NdotL = max(dot(n,lightDir),0.0);

	if (NdotL > 0.0) {
		halfV = normalize(halfVector);
		NdotHV = max(dot(n,halfV),0.0);
		color += gl_FrontMaterial.specular * gl_LightSource[0].specular * pow(NdotHV,gl_FrontMaterial.shininess);
		color += diffuse * NdotL;
	}
	
	color *= texture2D(tex0, texcoord0);

	gl_FragColor = color;
}
