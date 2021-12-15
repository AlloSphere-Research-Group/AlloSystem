/*
Allocore Example: MRC

Description:
This demonstrates loading MRC data

Author:
Graham Wakefield 2011
*/


#include "allocore/io/al_Window.hpp"
#include "allocore/io/al_File.hpp"
#include "allocore/system/al_MainLoop.hpp"
#include "allocore/types/al_Conversion.hpp" // swapBytes
#include "allocore/graphics/al_Graphics.hpp"
#include "allocore/graphics/al_Isosurface.hpp"
#include "allocore/graphics/al_Mesh.hpp"
#include "allocore/graphics/al_Shader.hpp"
#include "allocore/graphics/al_Texture.hpp"
#include "allocore/system/al_Time.h"

using namespace al;

static const char * vField = AL_STRINGIFY(
varying vec3 texcoord0;
void main(){
	texcoord0 = gl_Vertex.xyz / 32.;
	//texcoord0 = vec3(gl_MultiTexCoord0);
	gl_Position = ftransform();
}
);

static const char * fField = AL_STRINGIFY(
uniform sampler3D tex;
varying vec3 texcoord0;
void main() {
	float intensity = texture3D(tex, texcoord0).r;
	vec3 rgb = vec3(intensity, intensity, intensity);
	gl_FragColor = vec4(rgb, 0.95);
	//gl_FragColor = vec4(texcoord0, 1);
}
);

struct MyWindow : public Window {

	// As a 3D Texture:
	Texture tex{32, 32, 32, Graphics::LUMINANCE};

	// As an Isosurface:
	Isosurface iso;

	Graphics gl;
	Mesh mesh;
	ShaderProgram shader;

	float amean;

