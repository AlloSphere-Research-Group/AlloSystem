#ifndef INCLUDE_AL_GRAPHICS_HPP
#define INCLUDE_AL_GRAPHICS_HPP

/*	Allocore --
	Multimedia / virtual environment application class library

	Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology, UCSB.
	Copyright (C) 2012. The Regents of the University of California.
	All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

		Redistributions of source code must retain the above copyright notice,
		this list of conditions and the following disclaimer.

		Redistributions in binary form must reproduce the above copyright
		notice, this list of conditions and the following disclaimer in the
		documentation and/or other materials provided with the distribution.

		Neither the name of the University of California nor the names of its
		contributors may be used to endorse or promote products derived from
		this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
	IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
	ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
	LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
	CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
	POSSIBILITY OF SUCH DAMAGE.


	File description:
	A general high-level interface to graphics rendering

	File author(s):
	Lance Putnam, 2010, putnam.lance@gmail.com
	Graham Wakefield, 2010, grrrwaaa@gmail.com
	Wesley Smith, 2010, wesley.hoke@gmail.com
*/

#include <string>
#include "allocore/math/al_Vec.hpp"
#include "allocore/math/al_Quat.hpp"
#include "allocore/math/al_Matrix4.hpp"
#include "allocore/types/al_Array.hpp"
#include "allocore/types/al_Color.hpp"
#include "allocore/system/al_Printing.hpp"
#include "allocore/graphics/al_GPUObject.hpp"
#include "allocore/graphics/al_Light.hpp"
#include "allocore/graphics/al_Mesh.hpp"
#include "allocore/graphics/al_OpenGL.hpp"

#ifndef AL_MAX_LIGHTS
	#define AL_MAX_LIGHTS 4
#endif

/*!
	\def AL_GRAPHICS_ERROR(msg, ID)
	Used for reporting graphics errors from source files

*/
#ifdef AL_ENABLE_DEBUG
#define AL_GRAPHICS_ERROR(msg, ID)\
{	const char * errStr = al::Graphics::errorString();\
	if(errStr[0]){\
		if(ID>=0)	AL_WARN_ONCE("Error " msg " (id=%d): %s", ID, errStr);\
		else		AL_WARN_ONCE("Error " msg ": %s", errStr);\
	}\
}
#else
#define AL_GRAPHICS_ERROR(msg, ID) ((void)0)
#endif


namespace al {

class ShaderProgram;


/// A framed area on a display screen
/// @ingroup allocore
struct Viewport {
	float l, b, w, h;	///< left, bottom, width, height

	/// @param[in] w	width
	/// @param[in] h	height
	Viewport(float w=800, float h=600): l(0), b(0), w(w), h(h) {}

	/// @param[in] l	left edge coordinate
	/// @param[in] b	bottom edge coordinate
	/// @param[in] w	width
	/// @param[in] h	height
	Viewport(float l, float b, float w, float h): l(l), b(b), w(w), h(h) {}

	///
	Viewport(const Viewport& cpy): l(cpy.l), b(cpy.b), w(cpy.w), h(cpy.h) {}

	/// Get aspect ratio (width divided by height)
	float aspect() const { return (h!=0 && w!=0) ? float(w)/h : 1; }

	/// Set dimensions
	void set(float l_, float b_, float w_, float h_){ l=l_; b=b_; w=w_; h=h_; }
};


/// Interface for setting graphics state and rendering Mesh

///	It also owns a Mesh, to simulate immediate mode (where it draws its own data)
///
/// @ingroup allocore
class Graphics : public GPUObject {
public:

	enum Pipeline {
		FIXED=0,
		PROG
	};

	enum AntiAliasMode {
		DONT_CARE				= GL_DONT_CARE,				/**< No preference */
		FASTEST					= GL_FASTEST,				/**< Fastest render, possibly lower quality */
		NICEST					= GL_NICEST					/**< Highest quality, possibly slower render */
	};

	enum AttributeBit {
		COLOR_BUFFER_BIT		= GL_COLOR_BUFFER_BIT,		/**< Color-buffer bit */
		DEPTH_BUFFER_BIT		= GL_DEPTH_BUFFER_BIT,		/**< Depth-buffer bit */
	};

