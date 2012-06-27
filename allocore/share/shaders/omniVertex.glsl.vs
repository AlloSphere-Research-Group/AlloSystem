
// http://www.opengl.org/wiki/Vertex_Transformation
// http://3dengine.org/Right-up-back_from_modelview
// http://paulbourke.net/miscellaneous/domemirror/
// http://en.wikibooks.org/wiki/GLSL_Programming/Applying_Matrix_Transformations

uniform float osc;

// NOTE: mat4 ctor is transposed to allocore ctor:
mat4 modelview;
mat4 projection;

float PI = 3.141;
float PI_OVER_360 = PI / 360.;

/*
	A vertex shader's job is to take in an object-space position (gl_Vertex)
	and produce a clip-space position (gl_Position)

	For omnigraphics, each resultant pixel column should correspond to an angular offset from the center.
	What we want to know, is that for a given world-space vertex, which screen column would it occupy?

	E.g., an omniview that is 360' panoramic, each particular eye-space vertex would have a specific rotation from the view.
	That should simply be atan2(ecvertex.z,ecvertex.x), right?

	Not so simple. We are projecting out from a vertex, not sucking in from a pixel.
	What we do know is the eye-space position/orientation of the vertex centered on the eye.
	From that we can calculate which pixel it ought to be on, by a simple mercator-style projection.

	Maybe that is the way to simply bypass projection matrix altogether.

	Try cylindrical first. The output clip coordinate 
*/

