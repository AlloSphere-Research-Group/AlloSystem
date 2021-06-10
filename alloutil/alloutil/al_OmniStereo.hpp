#ifndef AL_OMNISTEREO_H
#define AL_OMNISTEREO_H

#include "allocore/math/al_Constants.hpp"
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
				else if (omni_face == 4) { vertex.xyz = vec3( vertex.x, -vertex.y,  -vertex.z); }
				// GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
				else					 { vertex.xyz = vec3( -vertex.x, -vertex.y, vertex.z); }
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
		struct Parameters {
			float projnum;			// ID of the projector
			float width, height;	// width/height in pixels
			Vec3f projector_position, screen_center, normal_unit, x_vec, y_vec;
			float screen_radius, bridge_radius, unused0;
		};

		Projection();

		void onCreate();

		// load warp/blend from disk:
		void readBlend(std::string path);
		void readWarp(std::string path);
		void readParameters(std::string path, bool verbose=true);
		void initParameters(bool verbose=true);

		// adjust the registration position:
		void registrationPosition(const Vec3d& pos);

		Texture& blend() { return mBlend; }
		Texture& warp() { return mWarp; }
		Viewport& viewport() { return mViewport; }


		void updatedWarp();

		Parameters params;

		// derived:
		Vec3f x_unit, y_unit;
		float x_pixel, y_pixel;
		float x_offset, y_offset;
		Vec3d position;

		// TODO: remove this
		int warpwidth, warpheight;

	protected:
		Texture mBlend, mWarp;
		Viewport mViewport;

		// the position/orientation of the raw map data relative to the real world
		Pose mRegistration;

		// the raw warp data:
		float * t;
		float * u;
		float * v;
	};

	/// Stereoscopic mode
	enum StereoMode {
		MONO = 0,
		SEQUENTIAL,		/**< Alternate left/right eye frames */
		ACTIVE,			/**< Active quad-buffered stereo */
		DUAL,			/**< Dual side-by-side stereo */
		ANAGLYPH,		/**< Red (left eye) / cyan (right eye) stereo */
		LEFT_EYE,		/**< Left eye only */
		RIGHT_EYE		/**< Right eye only */
	};

	/// Anaglyph mode
	enum AnaglyphMode {
		RED_BLUE = 0,	/**< */
		RED_GREEN,		/**< */
		RED_CYAN,		/**< */
		BLUE_RED,		/**< */
		GREEN_RED,		/**< */
		CYAN_RED		/**< */
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
	// @viewport is the pixel dimensions of the window
	void draw(const Lens& lens, const Pose& pose, const Viewport& vp);

	// typically they would be combined like this:
	void onFrame(OmniStereo::Drawable& drawable, const Lens& lens, const Pose& pose, const Viewport& vp) {
		capture(drawable, lens, pose);
		draw(lens, pose, vp);
	}

	// render front-view only (bypass FBO)
	void onFrameFront(OmniStereo::Drawable& drawable, const Lens& lens, const Pose& pose, const Viewport& vp);

	// send the proper uniforms to the shader:
	void uniforms(ShaderProgram& program) const;

	// enable/disable stereographic mode:
	OmniStereo& stereo(bool b) { mStereo = b; return *this; }
	bool stereo() const { return mStereo; }

	OmniStereo& mode(StereoMode m) { mMode = m; return *this; }
	StereoMode mode() { return mMode; }

	// returns true if configured for active stereo:
	bool activeStereo() { return mMode == ACTIVE; }

	// returns true if config file suggested fullscreen by default
	bool fullScreen() { return mFullScreen; }

	// get/set the background color
	Color& clearColor() { return mClearColor; }

	// get individual projector configurations:
	Projection& projection(int i) { return mProjections[i]; }
	int numProjections() const { return mNumProjections; }

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
	void drawSphereMap(Texture& map, const Lens& lens, const Pose& pose, const Viewport& vp);
	void drawDemo(const Lens& lens, const Pose& pose, const Viewport& vp);

	// adjust the registration position:
	void registrationPosition(const Vec3d& pos) {
		for (int i=0; i<numProjections(); i++) {
			projection(i).registrationPosition(pos);
		}
	}

protected:
	// supports up to 4 warps/viewports
	Projection mProjections[4];

	typedef void (OmniStereo::*DrawMethod)(const Pose& pose, double eye);
	void drawEye(const Pose& pose, double eye);
	void drawQuadEye(const Pose& pose, double eye);
	void drawDemoEye(const Pose& pose, double eye);

	template<DrawMethod F>
	void drawStereo(const Lens& lens, const Pose& pose, const Viewport& viewport);

	void drawQuad();

	void capture_eye(GLuint& tex, OmniStereo::Drawable& drawable);

	GLuint mTex[2];	// the cube map textures
	GLuint mFbo;
	GLuint mRbo;	// TODO: alternative depth cube-map texture option?
	ShaderProgram mCubeProgram, mSphereProgram, mWarpProgram, mDemoProgram;
	Mesh mQuad;

	Graphics gl;
	Matrix4d mModelView;
	Color mClearColor;

	// these become shader uniforms:
	int mFace;
	float mEyeParallax, mNear, mFar;

	unsigned mResolution;
	unsigned mNumProjections;
	int mFrame;

	StereoMode mMode;
	AnaglyphMode mAnaglyphMode;

	bool mStereo, mMipmap, mFullScreen;
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

template<OmniStereo::DrawMethod F>
inline void OmniStereo::drawStereo(const Lens& lens, const Pose& pose, const Viewport& vp) {
	double eye = lens.eyeSep();
	switch (mMode) {
		case SEQUENTIAL:
			if (mFrame & 1) {
				(this->*F)(pose, eye);
			} else {
				(this->*F)(pose, -eye);
			}
			break;

		case ACTIVE:
			glDrawBuffer(GL_BACK_RIGHT);
			gl.error("OmniStereo drawStereo GL_BACK_RIGHT");
			(this->*F)(pose, eye);

			glDrawBuffer(GL_BACK_LEFT);
			gl.error("OmniStereo drawStereo GL_BACK_LEFT");
			(this->*F)(pose, -eye);

			glDrawBuffer(GL_BACK);
			gl.error("OmniStereo drawStereo GL_BACK");
			break;

		case DUAL:
			gl.viewport(vp.l + vp.w*0.5, vp.b, vp.w*0.5, vp.h);
			(this->*F)(pose, eye);
			gl.viewport(vp.l, vp.b, vp.w*0.5, vp.h);
			(this->*F)(pose, -eye);
			break;

		case ANAGLYPH:
			switch(mAnaglyphMode){
				case RED_BLUE:
				case RED_GREEN:
				case RED_CYAN:	glColorMask(GL_TRUE, GL_FALSE,GL_FALSE,GL_TRUE); break;
				case BLUE_RED:	glColorMask(GL_FALSE,GL_FALSE,GL_TRUE, GL_TRUE); break;
				case GREEN_RED:	glColorMask(GL_FALSE,GL_TRUE, GL_FALSE,GL_TRUE); break;
				case CYAN_RED:	glColorMask(GL_FALSE,GL_TRUE, GL_TRUE, GL_TRUE); break;
				default:		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE ,GL_TRUE);
			}
			(this->*F)(pose, eye);

			switch(mAnaglyphMode){
				case RED_BLUE:	glColorMask(GL_FALSE,GL_FALSE,GL_TRUE, GL_TRUE); break;
				case RED_GREEN:	glColorMask(GL_FALSE,GL_TRUE, GL_FALSE,GL_TRUE); break;
				case RED_CYAN:	glColorMask(GL_FALSE,GL_TRUE, GL_TRUE, GL_TRUE); break;
				case BLUE_RED:
				case GREEN_RED:
				case CYAN_RED:	glColorMask(GL_TRUE, GL_FALSE,GL_FALSE,GL_TRUE); break;
				default:		glColorMask(GL_TRUE, GL_TRUE ,GL_TRUE, GL_TRUE);
			}
			// clear depth before this pass:
			gl.depthMask(1);
			gl.depthTesting(1);
			gl.clear(gl.DEPTH_BUFFER_BIT);
			(this->*F)(pose, -eye);

			break;

		case RIGHT_EYE:
			(this->*F)(pose, eye);
			break;

		case LEFT_EYE:
			(this->*F)(pose, -eye);
			break;

		case MONO:
		default:
			(this->*F)(pose, 0);
			break;
	}
}

} // al::

#endif
