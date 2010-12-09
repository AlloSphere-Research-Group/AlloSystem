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
		DEPTH_TEST,
		DEPTH_MASK,
		LIGHTING,
		SCISSOR_TEST
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
	virtual void blending(bool enable, BlendFunc src, BlendFunc dst) = 0;
	virtual void enable(Capability cap) = 0;
	virtual void disable(Capability cap) = 0;
	virtual void capability(Capability cap, bool value) = 0;
	
	void depthTesting(bool b);
	void blending(bool b);
	void depthMask(bool b);
	void lighting(bool b);
	void scissor(bool b);
	
	// Frame
	virtual void clear(int attribMask) = 0;
	virtual void clearColor(float r, float g, float b, float a) = 0;
	
	void clearColor(const Color& color) { clearColor(color.r, color.g, color.b, color.a); }

	// Coordinate Transforms
	virtual void viewport(int x, int y, int width, int height) = 0;
	virtual void matrixMode(MatrixMode mode) = 0;
	virtual void pushMatrix() = 0;
	virtual void popMatrix() = 0;
	virtual void loadIdentity() = 0;
	virtual void loadMatrix(const Matrix4d &m) = 0;
	virtual void multMatrix(const Matrix4d &m) = 0;
	virtual void translate(double x, double y, double z) = 0;
	virtual void rotate(double angle, double x, double y, double z) = 0;
	virtual void scale(double x, double y, double z) = 0;
	
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
	void texcoord(double u, double v);
	void normal(double x, double y, double z=0.);
	void normal(const Vec3d& v) { normal(v[0], v[1], v[2]); }
	void normal(const Vec3f& v) { normal(v[0], v[1], v[2]); }
	void color(double r, double g, double b, double a=1.);
	void color(const Color& v){ color(v.r, v.g, v.b, v.a); }
	void color(const Vec3d& v, double a=1.) { color(v[0], v[1], v[2], a); }
	void color(const Vec3f& v, double a=1.) { color(v[0], v[1], v[2], a); }

	virtual void draw(const Mesh& v) = 0;
	void draw() { draw(mMesh); }

	// Other state
	virtual void antialiasing(AntiAliasMode v) = 0;
	virtual void lineWidth(double v) = 0;
	virtual void pointSize(double v) = 0;
	
	// Textures
	Texture * textureNew() { return new Texture(this); }
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
	virtual void raw_color(double r, double g, double b, double a=1.) = 0;

	Mesh				mMesh;				// used for immediate mode style rendering
	bool				mInImmediateMode;	// flag for whether or not in immediate mode
};


///	Abstract base class for any object that can be rendered via Graphics
class Drawable {
public:
	virtual void onDraw(Graphics& gl) = 0;
	virtual ~Drawable(){}
};

/// ============== INLINE ============== 

inline void Graphics::depthTesting(bool b) { 
	if (b) { enable(DEPTH_TEST); } else { disable(DEPTH_TEST); }
}

inline void Graphics::blending(bool b) { 
	if (b) { enable(BLEND); } else { disable(BLEND); }
}

inline void Graphics::depthMask(bool b) { 
	if (b) { enable(DEPTH_MASK); } else { disable(DEPTH_MASK); }
}

inline void Graphics::lighting(bool b) { 
	if (b) { enable(LIGHTING); } else { disable(LIGHTING); }
}

inline void Graphics::scissor(bool b) { 
	if (b) { enable(SCISSOR_TEST); } else { disable(SCISSOR_TEST); }
}


} // ::al

#endif	/* include guard */

