#include "json/json.h" // Parses configuration
#include <iostream>
#include <fstream>

#include "allocore/io/al_Socket.hpp" // for hostname
#include "allocore/graphics/al_Image.hpp"
#include "allocore/graphics/al_Shader.hpp"
#include "allocore/io/al_File.hpp"
#include "alloutil/al_RayStereo.hpp"

using namespace al;

static Json::Value config;
static std::string errors;

static float fovy = M_PI;
static float aspect = 2.;

static void fillFishEye(float * value, double normx, double normy) {
	Vec3f& out = *(Vec3f *)value;
	
	// move (0,0) to center of texture:
	float sx = normx - 0.5;
	float sy = normy - 0.5;
	// azimuth covers full 360':
	float az = fovy * aspect * sx;
	// elevation covers 180':
	float el = fovy * sy;
	
	float sel = sin(el);
	float cel = cos(el);
	float saz = sin(az);
	float caz = cos(az);
	
	// assumes standard OpenGL coordinate frame
	// -Z forward, Y up, X right
	out.x =  cel*saz;
	out.y =  sel;
	out.z = -cel*caz;
	out.normalize();
	
	value[3] = 1.;
}

static void fillCylinder(float * value, double normx, double normy) {
	Vec3f& out = *(Vec3f *)value;
	
	// move (0,0) to center of texture:
	float sx = normx - 0.5;
	float sy = normy - 0.5;
	
	float y1 = sy * fovy * 2.;
	float y0 = 1. - fabs(y1);
	
	// azimuth covers full 360':
	float az = fovy * M_PI * aspect * sx;
	float saz = sin(az);
	float caz = cos(az);
	
	// assumes standard OpenGL coordinate frame
	// -Z forward, Y up, X right
	out.x =  y0*saz;
	out.y =  y1;
	out.z = -y0*caz;
	out.normalize();
	
	value[3] = 1.;
}

static void fillRect(float * value, double normx, double normy) {
	Vec3f& out = *(Vec3f *)value;
	
	// move (0,0) to center of texture:
	float sx = normx - 0.5;
	float sy = normy - 0.5;
	
	float f = 1./tan(fovy * 0.5);
	
	// assumes standard OpenGL coordinate frame
	// -Z forward, Y up, X right
	out.x =  f * sx * aspect;
	out.y =  f * sy;
	out.z = -1.;
	out.normalize();
	
	value[3] = 1.;
}

static void softEdge(uint8_t * value, double normx, double normy) {
	static const double mult = 20;
	
	// fade out at edges:
	value[0] = 255. * sin(M_PI_2 * al::min(1., mult*(0.5 - fabs(normx-0.5)))) * sin(M_PI_2 * al::min(1., mult*(0.5 - fabs(normy-0.5))));
}

#pragma mark Projection

RayStereo::Projection::Projection()
:	mViewport(0, 0, 1, 1) {
	
	// allocate blend map:
	mBlend.resize(128, 128)
  .target(Texture::TEXTURE_2D)
  .format(Graphics::LUMINANCE)
  .type(Graphics::UBYTE)
  .filterMin(Texture::LINEAR)
  .allocate();
  
  
	// allocate warp map:
	mWarp.resize(256, 256)
  .target(Texture::TEXTURE_2D)
  .format(Graphics::RGBA)
  .type(Graphics::FLOAT)
  .filterMin(Texture::LINEAR)
  .allocate();
  
	t = u = v = 0;
}

void RayStereo::Projection::onCreate() {
	mWarp.filterMin(Texture::LINEAR_MIPMAP_LINEAR);
	mWarp.filterMag(Texture::LINEAR);
	mWarp.texelFormat(GL_RGB32F_ARB);
	mWarp.dirty();
	
	mBlend.filterMin(Texture::LINEAR_MIPMAP_LINEAR);
	mBlend.filterMag(Texture::LINEAR);
	mBlend.dirty();
}

void RayStereo::Projection::readBlend(std::string path) {
	Image img(path);
	mBlend.allocate(img.array(), true);
	printf("read & allocated %s\n", path.c_str());
}

