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
    
    virtual void onAnimate(al_sec dt) {
		osc::Packet p;
		p.beginMessage("/nav");
		p << camera().pos()[0] <<  camera().pos()[1] <<  camera().pos()[2] <<  camera().rot()[0] << camera().rot()[1]<< camera().rot()[2] << camera().rot()[3];
		p.endMessage();
		
		osc::Send(12001, "192.168.0.26").send(p);
		osc::Send(12001, "192.168.0.27").send(p);
		osc::Send(12001, "192.168.0.28").send(p);
		osc::Send(12001, "192.168.0.29").send(p);
    }
    
   virtual void onMessage(osc::Message& message) {
		message.print();
        
        if(Socket::hostName() == "photon") {
            
            float value;
            
            if (message.addressPattern() == "/mx") {
				message >> value;
				camera().dx() = Vector(-value, 0, 0);
            }
            else if (message.addressPattern() == "/my") {
				message >> value;
				camera().dx() = Vector(0, value, 0);
			}
            else if (message.addressPattern() == "/mz") {
				message >> value;
				camera().dx() = Vector(0,0,-value);
			}
            else if (message.addressPattern() == "/tx") {
				message >> value;
				camera().db() = Biv::xz * -value;
			}
            else if (message.addressPattern() == "/ty") {
				message >> value;
				camera().db() = Biv::yz * value;
			}
            else if (message.addressPattern() == "/tz") {
				message >> value;
				camera().db() = Biv::xy * value;
			}
            else if (message.addressPattern() == "/h") {
				camera().reset();
			}
		}
		else {
			double px, py, pz, qx, qy, qz, qw;
			if (message.addressPattern() == "/nav") {
				message >> px >> py >> pz >> qx >> qy >> qz >> qw;
				camera().pos() = PT(px, py, pz);
				camera().rot() = Rot(qw, qx, qy, qz);
			}
		}
	}
};

//MyApp app;

int main(int argc, const char * argv[]){
	
    MyApp().start();
    
	return 0;

}
