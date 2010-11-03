#ifndef INCLUDE_AL_GRAPHICS_HPP
#define INCLUDE_AL_GRAPHICS_HPP

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

#include "allocore/math/al_Vec.hpp"
#include "allocore/math/al_Matrix4.hpp"
#include "allocore/types/al_Buffer.hpp"
#include "allocore/types/al_Color.hpp"

#include <stack>
using std::stack;

namespace al{
namespace gfx{

enum AntiAliasMode {
	DONT_CARE,
	FASTEST,
	NICEST
};

enum AttributeBit {
	COLOR_BUFFER_BIT	= 1<<0,		/**< Color-buffer bit */
	DEPTH_BUFFER_BIT	= 1<<1,		/**< Depth-buffer bit */
	ENABLE_BIT			= 1<<2,		/**< Enable bit */
	VIEWPORT_BIT		= 1<<3		/**< Viewport bit */
};

enum BlendFunc {
	SRC_ALPHA = 0,
	SRC_COLOR,
	DST_ALPHA,
	DST_COLOR,
	ZERO,
	ONE,
	SRC_ALPHA_SATURATE
};

enum MatrixMode {
	MODELVIEW = 0,
	PROJECTION
};

enum PolygonMode {
	POINT = 0,
	LINE,
	FILL
};

enum Primitive {
	POINTS = 0,
	LINES,
	LINE_STRIP,
	LINE_LOOP,
	TRIANGLES,
	TRIANGLE_STRIP,
	TRIANGLE_FAN,
	QUADS,
	QUAD_STRIP,
	POLYGON
};



struct State {
	State()
	:	blend_enable(false),
		blend_src(SRC_COLOR),
		blend_dst(DST_COLOR),
		lighting_enable(false),
		depth_enable(true),
		polygon_mode(FILL),
		antialias_mode(DONT_CARE)
	{}

	~State() {}


	// Blending
	bool blend_enable;
	BlendFunc blend_src;
	BlendFunc blend_dst;

	// Lighting
	bool lighting_enable;

	// Depth Testing
	bool depth_enable;

	// Polygon Mode
	PolygonMode polygon_mode;

	// Anti-Aliasing
	AntiAliasMode antialias_mode;

};


struct StateChange {
	StateChange()
	:	blending(true),
		lighting(true),
		depth_testing(true),
		polygon_mode(true),
		antialiasing(true)
	{}

	~StateChange(){}

	bool blending;
	bool lighting;
	bool depth_testing;
	bool polygon_mode;
	bool antialiasing;
};


/// Stores buffers related to rendering graphical objects
class GraphicsData {
public:

	typedef Vec3f			Vertex;
	typedef Vec3f			Normal;
	typedef Vec4f			Color;
	typedef Vec2f			TexCoord2;
	typedef Vec3f			TexCoord3;
	typedef unsigned int	Index;


	GraphicsData(): mPrimitive(gfx::POINTS){}

	/// Reset all buffers
	void resetBuffers();
	void equalizeBuffers();
	void getBounds(Vec3f& min, Vec3f& max);
	Vec3f getCenter(); // center at 0,0,0

	// destructive edits to internal vertices:
	void unitize();	/// scale to -1..1
	void scale(double x, double y, double z);
	void scale(Vec3f v) { scale(v[0], v[1], v[2]); }
	void scale(double s) { scale(s, s, s); }
	void translate(double x, double y, double z);
	void translate(Vec3f v) { translate(v[0], v[1], v[2]); }

	// generates smoothed normals for a set of vertices
	// will replace any normals currently in use
	// angle - maximum angle (in degrees) to smooth across
	void generateNormals(float angle);

	Primitive primitive() const { return mPrimitive; }
	const Buffer<Vertex>& vertices() const { return mVertices; }
	const Buffer<Normal>& normals() const { return mNormals; }
	const Buffer<Color>& colors() const { return mColors; }
	const Buffer<TexCoord2>& texCoord2s() const { return mTexCoord2s; }
	const Buffer<TexCoord3>& texCoord3s() const { return mTexCoord3s; }
	const Buffer<Index>& indices() const { return mIndices; }

	void addIndex(unsigned int i){ indices().append(i); }

	void addColor(float r, float g, float b, float a=1){ addColor(Color(r,g,b,a)); }
	void addColor(const Color& v) { colors().append(v); }
	void addColor(const al::Color& v) { addColor(v.r, v.g, v.b, v.a); }

	void addNormal(float x, float y, float z=0){ addNormal(Normal(x,y,z)); }
	void addNormal(const Normal& v) { normals().append(v); }