	enum BlendFunc {
		SRC_ALPHA				= GL_SRC_ALPHA,				/**< */
		ONE_MINUS_SRC_ALPHA		= GL_ONE_MINUS_SRC_ALPHA,	/**< */
		SRC_COLOR				= GL_SRC_COLOR,				/**< */
		ONE_MINUS_SRC_COLOR		= GL_ONE_MINUS_SRC_COLOR,	/**< */
		DST_ALPHA				= GL_DST_ALPHA,				/**< */
		ONE_MINUS_DST_ALPHA		= GL_ONE_MINUS_DST_ALPHA,	/**< */
		DST_COLOR				= GL_DST_COLOR,				/**< */
		ONE_MINUS_DST_COLOR		= GL_ONE_MINUS_DST_COLOR,	/**< */
		ZERO					= GL_ZERO,					/**< */
		ONE						= GL_ONE,					/**< */
		SRC_ALPHA_SATURATE		= GL_SRC_ALPHA_SATURATE		/**< */
	};

	enum BlendEq {
		#ifdef AL_GRAPHICS_SUPPORTS_BLEND_EQ
		FUNC_ADD				= GL_FUNC_ADD,				/**< Source + destination */
		FUNC_SUBTRACT			= GL_FUNC_SUBTRACT,			/**< Source - destination */
		FUNC_REVERSE_SUBTRACT	= GL_FUNC_REVERSE_SUBTRACT, /**< Destination - source */
		#else
		FUNC_ADD,
		FUNC_SUBTRACT,
		FUNC_REVERSE_SUBTRACT
		#endif
	};

	enum Capability {
		BLEND					= GL_BLEND,					/**< Blend rather than replace existing colors with new colors */
		DEPTH_TEST				= GL_DEPTH_TEST,			/**< Test depth of incoming fragments */
		SCISSOR_TEST			= GL_SCISSOR_TEST,			/**< Crop fragments according to scissor region */
		CULL_FACE				= GL_CULL_FACE,				/**< Cull faces */
		FOG						= 0x6000,					/**< Apply fog effect */
		COLOR_MATERIAL,										/**< Use vertex colors with materials */
		LIGHTING,											/**< Use lighting */
		#ifdef AL_GRAPHICS_SUPPORTS_FIXED_PIPELINE
		NORMALIZE				= GL_NORMALIZE,				/**< Rescale normals to counteract non-isotropic modelview scaling */
		RESCALE_NORMAL			= GL_RESCALE_NORMAL,		/**< Rescale normals to counteract an isotropic modelview scaling */
		#endif
	};

	enum DataType {
		BYTE					= GL_BYTE,					/**< */
		UBYTE					= GL_UNSIGNED_BYTE,			/**< */
		SHORT					= GL_SHORT,					/**< */
		USHORT					= GL_UNSIGNED_SHORT,		/**< */
		#ifdef AL_GRAPHICS_SUPPORTS_INT32
		INT						= GL_INT,					/**< */
		UINT					= GL_UNSIGNED_INT,			/**< */
		#endif
		FLOAT					= GL_FLOAT,					/**< */
		#ifdef AL_GRAPHICS_SUPPORTS_DOUBLE
		DOUBLE					= GL_DOUBLE
		#endif
	};

	enum Direction {
		NONE					= 0,
		FRONT					= GL_FRONT,
		BACK					= GL_BACK,
		FRONT_AND_BACK			= GL_FRONT_AND_BACK,

		#ifdef AL_GRAPHICS_SUPPORTS_LR_BUFFERS
		FRONT_LEFT				= GL_FRONT_LEFT,
		FRONT_RIGHT				= GL_FRONT_RIGHT,
		BACK_LEFT				= GL_BACK_LEFT,
		BACK_RIGHT				= GL_BACK_RIGHT,
		LEFT					= GL_LEFT,
		RIGHT					= GL_RIGHT,
		#endif
	};

	enum Format {
		LUMINANCE				= GL_LUMINANCE,				/**< */
		LUMINANCE_ALPHA			= GL_LUMINANCE_ALPHA,		/**< */
		ALPHA					= GL_ALPHA,					/**< */
		RGB						= GL_RGB,					/**< */
		RGBA					= GL_RGBA,					/**< */
		#ifdef AL_GRAPHICS_SUPPORTS_DEPTH_COMP
		DEPTH_COMPONENT			= GL_DEPTH_COMPONENT,		/**< */
		#endif
	};

	enum MatrixMode {
		MODELVIEW				= 0,						/**< Modelview matrix (object to eye space) */
		PROJECTION				= 1							/**< Projection matrix (eye to clip space) */
	};