void main()
{
	// perspective parameters:
	float fov = 90.;
	float aspect = 1.;
	float znear = 0.1;
	float zfar = 100.;
	float eyesep = 0.01;
	float focal = 1.;

	float omnipan = 2.*PI;	// 360' panoramic

	// derive unit vectors / position from current modelview:
	modelview = gl_ModelViewMatrix;
	// in actual uses, this could be derived from the modelview:
	vec3 ux, uy, uz, pos;
/*
	ux = vec3(modelview[0][0], modelview[0][1], modelview[0][2]);
	uy = vec3(modelview[1][0], modelview[1][1], modelview[1][2]);
	uz = vec3(modelview[2][0], modelview[2][1], modelview[2][2]);
*/	
	// weird coordinate transform for ShaderBuilder:
	ux = vec3(-modelview[0][0], modelview[0][2], modelview[0][1]);
	uy = vec3(-modelview[1][0], modelview[1][2], modelview[1][1]);
	uz = vec3(-modelview[2][0], modelview[2][2], modelview[2][1]);
	pos = vec3(modelview[3][0], modelview[3][1], modelview[3][2]);

	vec4 vertex = gl_Vertex;

/*
	// move vertex into eyespace:
	vec4 ecVertex = modelview * gl_Vertex;
	// calculate relative azimuth:
	float azimuth = atan(-ecVertex.z, ecVertex.x);

	// convert this to a fraction of the omniview:
	float rot = omnipan * azimuth / (2.*PI);
*/
	float rot = 2.*PI * (osc-0.5);

	// rotate the model to face it:
	float C = cos(rot); 
	float S = sin(rot);
	modelview = mat4(	
		C,  0,  -S, 0, 
		0,  1,  0,  0, 
		S,  0,  C,  0, 
		0,  0,  0,  1
	) * modelview;


	// equiv. glFrustum:
	//float shift = -offset*znear/focal;
	float top = znear * tan(fov * PI_OVER_360);	// height of view at distance = near
	float bottom = -top;
	float left = -aspect*top;// + shift;
	float right = aspect*top;// + shift;
	float W = right-left;	
	float W2 = right+left;
	float H = top-bottom;	
	float H2 = top+bottom;
	float D = zfar-znear;	
	float D2 = zfar+znear;
	float n2 = znear*2.;
	float fn2 = zfar*n2;
	projection = mat4(
		n2/W, 	0., 		0., 		0.,
		0., 		n2/H, 	0., 		0.,
		W2/W, 	H2/H, 	-D2/D, 	-1.,
		0., 		0., 		-fn2/D,	0.
	);

/*
	vec4 clip = projection * ecVertex;
	vec4 eye3 = ecVertex / ecVertex.w;
	
	float radius = 50.0;  // fisheye hemisphere radius
	vec4 pn = eye3;
	float d = length(pn);
	pn = normalize(pn);
	d = d / radius;	    
	float u = atan(pn.y,pn.x);  
	float v = 2.0 * acos(-pn.z) / 3.141529265358979;

		// change polar to cartesian coordinates on circle (with depth)
		gl_Position.x = cos(u) * v;
		gl_Position.y = sin(u) * v;
		gl_Position.z = d;// * -1.0 * abs(p.z) / p.z;
		gl_Position.w = 1.0;
*/
/*
	// equiv. glOrtho:
	projection = mat4(	
		2./W,	0,		0,		-W2/W,
		0,		2./H,	0,		-H2/H,
		0,		0,		-2./D,	-D2/D,
		0,		0,		0,		1.	
	);

	projection = mat4(	
		2./W,	0,		0,		0,
		0,		2./H,	0,		0,
		0,		0,		-2./D,	0,
		-W2/W,	-H2/H,	-D2/D,	1.	
	);
*/
	//projection = gl_ProjectionMatrix;

	///////////////////////////////////////////////////////////////
	// standard stuff:
	///////////////////////////////////////////////////////////////
	// get coordinate in eye-space:
	vec4 ecv = modelview * vertex;

	// get polar coordinates:
	float azimuth = atan(ecv.x, -ecv.z); 
	float elevation = atan(ecv.y, length(ecv.xz));

	// standard perspective projection:
	//gl_Position = projection * modelview * vertex;
	gl_Position.x = ecv.x*n2/W;	
	gl_Position.y = ecv.y*n2/H;	
	gl_Position.z = ecv.z*-D2/D + ecv.w*-fn2/D;
	gl_Position.w = ecv.z*-1.;
/*
	// ortho projection:
	gl_Position.x = ecv.x*2./W;	
	gl_Position.y = ecv.y*2./H;	
	gl_Position.z = ecv.z*-2./D;
	gl_Position.w = ecv.x*-W2/W + ecv.y*-H2/H + ecv.z*-1. + ecv.w;
*/	
	float ofovx = fov * PI/180.;
	float ofovy = ofovx;
	
	// map azimuth/elevation to the XY clipspace
	gl_Position.x = azimuth/PI; 
	gl_Position.y = elevation/PI; 
	
	// map (adjusted) depth z to clip space:
	gl_Position.z = (length(ecv)-znear)/(zfar-znear);
	gl_Position.w = 1.;
	
/*
	
*/
	/* 
		idea: convert entire world space (ecv coordinate) 
		into a space that fits into the frustum
		
		try cylindrical space first:
		move x,z coordinates into a cylindrical section.

		y is unchanged
		radius = length(xz)
		a = atan(z,x)
		a *= fov
		x = cos(a)
		z = sin(a)
	*/
/*
	vec2 xz = ecv.xz;
	float rxz = length(xz);
	float a = atan(ecv.z, ecv.x);
	float rot = -PI/2.;
	a = rot + ((a-rot) * 0.25);
	ecv.x = rxz*cos(a);
	ecv.z = rxz*sin(a);
*/
/*
	float rxy = length( ecv.xy ); 
	if( rxy != 0.0 ) { 
       	float phi = atan( rxy, -pos.z );
       	float lens_radius = sin(phi);//phi / (PI/2.);
 
      	ecv.xy *= ( lens_radius / rxy ); 
	} 
*/	

	//gl_Position = gl_ProjectionMatrix * ecv;
	//gl_Position = vec4(ecv.x, osc, 0., ecv.w); //modelview * vertex;

	//normal = gl_NormalMatrix * gl_Normal;

	gl_FrontColor = gl_Color;
	gl_FrontColor.r = -azimuth/PI;
	gl_TexCoord[0] = gl_MultiTexCoord0;
}
