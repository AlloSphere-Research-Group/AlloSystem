#ifndef INCLUDE_AL_GRAPHICS_HPP
#define INCLUDE_AL_GRAPHICS_HPP

/*
 *	OSC (Open Sound Control) send/receive
 *  AlloSphere Research Group / Media Arts & Technology, UCSB, 2009
 */

/*
	Copyright (C) 2006-2008. The Regents of the University of California (REGENTS). 
	All Rights Reserved.

	Permission to use, copy, modify, distribute, and distribute modified versions
	of this software and its documentation without fee and without a signed
	licensing agreement, is hereby granted, provided that the above copyright
	notice, the list of contributors, this paragraph and the following two paragraphs 
	appear in all copies, modifications, and distributions.

	IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
	SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING
	OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF REGENTS HAS
	BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
	THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
	PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED
	HEREUNDER IS PROVIDED "AS IS". REGENTS HAS  NO OBLIGATION TO PROVIDE
	MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
*/

/*
	Example code:
	
	Graphics gl();
	
	gl.begin(gl.POINTS);
	gl.vertex3d(0, 0, 0);
	gl.end();
*/

#include "math/al_Vec.hpp"
#include "types/al_VectorBuffer.hpp"
#include "types/al_Color.hpp"

namespace al{

namespace GraphicsBackend{
	enum type{
		None = 0,
		AutoDetect,
		OpenGL,
		OpenGLES1,
		OpenGLES2
	};
}

class Graphics {
public:
	
	// TODO: LJP: should we use generic structs to avoid extra dependencies?
//	typedef float Vertex[3];
//	typedef float Normal[3];
//	typedef float Color[4];
//	typedef float TexCoord[2];
	
	// TODO: LJP: should these have their own separate buffers? We may not
	//		want to assume every drawArrays is going to send colors, normals,
	//		tex coords, etc.
	struct VertexData {
		al::Vec3f position;
		al::Vec3f normal;
		al::Color color;
		//al::Vec4f color;
		float u, v;
	};

	Graphics(GraphicsBackend::type backend = GraphicsBackend::AutoDetect);
	~Graphics();

	void begin(int mode) { s_begin(this, mode); }
	void end() { s_end(this); }
	
	void vertex(double x, double y, double z=0.) { s_vertex(this, x, y, z); }
	void normal(double x, double y, double z=0.) { s_normal(this, x, y, z); }
	void color(double r, double g, double b, double a=1.) { s_color(this, r, g, b, a); }
	
	bool setBackend(GraphicsBackend::type backend);
	
	GraphicsBackend::type mBackend;
	int POINTS, LINES, LINE_LOOP, LINE_STRIP, TRIANGLES, TRIANGLE_STRIP, TRIANGLE_FAN, QUADS, QUAD_STRIP, POLYGON;
	
	// TODO: LJP: implement support for indexed buffers
	//al::VectorBuffer<int> mVertexIndices;
	
	// TODO: LJP: separate buffers for vertex data
	//		Only send those buffers that are populated.
//	al::VectorBuffer<Vertex> mVertices;
//	al::VectorBuffer<Normal> mNormals;
//	al::VectorBuffer<Color> mColors;
//	al::VectorBuffer<TexCoord> mTexCoords;
	
	al::VectorBuffer<VertexData> mVertexBuffer;
	int mMode;
	
	void (*s_begin)(Graphics * g, int mode);
	void (*s_end)(Graphics * g);
	void (*s_vertex)(Graphics * g, double x, double y, double z);
	void (*s_normal)(Graphics * g, double x, double y, double z);
	void (*s_color)(Graphics * g, double x, double y, double z, double a);
};

extern bool setBackendNone(Graphics * g);
extern bool setBackendOpenGL(Graphics * g);
extern bool setBackendOpenGLES(Graphics * g);
extern bool setBackendOpenGLES1(Graphics * g);
extern bool setBackendOpenGLES2(Graphics * g);

} // al::
	
#endif
	