	enum PolygonMode {
		#ifdef AL_GRAPHICS_SUPPORTS_POLYGON_MODE
		POINT					= GL_POINT,					/**< Render only points at vertices */
		LINE					= GL_LINE,					/**< Render only lines along edges */
		FILL					= GL_FILL					/**< Render vertices normally according to primitive */
		#else
		POINT,
		LINE,
		FILL
		#endif
	};

	enum Primitive {
		POINTS					= GL_POINTS,				/**< Points */
		LINES					= GL_LINES,					/**< Connect sequential vertex pairs with lines */
		LINE_STRIP				= GL_LINE_STRIP,			/**< Connect sequential vertices with a continuous line */
		LINE_LOOP				= GL_LINE_LOOP,				/**< Connect sequential vertices with a continuous line loop */
		TRIANGLES				= GL_TRIANGLES,				/**< Draw triangles using sequential vertex triplets */
		TRIANGLE_STRIP			= GL_TRIANGLE_STRIP,		/**< Draw triangle strip using sequential vertices */
		TRIANGLE_FAN			= GL_TRIANGLE_FAN			/**< Draw triangle fan using sequential vertices */
	};

	enum ShadeModel {
		#ifdef AL_GRAPHICS_SUPPORTS_SHADE_MODEL
		FLAT					= GL_FLAT,					/**< */
		SMOOTH					= GL_SMOOTH					/**< */
		#else
		FLAT,
		SMOOTH
		#endif
	};



	Graphics();
	virtual ~Graphics();


	/// Get the temporary mesh
	Mesh& mesh(){ return mMesh; }
	const Mesh& mesh() const { return mMesh; }


	// Capabilities

	/// Enable a capability
	void enable(Capability v);

	/// Disable a capability
	void disable(Capability v);

	/// Set a capability
	void capability(Capability cap, bool value);

	/// Turn blending on/off
	void blending(bool b);

	/// Turn color mask RGBA components on/off
	void colorMask(bool r, bool g, bool b, bool a);

	/// Turn color mask on/off (all RGBA components)
	void colorMask(bool b);

	/// Turn the depth mask on/off
	void depthMask(bool b);

	/// Turn depth testing on/off
	void depthTesting(bool b);

	/// Turn lighting on/off
	void lighting(bool b);

	/// Turn scissor testing on/off
	void scissorTest(bool b);

	/// Turn face culling on/off
	void cullFace(bool b);

	/// Turn face culling on/off and set the face direction to cull
	void cullFace(bool b, Direction d);


	/// Set antialiasing mode
	void antialiasing(AntiAliasMode v);

	/// Set antialiasing to nicest
	void nicest(){ antialiasing(NICEST); }

	/// Set antialiasing to fastest
	void fastest(){ antialiasing(FASTEST); }

	/// Set both line width and point diameter
	void stroke(float v){ lineWidth(v); pointSize(v); }

	/// Set width, in pixels, of lines
	void lineWidth(float v);

	/// Set diameter, in pixels, of points
	void pointSize(float v);

	/// Set distance attenuation of points. The scaling formula is clamp(size * sqrt(1/(c0 + c1*d + c2*d^2)))
	void pointAtten(float c2=0, float c1=0, float c0=1);


	/// Set polygon drawing mode
	void polygonMode(PolygonMode m, Direction d=FRONT_AND_BACK);

	/// Draw only edges of polygons with lines
	void polygonLine(Direction d=FRONT_AND_BACK){ polygonMode(LINE,d); }

	/// Draw filled polygons
	void polygonFill(Direction d=FRONT_AND_BACK){ polygonMode(FILL,d); }

	/// Set shading model
	void shadeModel(ShadeModel m);


	/// Set blend mode
	void blendMode(BlendFunc src, BlendFunc dst, BlendEq eq=FUNC_ADD);

	/// Set blend mode to additive (symmetric additive lighten)
	void blendModeAdd(){ blendMode(SRC_ALPHA, ONE, FUNC_ADD); }

	/// Set blend mode to subtractive (symmetric additive darken)
	void blendModeSub(){ blendMode(SRC_ALPHA, ONE, FUNC_REVERSE_SUBTRACT); }

	/// Set blend mode to screen (symmetric multiplicative lighten)
	void blendModeScreen(){ blendMode(ONE, ONE_MINUS_SRC_COLOR, FUNC_ADD); }