	bool onCreate(){
		// reconfigure textures based on arrays:
		tex.submit(tex.array(), true);

		// shader method:
		shader.compile(vField, fField);

		// create rendering mesh:
		mesh.reset();
		mesh.primitive(Graphics::POINTS);
		for (int x=0; x<32; x++) {
		for (int y=0; y<32; y++) {
		for (int z=0; z<32; z++) {
			mesh.texCoord(x/32., y/32., z/32.);
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

		gl.lineWidth(0.5);
		gl.color(1, 1, 1, 0.02);
		gl.pointSize(1);

		gl.polygonMode(gl.LINE);


		iso.level(amean + 4.*sin(fmod(al_time() * 0.125, 1.) * M_2PI));
		iso.generate((char *)tex.array().data.ptr, 32, 32, 32, 1./32, 1./32, 1./32);

		gl.pushMatrix();
			gl.scale(32, 32, 32);
			//gl.draw(iso);
		gl.popMatrix();

		// draw it:
		gl.blending(true);
		gl.blendModeAdd();

		shader.begin();
		shader.uniform("tex", 0);
		tex.bind();
		//gl.draw(mesh);
		tex.unbind();
		shader.end();

		gl.polygonMode(gl.FILL);
		gl.color(1, 1, 1, 0.3);
		tex.bind();
		for (float slice = 0; slice <= 1.; slice += 0.1) {
			float s = 32.;
			gl.begin(gl.TRIANGLE_STRIP);
				gl.texCoord(0, 0, slice); // BL
				gl.vertex(0, 0, slice*s);
				gl.texCoord(0, 1, slice); // BR
				gl.vertex(0, s, slice*s);
				gl.texCoord(1, 0, slice); // TL
				gl.vertex(s, 0, slice*s);
				gl.texCoord(1, 1, slice); // TR
				gl.vertex(s, s, slice*s);
			gl.end();
		}
		tex.unbind();

		return true;
	}
};

enum MRCMode {
	MRC_IMAGE_SINT8 = 0,		//image : signed 8-bit bytes range -128 to 127
	MRC_IMAGE_SINT16 = 1,		//image : 16-bit halfwords
	MRC_IMAGE_FLOAT32 = 2,		//image : 32-bit reals
	MRC_TRANSFORM_INT16 = 3,	//transform : complex 16-bit integers
	MRC_TRANSFORM_FLOAT32 = 4,  //transform : complex 32-bit reals
	MRC_IMAGE_UINT16 = 6        //image : unsigned 16-bit range 0 to 65535
};

struct MRCHeader {
	// @see http://ami.scripps.edu/software/mrctools/mrc_specification.php
	// or http://bio3d.colorado.edu/imod/doc/mrc_format.txt

	int32_t   nx;         /*  # of Columns                  */
	int32_t   ny;         /*  # of Rows                     */
	int32_t   nz;         /*  # of Sections.                */
	int32_t   mode;       /*  given by #define MRC_MODE...  */

	int32_t   startx;    /*  Starting point of sub image.  */
	int32_t   starty;
	int32_t   startz;

	int32_t   mx;         /* Number of rows to read.        */
	int32_t   my;
	int32_t   mz;

	float_t   xlen;       /* length of x element in um.     */
	float_t   ylen;       /* get scale = xlen/nx ...        */
	float_t   zlen;

	float_t   alpha;      /* cell angles, ignore */
	float_t   beta;
	float_t   gamma;

	int32_t   mapx;       /* map coloumn 1=x,2=y,3=z.       */
	int32_t   mapy;       /* map row     1=x,2=y,3=z.       */
	int32_t   mapz;       /* map section 1=x,2=y,3=z.       */

	float_t   amin;
	float_t   amax;
	float_t   amean;

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
	float_t   min2;
	float_t   max2;
	float_t   min3;
	float_t   max3;
	int32_t   imodStamp;
	int32_t   imodFlags;
	int16_t   idtype;
	int16_t   lens;
	int16_t   nd1;     /* Devide by 100 to get float value. */
	int16_t   nd2;
	int16_t   vd1;
	int16_t   vd2;
	float_t   tiltangles[6];  /* 0,1,2 = original:  3,4,5 = current */

	/* MRC 2000 standard */
	float_t   origin[3];
	char    cmap[4];
	char    machinestamp[4];
	float_t   rms;

	int32_t nlabl;	// number of labels
	char  labels[10][80];
} MrcHeader;



MRCHeader& mrcParse(const char * data, Array& array) {
	MRCHeader& header = *(MRCHeader *)data;

	// check for byte swap:
	bool swapped =
		(header.nx <= 0 || header.ny <= 0 || header.nz <= 0 ||
		(header.nx > 65535 && header.ny > 65535 && header.nz > 65535) ||
		header.mapx < 0 || header.mapx > 4 ||
		header.mapy < 0 || header.mapy > 4 ||
		header.mapz < 0 || header.mapz > 4);

	// ugh.
	if (swapped) {
		printf("swapping byte order...\n");
		swapBytes(&header.nx, 10);
		swapBytes(&header.xlen, 6);
		swapBytes(&header.mapx, 3);
		swapBytes(&header.amin, 3);
		swapBytes(&header.ispg, 2);
		swapBytes(&header.next, 1);
		swapBytes(&header.creatid, 1);
		swapBytes(&header.nint, 4);
		swapBytes(&header.min2, 4);
		swapBytes(&header.imodStamp, 2);
		swapBytes(&header.idtype, 6);
		swapBytes(&header.tiltangles[0], 6);
		swapBytes(&header.origin[0], 3);
		swapBytes(&header.rms, 1);
		swapBytes(&header.nlabl, 1);
	}

	printf("NX %d NY %d NZ %d\n", header.nx, header.ny, header.nz);
	printf("mode %d\n", header.mode);
	printf("startX %d startY %d startZ %d\n", header.startx, header.starty, header.startz);
	printf("intervals X %d intervals Y %d intervals Z %d\n", header.mx, header.my, header.mz);
	printf("angstroms X %f angstroms Y %f angstroms Z %f\n", header.xlen, header.ylen, header.zlen);
	printf("axis X %d axis Y %d axis Z %d\n", header.mapx, header.mapy, header.mapz);
	printf("density min %f max %f mean %f\n", header.amin, header.amax, header.amean);
	printf("origin %f %f %f\n", header.origin[0], header.origin[1], header.origin[2]);
	printf("map %s\n", header.cmap);
	printf("machine stamp %s\n", header.machinestamp);
	printf("rms %f\n", header.rms);
	printf("labels %d\n", header.nlabl);
	for (int i=0; i<header.nlabl; i++) {
		//printf("\t%02d: %s\n", i, header.labels[i]);
	}

	const char * start = data + 1024;

	AlloTy ty;

	// set type:
	switch (header.mode) {
		case MRC_IMAGE_SINT8:
			printf("signed int 8\n");
			ty = Array::type<int8_t>();
			break;
		case MRC_IMAGE_SINT16:
			printf("signed int 16\n");
			ty = Array::type<int16_t>();
			break;
		case MRC_IMAGE_FLOAT32:
			printf("float\n");
			ty = Array::type<float_t>();
			break;
		case MRC_IMAGE_UINT16:
			printf("unsigned int 16\n");
			ty = Array::type<uint16_t>();
			break;
		default:
			printf("MRC mode not supported\n");
			break;
	}

	array.formatAligned(1, ty, header.nx, header.ny, header.nz, 0);
	memcpy(array.data.ptr, start, array.size());

	if (swapped) {
		// set type:
		switch (header.mode) {
			case MRC_IMAGE_SINT8:
				swapBytes((int8_t *)array.data.ptr, array.cells());
				break;
			case MRC_IMAGE_SINT16:
				swapBytes((int16_t *)array.data.ptr, array.cells());
				break;
			case MRC_IMAGE_FLOAT32:
				swapBytes((float_t *)array.data.ptr, array.cells());
				break;
			case MRC_IMAGE_UINT16:
				swapBytes((uint16_t *)array.data.ptr, array.cells());
				break;
			default:
				break;
		}
	}

	return header;
}

int main(){

	std::string dataDir = "allocore/share/imod_data/";
	if(!File::searchBack(dataDir)){
		printf("Error: Failed to find data directory\n");
		exit(-1);
	}

	File f(dataDir + "golgi.mrc", "rb");

	if(!f.open()){
		printf("Error: Failed to open data file\n");
		exit(-1);
	}

	MyWindow win;

	Array& array = win.tex.array();
	MRCHeader& header = mrcParse(f.readAll(), array);

	win.amean = header.amean;

	win.append(*new StandardWindowKeyControls);
	win.create(Window::Dim(640, 480));

	MainLoop::start();
}

