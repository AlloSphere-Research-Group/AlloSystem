/*
Allocore Example: Fluid

Description:
This demonstrates a 3D fluid simulation

Author:
Graham Wakefield 2011
*/


#include "allocore/al_Allocore.hpp"
#include "allocore/graphics/al_Shader.hpp"
#include "alloutil/al_Field3D.hpp"

using namespace al;

// create a fluid on a 32x32x32 grid:
Fluid3D<float> fluid(32);

// create an intensity field (on a 32x32x32 grid) 
// to be driven around:
Field3D<float> intensities(3, 32);

// textures to show the fluid densities & velocities
Texture intensityTex, velocityTex;

Graphics gl;
Mesh mesh;
ShaderProgram shaderP;
Shader shaderV, shaderF;

static const char * srcV = AL_STRINGIFY(
uniform sampler3D velocityTex; 
uniform sampler3D intensityTex; 
varying vec3 velocity;
varying vec3 intensity;
void main(){
	vec3 xyz = gl_Vertex.xyz / 32.; 
	velocity = texture3D(velocityTex, xyz).rgb;
	// Array components = 2 defaults to GL_LUMINANCE_ALPHA:
	intensity = texture3D(intensityTex, xyz).rgb;
	vec4 vertex1 = gl_Vertex;
	vec2 texcoord0 = vec2(gl_MultiTexCoord0);
	if (texcoord0.s > 0.) {
		// displace by length:
		vertex1 += vec4(velocity, 0);
	} else {
		// displace with width:
		float displace = (texcoord0.t - 0.5);
		vec3 axis = cross(normalize(velocity), vec3(0, 0, -1));
		vertex1 += vec4(displace * axis, 0);
	}

	gl_Position = gl_ModelViewProjectionMatrix * vertex1;
}
);

static const char * srcF = AL_STRINGIFY(
varying vec3 velocity;
varying vec3 intensity;
void main() {
	float mag = 0.1+length(velocity);
	gl_FragColor = vec4(0.1+intensity, mag);
}
);

struct MyWindow : public Window {

	bool onCreate(){
	
		// reconfigure textures based on arrays:
		intensityTex.submit(intensities.front(), true);
		velocityTex.submit(fluid.velocities.front(), true);
		
		// shader method:
		shaderV.source(srcV, Shader::VERTEX).compile();
		shaderF.source(srcF, Shader::FRAGMENT).compile();
		shaderP.attach(shaderV).attach(shaderF).link();
		shaderV.printLog();
		shaderF.printLog();
		shaderP.printLog();

		// create rendering mesh of lines
		mesh.reset();
		mesh.primitive(Graphics::TRIANGLES);
		for (int x=0; x<32; x++) {
		for (int y=0; y<32; y++) {
		for (int z=0; z<32; z++) {
			// render 2 vertices at the same location
			// using texcoord as an attribute to distinguish 
			// 'start' and 'end' vertices
			mesh.texCoord(0, 0);
			mesh.vertex(x, y, z);
			mesh.texCoord(0, 1);	
			mesh.vertex(x, y, z);
			mesh.texCoord(1, 0);	
			mesh.vertex(x, y, z);	
		}}}
		
		return true;
	}
	
	bool onFrame(){
		gl.clearColor(0,0,0,0);
		gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
		gl.viewport(0,0, width(), height());
		
		gl.matrixMode(gl.PROJECTION);
		gl.loadMatrix(Matrix4d::perspective(45, aspect(), 0.1, 100));

		gl.matrixMode(gl.MODELVIEW);
		gl.loadMatrix(Matrix4d::lookAt(Vec3d(16, 16, 64), Vec3d(16, 16, 16), Vec3d(0,1,0)));
		
		
		// add some forces:
		float t = MainLoop::now() * 0.25;
		float r = 20;
		fluid.addVelocity(Vec3f(12, 12, 16), Vec3f(r*cos(t), r*sin(t), 1));
		fluid.addVelocity(Vec3f(20, 20, 16), Vec3f(-r*cos(t*2), -r*sin(t*2), 0));
		fluid.addVelocity(Vec3f(16, 16, 20), Vec3f(0, -r*cos(t*3), -r*sin(t*3)));
		
		// add some intensities:
		float v = 4;
		intensities.add(Vec3f(12, 20, 16), Vec3f(v, 0, 0).elems());
		intensities.add(Vec3f(20, 12, 16), Vec3f(0, v, 0).elems());
		intensities.add(Vec3f(16, 16, 12), Vec3f(0, 0, v).elems());

		
		// run a fluid step:
		fluid.update();
		
		// diffuse the intensities:
		intensities.diffuse();
		
		// use the fluid to advect the intensities:
		intensities.advect(fluid.velocities.front(), 3.);
		
		// some decay
		intensities.scale(0.999);
		
		// update texture data:
		intensityTex.submit(intensities.front());
		velocityTex.submit(fluid.velocities.front());
		
		// draw it:
		gl.blending(true);
		gl.blendModeAdd();
		gl.lineWidth(0.5);
		
		gl.polygonMode(gl.LINE);
		
		shaderP.begin();
		shaderP.uniform("velocityTex", 0);
		shaderP.uniform("intensityTex", 1);
		velocityTex.bind(0);
		intensityTex.bind(1);
		gl.draw(mesh);
		intensityTex.unbind(1);
		velocityTex.unbind(0);
		shaderP.end();

		return true;
	}
};


