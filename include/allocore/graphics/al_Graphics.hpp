#ifndef INCLUDE_AL_GRAPHICS_HPP
#define INCLUDE_AL_GRAPHICS_HPP

/*	Allocore --
	Multimedia / virtual environment application class library
	
	Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology, UCSB.
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


	File description:
	A general high-level interface to graphics rendering

	File author(s):
	Wesley Smith, 2010, wesley.hoke@gmail.com
	Lance Putnam, 2010, putnam.lance@gmail.com
	Graham Wakefield, 2010, grrrwaaa@gmail.com
*/

#include <stdio.h>

#include "allocore/math/al_Matrix4.hpp"
#include "allocore/types/al_Array.hpp"
#include "allocore/types/al_Color.hpp"

#include "allocore/graphics/al_Mesh.hpp"
#include "allocore/graphics/al_OpenGL.hpp"

//#include "allocore/math/al_Vec.hpp"
//#include "allocore/math/al_Quat.hpp"
//#include "allocore/types/al_Buffer.hpp"

//#include "allocore/graphics/al_Texture.hpp"
//#include "allocore/graphics/al_Surface.hpp"


namespace al {

/// Interface for setting graphics state and rendering Mesh

///	It also owns a Mesh, to simulate immediate mode (where it draws its own data)
///
class Graphics {
public:

	enum AntiAliasMode {
		DONT_CARE				= GL_DONT_CARE,
		FASTEST					= GL_FASTEST,
		NICEST					= GL_NICEST
	};

	enum AttributeBit {
		COLOR_BUFFER_BIT		= GL_COLOR_BUFFER_BIT,		/**< Color-buffer bit */
		DEPTH_BUFFER_BIT		= GL_DEPTH_BUFFER_BIT,		/**< Depth-buffer bit */
		ENABLE_BIT				= GL_ENABLE_BIT,			/**< Enable bit */
		VIEWPORT_BIT			= GL_VIEWPORT_BIT			/**< Viewport bit */
	};

	enum BlendFunc {
		SRC_ALPHA				= GL_SRC_ALPHA,
		ONE_MINUS_SRC_ALPHA		= GL_ONE_MINUS_SRC_ALPHA,
		SRC_COLOR				= GL_SRC_COLOR,
		ONE_MINUS_SRC_COLOR		= GL_ONE_MINUS_SRC_COLOR,
		DST_ALPHA				= GL_DST_ALPHA,
		ONE_MINUS_DST_ALPHA		= GL_ONE_MINUS_DST_ALPHA,
		DST_COLOR				= GL_DST_COLOR,
		ONE_MINUS_DST_COLOR		= GL_ONE_MINUS_DST_COLOR,
		ZERO					= GL_ZERO,
		ONE						= GL_ONE,
		SRC_ALPHA_SATURATE		= GL_SRC_ALPHA_SATURATE
	};
	
	enum BlendEq {
		FUNC_ADD				= GL_FUNC_ADD,
		FUNC_SUBTRACT			= GL_FUNC_SUBTRACT,
		FUNC_REVERSE_SUBTRACT	= GL_FUNC_REVERSE_SUBTRACT, 
		MIN						= GL_MIN,
		MAX						= GL_MAX
	};

	enum Capability {
		BLEND					= GL_BLEND,
		COLOR_MATERIAL			= GL_COLOR_MATERIAL,
		DEPTH_TEST				= GL_DEPTH_TEST,
		FOG						= GL_FOG,
		LIGHTING				= GL_LIGHTING,
		SCISSOR_TEST			= GL_SCISSOR_TEST,
		CULL_FACE				= GL_CULL_FACE
	};

	enum DataType {
		BYTE					= GL_BYTE,
		UBYTE					= GL_UNSIGNED_BYTE,
		SHORT					= GL_SHORT,
		USHORT					= GL_UNSIGNED_SHORT,
		INT						= GL_INT,
		UINT					= GL_UNSIGNED_INT,
		FLOAT					= GL_FLOAT,
		DOUBLE					= GL_DOUBLE
	};

	enum Face {
		FRONT					= GL_FRONT,
		BACK					= GL_BACK,
		FRONT_AND_BACK			= GL_FRONT_AND_BACK
	};

	enum Format {
		DEPTH_COMPONENT			= GL_DEPTH_COMPONENT,
		LUMINANCE				= GL_LUMINANCE,
		LUMINANCE_ALPHA			= GL_LUMINANCE_ALPHA,
		RED						= GL_RED,
		GREEN					= GL_GREEN,
		BLUE					= GL_BLUE,
		ALPHA					= GL_ALPHA,
		RGB						= GL_RGB,
		RGBA					= GL_RGBA,
		BGRA					= GL_BGRA
	};

	enum MatrixMode {
		MODELVIEW				= GL_MODELVIEW,
		PROJECTION				= GL_PROJECTION
	};

