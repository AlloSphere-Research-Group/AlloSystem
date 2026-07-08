#ifndef INC_AL_GRAPHICS_HPP
#define INC_AL_GRAPHICS_HPP

/*	Allocore -- Multimedia / virtual environment application class library

	Description:
	High-level interface to graphics rendering

	Author(s):
	Lance Putnam, 2010, putnam.lance@gmail.com
	Graham Wakefield, 2010, grrrwaaa@gmail.com
	Wesley Smith, 2010, wesley.hoke@gmail.com
*/

#include <functional>
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
#include "allocore/graphics/al_Shader.hpp"
#include "allocore/graphics/al_Viewport.hpp"

#ifndef AL_MAX_LIGHTS
	#define AL_MAX_LIGHTS 4
#endif

/*!	\def AL_GRAPHICS_ERROR(msg, ID)
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
		ALPHA					= GL_ALPHA,					/**< */
		RGB						= GL_RGB,					/**< */
		RGBA					= GL_RGBA,					/**< */
		#ifdef AL_GRAPHICS_SUPPORTS_LUMINANCE_COMP
		LUMINANCE				= GL_LUMINANCE,				/**< */
		LUMINANCE_ALPHA			= GL_LUMINANCE_ALPHA,		/**< */
		#else
		LUMINANCE				= GL_RED,					/**< Not supported directly, so choose 1 channel option */
		LUMINANCE_ALPHA			= GL_RG,					/**< Not supported directly, so choose 2 channel option */
		#endif
		#ifdef AL_GRAPHICS_SUPPORTS_DEPTH_COMP
		DEPTH_COMPONENT			= GL_DEPTH_COMPONENT,		/**< */
		#elif defined AL_GRAPHICS_SUPPORTS_DEPTH_COMP24
		DEPTH_COMPONENT			= GL_DEPTH_COMPONENT24,		/**< */
		#elif defined AL_GRAPHICS_SUPPORTS_DEPTH_COMP16
		DEPTH_COMPONENT			= GL_DEPTH_COMPONENT16,		/**< */
		#endif

		#ifdef AL_GRAPHICS_MAX_DEPTH_OFFSCREEN
			#if AL_GRAPHICS_MAX_DEPTH_OFFSCREEN == 16
			DEPTH_COMPONENT_OFFSCREEN = GL_DEPTH_COMPONENT16
			#elif AL_GRAPHICS_MAX_DEPTH_OFFSCREEN == 24
			DEPTH_COMPONENT_OFFSCREEN = GL_DEPTH_COMPONENT24
			#endif
		#else
		DEPTH_COMPONENT_OFFSCREEN = DEPTH_COMPONENT
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
	~Graphics() override;


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

	/// Set scissor region
	void scissor(int left, int bottom, int width, int height);

	/// Set scissor region
	void scissor(const Viewport& v);

	/// Turn face culling on/off
	void cullFace(bool b);

	/// Turn face culling on/off and set the face direction to cull
	void cullFace(bool b, Direction d);

	/// Turn back face culling on/off
	void cullBack(bool b){ cullFace(b, BACK); }


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

	/// Draw only edges of polygons with lines
	void wireframe(bool v, Direction d=FRONT_AND_BACK){ polygonMode(v?LINE:FILL,d); }

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

	/// Get read format of currently bound framebuffer
	static Format readFormat(Format v);
	/// Get read color components of currently bound framebuffer
	static int readComponents(Format v);
	/// Get read data type of currently bound framebuffer
	static DataType readType(DataType v);

	/// Minimal framebuffer for storing temporary results
	struct FrameBuffer{
		FrameBuffer(){}
		FrameBuffer(FrameBuffer&& other);
		~FrameBuffer();
		FrameBuffer& operator=(FrameBuffer&& other) noexcept;
		bool empty() const;
		void clear();
		unsigned numComponents() const { return Graphics::numComponents(format); }
		unsigned char * dataRow(unsigned row){ return data + width*numComponents()*row; }
		const unsigned char * dataRow(unsigned row) const { return const_cast<FrameBuffer*>(this)->dataRow(row); }
		unsigned char * data = nullptr;
		unsigned width = 0;
		unsigned height = 0;
		Format format;
		DataType type;
	};

	/// Retrieve remote color framebuffer

	/// Request color framebuffer from remote server and fill returned object
	/// with the result. The function will block until all values are received.
	/// The format and type of the returned framebuffer may differ from the 
	/// requested values depending on available support in the underlying
	/// rendering API.
	///
	/// \param[in] x		Left position of extraction region, in pixels
	/// \param[in] y		Bottom position of extraction region, in pixels
	/// \param[in] w		Width of extraction region, in pixels
	/// \param[in] h		Height of extraction region, in pixels
	/// \param[in] format	Desired pixel component format; returned format may differ
	/// \param[in] type		Desired pixel data type; returned type may differ
	static FrameBuffer getFrameBuffer(unsigned x, unsigned y, unsigned w, unsigned h, Format format, DataType type);
	static FrameBuffer getFrameBuffer(unsigned w, unsigned h, Format format, DataType type){
		return getFrameBuffer(0,0, w,h, format, type);
	}

	/// Get largest possible pixel alignment
	static unsigned pixelAlign(unsigned numBytes);
	static unsigned pixelAlign(unsigned width, Format f, DataType t);

	/// Get remote-to-local pixel alignment
	static unsigned pixelAlignDownload();
	/// Set remote-to-local pixel alignment (must be 1, 2, 4, or 8)
	static void pixelAlignDownload(unsigned v);

	/// Get local-to-remote pixel alignment
	static unsigned pixelAlignUpload();
	/// Set local-to-remote pixel alignment (must be 1, 2, 4, or 8)
	static void pixelAlignUpload(unsigned v);


	/// Set linear fog parameters

	/// \param[in] end		distance from viewer to fog end
	/// \param[in] start	distance from viewer to fog start
	/// \param[in] col		fog color
	void fog(float end, float start, const Color& col);


	// Coordinate Transforms

	/// Set viewport
	void viewport(int left, int bottom, int width, int height);

	/// Set viewport
	void viewport(const Viewport& v);

	/// Get current viewport
	Viewport viewport() const;

	/// Get current transform matrix (depending on matrix mode)
	const Mat4f& matrix() const;
	/// Get current model-view matrix
	const Mat4f& modelView() const;
	/// Get current projection matrix
	const Mat4f& projection() const;
	/// Get product of model-view and projection matrices (as P*V*M)
	Mat4f modelViewProjection() const { return projection()*modelView(); }

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

	/// Push specified matrix, call function, then pop
	void matrixScope(MatrixMode mmode, std::function<void(void)> f){
		pushMatrix(mmode); f(); popMatrix(mmode);
	}

	/// Push current matrix, call function, then pop
	void matrixScope(std::function<void(void)> f){
		pushMatrix(); f(); popMatrix();
	}

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
	void draw(const Mesh& m, const Mesh::Group& group);

	/// Draw internal vertex data
	void draw(){ draw(mMesh); }

	/// Draw mesh normals as lines
	void drawNormals(const Mesh& m, float length=0.1);

	/// Prepare global drawing state (matrices, fog, lighting, etc.)

	/// This only needs to be called before any drawing code not using the
	/// draw function defined in this class.
	/// Returns whether preparation was successful and drawing can proceed.
	bool prepareDraw();


	// Utility functions: converting, reporting, etc.

	/// Get integer parameter of current GPU state
	static int paramInt(int param);
	/// Get boolean parameter of current GPU state
	static bool paramBool(int param);

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

	/// Returns whether format is color
	static bool isColor(Format v);

	/// Returns number of bytes for given data type
	static int numBytes(DataType v);

	/// Returns number of bytes for given pixel format and data type
	static int numBytes(Format f, DataType t);

	/// Get Format best matching number of components

	/// For 1, 2, 3, and 4 components, the formats are 
	/// LUMINANCE, LUMINANCE_ALPHA, RGB, and RGBA.
	static Format toFormat(int numComps);

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
	Graphics& pipeline(Pipeline p);

	ShaderProgram& shader();

	/// Insert code in global scope of all shader stages
	Graphics& shaderPreamble(const std::string& s);
	Graphics& shaderPreambleVert(const std::string& s);
	Graphics& shaderPreambleFrag(const std::string& s);

	/// Insert code in vertex stage; output variables are 'vposObj', 'vcol', 'vnrm' and 'vtc2'
	Graphics& shaderOnVert(const std::string& s);

	/// Insert code in fragment shader just after all inputs set ('pos', 'col' and 'tc2')
	Graphics& shaderOnFragPre(const std::string& s);
	/// Insert code in fragment shader just after computing the base color; useful for alpha discard
	Graphics& shaderOnBaseColor(const std::string& s);
	/// Insert code to customize the material
	Graphics& shaderOnMaterial(const std::string& s);
	/// Insert code to customize the lighting
	Graphics& shaderOnLight(const std::string& s);

	/// Set custom shader
	Graphics& setShader(ShaderProgram& v, const std::function<void(void)>& onBind = [](){});
	/// Set shader back to built-in
	Graphics& unsetShader();
	/// Set custom shader temporarily, then set back to built-in
	Graphics& useShader(ShaderProgram& v, const std::function<void(void)>& onBind = [](){}){
		return setShader(v, onBind).unsetShader();
	}

	/// Set whether shader should be automatically bound/unbound around draw calls

	/// By setting this to false, the user is responsible for ensuring the
	/// shader is bound before any calls to draw or prepareDraw.
	Graphics& shaderAutoBind(bool v);

	/// Set current vertex buffer and optionally update

	/// The buffer is automatically updated the first time a mesh is used.
	///
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

		virtual const Mat4f& modelView() const = 0;
		virtual const Mat4f& projection() const = 0;
		virtual void matrixMode(MatrixMode mode){}
		virtual MatrixMode matrixMode() const = 0;
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
		void draw(const Mesh& m, int count=-1, int begin=0);

	protected:
		Graphics& mGraphics;
	};

	class BackendProg;
	class BackendFixed;
	Backend * mBackends[2] = {0};
	Backend * mBackend = 0;
	BackendProg * backendProg();
	BackendFixed * backendFixed();

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

	struct MaterialLocs{int diffuse,emission,specular,shininess,reflectance,ambient;};
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

	void onCreate() override; // GPUObject
	void onDestroy() override; // GPUObject
};



