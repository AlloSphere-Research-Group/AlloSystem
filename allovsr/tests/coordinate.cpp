//
//  coordinate.cpp
//  allovsr
//

/*

Handedness Sanity Check

*/
//  Created by Pablo Colapinto on 3/4/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//


//vsr Includes
#include "vsr/vsr.h"
#include "vsr/vsr_op.h"
#include "vsr/vsr_draw.h"
#include "vsr/vsr_frame.h"
#include "vsr/vsr_motor.h"
#include "vsr/vsr_mesh.h"

//allo includes
#include "allocore/al_Allocore.hpp"
#include "allocore/io/al_ControlNav.hpp"
#include "alloGLV/al_controlGLV.hpp"

//Glue
#include "allovsr/al_vsrInterface.hpp"
#include <iostream>

#include <iostream>


using std::cout;
using std::endl;

using namespace al;
using namespace vsr;

#define PRESET \
static bool bSet = false;\
if(!bSet){ bSet = true;

#define ENDSET }

class MyApp;

void poseTest(MyApp& app){

}


struct warpData{

    float * t;
    float * u;
    float * v;
    
    int32_t num, width, height;
    
    Vector at(int idx) { return Vector(t[idx],u[idx],v[idx]); }

    void readWarp(std::string path) {

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

struct Projection{
    warpData warp;
    Texture3D tex;
}

struct MyApp : public al::VsrApp {


    MyApp() : al::VsrApp() { 
    
//        stereo.stereo(true);
//        stereo.mode( Stereographic::DUAL );
    
    }

    virtual void onDraw(Graphics& gl){
    
        //Model Transform
        Rot t = Gen::aa( scene().model.rot() ); 
        GL::rotate( t.w() );
    
        static warpData wd[12];
        static vector<int> c;
        
        static UVMesh uv[12];
        
        static double vw, vh;
        
        PRESET
        
            glv.gui(vw)(vh);
            
            for (int i = 1; i <= 12; ++i){
                stringstream os; os << "map3D"<<i<<".bin";
                wd[i-1].readWarp(os.str());
            }
            
            
             for (int i = 0; i < 12; ++i){
                
                int w = wd[i].width;
                int h = wd[i].height;
                
                int sw = 20; // segments of width
                int sh = 20; // segments of height
                
                int wspacing = 1.0 * w / sw;
                int hspacing = 1.0 * h / sh;

                for (int j = 0; j < sh; ++j){
                    int xw = j * hspacing;
                    for (int k = 0; k < sw; ++k){
                        int xh = k * wspacing;
                        int ix = xw * w + xh;
                        uv[i].add( wd[i].at(ix) );
                    }
                }
                
                uv[i].u = sw;
                uv[i].v = sh;
            }
             
        ENDSET
    
        int idx;
        stringstream os; os << interface.keyboard.code;
        os >> idx;
        idx -= 48;
    
        for (int i = 0; i < 12; ++i){
                      
            if (idx == i+1) uv[i].draw(1,0,0);
            else uv[i].draw(.3,.3,.3);
        
        }
    }
    
    
};

MyApp app;

int main(int argc, const char * argv[]){

    cout << argv[0] << endl; 

    app.create(Window::Dim(800, 600), "Allovsr Test: Coordinate System");
	
    MainLoop::start();
    
	return 0;

}