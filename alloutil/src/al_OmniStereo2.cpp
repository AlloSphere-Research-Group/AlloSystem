//
//  al_OmniStereo2.cpp
//  
//
//  Created by Graham Wakefield, Karl Yerkes, Pablo Colapinto on 3/5/13.
//  
//

#include <iostream>

#include "allocore/io/al_Socket.hpp" // for hostname
#include "alloutil/al_Lua.hpp" // for hostname
#include "allocore/graphics/al_Image.hpp"
#include "allocore/graphics/al_Shader.hpp"
#include "allocore/io/al_File.hpp"


#include "alloutil/al_OmniStereo2.hpp"

static al::Lua L;

namespace al {

    OmniStereo2::OmniStereo2(unsigned resolution, bool useMipMaps)
    :	mFace(5),
        mEyeParallax(0),
        mNear(0.1),
        mFar(100),
        mResolution(resolution),
        mNumProjections(1),
        mFrame(0),
        mMode(MONO),
        mStereo(0),
        mAnaglyphMode(RED_CYAN),
        mMipmap(useMipMaps),
        mFullScreen(false)
    {
        mFbo = mRbo = 0;
        mTex[0] = mTex[1] = 0;
        
        mQuad.reset();
        mQuad.primitive(gl.TRIANGLE_STRIP);
        mQuad.texCoord	( 0, 0);
        mQuad.vertex	( 0, 0, 0);
        mQuad.texCoord	( 1, 0);
        mQuad.vertex	( 1, 0, 0);
        mQuad.texCoord	( 0, 1);
        mQuad.vertex	( 0, 1, 0);
        mQuad.texCoord	( 1, 1);
        mQuad.vertex	( 1, 1, 0);
        
        mClearColor.set(0.);
        
        configure(FISHEYE).configure(SOFTEDGE);
    }

    OmniStereo2& OmniStereo2::resolution(unsigned r) {
        mResolution = r;
        
        // force GPU reallocation:	
        mFbo = mRbo = 0;
        mTex[0] = mTex[1] = 0;
        return *this;
    }

    void OmniStereo2::onCreate() {

        // force allocation of warp/blend textures:
        for (unsigned i=0; i<4; i++) {
            mProjections[i].onCreate();
        }
        
            compile();
            
            generate();
    }


    // Compile Shaders That Use Warp Data or Sphereical Coordinates to Index CubeMap
    // Note: Perhaps this should be in its own class (i.e. a CubeMap class)

    void OmniStereo2::compile(){
        
        
        //USE IN ALLOSPHERE
        Shader cubeV, cubeF;
        cubeV.source(GLSL::vGeneric, Shader::VERTEX).compile();
        cubeF.source(GLSL::fCube, Shader::FRAGMENT).compile();
        mCubeProgram.attach(cubeV).attach(cubeF);
        mCubeProgram.link(false);	// false means do not validate
        // set uniforms before validating to prevent validation error
        mCubeProgram.begin();
            mCubeProgram.uniform("alphaMap", 2);
            mCubeProgram.uniform("pixelMap", 1);
            mCubeProgram.uniform("cubeMap", 0);
        mCubeProgram.end();
        mCubeProgram.validate();
        cubeV.printLog();
        cubeF.printLog();
        mCubeProgram.printLog();
        Graphics::error("cube program onCreate");
        
        //USE ON DESKTOP (UNWRAPS SPHERE TO 2D IMAGE PLANE)
        Shader sphereV, sphereF;
        sphereV.source(GLSL::vGeneric, Shader::VERTEX).compile();
        sphereF.source(GLSL::fSphere, Shader::FRAGMENT).compile();
        mSphereProgram.attach(sphereV).attach(sphereF);
        mSphereProgram.link(false);	// false means do not validate
        // set uniforms before validating to prevent validation error
        mSphereProgram.begin();
            mSphereProgram.uniform("alphaMap", 2);
            mSphereProgram.uniform("pixelMap", 1);
            mSphereProgram.uniform("sphereMap", 0);
        mSphereProgram.end();
        mSphereProgram.validate();
        sphereV.printLog();
        sphereF.printLog();
        mSphereProgram.printLog();
        Graphics::error("sphere program onCreate");
    }


