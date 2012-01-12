varying vec3 normal, lightDir;
varying vec2 texCoord;

uniform float mode;

void main() {
	vec4 ecPos;
	ecPos = vec4(gl_ModelViewMatrix * gl_Vertex);
	
	
	if (mode > 0.5) {
		ecPos += vec4(gl_Normal * -0.1, 1.);
		gl_FrontColor = vec4(0, 0, 0, 1);
	} else {
		gl_FrontColor = vec4(1, 1, 1, 1);
	}
	lightDir = normalize(vec3(gl_LightSource[0].position) - ecPos.xyz);
	normal = normalize(gl_NormalMatrix * gl_Normal);

	
	
	texCoord = vec2(gl_MultiTexCoord0);
	gl_Position = gl_ProjectionMatrix * ecPos;
}