	/// Set blend mode to multiplicative (symmetric multiplicative darken)
	void blendModeMul(){ blendMode(DST_COLOR, ZERO, FUNC_ADD); }

	/// Set blend mode to transparent (asymmetric)
	void blendModeTrans(){ blendMode(SRC_ALPHA, ONE_MINUS_SRC_ALPHA, FUNC_ADD); }

	/// Turn blending states on (without setting mode)
	void blendOn(){ depthMask(false); blending(true); }

	/// Set states for additive blending
	void blendAdd(){ blendOn(); blendModeAdd(); }

	/// Set states for subtractive blending
	void blendSub(){ blendOn(); blendModeSub(); }

	/// Set states for screen blending
	void blendScreen(){ blendOn(); blendModeScreen(); }

	/// Set states for multiplicative blending
	void blendMul(){ blendOn(); blendModeMul(); }

	/// Set states for transparent blending
	void blendTrans(){ blendOn(); blendModeTrans(); }

	/// Turn blending states off (opaque rendering)
	void blendOff(){ depthMask(true); blending(false); }


	/// Clear frame buffer(s)
	void clear(AttributeBit bits);

	/// Set clear color
	void clearColor(float r, float g, float b, float a);

	/// Set clear color
	void clearColor(const Color& color);

	/// Set draw buffer
	void drawBuffer(Direction d);

	/// Set read buffer
	void readBuffer(Direction d);


	/// Set linear fog parameters

	/// \param[in] end		distance from viewer to fog end
	/// \param[in] start	distance from viewer to fog start
	/// \param[in] col		fog color
	void fog(float end, float start, const Color& col);


	// Coordinate Transforms

	/// Set viewport
	void viewport(int left, int bottom, int width, int height);

	/// Set viewport
	void viewport(const Viewport& v){ viewport(v.l,v.b,v.w,v.h); }

	/// Get current viewport
	Viewport viewport() const;


	/// Set current matrix
	void matrixMode(MatrixMode mode);

	/// Push current matrix stack
	void pushMatrix();

	/// Push designated matrix stack
	void pushMatrix(MatrixMode v){ matrixMode(v); pushMatrix(); }

	/// Pop current matrix stack
	void popMatrix();

	/// Pop designated matrix stack
	void popMatrix(MatrixMode v){ matrixMode(v); popMatrix(); }

	/// Set current matrix to identity
	void loadIdentity();

	/// Set current matrix
	void loadMatrix(const Mat4d& m);
	void loadMatrix(const Mat4f& m);

	/// Multiply current matrix
	void multMatrix(const Mat4d& m);
	void multMatrix(const Mat4f& m);

	/// Set current modelview matrix
	void modelView(const Mat4d& m){ matrixMode(MODELVIEW); loadMatrix(m); }
	void modelView(const Mat4f& m){ matrixMode(MODELVIEW); loadMatrix(m); }

	/// Set current view matrix (inverse camera pose)

	/// This must be set to obtain correct lighting with the fixed pipeline
	/// emulation.
	void view(const Mat4f& v){ mUpdateView=true; mView=v; modelView(v); }

	/// Set current projection matrix
	void projection(const Mat4d& m){ matrixMode(PROJECTION); loadMatrix(m); }
	void projection(const Mat4f& m){ matrixMode(PROJECTION); loadMatrix(m); }

	/// Rotate current matrix

	/// \param[in] angle	angle, in degrees
	/// \param[in] x		x component of rotation axis
	/// \param[in] y		y component of rotation axis
	/// \param[in] z		z component of rotation axis
	void rotate(float angle, float x=0., float y=0., float z=1.);

	/// Rotate current matrix
	void rotate(const Quatd& q);

	/// Rotate current matrix

	/// \param[in] angle	angle, in degrees
	/// \param[in] axis		rotation axis
	template <class T>
	void rotate(float angle, const Vec<3,T>& axis){
		rotate(angle, axis[0],axis[1],axis[2]); }

	/// Scale current matrix uniformly
	void scale(float s);

	/// Scale current matrix along each dimension
	void scale(float x, float y, float z=1.);

	/// Scale current matrix along each dimension
	template <class T>
	void scale(const Vec<3,T>& v){ scale(v[0],v[1],v[2]); }

