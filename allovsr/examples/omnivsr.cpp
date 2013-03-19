//
//  omnivsr.cpp
//  allovsr
//  

// A VERSOR controlled OMNI App

//  Created by Pablo Colapinto on 3/11/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include <iostream>


//
//  twists.cpp
//  allovsr
//
//  Created by Pablo Colapinto on 12/3/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//


//vsr Includes
#include "vsr/vsr.h"
#include "vsr/vsr_op.h"
#include "vsr/vsr_draw.h"
#include "vsr/vsr_frame.h"
//#include "vsr/vsr_field.h"

//allocore includes
#include "allocore/al_Allocore.hpp"
#include "allocore/io/al_ControlNav.hpp"

//alloGLV
#include "alloGLV/al_ControlGLV.hpp"

//allovsr includes
#include "allovsr/al_omnivsr.hpp"

#include <iostream>



using std::cout;
using std::endl;

using namespace al;
using namespace vsr;

#define PRESET \
static bool bSet = false;\
if(!bSet){ bSet = true;

#define ENDSET }

//CUBE MAP SANITY CHECK!
void cmcheck(){
    double t = 3;
    //X Direction Facing a Sphere
    DRAW( Ro::dls(t,0.0,0.0) );
    //NX Direction Facing a Circle
    DRAW( CYZ(1).trs(-t,0,0) );
    //Y Direction Facing A Small Red Circle
    DRAW3( CXZ(.3).trs(0,t,0), 1,0,0 );
    //NY Direction Facing A Pair of Green Points 
    DRAW3( PAIR(1,0,0).trs(0,-t,0), 0,1,0 );
    //Z Direction an Arrow Pointing to WORLD SPACE -X, Y (tricky one . . .)
    GL::push(); GL::translate(0.,0.,t); DRAW( Vector(-1,1,0) ) ; GL::pop();
    //NZ Direction a Plane (FRONT)
    DRAW( Dlp(0,0,1,0).trs(0,0,-t) );
}

struct MyApp : public al::OmniVsrApp {

    Light light;

    MyApp() : al::OmniVsrApp() { 
        //mOmni.mode( Renderer::ANAGLYPH ).stereo(true); //amode( Renderer::RED_CYAN).
        
        camera().pos(0,0,0); // center camera

        for (int i = 0; i < mOmni.numProjections(); ++i){
            mOmni.projection(i).flipZ();
        }
    }
    
      virtual ~MyApp() {
        
      
      }
  
    virtual void onDraw(Graphics& gl){
    
//        static Field<Cir> f(4,4,4);
    
        static double eyesep, fovy;
        
        PRESET
            glv.gui(eyesep, "eyesep",-10,10)(fovy,"fovy",0,180);
            eyesep = .02; fovy = 45;
            
//            for (int i = 0; i < f.num(); ++i){
//                f[i] = CXY(1).trs( f.grid(i) );
//            }
        ENDSET
    
        light();
        // say how much lighting you want
        mShader.uniform("lighting", 1.0);

    
        //Frame2Pose( camera() ).print(); 

        mLens.eyeSep(eyesep);
        mLens.fovy(fovy);
        
        cmcheck();
        
        //f.draw();

    }
    
    virtual bool onKeyDown(){
        return true;
    }
};

//MyApp app;

int main(int argc, const char * argv[]){
	
    MyApp().start();
    
	return 0;

}
