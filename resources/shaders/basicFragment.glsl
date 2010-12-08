varying vec4 diffuse,ambient;
varying vec3 normal,lightDir,halfVector;
varying vec4 texcoord0;

uniform sampler2D tex;

void main()
{
	vec3 n,halfV,viewV,ldir;
	float NdotL,NdotHV;
	
	vec4 color;
	color = vec4(0.2, 0.2, 0, 1);
	color = ambient;
	color = vec4(0.5, texcoord0.st, 1); 
	color = texture2D(tex, texcoord0.st);
	
	
//	/* a fragment shader can't write a verying variable, hence we need
//	a new variable to store the normalized interpolated normal */
//	n = normalize(normal);
//	
//	/* compute the dot product between normal and ldir */
//	NdotL = max(dot(n,lightDir),0.0);
//
//	if (NdotL > 0.0) {
//		halfV = normalize(halfVector);
//		NdotHV = max(dot(n,halfV),0.0);
//		color += gl_FrontMaterial.specular * gl_LightSource[0].specular * pow(NdotHV,gl_FrontMaterial.shininess);
//		color += diffuse * NdotL;
//	}

	gl_FragColor = color;
}
