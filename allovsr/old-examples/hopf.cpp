//
//  hopf.cpp
//  allovsr
//
//  Created by Pablo Colapinto on 11/28/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

//vsr Includes
#include "VSR/vsr.h"
#include "VSR/vsr_op.h"
#include "VSR/vsr_draw.h"
#include "VSR/vsr_camera.h"
#include "VSR/vsr_fiber.h"
#include "VSR/vsr_coord.h"

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
        app.glv.gui(num,"num",0,1000)(res,"res",0,1000)(phase,"phase",-100,100)(amt,"amt",0,10)(minDistance,"minDistance",0,1000);
        app.glv.gui(handed)(bDrawCir, "drawcir")(bDrawPnt,"drawpnt")(bReset,"reset"); 
    }    
        
    vector<Pnt> vp;
    for (int i = 0; i < num; ++i){
        double u = 1.0 * i/num;

        for (int j = 0; j < res; ++j){
            double v = 1.0 * j/res;

            Cir tc = hf.fiber(-1.0 + 2*u, -.5 + v);
            
            Pnt tp = Ro::pnt_cir( tc, tphase * Ro::cur(tc) * phase);
            if(bDrawPnt)DRAW3(tp,u,v,1-u);
            if(bDrawCir)DRAW3(tc,v,u,1-u);
        
            vp.push_back( Ro::pnt_cir( tc, v * u * PI ) );
        }
    }
    

    static Dll dll = DLN(0,1,0); app.interface.touch(dll); DRAW(dll);
    static Point point = PT(2,0,0); app.interface.touch(point);  DRAW3(point,1,0,0);
    
    for (int i = 0; i <= 1; ++i){
        double t = i/1.0;
        Dll tdll = dll.sp( Gen::rot(Biv::xy * t*PIOVERTWO) );
        Pnt pa = Ro::split( ( tdll ^ Ro::sur(hf.cir) ).dual(), true);
        Pnt pb = Ro::split( ( tdll ^ Ro::sur(hf.cir) ).dual(), false);
        
        DRAW3(pa,1,1,0); DRAW3(pb,1,1,0);
    
        Coord::Sph spha = Coord::vec2sph( Vector(pa).unit() );
        Coord::Sph sphb = Coord::vec2sph( Vector(pb).unit() );
                
        Cir cca = hf.fiber( spha.theta / PI, spha.phi / PI );
        Cir ccb = hf.fiber( sphb.theta / PI, sphb.phi / PI );
        
        Bst bst = Gen::bst( (cca.dual() * PI/3.0 + ccb.dual() * PI/2.0 )* amt );
        
        
        DRAW3(cca,0,1,0); DRAW3(ccb,0,0,1);
        
        vector<Pnt> pp; 
        Pnt tp = point;//.trs(-t,0,0);
        for (int i = 0; i<1000; ++i){
            tp = Ro::loc( tp.sp( bst ) );
            pp.push_back(tp);
        }
        
        glColor3f(1,0,0);
        glBegin(GL_LINE_STRIP);
         
            for (int i = 0; i<pp.size(); ++i ){
                GL::vertex( pp[i].w() );
            }
         
        glEnd();
    }
    
    int tnum = 10;
    
//    for (int j = 0; j < tnum; ++j){
//    
//        double t = 1.0 * j/tnum;        
//        
//        Bst bst2 = hf.mtt( );
//        
//        for (int i = 0; i<pp.size(); ++i ){
//            for(int k = 0; k < 10; ++k){
//            
//            }
//        }
//
//    }
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