void RayStereo::Projection::readWarp(std::string path) {
	File f(path, "rb");
	if (!f.open()) {
		printf("failed to open file %s\n", path.c_str());
		exit(-1);
	}
	
	if (t) free(t);
	if (u) free(u);
	if (v) free(v);
	
	int32_t dim[2];
	f.read((void *)dim, sizeof(int32_t), 2);
	
	int32_t w = dim[1];
	int32_t h = dim[0]/3;
	
	printf("warp dim %dx%d\n", w, h);
	
	int32_t elems = w*h;
	t = (float *)malloc(sizeof(float) * elems);
	u = (float *)malloc(sizeof(float) * elems);
	v = (float *)malloc(sizeof(float) * elems);
	
	int r = 0;
	r = f.read((void *)t, sizeof(float), elems);
	r = f.read((void *)u, sizeof(float), elems);
	r = f.read((void *)v, sizeof(float), elems);
	f.close();
	
	mWarp.resize(w, h)
  .target(Texture::TEXTURE_2D)
  .format(Graphics::RGBA)
  .type(Graphics::FLOAT)
  .filterMin(Texture::LINEAR)
  .allocate();
  
  updatedWarp();
  
	printf("read %s\n", path.c_str());
}

void RayStereo::Projection::updatedWarp() {
	Array& arr = mWarp.array();
	int w = arr.width();
	int h = arr.height();
	for (int y=0; y<h; y++) {
		for (int x=0; x<w; x++) {
			// Y axis appears to be inverted
			int32_t y1 = (h-y-1);
			// input data is row-major format
			int32_t idx = y1*w+x;
      
			float * cell = arr.cell<float>(x, y);
			Vec3f& out = *(Vec3f *)cell;
			
      //			// coordinate system change?
      //			out.x = v[idx];
      //			out.y = u[idx];
      //			out.z = -t[idx];
      
      // Matt negates x as an expedient: pablo undoes
			out.x = t[idx];
			out.y = u[idx];
			out.z = v[idx];
			
			// TODO:
			// out -= mRegistration.pos();
			// // & unrotate by mRegistration.quat()
			// do not normalize; instead capsule fit
			
			// normalize here so the shaders don't have to
			//out.normalize();
			
			// fourth element is currently unused:
			cell[3] = 1.;
			
			if (y == 32 && x == 32) {
				printf("example: %f %f %f -> %f %f %f\n",
               t[idx], u[idx], v[idx],
               cell[0], cell[1], cell[2]);
			}
		}
	}
	
	mWarp.dirty();
	
	printf("updated Warp\n");
}

#pragma mark RayStereo

RayStereo::RayStereo(unsigned resolution)
:	mResolution(resolution),
mNumProjections(1),
mFrame(0),
mMode(MONO),
mAnaglyphMode(RED_CYAN),
mFullScreen(false),
mActivePossible(false)
{
	mFbo = mRbo = 0;
	mTex[0] = mTex[1] = 0;
	
	mQuad.reset();
	mQuad.primitive(gl.TRIANGLE_STRIP);
	mQuad.texCoord(0, 0);
	mQuad.vertex	(0, 0, 0);
	mQuad.texCoord(1, 0);
	mQuad.vertex	(1, 0, 0);
	mQuad.texCoord(0, 1);
	mQuad.vertex	(0, 1, 0);
	mQuad.texCoord(1, 1);
	mQuad.vertex	(1, 1, 0);
	
  //	generateConfig(FISHEYE).generateConfig(SOFTEDGE);
  generateConfig(RECT).generateConfig(NOBLEND);
}

RayStereo& RayStereo::resolution(unsigned r) {
	mResolution = r;
	// force GPU reallocation:
	mFbo = mRbo = 0;
	mTex[0] = mTex[1] = 0;
	return *this;
	
}

RayStereo& RayStereo::mode(StereoMode m) {
  if (m == ACTIVE) {
    if (mActivePossible == true)
      mMode = m;
    else
      printf("active stereo not possible\n");
  } else {
    mMode = m;
  }
  return *this;
}

RayStereo& RayStereo::generateConfig(WarpMode wm, float a, float f) {
	mNumProjections = 1;
	Projection& p = mProjections[0];
	switch (wm) {
		case FISHEYE:
			fovy = f;
			aspect = a;
			p.warp().array().fill(fillFishEye);
			p.warp().dirty();
			break;
		case CYLINDER:
			fovy = f / M_PI;
			aspect = a;
			p.warp().array().fill(fillCylinder);
			p.warp().dirty();
			break;
		default:
			fovy = f / 2.;
			aspect = a;
			p.warp().array().fill(fillRect);
			p.warp().dirty();
			break;
	}
	return *this;
}

RayStereo& RayStereo::generateConfig(BlendMode bm) {
	mNumProjections = 1;
	Projection& p = mProjections[0];
	switch (bm) {
		case SOFTEDGE:
			p.blend().array().fill(softEdge);
			p.blend().dirty();
			break;
		default:
			// default blend of 1:
			uint8_t white = 255;
			p.blend().array().set2d(&white);
			p.blend().dirty();
			break;
	}
	return *this;
}

