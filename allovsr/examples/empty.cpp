//
//  twists.cpp
//  allovsr
//
//  Created by Pablo Colapinto on 12/3/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//


//vsr Includes
#include "VSR/vsr.h"
#include "VSR/vsr_op.h"
#include "VSR/vsr_draw.h"
#include "VSR/vsr_frame.h"
#include "VSR/vsr_motor.h"

//allo includes
#include "allocore/al_Allocore.hpp"
#include "allocore/io/al_ControlNav.hpp"
#include "alloGLV/al_ControlGLV.hpp"

//Glue
#include "allovsr/al_vsrInterface.hpp"

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


    MyApp() : al::VsrApp() { 
    
        stereo.stereo(true);
        stereo.mode( Stereographic::DUAL );
    
    }

    virtual void onDraw(Graphics& gl){
    
        //Model Transform
        Rot t = Gen::aa( scene().model.rot() ); 
        GL::rotate( t.w() );
    
        //YOUR CODE HERE
        Frame f; DRAW(f);
        //if (stereo.omni()) printf("omni");
        //if (stereo.mode() == Stereographic::DUAL ) printf("dual\n");
    }
    
    
};

MyApp app;

int main(int argc, const char * argv[]){

    app.create(Window::Dim(800, 600), "Allovsr Example: Robot Arm");
	
    MainLoop::start();
    
	return 0;

}