    // CREATE cubemap textures:
    //Note: Move to CubeMap class?
    void OmniStereo2::generate(){
    
        glGenTextures(2, mTex);

        for (int i=0; i<2; i++) {
            // create cubemap texture:
            glBindTexture(GL_TEXTURE_CUBE_MAP, mTex[i]);
            
            // each cube face should clamp at texture edges:
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
            
            // filtering
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            
            // TODO: verify? 
            // Domagoj also has:
            // but these auto texcoord generators are only applied if enabled right?. . .

//            glTexGeni( GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR );
//            glTexGeni( GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR );
//            glTexGeni( GL_R, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR );
//            float X[4] = { 1,0,0,0 };
//            float Y[4] = { 0,1,0,0 };
//            float Z[4] = { 0,0,1,0 };
//            glTexGenfv( GL_S, GL_OBJECT_PLANE, X );
//            glTexGenfv( GL_T, GL_OBJECT_PLANE, Y );
//            glTexGenfv( GL_R, GL_OBJECT_PLANE, Z );


            //NOTE SWITCH GL_BGRA??
            // RGBA8 Cubemap texture, 24 bit depth texture, mResolution x mResolution
            // NULL means reserve texture memory, but texels are undefined
            for (int f=0; f<6; f++) {
                glTexImage2D(
                    GL_TEXTURE_CUBE_MAP_POSITIVE_X+f, 
                    0, GL_RGBA8, 
                    mResolution, mResolution, 
                    0, GL_BGRA, GL_UNSIGNED_BYTE, 
                    NULL);
            }
            
            // clean up:
            glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
            Graphics::error("creating cubemap texture");
        }
        
        /////////////////////////////////////////////////////////////////////////////////
        /// ATTACH TEXTURES TO FRAMEBUFFER (Which will be BOUND during CAPTURE method) //
        /////////////////////////////////////////////////////////////////////////////////
        
        //Note: Encapsulate. FBOs could be their own class
        
        // one FBO to rule them all...
        glGenFramebuffers(1, &mFbo);
        glBindFramebuffer(GL_FRAMEBUFFER, mFbo);
        //Attach one of the faces of the Cubemap texture to this FBO
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X, mTex[0], 0);
        
