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
#include "vsr/vsr_motor.h"

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

struct MyApp : public al::VsrApp {


    MyApp() : al::VsrApp() { }

    virtual void onDraw(Graphics& gl){
    
    //YOUR CODE HERE
    
        static Frame fa( PT(-1,0,0) );
        static Frame fb( PT(1,0,0) );
        DRAW(fa); DRAW(fb);
        
        interface.touch(fa);
        interface.touch(fb);
        
        for (int i = 0; i < 1000; ++i){
            double t= 1.0 * i/1000;
             
            Dll dll = fa.dll() * (1-t) + fb.dll() * t;                       
            Frame frame(Gen::mot(dll) );//Frame::Twist( fa, fb, t );
            
            DRAW(frame);
        }
    
    }
    
    
};

MyApp app;

int main(int argc, const char * argv[]){

    app.create(Window::Dim(800, 600), "Allovsr Example: Robot Arm");
	
    MainLoop::start();
    
	return 0;

}