	/// Scale current matrix along each dimension
	template <class T>
	void scale(const Vec<2,T>& v){ scale(v[0],v[1]); }

	/// Translate current matrix
	void translate(float x, float y, float z=0.);

	/// Translate current matrix
	template <class T>
	void translate(const Vec<3,T>& v){ translate(v[0],v[1],v[2]); }

	/// Translate current matrix
	template <class T>
	void translate(const Vec<2,T>& v){ translate(v[0],v[1]); }


	// Immediate Mode

	/// Begin "immediate" mode drawing
	void begin(Primitive v);

	/// End "immediate" mode
	void end();

	/// Set current color
	void currentColor(float r, float g, float b, float a);

	/// Add vertex (immediate mode)
	void vertex(double x, double y, double z=0.);

	template<class T>
	void vertex(const Vec<2,T>& v){ vertex(v[0],v[1],T(0)); }

	template<class T>
	void vertex(const Vec<3,T>& v){ vertex(v[0],v[1],v[2]); }


	/// Add texture coordinate (immediate mode)
	void texCoord(double u, double v);

	template<class T>
	void texCoord(const Vec<2,T>& v){ texCoord(v[0],v[1]); }

	void texCoord(double u, double v, double w);

	template<class T>
	void texCoord(const Vec<3,T>& v){ texCoord(v[0],v[1],v[2]); }


	/// Add normal (immediate mode)
	void normal(double x, double y, double z=0.);

	template<class T>
	void normal(const Vec<3,T>& v){ normal(v[0],v[1],v[2]); }


	/// Add color (immediate mode)
	void color(double r, double g, double b, double a=1.);
	void color(double gray, double a=1.) { color(gray, gray, gray, a); }
	void color(const Color& v){ color(v.r, v.g, v.b, v.a); }
	void color(const Vec3d& v, double a=1.) { color(v[0], v[1], v[2], a); }
	void color(const Vec3f& v, double a=1.) { color(v[0], v[1], v[2], a); }
	void color(const Vec4d& v) { color(v[0], v[1], v[2], v[3]); }
	void color(const Vec4f& v) { color(v[0], v[1], v[2], v[3]); }

	/// Draw vertex data

	/// This draws a range of vertices from a mesh. If the mesh contains
	/// vertex indices, then the range corresponds to the vertex indices array.
	/// Negative count or index amounts are relative to one plus the maximum
	/// possible value.
	///
	/// @param[in] m		Vertex data to draw
	/// @param[in] count	Number of vertices or indices to draw
	/// @param[in] begin	Begin index of vertices or indices to draw (inclusive)
	void draw(const Mesh& m, int count=-1, int begin=0);

	/// Draw internal vertex data
	void draw(){ draw(mMesh); }

	/// Prepare global drawing state (matrices, fog, lighting, etc.)

	/// This only needs to be called before any drawing code not using the
	/// draw function defined in this class.
	/// Returns whether preparation was successful and drawing can proceed.
	bool prepareDraw();


	// Utility functions: converting, reporting, etc.

	/// Print current GPU error state

	/// @param[in] msg		Custom error message
	/// @param[in] ID		Graphics object ID (-1 for none)
	/// \returns whether there was an error
	static bool error(const char *msg="", int ID=-1);

	/// Get current GPU error string

	/// \returns the error string or an empty string if no error
	///
	static const char * errorString(bool verbose=false);

	/// Get a space-separated list of extensions
	static const std::string& extensions();

	/// Perform a check at run-time to see if an extension is supported
	static bool extensionSupported(const std::string& name, bool exactMatch=false);

	/// Returns number of components for given color type
	static int numComponents(Format v);

	/// Returns number of bytes for given data type
	static int numBytes(DataType v);

	/// Get DataType associated with a basic C type
	template<typename Type>
	static DataType toDataType();

	/// Returns AlloTy type for a given GL data type:
	static AlloTy toAlloTy(DataType v);

	/// Returns DataType for a given AlloTy
	static DataType toDataType(AlloTy type);

	/// Get light at specified index
	Light& light(unsigned index=0);
	const Light& light(unsigned index=0) const;

	/// Get material
	Material& material();
	const Material& material() const;

	Material& materialFront();
	const Material& materialFront() const;

	Material& materialBack();
	const Material& materialBack() const;

	/// Set pipeline used for rendering meshes
	void pipeline(Pipeline p);

