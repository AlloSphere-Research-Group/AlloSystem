
uniform float fov; 		// e.g. 90.;
uniform float aspect; 	// e.g. 1.;
uniform float znear; 	// e.g. 0.1;
uniform float zfar;		// e.g. 100.;
uniform float focal;	// e.g. 1.;
uniform float offset; 	// e.g. 0.01 or -0.01 for right/left eye


///////////////////////////////////////////////////////////////
// Stereographics:
// takes in the gl_Vertex (object space)
// returns the gl_Position (clip space)
// also modifies modelview matrix
// and generates a projection matrix
///////////////////////////////////////////////////////////////
mat4 modelview, projection;
vec4 stereographic(vec4 vertex) 
{
	float PI = 3.141;
	float PI_OVER_360 = PI / 360.;

	// derive position/unit vectors from the current modelview:
	mat4 modelview = gl_ModelViewMatrix;
	
	vec3 ux, uy, uz, pos;
	ux = vec3(modelview[0][0], modelview[0][1], modelview[0][2]);
	uy = vec3(modelview[1][0], modelview[1][1], modelview[1][2]);
	uz = vec3(modelview[2][0], modelview[2][1], modelview[2][2]);
	pos = vec3(modelview[3][0], modelview[3][1], modelview[3][2]);

	// shift eye position for off-axis stereo:
	vec3 eyepos = pos - ux*offset;
	modelview[3][0] = eyepos.x; 
	modelview[3][1] = eyepos.y; 
	modelview[3][2] = eyepos.z;

	// equiv. glFrustum; off-axis stereo:
	float shift = -offset*znear/focal;
	float top = znear * tan(fov * PI_OVER_360);	// height of view at distance = near
	float bottom = -top;
	float left = -aspect*top + shift;
	float right = aspect*top + shift;
	float W = right-left;	
	float W2 = right+left;
	float H = top-bottom;	
	float H2 = top+bottom;
	float D = zfar-znear;	
	float D2 = zfar+znear;
	float n2 = znear*2.;
	float fn2 = zfar*n2;
	projection = mat4(
		n2/W, 0., 0., 0.,
		0., n2/H, 0., 0.,
		W2/W, H2/H, -D2/D, -1.,
		0., 0., -fn2/D, 0.
	);
	
	return projection * modelview * vertex;
}

void main()
{
	gl_Position = stereographic(gl_Vertex);
	normal = gl_NormalMatrix * gl_Normal;		// should normal matrix shift?
	gl_FrontColor = gl_Color;
	gl_TexCoord[0] = gl_MultiTexCoord0;
}

