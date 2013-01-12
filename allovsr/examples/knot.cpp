//
//  hopf.cpp
//  allovsr
//
//  Created by Pablo Colapinto on 11/28/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

//vsr Includes
#include "vsr/vsr.h"
#include "vsr/vsr_op.h"
#include "vsr/vsr_draw.h"
#include "vsr/vsr_camera.h"
#include "vsr/vsr_fiber.h"

//allo includes
#include "allocore/al_Allocore.hpp"
#include "allocore/io/al_ControlNav.hpp"
#include "alloGLV/al_ControlGLV.hpp"

//Glue
#include "allovsr/al_vsrInterface.hpp"
#include <iostream>


using namespace al;
using namespace vsr;

#define PRESET \
    static bool bSet = 0;\
    if (!bSet) { \
        bSet = 1; 


void knot(al::VsrApp& app){

    HopfFiber hf;
    

    //A Circle you can touch (in the xz plane)
    static Circle ca = CXZ(1);
    app.interface.touch(ca);
    
    //The Axis of the Circle
    DualLine cb = Inf(1) <= ca;
    //Normalized . . .
    cb = cb.runit();
    

    //A Point you can Touch
    static Point pt = PT(1,0,0); 
    app.interface.touch(pt); 
    
    //GUI
    static double m,n,amt,iter;
    static double theta, phi;
    PRESET
        app.glv.gui(m,"m",0,10)(n,"n",0,10)(amt,"amt",-10,10)(iter,"iter",1,1000);
        app.glv.gui(theta)(phi);
        m = 1; n = 5; amt = .005; iter = 1000;
    }
    

    vector<Cir> cp = hf.poles(-1 + theta * 2,phi);
    DRAW(cp[0]); DRAW(cp[1]);

    //A Point Pair "Boost" Generator . . .
    PointPair tp = ca.dual() * PI/m + cb * PI/n;
    //A Point Pair "Boost" Generator . . .
    PointPair tp2 = cp[0].dual() * PI/m + cp[1].dual() * PI/n;

            
    vector<Pnt> vp;
    Point np = pt;
    for (int i = 0; i < iter; ++i){
    
        np = Ro::loc( np.sp( Gen::bst( tp2*amt ) ) );
        vp.push_back(np);
        
    }


    //DRAW ROUTINES:
    
    //Draw the Circle and its Axis, and the Point
    DRAW3(ca,0,0,1); DRAW3(cb,0,1,0); DRAW3(pt,1,0,0);
    
    //DRAW the Knot Strip
    glBegin(GL_LINE_STRIP);
        for (int i = 0; i < vp.size(); ++i){
            GL::vertex(vp[i].w());
        }   
    glEnd();
    
}



class MyApp : public al::VsrApp {

    public:
    
    MyApp() : al::VsrApp() { 
        
       
    
    }

    virtual void onDraw(Graphics& gl){
        
        //Model Transform
        Rot t = Gen::aa( scene().model.rot() ); 
        GL::rotate( t.w() );
        
        knot(*this);
    }

};

MyApp app;

int main(int argc, const char * argv[]){

    app.create(Window::Dim(800, 600), "Allovsr Demo: Hopf Fibration");
	
    MainLoop::start();
    
	return 0;

}