RayStereo& RayStereo::loadConfig(std::string configpath, std::string configname) {
  if (configpath == "") {
    #ifdef AL_WINDOWS
    // Calibration files should be kept at the root of the C: drive on Windows.
    configpath = "C:";
    #else
    // A bit of C trickery to get the absolute path of the home directory on *nix systems.
    FILE *pipe = popen("echo ~", "r");
    if (pipe) {
      char c;
      while((c = getc(pipe)) != EOF) {
        if (c == '\r' || c == '\n')
          break;
        configpath += c;
      }
      pclose(pipe);
    }
    #endif
    
    configpath += "/calibration-current";
  }
  
  std::string config_file_name = configpath + "/" + configname + ".json";
  
  static std::ifstream config_file(config_file_name.c_str(), std::ifstream::binary | std::ifstream::in);
  if (config_file.fail()){
    std::cout  << " Failed to find configuration file: " << configname << ".json" << std::endl;
    std::cout << " Using default configuration file." << std::endl;
    return *this;
  } else {
    config_file >> config;
    config_file.close();
  }
  
  if ( config.empty() ){
    std::cout  << " Failed to parse configuration file: " << configname << ".json" << std::endl;
    std::cout << errors << std::endl;
    std::cout << " Using default configuration file." << std::endl;
    return *this;
  } else {
    std::cout << " Parsed configuration file: " << configname << ".json" << std::endl;
  }
  
  if ( config["active"].asBool() ) {
    mMode = ACTIVE;
    mActivePossible = true;
  }
  
  if ( config["fullscreen"].asBool() ) {
    mFullScreen = true;
  }
  
  if ( config["resolution"].isUInt() ) {
    resolution(config["resolution"].asUInt());
  }
  
  mNumProjections = config["projections"].size();
  printf("Found %d viewports.\n", mNumProjections);
  
  for (unsigned i=0; i<mNumProjections; i++) {
    Json::Value& projection = config["projections"][i];
    Projection& mprojection = mProjections[i];
    
    Json::Value& viewport = projection["viewport"];
    if ( ! viewport.isNull() ) {
      mprojection.viewport().l = viewport["l"].asFloat();
      mprojection.viewport().b = viewport["b"].asFloat();
      mprojection.viewport().w = viewport["w"].asFloat();
      mprojection.viewport().h = viewport["h"].asFloat();
    }
    
    Json::Value& warp = projection["warp"];
    if ( ! warp.isNull() ) {
      if ( warp["file"].isString() ) {
        mprojection.readWarp( configpath + "/" + warp["file"].asString() );
      } else {
        mprojection.warp().array().fill(fillRect);
        mprojection.warp().dirty();
      }
    }
    
    Json::Value& blend = projection["blend"];
    if ( ! blend.isNull() ) {
      if ( blend["file"].isString() ) {
        mprojection.readBlend( configpath + "/" + blend["file"].asString() );
      } else {
        // default blend of 1:
        uint8_t white = 255;
        mprojection.blend().array().set2d(&white);
        mprojection.blend().dirty();
      }
    }
  }
  
  return *this;
 }

void RayStereo::createTexture(GLuint* id_tex) {

  // create texture:
  glGenTextures(1, id_tex);

  glBindTexture(GL_TEXTURE_2D, *id_tex);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  
  // filtering
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  
  // TODO: verify?
  // Domagoj also has:
  glTexGeni( GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR );
  glTexGeni( GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR );
  glTexGeni( GL_R, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR );
  float X[4] = { 1,0,0,0 };
  float Y[4] = { 0,1,0,0 };
  float Z[4] = { 0,0,1,0 };
  glTexGenfv( GL_S, GL_OBJECT_PLANE, X );
  glTexGenfv( GL_T, GL_OBJECT_PLANE, Y );
  glTexGenfv( GL_R, GL_OBJECT_PLANE, Z );
  
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, mResolution, mResolution, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
  
  // clean up:
  glBindTexture(GL_TEXTURE_2D, 0);
  Graphics::error("creating texture");
}

