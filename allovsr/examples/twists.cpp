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

        //Model Transform (to enable rotation of model with Option + Arrow Keys)
        Rot t = Gen::aa( scene().model.rot() ); 
        GL::rotate( t.w() );

        static double iter, period, pitch, ext;
        static bool bFollow;
        PRESET
            glv.gui(iter,"iter",10,1000)(period, "period", -10,10)(pitch,"pitch", -10,10)(ext,"extrapolation",-10,10);
            glv.gui(bFollow,"follow");
            glv.gui.colors().back.set(0,0,0);
        ENDSET
        
        //A Twist Operator
        Twist twist;
        
        //A Frame
        static Frame frame;
        interface.touch(frame);
        
        //A Dual Line in direction 0,1,0 translated to 1,0,0
        Dll dll = DLN(0,1,0).trs(1,0,0);
        
        Dll ndll = twist.along( dll, period * PI, pitch * PI).dll();
        
        for (int i = 0; i < iter; ++i){
            double t= 1.0 * i/iter;
            Mot m = Gen::mot( ndll * ext);
            Frame tf(m*frame.mot()); DRAW(tf);
            DRAW3( frame.cxy().sp( m ), t, 0, 1-t );
            
            //Twist the twisting line . . .
            ndll = ndll.sp( twist.along( DLN(1,0,0), period * PI, pitch * PI).mot( t ) );
            DRAW(ndll);
        }
    

    }
    
};

MyApp app;

int main(int argc, const char * argv[]){

    app.create(Window::Dim(800, 600), "Allovsr Example: Robot Arm");
	
    MainLoop::start();
    
	return 0;

}