MyWindow win;

struct MRCHeader {
	int32_t   nx;         /*  # of Columns                  */
	int32_t   ny;         /*  # of Rows                     */
	int32_t   nz;         /*  # of Sections.                */
	int32_t   mode;       /*  given by #define MRC_MODE...  */

	int32_t   nxstart;    /*  Starting point of sub image.  */
	int32_t   nystart;
	int32_t   nzstart;

	int32_t   mx;         /* Number of rows to read.        */
	int32_t   my;
	int32_t   mz;

	float   xlen;       /* length of x element in um.     */
	float   ylen;       /* get scale = xlen/nx ...        */
	float   zlen;

	float   alpha;      /* cell angles, ignore */
	float   beta;
	float   gamma;

	int32_t   mapc;       /* map coloumn 1=x,2=y,3=z.       */
	int32_t   mapr;       /* map row     1=x,2=y,3=z.       */
	int32_t   maps;       /* map section 1=x,2=y,3=z.       */

	float   amin;
	float   amax;
	float   amean;

	int16_t   ispg;       /* image type */
	int16_t   nsymbt;     /* space group number */

	/* IMOD-SPECIFIC */
	int32_t   next;
	int16_t   creatid;  /* Used to be creator id, hvem = 1000, now 0 */
	char    blank[30];
	int16_t   nint;
	int16_t   nreal;
	int16_t   sub;
	int16_t   zfac;
	float   min2;
	float   max2;
	float   min3;
	float   max3;
	int32_t   imodStamp;
	int32_t   imodFlags;
	int16_t   idtype;
	int16_t   lens;
	int16_t   nd1;     /* Devide by 100 to get float value. */
	int16_t   nd2;
	int16_t   vd1;
	int16_t   vd2;
	float   tiltangles[6];  /* 0,1,2 = original:  3,4,5 = current */

	/* MRC 2000 standard */
	float   origin[3];
	char    cmap[4];
	char    machinestamp[4];
	float   rms;

	int32_t nlabl;	// number of labels
	char  labels[10][80];
} MrcHeader;

void mrc_swap_header(MRCHeader * header) {
	byteswap(&header->nx, 10);
	byteswap(&header->xlen, 6);
	byteswap(&header->mapc, 3);
	byteswap(&header->amin, 3);
	byteswap(&header->ispg, 2);
	byteswap(&header->next, 1);
	byteswap(&header->creatid, 1);
	byteswap(&header->nint, 4);
	byteswap(&header->min2, 4);
	byteswap(&header->imodStamp, 2);
	byteswap(&header->idtype, 6);
	byteswap(&header->tiltangles[0], 6);
	byteswap(&header->origin[0], 3);
	byteswap(&header->rms, 1);
	byteswap(&header->nlabl, 1);
}

void mrcParse(const char * data) {
	MRCHeader& header = *(MRCHeader *)data;
	
	// check for byte swap:
	bool swapped = 
		(header.nx <= 0 || header.ny <= 0 || header.nz <= 0 || 
		(header.nx > 65535 && header.ny > 65535 && header.nz > 65535) ||
		header.mapc < 0 || header.mapc > 4 ||
		header.mapr < 0 || header.mapr > 4 ||
		header.maps < 0 || header.maps > 4);
		
	// ugh.
	if (swapped) {
		byteswap(&header.nx, 10);
		byteswap(&header.xlen, 6);
		byteswap(&header.mapc, 3);
		byteswap(&header.amin, 3);
		byteswap(&header.ispg, 2);
		byteswap(&header.next, 1);
		byteswap(&header.creatid, 1);
		byteswap(&header.nint, 4);
		byteswap(&header.min2, 4);
		byteswap(&header.imodStamp, 2);
		byteswap(&header.idtype, 6);
		byteswap(&header.tiltangles[0], 6);
		byteswap(&header.origin[0], 3);
		byteswap(&header.rms, 1);
		byteswap(&header.nlabl, 1);
	}
	
	printf("swapped %d NX %d NY %d NZ %d\n", swapped, header.nx, header.ny, header.nz);
}

int main(){

	SearchPaths paths;
	paths.addAppPaths();
	paths.addSearchPath(paths.appPath() + "../../", true);
	paths.print();
	std::string mrcpath = paths.find("golgi.mrc").filepath();
	printf("golgi: %s\n", mrcpath.c_str());
	File f(mrcpath, "rb", true);
	const char * data = f.readAll();
	mrcParse(data);
	
	printf("char %d\n", sizeof(char));

	win.append(*new StandardWindowKeyControls);
	win.create(Window::Dim(640, 480));

	//MainLoop::start();
	return 0;
}