void RayStereo::bindTexToFBO(GLuint id_tex) {
  glBindFramebuffer(GL_FRAMEBUFFER, mFbo);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, id_tex, 0);
  // glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RayStereo::bindDepthTexToFBO(GLuint id_tex) {
  glBindFramebuffer(GL_FRAMEBUFFER, mFbo);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, id_tex, 0);
  // glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RayStereo::onCreate() {
	// force allocation of warp/blend textures:
	for (unsigned i=0; i<4; i++) {
		mProjections[i].onCreate();
	}

	// create textures:
	glGenTextures(2, mTex);
  for (int i=0; i<2; i++) {
    glBindTexture(GL_TEXTURE_2D, mTex[i]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    
    // filtering
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		
		// TODO: verify?
		// Domagoj also has:
		glTexGeni( GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR );
		glTexGeni( GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR );
		glTexGeni( GL_R, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR );
		float X[4] = { 1,0,0,0 };
		float Y[4] = { 0,1,0,0 };
		float Z[4] = { 0,0,1,0 };
		glTexGenfv( GL_S, GL_OBJECT_PLANE, X );
		glTexGenfv( GL_T, GL_OBJECT_PLANE, Y );
		glTexGenfv( GL_R, GL_OBJECT_PLANE, Z );
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, mResolution, mResolution, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
    
		// clean up:
		glBindTexture(GL_TEXTURE_2D, 0);
		Graphics::error("creating base texture");
  }
	
	// one FBO to rule them all...
	glGenFramebuffers(1, &mFbo);
	glBindFramebuffer(GL_FRAMEBUFFER, mFbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTex[0], 0);
	
	glGenRenderbuffers(1, &mRbo);
	glBindRenderbuffer(GL_RENDERBUFFER, mRbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mResolution, mResolution);
	// Attach depth buffer to FBO
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mRbo);
	
	//Does the GPU support current FBO configuration?
	GLenum status;
	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		printf("GPU does not support required FBO configuration\n");
		exit(0);
	}
	
	// cleanup:
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	Graphics::error("RayStereo onCreate");
}

void RayStereo::onDestroy() {
	glDeleteTextures(2, mTex);
	mTex[0] = mTex[1] = 0;
	
	glDeleteRenderbuffers(1, &mRbo);
	glDeleteFramebuffers(1, &mFbo);
	mRbo = mFbo = 0;
}

inline void RayStereo::drawQuad(const ShaderProgram* shaderProgram, const double eye) {
	shaderProgram->uniform("eyesep", eye);
  gl.draw(mQuad);
}

void RayStereo::draw(const ShaderProgram* shaderProgram, const Lens& lens, const Viewport& vp) {
	mFrame++;
	
	gl.viewport(vp);
	gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
	for (int i=0; i<numProjections(); i++) {
		Projection& p = projection(i);
		Viewport& v = p.viewport();
		Viewport viewport(
                      vp.l + v.l * vp.w,
                      vp.b + v.b * vp.h,
                      v.w * vp.w,
                      v.h * vp.h
                      );
		gl.viewport(viewport);
		gl.clear(Graphics::COLOR_BUFFER_BIT | Graphics::DEPTH_BUFFER_BIT);
		p.blend().bind(2);
		p.warp().bind(1);
		
    double eye = lens.eyeSep();

		switch (mMode) {
      case SEQUENTIAL:
        if (mFrame & 1) {
          drawQuad(shaderProgram, eye);
        } else {
          drawQuad(shaderProgram, -eye);
        }
        break;
        
      case ACTIVE:
        glDrawBuffer(GL_BACK_RIGHT);
        gl.error("RayStereo drawStereo GL_BACK_RIGHT");
        drawQuad(shaderProgram, eye);
        
        glDrawBuffer(GL_BACK_LEFT);
        gl.error("RayStereo drawStereo GL_BACK_LEFT");
        drawQuad(shaderProgram, -eye);
        
        glDrawBuffer(GL_BACK);
        gl.error("RayStereo drawStereo GL_BACK");
        break;
        
      case DUAL:
        gl.viewport(vp.l + vp.w*0.5, vp.b, vp.w*0.5, vp.h);
        drawQuad(shaderProgram, eye);
        gl.viewport(vp.l, vp.b, vp.w*0.5, vp.h);
        drawQuad(shaderProgram, -eye);
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
        drawQuad(shaderProgram, eye);
        
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
        drawQuad(shaderProgram, -eye);
        
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        break;
        
      case RIGHT_EYE:
        drawQuad(shaderProgram, eye);
        break;
        
      case LEFT_EYE:
        drawQuad(shaderProgram, -eye);
        break;
        
      case MONO:
      default:
        drawQuad(shaderProgram, 0);
        break;
    }
		
		p.blend().unbind(2);
		p.warp().unbind(1);
	}
}