	ShaderProgram& shader();
	Graphics& shaderPreamble(const std::string& s);
	Graphics& shaderOnVertex(const std::string& s);
	Graphics& shaderOnMaterial(const std::string& s);

	/// Set current vertex buffer and optionally update
	void setVertexBuffer(const Mesh& m, bool updateBuffer=false);

	/// Draw currently set vertex buffer
	void drawVertexBuffer();

public: 
	class RawMeshData;

protected:

	class Backend{
	public:
		Backend(Graphics& g): mGraphics(g){}
		virtual ~Backend(){}

		virtual void enable(Capability v){}
		virtual void disable(Capability v){}
		virtual void currentColor(float r, float g, float b, float a){}

		virtual void matrixMode(MatrixMode mode){}
		virtual void pushMatrix(){}
		virtual void popMatrix(){}
		virtual void loadIdentity(){}
		virtual void loadMatrix(const Mat4d& m){}
		virtual void loadMatrix(const Mat4f& m){}
		virtual void multMatrix(const Mat4d& m){}
		virtual void multMatrix(const Mat4f& m){}
		virtual void translate(float x, float y, float z){}
		virtual void rotate(float angle, float x, float y, float z){}
		virtual void scale(float s){ scale(s,s,s); }
		virtual void scale(float x, float y, float z){}

		virtual void pointSize(float v){}
		virtual void pointAtten(float c2, float c1, float c0){}

		virtual bool prepareDraw(){ return true; }
		virtual void draw(const RawMeshData& m){}
		virtual void draw(const Mesh& m, int count=-1, int begin=0){}

		virtual void drawVertexBuffer(){}

	protected:
		Graphics& mGraphics;
	};

	class BackendProg;
	class BackendFixed;
	Backend * mBackends[2] = {0};
	Backend * mBackend = 0;

	template <class T, class Loc=int>
	class ShaderData{
	public:
		ShaderData(){}
		ShaderData(const T& val): mData(val), mUpdate(true){}
		ShaderData& operator= (const T& v){ if(mData!=v) update(); mData=v; return *this; }
		bool operator== (const T& v) const { return mData==v; }
		const T& get() const { return mData; }
		T& var(){ update(); return mData; }
		void update(){ mUpdate=true; }
		bool handleUpdate() const { auto r=mUpdate; mUpdate=false; return r; }
		Loc& loc(){ return mLoc; }
		const Loc& loc() const { return mLoc; }
	private:
		T mData;
		Loc mLoc;
		mutable bool mUpdate = false;
	};

	struct LightLocs{int pos,dir,halfDist,spread,strength,diffuse,specular,ambient;};
	ShaderData<Light,LightLocs> mLights[AL_MAX_LIGHTS];
	ShaderData<bool> mDoLighting{false};

	struct MaterialLocs{int diffuse,emission,specular,shininess;};
	ShaderData<Material,MaterialLocs> mMaterials[2];
	ShaderData<bool> mMaterialOneSided{true};

	struct Fog{
		al::RGB color;
		float start=0, end=1;
		float scale = 1; // 1 / (end - start)
	};
	struct FogLocs{int color,start,end,scale;};
	ShaderData<Fog, FogLocs> mFog;
	bool mDoFog = false;

	Mat4f mView;
	bool mUpdateView = true;
	Mesh mMesh;				// used for immediate mode style rendering
	bool mInImmediateMode;	// flag for whether or not in immediate mode

	virtual void onCreate(); // GPUObject
	virtual void onDestroy(); // GPUObject

public:
	/// \deprecated
	void draw(int numVertices, const Mesh& m);
};



///	Abstract base class for any object that can be rendered via Graphics
class Drawable {
public:

	/// Place drawing code here
	virtual void onDraw(Graphics& gl) = 0;

