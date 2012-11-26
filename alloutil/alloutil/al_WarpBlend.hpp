#ifndef AL_WARPBLEND_H
#define AL_WARPBLEND_H

#include "allocore/graphics/al_Texture.hpp"
#include "allocore/graphics/al_Shader.hpp"
#include "allocore/spatial/al_Pose.hpp"

namespace al {

class WarpnBlend {
public:
	struct Projector {
		float projnum;			// ID of the projector
		float width, height;	// width/height in pixels
		Vec3f projector_position, sphere_center, screen_center, normal_unit, x_vec, y_vec;
		
		// calculated on init
		float screen_radius;
		Vec3f screen_center_unit;
		
		Vec3f x_unit;
		Vec3f y_unit;
		float x_pixel, y_pixel;
		float x_offset, y_offset;
		
		void init();	// calculate additional members
		void print();	// debug printout
	};

	WarpnBlend();

	void readNone();
	void readID(std::string id);

	void readBlend(std::string path);
	void readWarp(std::string path);
	void read3D(std::string path);
	void readModelView(std::string path);
	void readPerspective(std::string path, double near = 0.1, double far = 100);
	void readProj(std::string path);
	
	void rotate(const Matrix4d& r);
	
	void onCreate();
	
	void draw(Texture& scene);
	
	// debugging:
	void drawWarp();
	void drawWarp3D();
	//void drawInverseWarp3D();
	void drawBlend();
	void drawDemo(const Pose& pose, double eyesep);
	
	void drawPreDistortDemo(const Pose& pose, float aspect, double uvscalar, bool blend=true);

	
	Projector projector;
	Texture geometryMap, alphaMap, pixelMap; //, inversePixelMap;
	Mesh pixelMesh;
	Matrix4d modelView, perspective;
	Pose center;
	
	Mesh testscene;
	
	ShaderProgram geomP;
	Shader geomV, geomF;
	ShaderProgram geomP3D;
	Shader geomV3D, geomF3D;
	ShaderProgram geomPI3D;
	Shader geomVI3D, geomFI3D;
	ShaderProgram warpP;
	Shader warpV, warpF;
	ShaderProgram predistortP;
	Shader predistortV, predistortF;
	ShaderProgram demoP;
	Shader demoV, demoF;
	Shader alphaV, alphaF; 
	ShaderProgram alphaP;
	std::string imgpath;
	bool loaded;
};
	
} // al::

#endif
