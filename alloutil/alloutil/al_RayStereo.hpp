#ifndef AL_RAYSTEREO_H
#define AL_RAYSTEREO_H

#include "allocore/graphics/al_Lens.hpp"
#include "allocore/graphics/al_Shader.hpp"
#include "allocore/graphics/al_Texture.hpp"

using namespace al;

// Object to encapsulate rendering omni-stereo worlds
class RayStereo {
public:
  
  /// Encapsulate the trio of fractional viewport, warp & blend maps:
  class Projection {
  public:
    struct Parameters {
      float projnum;			// ID of the projector
      float width, height;	// width/height in pixels
    };
    
    Projection();
    
    void onCreate();
    
    // load warp/blend from disk:
    void readBlend(std::string path);
    void readWarp(std::string path);
    
    Texture& blend() { return mBlend; }
    Texture& warp() { return mWarp; }
    Viewport& viewport() { return mViewport; }
    
    void updatedWarp();
    
    Parameters params;
    
  protected:
    Texture mBlend, mWarp;
    Viewport mViewport;
    
    // the raw warp data:
    float * t;
    float * u;
    float * v;
  };
  
  /// Stereoscopic mode
  enum StereoMode {
    MONO = 0,
    SEQUENTIAL,	/**< Alternate left/right eye frames */
    ACTIVE,			/**< Active quad-buffered stereo */
    DUAL,       /**< Dual side-by-side stereo */
    ANAGLYPH,		/**< Red (left eye) / cyan (right eye) stereo */
    LEFT_EYE,		/**< Left eye only */
    RIGHT_EYE		/**< Right eye only */
  };
  
  /// Anaglyph mode
  enum AnaglyphMode {
    RED_BLUE = 0,
    RED_GREEN,
    RED_CYAN,
    BLUE_RED,
    GREEN_RED,
    CYAN_RED
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
  
  // @resolution sets the resolution of the textures / render buffers:
  RayStereo(unsigned resolution = 1024);
  
  // @resolution should be a power of 2
  RayStereo& resolution(unsigned resolution);
  unsigned resolution() { return mResolution; }
  
  // configure the projections according to files
  RayStereo& loadConfig(std::string configpath="", std::string configname="default");
  
  // configure generatively:
  RayStereo& generateConfig(WarpMode wm, float aspect=2., float fovy=M_PI);
  RayStereo& generateConfig(BlendMode bm);
  
  void initShader(ShaderProgram& shaderProgram);
  
  RayStereo& mode(StereoMode m);
  StereoMode mode() { return mMode; }
  
  // returns true if configured for active stereo:
  bool activeStereo() { return mMode == ACTIVE; }
  
  // returns true if config file suggested fullscreen by default
  bool fullScreen() { return mFullScreen; }
  
  // get individual projector configurations:
  Projection& projection(int i) { return mProjections[i]; }
  int numProjections() const { return mNumProjections; }
  
  // useful accessors:
  GLuint texture() const { return mTex[0]; }
  GLuint textureLeft() const { return mTex[0]; }
  GLuint textureRight() const { return mTex[1]; }
  
  // create GPU resources:
  void onCreate();
  void onDestroy();
  
  void drawQuad(const ShaderProgram& shaderProgram, double eye);
  void draw(const ShaderProgram& shaderProgram, const Lens& lens, const Viewport& vp);
  
protected:
  // supports up to 4 warps/viewports
  Projection mProjections[4];
  
  GLuint mTex[2];
  GLuint mFbo;
  GLuint mRbo;
  Mesh mQuad;
  
  Graphics gl;
  
  unsigned mResolution;
  unsigned mNumProjections;
  int mFrame;
  
  StereoMode mMode;
  AnaglyphMode mAnaglyphMode;
  
  bool mFullScreen, mActivePossible;
};

#endif