///	Abstract base class for any object that can be rendered via Graphics
class Drawable {
public:

	/// Place drawing code here
	virtual void onDraw(Graphics&) = 0;

	virtual ~Drawable(){}
};


const char * toString(Graphics::DataType v);
const char * toString(Graphics::Format v);
const char * toString(Graphics::Primitive v);


// ============== INLINE ==============
inline void Graphics::enable(Capability v){ mBackend->enable(v); }
inline void Graphics::disable(Capability v){ mBackend->disable(v); }
inline void Graphics::currentColor(float r, float g, float b, float a){ mBackend->currentColor(r,g,b,a); }
inline const Mat4f& Graphics::matrix() const {
	static Mat4f identity{1.f};
	switch(mBackend->matrixMode()){
	case MODELVIEW: return mBackend->modelView();
	case PROJECTION: return mBackend->projection();
	default: return identity;
	}
}
inline const Mat4f& Graphics::modelView() const { return mBackend->modelView(); }
inline const Mat4f& Graphics::projection() const { return mBackend->projection(); }
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
inline void Graphics::draw(const Mesh& m, const Mesh::Group& g){ draw(m, g.count(),g.begin); }
inline bool Graphics::prepareDraw(){ return mBackend->prepareDraw(); }

inline void Graphics::readBuffer(Direction d){
#ifdef AL_GRAPHICS_SUPPORTS_SET_R_BUFFER
	glReadBuffer(d);
#endif
}
inline void Graphics::drawBuffer(Direction d){
#ifdef AL_GRAPHICS_SUPPORTS_SET_W_BUFFER
	glDrawBuffer(d);
#endif
}

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
