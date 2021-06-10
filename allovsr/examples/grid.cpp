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
#include "vsr/vsr_field.h"

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


    MyApp() : al::VsrApp() {

        // stereo.stereo(true);
        // stereo.mode( Stereoscopic::DUAL );

    }

    virtual void onDraw(Graphics& gl){

        //Model Transform
        Rot t = Gen::aa( scene().model.rot() );
        GL::rotate( t.w() );

        //YOUR CODE HERE

		static Field<Pnt> f(20,20,12)

    }


};

MyApp app;

int main(int argc, const char * argv[]){

    app.create(Window::Dim(800, 600), "Allovsr Example: Robot Arm");
//    app.create(Window::Dim(800, 600), "Allovsr Example: Robot Arm",60, Window::DEFAULT_BUF | Window::STEREO_BUF);

    MainLoop::start();

	return 0;

}
