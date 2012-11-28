#ifndef AL_OMNISTEREO_H
#define AL_OMNISTEREO_H


#include "allocore/graphics/al_Lens.hpp"
#include "allocore/graphics/al_Shader.hpp"
#include "allocore/graphics/al_Texture.hpp"

namespace al {

// prefix this string to each vertex shader used in rendering the scene
// use it as e.g.:
// gl_Position = omni_cube(gl_ModelViewMatrix * gl_Vertex);
// also be sure to call omni.uniforms(shader) in the OmniStereoDrawable callback

#define AL_OMNISTEREOCUBEMAP_GLSL "\n\
	// AL_OMNISTEREOCUBEMAP_GLSL	\n\
		\n\
	// @omni_eye: the eye parallax distance. 	\n\
	//	This will be zero for mono, and positive/negative for right/left eyes.	\n\
	//	Pass this uniform to the shader in the OmniStereoDrawable callback 	\n\
	uniform float omni_eye;	\n\
	// @omni_face: the GL_TEXTURE_CUBE_MAP face being rendered. 	\n\
	//	For a typical forward-facing view, this should == 5.	\n\
	//	Pass this uniform to the shader in the OmniStereoDrawable callback 	\n\
	uniform int omni_face;	\n\
	// @omni_near: the near clipping plane. 	\n\
	uniform float omni_near;	\n\
	// @omni_far: the far clipping plane. 	\n\
	uniform float omni_far;	\n\
		\n\
	// omni_render(vertex)	\n\
	// @vertex: the eye-space vertex to be rendered.	\n\
	//	Typically gl_Position = omni_render(gl_ModelViewMatrix * gl_Vertex);	\n\
	vec4 omni_render(in vec4 vertex) {	\n\
		// unit direction vector:	\n\
		vec3 vn = normalize(vertex.xyz);	\n\
		// omni-stereo effect (in eyespace XZ plane)	\n\
		// cross-product with up vector also ensures stereo fades out at Y poles	\n\
		//v.xyz -= omni_eye * cross(vn, vec3(0, 1, 0));	\n\
		// simplified:	\n\
		vertex.xz += vec2(omni_eye * vn.z, omni_eye * -vn.x);	\n\
		// convert eye-space into cubemap-space:	\n\
		// GL_TEXTURE_CUBE_MAP_POSITIVE_X  	\n\
		     if (omni_face == 0) { vertex.xyz = vec3(-vertex.z, -vertex.y, -vertex.x); }	\n\
		// GL_TEXTURE_CUBE_MAP_NEGATIVE_X	\n\
		else if (omni_face == 1) { vertex.xyz = vec3( vertex.z, -vertex.y,  vertex.x); }	\n\
		// GL_TEXTURE_CUBE_MAP_POSITIVE_Y  	\n\
		else if (omni_face == 2) { vertex.xyz = vec3( vertex.x,  vertex.z, -vertex.y); }	\n\
		// GL_TEXTURE_CUBE_MAP_NEGATIVE_Y 	\n\
		else if (omni_face == 3) { vertex.xyz = vec3( vertex.x, -vertex.z,  vertex.y); }	\n\
		// GL_TEXTURE_CUBE_MAP_POSITIVE_Z  	\n\
		else if (omni_face == 4) { vertex.xyz = vec3( vertex.x, -vertex.y, -vertex.z); }	\n\
		// GL_TEXTURE_CUBE_MAP_NEGATIVE_Z   \n\
		else					 { vertex.xyz = vec3(-vertex.x, -vertex.y,  vertex.z); }	\n\
		// convert into screen-space:	\n\
		// simplified perspective projection since fovy = 90 and aspect = 1	\n\
		vertex.zw = vec2(	\n\
			(vertex.z*(omni_far+omni_near) + vertex.w*omni_far*omni_near*2.)/(omni_near-omni_far),	\n\
			-vertex.z	\n\
		);	\n\
		return vertex;	\n\
	}	\n\
	"

// Object to encapsulate rendering omni-stereo worlds via cube-maps:
class OmniStereo {
public:

	///	Abstract base class for any object that can be rendered via OmniStereo:
	class Drawable  {
	public:

		/// Place drawing code here
		virtual void onDraw(OmniStereo& omni) = 0;
		
		virtual ~Drawable(){}
	};
	
	/// Encapsulate the trio of fractional viewport, warp & blend maps:
	class Projection {
	public:
		Projection();
		
		void onCreate();
		
		// load warp/blend from disk:
		void readBlend(std::string path);
		void readWarp(std::string path);
				