        glGenRenderbuffers(1, &mRbo);
        glBindRenderbuffer(GL_RENDERBUFFER, mRbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mResolution, mResolution);
        // Attach depth buffer to FBO
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mRbo);
        
        // ...and in the darkness bind them:
        for (mFace=0; mFace<6; mFace++) {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+mFace, GL_TEXTURE_CUBE_MAP_POSITIVE_X+mFace, mTex[0], 0);
        }
        
        //Does the GPU support current FBO configuration?
        GLenum status;
        status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE) {
            printf("GPU does not support required FBO configuration\n");
            exit(0);
        }
        
        // cleanup and UNBIND
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        
        Graphics::error("OmniStereo Generate Cubemap");
    }
    
    
    ////////////////////////
    // CAPTURE TO CUBEMAP //
    ////////////////////////
    
    void OmniStereo2::capture(OmniStereo2::Drawable& drawable, const Lens& lens, const Pose& pose) {
        
        if (mCubeProgram.id() == 0) onCreate();
        
        gl.error("OmniStereo2 capture begin");
        
        // LENS INFO
        mNear = lens.near();
        mFar = lens.far();
        const double eyeSep = mStereo ? lens.eyeSep() : 0.;

        //PROJECTION IS IDENTITY
        gl.projection(Matrix4d::identity());

        //MODELVIEW      
        Vec3d pos = pose.pos();
        Vec3d ux, uy, uz; 
        pose.unitVectors(ux, uy, uz);
        mModelView = Matrix4d::lookAt(ux, uy, uz, pos);
        
        gl.pushMatrix(gl.MODELVIEW);
        gl.loadMatrix(mModelView);
        
            //Push Attributes
            glPushAttrib(GL_ALL_ATTRIB_BITS);
                
                //Bind Framebuffer
                glBindFramebuffer(GL_FRAMEBUFFER, mFbo);
                gl.viewport(0, 0, mResolution, mResolution);
                
                //PER EYE
                for (int i=0; i<(mStereo+1); i++) {
                    mEyeParallax = eyeSep * (i-0.5);
                    
                    //PER CUBE FACE
                    for (mFace=0; mFace<6; mFace++) {
                        
                        //SPECIFY COLOR BUFFER
                        glDrawBuffer(GL_COLOR_ATTACHMENT0 + mFace);
                        //SPECIFY TEXTURE (again?)
                        glFramebufferTexture2D(
                            GL_FRAMEBUFFER, 
                            GL_COLOR_ATTACHMENT0 + mFace, 
                            GL_TEXTURE_CUBE_MAP_POSITIVE_X + mFace, 
                            mTex[i], 0);
                        
                        gl.clearColor(mClearColor);
                        gl.depthTesting(1);
                        gl.depthMask(1);
                        gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
                        
                        //DRAW SCENE TO BOUND CUBEMAP FACE (mFace gets passed as uniform to shader)
                        drawable.onDrawOmni(*this);		
                    }
                }
                
                 /// Unbind Framebuffer
                glBindFramebuffer(GL_FRAMEBUFFER, 0);                  

            glPopAttrib();

        gl.popMatrix(gl.MODELVIEW);

        gl.error("OmniStereo capture end");
        
        if (mMipmap) mipmap();

    }	
    
    // FBOs don't generate mipmaps by default; do it here:
    void OmniStereo2::mipmap(){
        
        gl.error("OmniStereo FBO mipmap begin");
        
        glActiveTexture(GL_TEXTURE0);
        glEnable(GL_TEXTURE_CUBE_MAP);
        
        glBindTexture(GL_TEXTURE_CUBE_MAP, mTex[0]);
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
        gl.error("generating mipmap");
        
        if (mStereo) {
            glBindTexture(GL_TEXTURE_CUBE_MAP, mTex[1]);
            glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
            gl.error("generating mipmap");
        }
        
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
        glDisable(GL_TEXTURE_CUBE_MAP);
       
        gl.error("OmniStereo FBO mipmap end");
    
    }

    ////////////////////////////////////////
    // Draw Bound Cubemap Using Warp Data //
    ////////////////////////////////////////
    
    void OmniStereo2::draw(const Lens& lens, const Pose& pose, const Viewport& vp) {
        mFrame++;
        if (mCubeProgram.id() == 0) onCreate();
        
        gl.error("OmniStereo draw begin");
        
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
        
            gl.error("OmniStereo cube draw begin");
            
            mCubeProgram.begin();
            glActiveTexture(GL_TEXTURE0);
            glEnable(GL_TEXTURE_CUBE_MAP);	
            
            gl.error("OmniStereo cube drawStereo begin");
            
            drawStereo<&OmniStereo2::drawEye>(lens, pose, viewport);		
            
            gl.error("OmniStereo cube drawStereo end");
            
            glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
            glDisable(GL_TEXTURE_CUBE_MAP);
            
            mCubeProgram.end();
            gl.error("OmniStereo cube draw end");
            
            p.blend().unbind(2);
            p.warp().unbind(1);
        }
        gl.error("OmniStereo draw end");
    }
    
    void OmniStereo2::drawEye(const Pose& pose, double eye) {
        if (eye > 0.) {
            glBindTexture(GL_TEXTURE_CUBE_MAP, mTex[1]);
        } else {
            glBindTexture(GL_TEXTURE_CUBE_MAP, mTex[0]);
        }
        gl.error("OmniStereo drawEye after texture");
        gl.draw(mQuad);
        gl.error("OmniStereo drawEye after quad");
    }

    //////////////////////////////////////////////////////////////
    ////// DEFAULT CONFIGURATIONS USED IF NO WARP DATA PRESENT ///
    //////////////////////////////////////////////////////////////
    
    OmniStereo2& OmniStereo2::configure(WarpMode wm, float a, float f) {
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

OmniStereo2& OmniStereo2::configure(BlendMode bm) {
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

    
    /////////////////////////////////////////////////////////////////////////////////
    // LOAD PROJECTION CONFIGURATIONS FROM LUA FILE                  ////////////////
    /////////////////////////////////////////////////////////////////////////////////
    
    //Note: Messy -> Could this be encapsulated in a LuaLoader?
    
    OmniStereo2& OmniStereo2::configure(std::string configpath, std::string configname) {
    
    //NO LUA FILE?
	if (L.dofile(configpath + "/" + configname + ".lua", 0)) return *this;
	
    //NO PROJECTIONS?
	L.getglobal("projections");
	if (!lua_istable(L, -1)) {
		printf("config file %s has no projections\n", configpath.c_str());
		return *this;
	}
    
    
	int projections = L.top();
	
	// set active stereo
	lua_getfield(L, projections, "active");
	if (lua_toboolean(L, -1)) {
		mMode = ACTIVE;
	}
	L.pop(); //active
	
	// set fullscreen by default mode?
	lua_getfield(L, projections, "fullscreen");
	if (lua_toboolean(L, -1)) {
		mFullScreen = true;
	}
	L.pop(); // fullscreen
	
	// set resolution?
	lua_getfield(L, projections, "resolution");
	if (lua_isnumber(L, -1)) {
		resolution(lua_tonumber(L, -1));
	}
	L.pop(); // resolution
	
	mNumProjections = lua_objlen(L, projections);
	printf("found %d viewports\n", mNumProjections);
	
	for (unsigned i=0; i<mNumProjections; i++) {
		L.push(i+1);
		lua_gettable(L, projections);
		int projection = L.top();
		//L.dump("config");
		
		lua_getfield(L, projection, "viewport");
		if (lua_istable(L, -1)) {
			int viewport = L.top();
			lua_getfield(L, viewport, "l");
			mProjections[i].viewport().l = L.to<float>(-1);
			L.pop();
			
			lua_getfield(L, viewport, "b");
			mProjections[i].viewport().b = L.to<float>(-1);
			L.pop();
			
			lua_getfield(L, viewport, "w");
			mProjections[i].viewport().w = L.to<float>(-1);
			L.pop();
			
			lua_getfield(L, viewport, "h");
			mProjections[i].viewport().h = L.to<float>(-1);
			L.pop();
			
		}
		L.pop(); // viewport
		
		lua_getfield(L, projection, "warp");
		if (lua_istable(L, -1)) {
			int warp = L.top();
			
//			lua_getfield(L, warp, "width");
//			if (lua_isnumber(L, -1)) {
//				mProjections[i].warpData().width = lua_tonumber(L, -1);
//			}
//			L.pop();
//			
//			lua_getfield(L, warp, "height");
//			if (lua_isnumber(L, -1)) {
//				mProjections[i].warpData().height = lua_tonumber(L, -1);
//			}
//			L.pop();
			
			lua_getfield(L, warp, "file");
			if (lua_isstring(L, -1)) {
				// load from file
				mProjections[i].readWarp(configpath + "/" + lua_tostring(L, -1));
			}
			L.pop();
		}
		L.pop(); // warp
		
		lua_getfield(L, projection, "blend");
		if (lua_istable(L, -1)) {
			int blend = L.top();
			lua_getfield(L, blend, "file");
			if (lua_isstring(L, -1)) {
				// load from file
				mProjections[i].readBlend(configpath + "/" + lua_tostring(L, -1));
			} else {
				// TODO: generate blend...
			}
			L.pop();
		}
		L.pop(); // blend
		
		lua_getfield(L, projection, "params");
		if (lua_istable(L, -1)) {
			int params = L.top();
			lua_getfield(L, params, "file");
			if (lua_isstring(L, -1)) {
				// load from file
				mProjections[i].loadParameters(configpath + "/" + lua_tostring(L, -1)); //, true);
			} 
			L.pop();
		}
		L.pop(); // params
		
		lua_getfield(L, projection, "position");
		if (lua_istable(L, -1)) {
			int position = L.top();
			lua_rawgeti(L, position, 1);
			mProjections[i].position.x = L.to<double>(-1);
			L.pop();
			lua_rawgeti(L, position, 2);
			mProjections[i].position.y = L.to<double>(-1);
			L.pop();
			lua_rawgeti(L, position, 3);
			mProjections[i].position.z = L.to<double>(-1);
			L.pop();			
		}
		L.pop(); // position

		
		L.pop(); // projector
	}
	
	L.pop(); // the projections table
	return *this;
}

}