	virtual ~Drawable(){}
};


const char * toString(Graphics::DataType v);
const char * toString(Graphics::Format v);


// ============== INLINE ==============
inline void Graphics::enable(Capability v){ mBackend->enable(v); }
inline void Graphics::disable(Capability v){ mBackend->disable(v); }
inline void Graphics::currentColor(float r, float g, float b, float a){ mBackend->currentColor(r,g,b,a); }
inline void Graphics::matrixMode(MatrixMode mode){ mBackend->matrixMode(mode); }
inline void Graphics::pushMatrix(){ mBackend->pushMatrix(); }
inline void Graphics::popMatrix(){ mBackend->popMatrix(); }
inline void Graphics::loadIdentity(){ mBackend->loadIdentity(); }
inline void Graphics::loadMatrix(const Mat4f& m){ mBackend->loadMatrix(m); }
inline void Graphics::multMatrix(const Mat4f& m){ mBackend->multMatrix(m); }
inline void Graphics::loadMatrix(const Mat4d& m){ mBackend->loadMatrix(m); }
inline void Graphics::multMatrix(const Mat4d& m){ mBackend->multMatrix(m); }
inline void Graphics::translate(float x, float y, float z){ mBackend->translate(x,y,z); }
inline void Graphics::rotate(float angle, float x, float y, float z){ mBackend->rotate(angle, x,y,z); }
inline void Graphics::scale(float s){ mBackend->scale(s); }
inline void Graphics::scale(float x, float y, float z){ mBackend->scale(x,y,z); }
inline void Graphics::pointSize(float v){ mBackend->pointSize(v); }
inline void Graphics::pointAtten(float c2, float c1, float c0){ mBackend->pointAtten(c2,c1,c0); }
inline void Graphics::draw(const Mesh& m, int count, int begin){ mBackend->draw(m, count, begin); }
inline bool Graphics::prepareDraw(){ return mBackend->prepareDraw(); }

#ifdef AL_GRAPHICS_SUPPORTS_SET_RW_BUFFERS
inline void Graphics::drawBuffer(Direction d){ glDrawBuffer(d); }
inline void Graphics::readBuffer(Direction d){ glReadBuffer(d); }
#else
inline void Graphics::drawBuffer(Direction d){}
inline void Graphics::readBuffer(Direction d){}
#endif

#ifdef AL_GRAPHICS_SUPPORTS_POLYGON_MODE
inline void Graphics::polygonMode(PolygonMode m, Direction d){ glPolygonMode(d,m); }
#else
inline void Graphics::polygonMode(PolygonMode m, Direction d){}
#endif

#ifdef AL_GRAPHICS_SUPPORTS_SHADE_MODEL
inline void Graphics::shadeModel(ShadeModel m){ glShadeModel(m); }
#else
inline void Graphics::shadeModel(ShadeModel m){}
#endif

// Supported across all backends
inline void Graphics::clear(AttributeBit bits){ glClear(bits); }
inline void Graphics::clearColor(float r, float g, float b, float a){ glClearColor(r, g, b, a); }
inline void Graphics::clearColor(const Color& c){ clearColor(c.r, c.g, c.b, c.a); }

inline void Graphics::blendMode(BlendFunc src, BlendFunc dst, BlendEq eq){
	#ifdef AL_GRAPHICS_SUPPORTS_BLEND_EQ
		glBlendEquation(eq);
	#endif
	glBlendFunc(src, dst);
}

inline void Graphics::capability(Capability cap, bool v){
	v ? enable(cap) : disable(cap);
}

inline void Graphics::blending(bool b){ capability(BLEND, b); }
inline void Graphics::colorMask(bool r, bool g, bool b, bool a){
	glColorMask(
		r?GL_TRUE:GL_FALSE,
		g?GL_TRUE:GL_FALSE,
		b?GL_TRUE:GL_FALSE,
		a?GL_TRUE:GL_FALSE
	);
}
inline void Graphics::colorMask(bool b){ colorMask(b,b,b,b); }
inline void Graphics::depthMask(bool b){ glDepthMask(b?GL_TRUE:GL_FALSE); }
inline void Graphics::depthTesting(bool b){ capability(DEPTH_TEST, b); }
inline void Graphics::lighting(bool b){ capability(LIGHTING, b); }
inline void Graphics::scissorTest(bool b){ capability(SCISSOR_TEST, b); }
inline void Graphics::cullFace(bool b){ capability(CULL_FACE, b); }
inline void Graphics::cullFace(bool b, Direction d) {
	capability(CULL_FACE, b);
	glCullFace(d);
}
inline void Graphics::rotate(const Quatd& q) {
	Mat4d m;
	q.toMatrix(m.elems());
	multMatrix(m);
}
inline void Graphics::lineWidth(float v) { glLineWidth(v); }

inline Graphics::AttributeBit operator| (
	const Graphics::AttributeBit& a, const Graphics::AttributeBit& b
){
	return static_cast<Graphics::AttributeBit>(+a|+b);
}

} // al::

#endif