	enum PolygonMode {
		POINT					= GL_POINT,
		LINE					= GL_LINE,
		FILL					= GL_FILL
	};

	enum Primitive {
		POINTS					= GL_POINTS,
		LINES					= GL_LINES,
		LINE_STRIP				= GL_LINE_STRIP,
		LINE_LOOP				= GL_LINE_LOOP,
		TRIANGLES				= GL_TRIANGLES,
		TRIANGLE_STRIP			= GL_TRIANGLE_STRIP,
		TRIANGLE_FAN			= GL_TRIANGLE_FAN,
		QUADS					= GL_QUADS,
		QUAD_STRIP				= GL_QUAD_STRIP,
		POLYGON					= GL_POLYGON
	};

	enum ShadeModel {
		FLAT					= GL_FLAT,
		SMOOTH					= GL_SMOOTH
	};


	
	Graphics();
	virtual ~Graphics();


	const Mesh& mesh() const { return mMesh; }
	Mesh& mesh(){ return mMesh; }


	// Capabilities
	void enable(Capability v){ glEnable(v); }
	void disable(Capability v){ glDisable(v); }
	void capability(Capability cap, bool value);
	void blending(bool b);	
	void depthMask(bool b);
	void depthTesting(bool b);
	void lighting(bool b);
	void scissor(bool b);
	void cullFace(bool b);
	void cullFace(bool b, Face face);


	// Other state
	void antialiasing(AntiAliasMode v);
	void lineWidth(float v);

	/// Set distance attenuation of points. The scaling formula is clamp(size * sqrt(1/(c0 + c1*d + c2*d^2)))
	void pointAtten(float c2=0, float c1=0, float c0=1);
	void pointSize(float v);
	void polygonMode(PolygonMode m, Face f=FRONT_AND_BACK);
	void shadeModel(ShadeModel m);
	void fog(float end, float start, const Color& c);

	/// Set blending mode
	void blendMode(BlendFunc src, BlendFunc dst, BlendEq eq=FUNC_ADD);

	/// Set blending mode to additive
	void blendModeAdd(){ blendMode(SRC_ALPHA, ONE, FUNC_ADD); }
	
	/// Set blending mode to transparent
	void blendModeTrans(){ blendMode(SRC_ALPHA, ONE_MINUS_SRC_ALPHA, FUNC_ADD); }
								
	void clear(AttributeBit bits);
	void clearColor(float r, float g, float b, float a);
	void clearColor(const Color& color) { clearColor(color.r, color.g, color.b, color.a); }


	// Coordinate Transforms
	void viewport(int x, int y, int width, int height);
	void matrixMode(MatrixMode mode);
	void pushMatrix();
	void popMatrix();
	void loadIdentity();
	void loadMatrix(const Matrix4d &m);
	void multMatrix(const Matrix4d &m);
	void modelView(const Matrix4d& m){ matrixMode(MODELVIEW); loadMatrix(m); }
	void projection(const Matrix4d& m){ matrixMode(PROJECTION); loadMatrix(m); }
	void translate(double x, double y, double z);
	void rotate(double angle, double x, double y, double z);
	void rotate(const Quatd& q);
	void scale(double x, double y, double z);
	
	void pushMatrix(MatrixMode v){ matrixMode(v); pushMatrix(); }
	void popMatrix(MatrixMode v){ matrixMode(v); popMatrix(); }
	void translate(const Vec3d& v) { translate(v[0], v[1], v[2]); }
	void translate(const Vec3f& v) { translate(v[0], v[1], v[2]); }
	void rotate(double angle, const Vec3d& v) { rotate(angle, v[0], v[1], v[2]); }
	void scale(double s) { scale(s, s, s); }
	void scale(const Vec3d& v) { scale(v[0], v[1], v[2]); }
	void scale(const Vec3f& v) { scale(v[0], v[1], v[2]); }

	// Immediate Mode

	/// Begin "immediate" mode drawing
	void begin(Primitive v);
	
	/// End "immediate" mode
	void end();

	void currentColor(float r, float g, float b, float a);

	void vertex(double x, double y, double z=0.);
	void vertex(const Vec2d& v) { vertex(v[0], v[1], 0); }
	void vertex(const Vec2f& v) { vertex(v[0], v[1], 0); }
	void vertex(const Vec3d& v) { vertex(v[0], v[1], v[2]); }
	void vertex(const Vec3f& v) { vertex(v[0], v[1], v[2]); }
	
	void texCoord(double u, double v);
	void texCoord(const Vec2d& v) { texCoord(v[0], v[1]); }
	void texCoord(const Vec2f& v) { texCoord(v[0], v[1]); }
	void texCoord(double u, double v, double w);
	void texCoord(const Vec3d& v) { texCoord(v[0], v[1], v[2]); }
	void texCoord(const Vec3f& v) { texCoord(v[0], v[1], v[2]); }
	
