//
//  al_OmniStereo2.hpp
//  
//
//  Created by Graham Wakefield, Karl Yerkes, Pablo Colapinto on 3/5/13.
//  
//

#ifndef allovsr_al_OmniStereo2_hpp
#define allovsr_al_OmniStereo2_hpp

#include "al_Projection.hpp"
#include "al_OmniUtil.hpp"

namespace al {

    class OmniStereo {
    
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
	void compile();     ///< Compile Shaders
    void generate();    ///< Generate Cubemap Textures
    void mipmap();      ///< Generate MipMap for Cubemap Texturs
    
    
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
            
            unsigned mResolution;
            unsigned mNumProjections;
            int mFrame;

            Graphics gl;                            ///< Graphics Context
            Color mClearColor;                      

            Matrix4d mModelView;                    ///< 
            
            //Function Pointer to Draw Methods
            typedef void (OmniStereo::*DrawMethod)(const Pose& pose, double eye);
            
            //Possible DrawMethods
            void drawEye(const Pose& pose, double eye);
            void drawQuadEye(const Pose& pose, double eye);
            void drawDemoEye(const Pose& pose, double eye);
            
            //Stereomode-switcher and DrawMethod-caller
            template<DrawMethod F>
            void drawStereo(const Lens& lens, const Pose& pose, const Viewport& viewport);
            
            void drawQuad();

            void capture_eye(GLuint& tex, OmniStereo::Drawable& drawable);
            

            GLuint mTex[2];             // the cube map textures
            GLuint mRbo;                // the depthbuffer (TODO: alternative depth cube-map texture option?)
            GLuint mFbo;                // the framebuffer object to bind during capture phase
            
            ShaderProgram mCubeProgram, mSphereProgram, mWarpProgram, mDemoProgram;
            Mesh mQuad; // The Slab to Render to
            
            
            //MODES and FLAGS
            StereoMode mMode;
            AnaglyphMode mAnaglyphMode;
            
            bool mStereo, mMipmap, mFullScreen;	
            
            //These become shader uniforms:
            int mFace;
            float mEyeParallax, mNear, mFar;
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

//DRAWS DEPENDENDING ON STEREO MODE
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

}

#endif
