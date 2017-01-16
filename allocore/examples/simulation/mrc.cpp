/*
Allocore Example: MRC

Description:
This demonstrates loading MRC data

Author:
Graham Wakefield 2011
*/


#include "allocore/al_Allocore.hpp"
#include "allocore/graphics/al_Shader.hpp"
#include "allocore/graphics/al_Isosurface.hpp"

using namespace al;

// As a 3D Texture:
Texture tex(32, 32, 32, Graphics::LUMINANCE);

float amean;

// As an Isosurface:
Isosurface iso;

Graphics gl;
Mesh mesh;
ShaderProgram shaderP;
Shader shaderV, shaderF;

static const char * vField = R"(
varying vec3 texcoord0;
void main(){
	texcoord0 = gl_Vertex.xyz / 32.;
	//texcoord0 = vec3(gl_MultiTexCoord0);
	gl_Position = ftransform();
}
)";

static const char * fField = R"(
uniform sampler3D tex;
varying vec3 texcoord0;
void main() {
	float intensity = texture3D(tex, texcoord0).r;
	vec3 rgb = vec3(intensity, intensity, intensity);
	gl_FragColor = vec4(rgb, 0.95);
	//gl_FragColor = vec4(texcoord0, 1);
}
)";

struct MyWindow : public Window {

	bool onCreate(){

		// reconfigure textures based on arrays:
		tex.submit(tex.array(), true);


		// shader method:
		shaderV.source(vField, Shader::VERTEX).compile();
		shaderF.source(fField, Shader::FRAGMENT).compile();
		shaderP.attach(shaderV).attach(shaderF).link();
		shaderV.printLog();
		shaderF.printLog();
		shaderP.printLog();

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

		shaderP.begin();
		shaderP.uniform("tex", 0);
		tex.bind();
		//gl.draw(mesh);
		tex.unbind();
		shaderP.end();

		gl.polygonMode(gl.FILL);
		gl.color(1, 1, 1, 0.3);
		tex.bind();
		for (float slice = 0; slice <= 1.; slice += 0.1) {
			float s = 32.;
			gl.begin(gl.QUADS);
				gl.texCoord(0, 0, slice);
				gl.vertex(0, 0, slice*s);
				gl.texCoord(0, 1, slice);
				gl.vertex(0, s, slice*s);
				gl.texCoord(1, 1, slice);
				gl.vertex(s, s, slice*s);
				gl.texCoord(1, 0, slice);
				gl.vertex(s, 0, slice*s);
			gl.end();
		}
		tex.unbind();

		return true;
	}
};

MyWindow win;

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
	printf("startX %d startY %d startZ %d\n", header.nxstart, header.nystart, header.nzstart);
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

	SearchPaths paths;
	paths.addAppPaths();
	paths.addSearchPath(paths.appPath() + "../../", true);
	paths.print();
	std::string mrcpath = paths.find("golgi.mrc").filepath();
	//std::string mrcpath = paths.find("g-actin.mrc").filepath();
	//std::string mrcpath = paths.find("arp23_cf26.map").filepath();
	File f(mrcpath, "rb");

	if(!f.open()){
		AL_WARN("Cannot open MRC file.", mrcpath.c_str());
		exit(EXIT_FAILURE);
	}

	Array& array = tex.array();
	MRCHeader& header = mrcParse(f.readAll(), array);

	amean = header.amean;

	win.append(*new StandardWindowKeyControls);
	win.create(Window::Dim(640, 480));

	iso.primitive(Graphics::TRIANGLES);

	MainLoop::start();
}