	void normal(double x, double y, double z=0.);
	void normal(const Vec3d& v) { normal(v[0], v[1], v[2]); }
	void normal(const Vec3f& v) { normal(v[0], v[1], v[2]); }
	
	void color(double r, double g, double b, double a=1.);
	void color(double gray, double a=1.) { color(gray, gray, gray, a); }
	void color(const Color& v){ color(v.r, v.g, v.b, v.a); }
	void color(const Vec3d& v, double a=1.) { color(v[0], v[1], v[2], a); }
	void color(const Vec3f& v, double a=1.) { color(v[0], v[1], v[2], a); }
	void color(const Vec4d& v) { color(v[0], v[1], v[2], v[3]); }
	void color(const Vec4f& v) { color(v[0], v[1], v[2], v[3]); }


	void draw(const Mesh& v);
	void draw(){ draw(mMesh); }


	// Utility functions: converting, reporting, etc.
	
	/// Returns number of components for given color type
	static int numComponents(Format v);
	
	/// Returns number of bytes for given data type
	static int numBytes(DataType v);

	/// Print current error state to file
	static bool error(const char *msg="", FILE * fp=stdout);
	
	/// Returns AlloTy type for a given GL data type:
	static AlloTy toAlloTy(DataType v);

	/// Returns DataType for a given AlloTy
	static DataType toDataType(AlloTy type);
	
protected:
	Mesh mMesh;				// used for immediate mode style rendering
	bool mInImmediateMode;	// flag for whether or not in immediate mode
};



///	Abstract base class for any object that can be rendered via Graphics
class Drawable {
public:
	virtual void onDraw(Graphics& gl) = 0;
	virtual ~Drawable(){}
};



// ============== INLINE ============== 

inline void Graphics::clear(AttributeBit bits){ glClear(bits); }
inline void Graphics::clearColor(float r, float g, float b, float a){ glClearColor(r, g, b, a); }

inline void Graphics::blendMode(BlendFunc src, BlendFunc dst, BlendEq eq){
	glBlendEquation(eq);
	glBlendFunc(src, dst);
}

inline void Graphics::capability(Capability cap, bool v){
	v ? enable(cap) : disable(cap);
}

inline void Graphics::blending(bool b){ capability(BLEND, b); }
inline void Graphics::depthMask(bool b){ glDepthMask(b?GL_TRUE:GL_FALSE); }
inline void Graphics::depthTesting(bool b){ capability(DEPTH_TEST, b); }
inline void Graphics::lighting(bool b){ capability(LIGHTING, b); }
inline void Graphics::scissor(bool b){ capability(SCISSOR_TEST, b); }
inline void Graphics::cullFace(bool b){ capability(CULL_FACE, b); }
inline void Graphics::cullFace(bool b, Face face) {
	capability(CULL_FACE, b);
	glCullFace(face);
}
inline void Graphics::matrixMode(MatrixMode mode){ glMatrixMode(mode); }
inline void Graphics::pushMatrix(){ glPushMatrix(); }
inline void Graphics::popMatrix(){ glPopMatrix(); }
inline void Graphics::loadIdentity(){ glLoadIdentity(); }
inline void Graphics::loadMatrix(const Matrix4d& m){ glLoadMatrixd(m.elems()); }
inline void Graphics::multMatrix(const Matrix4d& m){ glMultMatrixd(m.elems()); }
inline void Graphics::translate(double x, double y, double z){ glTranslated(x,y,z); }
inline void Graphics::rotate(double angle, double x, double y, double z){ glRotated(angle,x,y,z); }
inline void Graphics::scale(double x, double y, double z){ glScaled(x, y, z); }

inline void Graphics::lineWidth(float v) { glLineWidth(v); }
inline void Graphics::pointSize(float v) { glPointSize(v); }
inline void Graphics::pointAtten(float c2, float c1, float c0){
	GLfloat att[3] = {c0, c1, c2};
	glPointParameterfv(GL_POINT_DISTANCE_ATTENUATION, att);
}
inline void Graphics::polygonMode(PolygonMode m, Face f){ glPolygonMode(f,m); }
inline void Graphics::shadeModel(ShadeModel m){ glShadeModel(m); }
inline void Graphics::currentColor(float r, float g, float b, float a){ glColor4f(r,g,b,a); }

inline void Graphics::rotate(const Quatd& q) {
	Matrix4d m;
	q.toMatrix(m.elems());
	multMatrix(m);
}

inline Graphics::AttributeBit operator| (const Graphics::AttributeBit& a, const Graphics::AttributeBit& b){
	return static_cast<Graphics::AttributeBit>(+a|+b);
}

} // al::

#endif
