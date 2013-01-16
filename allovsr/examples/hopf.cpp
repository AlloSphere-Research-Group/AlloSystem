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
#include "alloGLV/al_controlGLV.hpp"

//Glue
#include "allovsr/al_vsrInterface.hpp"
#include <iostream>


using namespace al;
using namespace vsr;

#define PRESET \
    static bool bSet = 0;\
    if (!bSet) { \
        bSet = 1; 

void hopf(al::VsrApp& app){
    
    HopfFiber hf;
    
    static int time = 0; time++;  double tphase = PI * time/180.0;

    //GUI Controlled Parameters
    static int num = 2;
    static int res = 3;
    static bool handed, bDrawCir, bDrawPnt,bReset;
    static double phase,amt,minDistance;
    
    //SET UP Gui
    static bool bSet = 0;
    if (!bSet) { 
        bSet = 1; 
        app.glv.gui(num,"num",0,1000)(res,"res",0,1000)(phase,"phase",1,100)(amt,"amt",0,10)(minDistance,"minDistance",0,1000);
        app.glv.gui(handed)(bDrawCir, "drawcir")(bDrawPnt,"drawpnt")(bReset,"reset"); 
    }    
        
    vector<Pnt> vp;
    for (int i = 0; i < num; ++i){
        double u = 1.0 * i/num;

        for (int j = 0; j < res; ++j){
            double v = 1.0 * j/res;

            Cir tc = hf.fiber(-1.0 + 2*u, v);
            
            Pnt tp = Ro::pnt_cir( tc, tphase * Ro::cur(tc) * phase);
            if(bDrawPnt)DRAW3(tp,u,v,1-u);
            if(bDrawCir)DRAW3(tc,v,u,1-u);
        
            vp.push_back( Ro::pnt_cir( tc, v * u * PI ) );
        }
    }
            
}


class MyApp : public al::VsrApp {

    public:
    
    MyApp() : al::VsrApp() { }

    virtual void onDraw(Graphics& gl){
        
        modelTransform();
        
        hopf(*this);
    }

};

MyApp app;

int main(int argc, const char * argv[]){

    app.create( Window::Dim(800, 600), "Hopf Fibration");
	
    MainLoop::start();
    
	return 0;

}
