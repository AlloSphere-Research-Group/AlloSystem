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
#include "allocore/math/al_Quat.hpp"
#include "allocore/types/al_Buffer.hpp"
#include "allocore/types/al_Color.hpp"

#include "allocore/graphics/al_Mesh.hpp"
#include "allocore/graphics/al_Texture.hpp"
#include "allocore/graphics/al_Surface.hpp"

namespace al {

/// Interface for setting graphics state and rendering Mesh

///	It also owns a Mesh, to simulate immediate mode (where it draws its own data)
///
class Graphics {
public:

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
		ONE_MINUS_SRC_ALPHA,
		SRC_COLOR,
		ONE_MINUS_SRC_COLOR,
		DST_ALPHA,
		ONE_MINUS_DST_ALPHA,
		DST_COLOR,
		ONE_MINUS_DST_COLOR,
		ZERO,
		ONE,
		SRC_ALPHA_SATURATE
	};

	enum Capability {
		BLEND,
		COLOR_MATERIAL,
		DEPTH_TEST,
		DEPTH_MASK,
		FOG,
		LIGHTING,
		SCISSOR_TEST,
		CULL_FACE
	};

	enum Face {
		FRONT = 0,
		BACK,
		FRONT_AND_BACK
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
	
	enum ShadeModel {
		FLAT = 0,
		SMOOTH
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
	
	Graphics();
	virtual ~Graphics();
	
	// Rendering State
	void blending(bool enable, BlendFunc src, BlendFunc dst){ p_blending(enable, src, dst); }
	void enable(Capability cap){ p_enable(cap); }
	void disable(Capability cap){ p_disable(cap); }
	
	void capability(Capability cap, bool value);

	void blending(bool b);	
	void depthMask(bool b);
	void depthTesting(bool b);
	void lighting(bool b);
	void scissor(bool b);
	void cullFace(bool b);
	
	// Frame
	void clear(int attribMask){ p_clear(attribMask); }
	void clearColor(float r, float g, float b, float a){ p_clearColor(r,g,b,a); }
	void clearColor(const Color& color) { clearColor(color.r, color.g, color.b, color.a); }

	// Coordinate Transforms
	void viewport(int x, int y, int width, int height){ p_viewport(x,y,width,height); }
	void matrixMode(MatrixMode mode){ p_matrixMode(mode); }
	void pushMatrix(){ p_pushMatrix(); }
	void popMatrix(){ p_popMatrix(); }
	void loadIdentity(){ p_loadIdentity(); }
	void loadMatrix(const Matrix4d &m){ p_loadMatrix(m); }
	void multMatrix(const Matrix4d &m){ p_multMatrix(m); }
	void modelView(const Matrix4d& m){ matrixMode(MODELVIEW); loadMatrix(m); }
	void projection(const Matrix4d& m){ matrixMode(PROJECTION); loadMatrix(m); }
	void translate(double x, double y, double z){ p_translate(x,y,z); }
	void rotate(double angle, double x, double y, double z){ p_rotate(angle, x,y,z); }
	void rotate(const Quatd& q);
	void scale(double x, double y, double z){ p_scale(x,y,z); }
	
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
	void begin(Primitive mode);
	
	/// End "immediate" mode
	void end();


	const Mesh& mesh() const { return mMesh; }
	Mesh& mesh(){ return mMesh; }

	void vertex(double x, double y, double z=0.);
	void vertex(const Vec2d& v) { vertex(v[0], v[1], 0); }
	void vertex(const Vec2f& v) { vertex(v[0], v[1], 0); }
	void vertex(const Vec3d& v) { vertex(v[0], v[1], v[2]); }
	void vertex(const Vec3f& v) { vertex(v[0], v[1], v[2]); }
	void texCoord(double u, double v);
	void normal(double x, double y, double z=0.);
	void normal(const Vec3d& v) { normal(v[0], v[1], v[2]); }
	void normal(const Vec3f& v) { normal(v[0], v[1], v[2]); }
	void color(double r, double g, double b, double a=1.);
	void color(const Color& v){ color(v.r, v.g, v.b, v.a); }
	void color(const Vec3d& v, double a=1.) { color(v[0], v[1], v[2], a); }
	void color(const Vec3f& v, double a=1.) { color(v[0], v[1], v[2], a); }

	void draw(const Mesh& v){ p_draw(v); }
	void draw(){ draw(mMesh); }

	// Other state
	void antialiasing(AntiAliasMode v){ p_antialiasing(v); }
	void lineWidth(double v){ p_lineWidth(v); }