	void addTexCoord(float u, float v){ addTexCoord(TexCoord2(u,v)); }
	void addTexCoord(const TexCoord2& v){ texCoord2s().append(v); }
	
	void addTexCoord(float u, float v, float w){ addTexCoord(TexCoord3(u,v,w)); }
	void addTexCoord(const TexCoord3& v){ texCoord3s().append(v); }

	void addVertex(float x, float y, float z=0){ addVertex(Vertex(x,y,z)); }
	void addVertex(const Vertex& v){ vertices().append(v); }

	void primitive(Primitive prim){ mPrimitive=prim; }

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

	Primitive mPrimitive;
};



class GraphicsBackend;

/// Interface for setting graphics state and rendering GraphicsData

///	It also owns a GraphicsData, to simulate immediate mode (where it draws its own data)
///
class Graphics {
public:

//	Graphics(gfx::Backend::type backend = gfx::Backend::AutoDetect);
	Graphics(GraphicsBackend *backend);
	~Graphics();

	// Rendering State
	void blending(bool enable, BlendFunc src, BlendFunc dst);
	void depthTesting(bool enable);
	void lighting(bool enable);
	void pushState(State &state);
	void popState();

	// Frame
	void clear(int attribMask);
	void clearColor(float r, float g, float b, float a);
	void clearColor(const Color& color) { clearColor(color.r, color.g, color.b, color.a); }

	// Coordinate Transforms
	void viewport(int x, int y, int width, int height);
	void matrixMode(MatrixMode mode);
	MatrixMode matrixMode();
	void pushMatrix();
	void popMatrix();
	void loadIdentity();
	void loadMatrix(const Matrix4d &m);
	void multMatrix(const Matrix4d &m);
	void translate(double x, double y, double z);
	void translate(const Vec3d& v) { translate(v[0], v[1], v[2]); }
	void translate(const Vec3f& v) { translate(v[0], v[1], v[2]); }
	void rotate(double angle, double x, double y, double z);
	void rotate(double angle, const Vec3d& v) { rotate(angle, v[0], v[1], v[2]); }
	void scale(double x, double y, double z);
	void scale(double s) { scale(s, s, s); }
	void scale(const Vec3d& v) { scale(v[0], v[1], v[2]); }
	void scale(const Vec3f& v) { scale(v[0], v[1], v[2]); }

	// Immediate Mode

	/// Begin "immediate" mode drawing
	void begin(Primitive mode);
	
	/// End "immediate" mode
	void end();

	void vertex(double x, double y, double z=0.);
	void vertex(const Vec3d& v) { vertex(v[0], v[1], v[2]); }
	void vertex(const Vec3f& v) { vertex(v[0], v[1], v[2]); }
	void texcoord(double u, double v);
	void normal(double x, double y, double z=0.);
	void normal(const Vec3d& v) { normal(v[0], v[1], v[2]); }
	void normal(const Vec3f& v) { normal(v[0], v[1], v[2]); }
	void color(double r, double g, double b, double a=1.);
	void color(const Color& v){ color(v.r, v.g, v.b, v.a); }
	void color(const Vec3d& v, double a=1.) { color(v[0], v[1], v[2], a); }
	void color(const Vec3f& v, double a=1.) { color(v[0], v[1], v[2], a); }

	// Other state
	void antialiasing(AntiAliasMode v);
	void lineWidth(double v);
	void pointSize(double v);

	// Buffer drawing

	/// Render data in graphic buffers
	void draw(const GraphicsData& v);

	/// Render data in internal graphic buffers
	void draw();

	/// Get implementation backend
	GraphicsBackend * backend(){ return mBackend; }
	
	/// Get internal graphic buffers
	GraphicsData& data(){ return mGraphicsData; }

	void drawBegin();
	void drawEnd();

protected:
	GraphicsBackend	*	mBackend;			// graphics API implementation
	GraphicsData		mGraphicsData;		// used for immediate mode style rendering
	bool				mInImmediateMode;	// flag for whether or not in immediate mode
	MatrixMode			mMatrixMode;		// matrix stack to use
	stack<Matrix4d>		mProjectionMatrix;	// projection matrix stack
	stack<Matrix4d>		mModelViewMatrix;	// modelview matrix stack
	StateChange			mStateChange;		// state difference to mark changes
	stack<State>		mState;				// state stack

	void compareState(const State &prev_state, const State &state);
	void enableState();
	stack<Matrix4d>& matrixStackForMode(MatrixMode mode);
};

/*
	Abstract base class for any object that can be rendered via Graphics:
*/
class Drawable {
public:
	virtual void onDraw(Graphics& gl) = 0;
	virtual ~Drawable(){}
};

} // ::al::gfx
} // ::al

#endif

