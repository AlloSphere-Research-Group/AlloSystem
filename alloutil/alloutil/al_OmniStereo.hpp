#ifndef AL_OMNISTEREO_H
#define AL_OMNISTEREO_H


#include "allocore/graphics/al_Lens.hpp"
#include "allocore/graphics/al_Shader.hpp"
#include "allocore/graphics/al_Texture.hpp"

namespace al {

// Object to encapsulate rendering omni-stereo worlds via cube-maps:
class OmniStereo {
public:

	// prefix this string to every vertex shader used in rendering the scene
	// use it as e.g.:
	// gl_Position = omni_cube(gl_ModelViewMatrix * gl_Vertex);
	// also be sure to call omni.uniforms(shader) in the OmniStereoDrawable callback
	static std::string glsl() {
		return AL_STRINGIFY(	
			// @omni_eye: the eye parallax distance. 	
			//	This will be zero for mono, and positive/negative for right/left eyes.
			//	Pass this uniform to the shader in the OmniStereoDrawable callback 
			uniform float omni_eye;
				
			// @omni_face: the GL_TEXTURE_CUBE_MAP face being rendered. 	
			//	For a typical forward-facing view, this should == 5.	
			//	Pass this uniform to the shader in the OmniStereoDrawable callback 
			uniform int omni_face;	

			// @omni_near: the near clipping plane. 	
			uniform float omni_near;	

			// @omni_far: the far clipping plane. 	
			uniform float omni_far;	
				
			// omni_render(vertex)	
			// @vertex: the eye-space vertex to be rendered.	
			//	Typically gl_Position = omni_render(gl_ModelViewMatrix * gl_Vertex);	
			vec4 omni_render(in vec4 vertex) {	
				// unit direction vector:	
				vec3 vn = normalize(vertex.xyz);	
				// omni-stereo effect (in eyespace XZ plane)	
				// cross-product with up vector also ensures stereo fades out at Y poles	
				//v.xyz -= omni_eye * cross(vn, vec3(0, 1, 0));	
				// simplified:	
				vertex.xz += vec2(omni_eye * vn.z, omni_eye * -vn.x);	
				// convert eye-space into cubemap-space:	
				// GL_TEXTURE_CUBE_MAP_POSITIVE_X  	
					 if (omni_face == 0) { vertex.xyz = vec3(-vertex.z, -vertex.y, -vertex.x); }	
				// GL_TEXTURE_CUBE_MAP_NEGATIVE_X	
				else if (omni_face == 1) { vertex.xyz = vec3( vertex.z, -vertex.y,  vertex.x); }	
				// GL_TEXTURE_CUBE_MAP_POSITIVE_Y  	
				else if (omni_face == 2) { vertex.xyz = vec3( vertex.x,  vertex.z, -vertex.y); }	
				// GL_TEXTURE_CUBE_MAP_NEGATIVE_Y 	
				else if (omni_face == 3) { vertex.xyz = vec3( vertex.x, -vertex.z,  vertex.y); }	
				// GL_TEXTURE_CUBE_MAP_POSITIVE_Z  	
				else if (omni_face == 4) { vertex.xyz = vec3( vertex.x, -vertex.y, -vertex.z); }	
				// GL_TEXTURE_CUBE_MAP_NEGATIVE_Z   
				else					 { vertex.xyz = vec3(-vertex.x, -vertex.y,  vertex.z); }	
				// convert into screen-space:	
				// simplified perspective projection since fovy = 90 and aspect = 1	
				vertex.zw = vec2(	
					(vertex.z*(omni_far+omni_near) + vertex.w*omni_far*omni_near*2.)/(omni_near-omni_far),	
					-vertex.z	
				);	
				return vertex;	
			}
		);
	}

	///	Abstract base class for any object that can be rendered via OmniStereo:
	class Drawable  {
	public:

		/// Place drawing code here
		virtual void onDrawOmni(OmniStereo& omni) = 0;
		
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
	
protected:
	// supports up to 4 warps/viewports
	Projection mProjections[4];

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