	/// Set distance attenuation of points. The scaling formula is clamp(size * sqrt(1/(c0 + c1*d + c2*d^2)))
	void pointAtten(float c2=0, float c1=0, float c0=1){ p_pointAtten(c2,c1,c0); }
	void pointSize(double v){ p_pointSize(v); }
	void polygonMode(PolygonMode m, Face f=FRONT_AND_BACK){ p_polygonMode(m,f); }
	void shadeModel(ShadeModel m) { p_shadeModel(m); }
	void fog(float end, float start, const Color& c){ p_fog(end, start, c); }
	
	// Textures
	Texture * textureNew() { return new Texture(*this); }
	virtual void textureCreate(Texture *tex) = 0;
	virtual void textureDestroy(Texture *tex) = 0;
	virtual void textureBind(Texture *tex, int unit) = 0;
	virtual void textureUnbind(Texture *tex, int unit) = 0;
	virtual void textureEnter(Texture *tex, int unit) = 0;
	virtual void textureLeave(Texture *tex, int unit) = 0;
	virtual void textureSubmit(Texture *tex) = 0;
	virtual void textureToArray(Texture *tex) = 0;
	
	// surfaces
	virtual Surface * surfaceNew() = 0;
	virtual void surfaceCreate(Surface *surface) = 0;
	virtual void surfaceDestroy(Surface *surface) = 0;
	virtual void surfaceBind(Surface *surface) = 0;
	virtual void surfaceUnbind(Surface *surface) = 0;
	virtual void surfaceEnter(Surface *surface) = 0;
	virtual void surfaceLeave(Surface *surface) = 0;
	virtual void surfaceClear(Surface *surface) = 0;
	virtual void surfaceCopy(Surface *surface, Texture *texture) = 0;
	
protected:
	Mesh mMesh;				// used for immediate mode style rendering
	bool mInImmediateMode;	// flag for whether or not in immediate mode

private:
	virtual void p_blending(bool enable, BlendFunc src, BlendFunc dst) = 0;
	virtual void p_enable(Capability cap) = 0;
	virtual void p_disable(Capability cap) = 0;

	virtual void p_clear(int attribMask) = 0;
	virtual void p_clearColor(float r, float g, float b, float a) = 0;

	virtual void p_currentColor(double r, double g, double b, double a) = 0;

	virtual void p_draw(const Mesh& v) = 0;

	virtual void p_viewport(int x, int y, int width, int height) = 0;
	virtual void p_matrixMode(MatrixMode mode) = 0;
	virtual void p_pushMatrix() = 0;
	virtual void p_popMatrix() = 0;
	virtual void p_loadIdentity() = 0;
	virtual void p_loadMatrix(const Matrix4d &m) = 0;
	virtual void p_multMatrix(const Matrix4d &m) = 0;
	virtual void p_translate(double x, double y, double z) = 0;
	virtual void p_rotate(double angle, double x, double y, double z) = 0;
	virtual void p_scale(double x, double y, double z) = 0;

	virtual void p_antialiasing(AntiAliasMode v) = 0;
	virtual void p_lineWidth(double v) = 0;
	virtual void p_pointSize(double v) = 0;
	virtual void p_pointAtten(float c2, float c1, float c0) = 0;
	virtual void p_polygonMode(PolygonMode m, Face f) = 0;
	virtual void p_shadeModel(ShadeModel m) = 0;
	
	virtual void p_fog(float end, float start, const Color& c) = 0;
};



///	Abstract base class for any object that can be rendered via Graphics
class Drawable {
public:
	virtual void onDraw(Graphics& gl) = 0;
	virtual ~Drawable(){}
};



// ============== INLINE ============== 

inline void Graphics::capability(Graphics::Capability cap, bool v){
	v ? enable(cap) : disable(cap);
}

inline void Graphics::blending(bool b) { capability(BLEND, b); }
inline void Graphics::depthMask(bool b) { capability(DEPTH_MASK, b); }
inline void Graphics::depthTesting(bool b){ capability(DEPTH_TEST, b); }
inline void Graphics::lighting(bool b) { capability(LIGHTING, b); }
inline void Graphics::scissor(bool b) { capability(SCISSOR_TEST, b); }
inline void Graphics::cullFace(bool b) { capability(CULL_FACE, b); }

inline void Graphics::rotate(const Quatd& q) {
	Matrix4d m;
	q.toMatrix(m.elems);
	multMatrix(m);
}

} // ::al

#endif	/* include guard */