		Texture mBlend, mWarp;
		Viewport mViewport;
	};
	
	/// Stereographic mode
	enum StereoMode {
		MONO = 0,
		SEQUENTIAL,		/**< Alternate left/right eye frames */
		ACTIVE,			/**< Active quad-buffered stereo */
		DUAL,			/**< Dual side-by-side stereo */
		ANAGLYPH,		/**< Red (left eye) / cyan (right eye) stereo */
		LEFT_EYE,		/**< Left eye only */
		RIGHT_EYE		/**< Right eye only */
		
	};
	
	enum WarpMode {
		FISHEYE,
		CYLINDER,
		RECT
	};
	
	enum BlendMode {
		NOBLEND,
		SOFTEDGE
	};

	// @resolution sets the resolution of the cube textures / render buffers:
	OmniStereo(unsigned resolution = 1024, bool useMipMaps=true);

	// @resolution should be a power of 2
	OmniStereo& resolution(unsigned resolution);
	unsigned resolution() { return mResolution; }
	
	// configure the projections according to files
	OmniStereo& configure(std::string configpath, std::string configname="default");
	
	// configure generatively:
	OmniStereo& configure(WarpMode wm, float aspect=2., float fovy=M_PI);
	OmniStereo& configure(BlendMode bm);
	
	// capture a scene to the cubemap textures 
	// this is likely to be an expensive call, as it will render the scene 
	// six (twelve for stereo) times, by calling back into @draw
	// @draw should render without changing the viewport, modelview or projection matrices
	// @lens is used for near/far clipping planes, and eye separation
	// @pose sets the camera position/orientation
	void capture(OmniStereo::Drawable& drawable, const Lens& lens, const Pose& pose);
	
	// render the captured scene to multiple warp maps and viewports
	// @w, @h are the pixel dimensions of the window
	void draw(const Viewport& vp);
	
	// typically they would be combined like this:
	void onFrame(OmniStereo::Drawable& drawable, const Lens& lens, const Pose& pose, const Viewport& vp) {
		capture(drawable, lens, pose);
		draw(vp);
	}
	
	// send the proper uniforms to the shader:
	void uniforms(ShaderProgram& program) const;
	
	// enable/disable stereographic mode:
	OmniStereo& stereo(bool b) { mStereo = b; return *this; }
	bool stereo() const { return mStereo; }	

	OmniStereo& mode(StereoMode m) { mMode = m; return *this; }
	StereoMode mode() { return mMode; }
	
	// get/set the background color
	Color& clearColor() { return mClearColor; }
	
	// useful accessors:
	GLuint texture() const { return mTex[0]; }
	GLuint textureLeft() const { return mTex[0]; }
	GLuint textureRight() const { return mTex[1]; }
	
	// the current face being rendered:
	int face() const { return mFace; }
	
	// the current eye parallax 
	// (positive for right eye, negative for left, zero for mono):
	float eye() const { return mEyeParallax; }
	
	// create GPU resources:
	void onCreate();
	void onDestroy();
	
	// configure the warp maps
	void loadWarps(std::string calibrationpath, std::string hostname = "default");
	
	// debugging:
	void drawWarp(const Viewport& vp);
	void drawBlend(const Viewport& vp);
	void drawDemo(const Lens& lens, const Pose& pose, const Viewport& vp);
	
	Projection& projection(int i) { return mProjections[i]; }
	//WarpnBlend& warp(int i) { return warps[i]; }
	
protected:

	

	// supports up to 4 warps/viewports
	Projection mProjections[4];
	
//	WarpnBlend warps[4];
//	Viewport vps[4];

	void capture_eye(GLuint& tex, OmniStereo::Drawable& drawable);
	
	GLuint mTex[2];	// the cube map textures
	GLuint mFbo;
	GLuint mRbo;	// TODO: alternative depth cube-map texture option?
	ShaderProgram mCubeProgram;
	Mesh mQuad;
	
	Graphics gl;
	Matrix4d mModelView;
	Color mClearColor;
	
	// these become shader uniforms:
	int mFace;
	float mEyeParallax, mNear, mFar;
	
	unsigned mResolution;
	unsigned mNumProjections;
	
	StereoMode mMode;
	
	bool mStereo, mMipmap;	
}; 	

/* inline implementation */

inline 
void OmniStereo::uniforms(ShaderProgram& program) const {
	program.uniform("omni_face", mFace);
	program.uniform("omni_eye", mEyeParallax);
	program.uniform("omni_near", mNear);
	program.uniform("omni_far", mFar);
	gl.error("sending OmniStereo uniforms");
}
	
} // al::

#endif
