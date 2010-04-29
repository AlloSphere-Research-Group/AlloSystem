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
#include "types/al_Buffer.hpp"
#include "types/al_VectorBuffer.hpp"
#include "types/al_Color.hpp"

namespace al{
namespace gfx{


namespace Backend{
	enum type{
		None = 0,
		AutoDetect,
		OpenGL,
		OpenGLES1,
		OpenGLES2
	};
}



template <int N, class T>
struct ArrayN{
	ArrayN(const T& v=T()){ for(int i=0; i<N; ++i) mElems[i]=v; }
	ArrayN(const T& a, const T& b){ mElems[0]=a; mElems[1]=b; }
	ArrayN(const T& a, const T& b, const T& c){ mElems[0]=a; mElems[1]=b; mElems[2]=c; }
	ArrayN(const T& a, const T& b, const T& c, const T& d){ mElems[0]=a; mElems[1]=b; mElems[2]=c; mElems[3]=d; }
	T& operator[](int i){ return mElems[i]; }
	const T& operator[](int i) const { return mElems[i]; }
	T mElems[N];
};


class GraphicsData{
public:

	typedef ArrayN<3,float>	Vertex;
	typedef ArrayN<3,float>	Normal;
	typedef ArrayN<4,float>	Color;
	typedef ArrayN<2,float>	TexCoord2;
	typedef ArrayN<3,float>	TexCoord3;
	typedef unsigned int	Index;

	/// Reset all buffers
	void resetBuffers();

	int primitive() const { return mPrimitive; } 
	const Buffer<Vertex>& vertices() const { return mVertices; }
	const Buffer<Normal>& normals() const { return mNormals; }
	const Buffer<Color>& colors() const { return mColors; }
	const Buffer<TexCoord2>& texCoord2s() const { return mTexCoord2s; }
	const Buffer<TexCoord3>& texCoord3s() const { return mTexCoord3s; }
	const Buffer<Index>& indices() const { return mIndices; }

	void addIndex(unsigned int i){ indices().append(i); }
	void addColor(float r, float g, float b, float a=1){ colors().append(Color(r,g,b,a)); }
	void addNormal(float x, float y, float z=0){ normals().append(Normal(x,y,z)); }
	void addTexCoord(float u, float v){ texCoord2s().append(TexCoord2(u,v)); }
	void addTexCoord(float u, float v, float w){ texCoord3s().append(TexCoord3(u,v,w)); }
	void addVertex(float x, float y, float z=0){ vertices().append(Vertex(x,y,z)); }
	void primitive(int prim){ mPrimitive=prim; }

	Buffer<Vertex>& vertices(){ return mVertices; }
	Buffer<Normal>& normals(){ return mNormals; }
	Buffer<Color>& colors(){ return mColors; }
	Buffer<TexCoord2>& texCoord2s(){ return mTexCoord2s; }
	Buffer<TexCoord3>& texCoord3s(){ return mTexCoord3s; }
	Buffer<Index>& indices(){ return mIndices; }

protected:

	// Only populated (size>0) buffers will be used
	Buffer<Vertex> mVertices;
	Buffer<Normal> mNormals;
	Buffer<Color> mColors;
	Buffer<TexCoord2> mTexCoord2s;
	Buffer<TexCoord3> mTexCoord3s;
	
	Buffer<Index> mIndices;

	int mPrimitive;
};



class Graphics : public GraphicsData {
public:

//	struct VertexData {
//		al::Vec3f position;
//		al::Vec3f normal;
//		al::Color color;
//		//al::Vec4f color;
//		float u, v;
//	};

	Graphics(gfx::Backend::type backend = gfx::Backend::AutoDetect);
	~Graphics();

	void clear(int attribMask){ s_clear(attribMask); }
	void clearColor(float r, float g, float b, float a){ s_clearColor(r,g,b,a); }
	void loadIdentity(){ s_loadIdentity(); }
	void draw(const GraphicsData& v){ s_draw(v); }
	void draw(){ draw(*this); }
	void viewport(int x, int y, int width, int height){ s_viewport(x,y,width,height); }

	void begin(int mode) { s_begin(this, mode); }
	void end() { s_end(this); }
	
	void vertex(double x, double y, double z=0.) { s_vertex(this, x, y, z); }
	void normal(double x, double y, double z=0.) { s_normal(this, x, y, z); }
	void color(double r, double g, double b, double a=1.) { s_color(this, r, g, b, a); }
	
	bool setBackend(gfx::Backend::type backend);
	
	gfx::Backend::type mBackend;
	int POINTS, LINES, LINE_LOOP, LINE_STRIP, TRIANGLES, TRIANGLE_STRIP, TRIANGLE_FAN, QUADS, QUAD_STRIP, POLYGON;
	int COLOR_BUFFER_BIT, DEPTH_BUFFER_BIT;

//	al::VectorBuffer<VertexData> mVertexBuffer;
//	int mMode;

	
	void (*s_draw)(const GraphicsData& v);
	void (*s_clear)(int mask);
	void (*s_clearColor)(float r, float g, float b, float a);
	void (*s_loadIdentity)();
	void (*s_viewport)(int x, int y, int w, int h);
	
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


} // ::al::gfx
} // ::al
	
#endif
	
