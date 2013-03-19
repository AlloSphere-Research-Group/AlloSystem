//
//  al_Projection.hpp
//  allovsr
//
//  Created by Pablo Colapinto on 3/5/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#ifndef allovsr_al_Projection_hpp
#define allovsr_al_Projection_hpp


/*

1. Loads Projector Parameters From Configuration File
2. Load Warp and Blend Data, and Saves Data to Textures

*/


#include "allocore/al_Allocore.hpp"


namespace al {

    struct Projection {

        //Do not modify order of Parameters -- data is read in from file!
		struct Parameters {
			float projnum;			// ID of the projector
			float width, height;	// width/height in pixels
			Vec3f projector_position, screen_center, normal_unit, x_vec, y_vec;
			float screen_radius, bridge_radius, unused0;
		};

        //Load Parameters 
        void loadParameters(std::string path){
        
            File f(path, "rb");
            if (!f.open()) {
                printf("failed to open Projector configuration file %s\n", path.c_str());
                return;
            }

            f.read((void *)(&mParam), sizeof( Projection::Parameters ), 1);
            f.close();

            initParameters();
            
            printf("read %s\n", path.c_str());
        }

        void initParameters() {
        
            Parameters& p = mParam;
            // initialize:
            Vec3f v = p.screen_center - p.projector_position;
            float screen_perpendicular_dist = p.normal_unit.dot(v);
            Vec3f compensated_center = (v) / screen_perpendicular_dist + p.projector_position;
            Vec3f cv = compensated_center - p.projector_position;
            
            // calculate uv parameters
            float x_dist = p.x_vec.mag();
            float y_dist = p.y_vec.mag();
            
            x_unit = p.x_vec / x_dist;
            x_pixel = x_dist / p.width;
            x_offset = x_unit.dot(cv);
            
            y_unit = p.y_vec / y_dist;
            y_pixel = y_dist / p.height;
            y_offset = y_unit.dot( cv );
        }

    
        struct WarpData{
            
            float * t;
            float * u;
            float * v;
            
            int32_t num, width, height;

            WarpData(){}
            WarpData(std::string path) { load(path); }
            
            Vec3f at(int idx) { return Vec3f(t[idx],u[idx],v[idx]); }

            //Loads Warp Data
            void load(std::string path) {

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
                
                //Allocate
                int32_t elems = w*h;
                width = w;
                height = h;
                num = elems;
                
                t = (float *)malloc(sizeof(float) * elems);
                u = (float *)malloc(sizeof(float) * elems);
                v = (float *)malloc(sizeof(float) * elems);
                
                //Fill
                int r = 0;	
                r = f.read((void *)t, sizeof(float), elems);
                r = f.read((void *)u, sizeof(float), elems);
                r = f.read((void *)v, sizeof(float), elems);
                f.close();
                
                printf("read %s\n", path.c_str());
            }
        };
        
        
        Projection() 
        :	mViewport(0, 0, 1, 1) {
	
            // allocate blend map:
            mBlendTex.resize(128, 128)
                .target(Texture::TEXTURE_2D)
                .format(Graphics::LUMINANCE)
                .type(Graphics::UBYTE)
                .filterMin(Texture::LINEAR)
                .allocate();
                

            // allocate warp map:
            mWarpTex.resize(256, 256)
                .target(Texture::TEXTURE_2D)
                .format(Graphics::RGBA)
                .type(Graphics::FLOAT)
                .filterMin(Texture::LINEAR)
                .allocate();
                
            mWarpData.t = mWarpData.u = mWarpData.v = 0;
        }
        
        void onCreate(){
            //DEFAULTS USED IF CONFIGURATION FILES ARE NOT FOUND
            mWarpTex.filterMin(Texture::LINEAR_MIPMAP_LINEAR);
            mWarpTex.filterMag(Texture::LINEAR);
            mWarpTex.texelFormat(GL_RGB32F_ARB);
            mWarpTex.dirty();
            
            mBlendTex.filterMin(Texture::LINEAR_MIPMAP_LINEAR);
            mBlendTex.filterMag(Texture::LINEAR);
            mBlendTex.dirty();

        }

    
        void readWarp(std::string path){
            
            //Load Warp Data
//            stringstream os; os << "map3D"<<mParam.projnum<<".bin";            
            mWarpData.load( path );

            //Save to Texture
            mWarpTex.resize( mWarpData.width, mWarpData.height)
            		.target(Texture::TEXTURE_2D)
                    .format(Graphics::RGBA)
                    .texelFormat(GL_RGB32F_ARB)
                    .type(Graphics::FLOAT)
                    .filterMin(Texture::LINEAR)
                    .allocate();
        }
        
        void readBlend(std::string path){
            
            //Load Blend Alpha Data
            Image img(path);
            
            //Save to Texture
            mBlendTex.allocate(img.array(), true);
            printf("read & allocated %s\n", path.c_str());
        }
    
        ///////////////////        
        // MEMBERS      ///
        ///////////////////
        
        Viewport& viewport() { return mViewport; }
        Viewport viewport() const { return mViewport; }
        WarpData& warpData() { return mWarpData; }
        WarpData warpData() const { return mWarpData; }

        Texture& warp() { return mWarpTex; }
        Texture& blend() { return mBlendTex; }
        Texture warp() const { return mWarpTex; }
        Texture blend() const { return mBlendTex; }
        
        Vec3d position;

        Vec3f x_unit, y_unit;
		float x_pixel, y_pixel;
		float x_offset, y_offset;  
        
        //remapping experiments (flip z for reading left handed cubemap)   
        void flipZ(){
            Array& arr = mWarpTex.array();
            int w= arr.width();
            int h = arr.height();
            for (int y=0; y<h; y++) {
                for (int x=0; x<w; x++) {
                
                    float * cell = arr.cell<float>(x, y);
                    Vec3f& out = *(Vec3f *)cell;
                    out.z = -out.z;
                }
            }
            
            mWarpTex.dirty();
        }
        
        
        protected:
        
        Viewport mViewport;                 ///< Width, Height, Registration
        WarpData mWarpData;                 ///< Data from map3d.bin files
        Texture mWarpTex, mBlendTex;        ///< Textures of Data 


        Parameters mParam;
        
        
        
    };

}


#endif
