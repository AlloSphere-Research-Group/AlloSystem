//
//  al_OmniStereo2.cpp
//  
//
//  Created by Graham Wakefield, Karl Yerkes, Pablo Colapinto on 3/5/13.
//  
//

#include <iostream>


namespace al {

    OmniStereo::OmniStereo(unsigned resolution, bool useMipMaps)
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

    OmniStereo& OmniStereo::resolution(unsigned r) {
        mResolution = r;
        
        // force GPU reallocation:	
        mFbo = mRbo = 0;
        mTex[0] = mTex[1] = 0;
        return *this;
    }

    void OmniStereo::onCreate() {

        // force allocation of warp/blend textures:
        for (unsigned i=0; i<4; i++) {
            mProjection[i].onCreate();
        }
        
            compile();
            
            generate();
    }


    void OmniStereo::compile(){
        
        Shader cubeV, cubeF;
        cubeV.source(vGeneric, Shader::VERTEX).compile();
        cubeF.source(fCube, Shader::FRAGMENT).compile();
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
    }

    // create cubemap textures:
    void OmniStereo::generate(){
    
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
                    0, GL_RGBA8, GL_UNSIGNED_BYTE, 
                    NULL);
            }
            
            // clean up:
            glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
            Graphics::error("creating cubemap texture");
        }
        
        /////////////////////////////////////////////////////////////////////////////////
        /// ATTACH TEXTURES TO FRAMEBUFFER (Which will be BOUND during CAPTURE method) //
        /////////////////////////////////////////////////////////////////////////////////
        
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
    
    void OmniStereo::capture(OmniStereo::Drawable& drawable, const Lens& lens, const Pose& pose) {
        
        if (mCubeProgram.id() == 0) onCreate();
        
        gl.error("OmniStereo capture begin");
        
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
                        //SPECIFY TEXTURE
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
    void OmniStereo::mipmap(){
        
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
    
    
}