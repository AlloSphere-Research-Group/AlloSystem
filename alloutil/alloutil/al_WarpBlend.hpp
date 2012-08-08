#ifndef AL_WARPBLEND_H
#define AL_WARPBLEND_H

#include "allocore/graphics/al_Texture.hpp"
#include "allocore/graphics/al_Shader.hpp"
#include "allocore/spatial/al_Pose.hpp"

namespace al {

class WarpnBlend {
public:
	WarpnBlend();

	void readID(std::string id);

	void readBlend(std::string path);
	void readWarp(std::string path);
	void read3D(std::string path);
	void readModelView(std::string path);
	void readPerspective(std::string path, double near = 0.1, double far = 100);
	
	void rotate(const Matrix4d& r);
	
	void onCreate();
	
	void draw(Texture& scene);
	
	// debugging:
	void drawWarp();
	void drawWarp3D();
	void drawInverseWarp3D();
	void drawBlend();
	void drawDemo(const Pose& pose, double eyesep);
	
	ShaderProgram geomP;
	Shader geomV, geomF;
	ShaderProgram geomP3D;
	Shader geomV3D, geomF3D;
	ShaderProgram geomPI3D;
	Shader geomVI3D, geomFI3D;
	ShaderProgram warpP;
	Shader warpV, warpF;
	ShaderProgram demoP;
	Shader demoV, demoF;
	
	Texture geometryMap, alphaMap, pixelMap, inversePixelMap;
	Mesh pixelMesh;
	Matrix4d modelView, perspective;
	Pose center;
	
	std::string imgpath;
	
	bool loaded;
};
	
} // al::

#endif
