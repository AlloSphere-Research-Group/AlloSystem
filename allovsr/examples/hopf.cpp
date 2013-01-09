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

struct HopfFiber{
    /// Feed in Spherical Coordinates of a 2-Sphere, get 3-Sphere Fiber out
    
    bool bHanded;

    //LINE AT NORTH POLE
    Line mInf;
    
    //CIRCLE AT SOUTH POLE
    Circle mCir;
    
    
    HopfFiber(bool hand=true) : bHanded(hand) {
        mInf =  PAO ^ Ro::null(0, bHanded? 1 :-1,0) ^ Inf(1);
        mCir = CXZ(1);
    }
    
    
    Cir cir(double theta, double phi){

        double ptheta = PIOVERTWO * theta;

        Vector v = Vector::x.rot( Biv::xz * ptheta );
        Lin lim = mCir.sp( Gen::trv(v) );                                      //<-- Circle to a Line (Limit)
        Mot mot = Gen::ratio( lim.dual(), mInf.dual(), phi);
        
        return mCir.sp( mot * Gen::trv(v * phi )  ) ; 
        
    }
    
    Pnt phase(double theta, double phi, double phs){
        return Ro::pnt_cir( cir(theta,phi), phs * PI);
    }
};

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
    
    //TRACE
//    static Pnt pnt = Ro::null(1,1,1);
//    if (bReset) { pnt = Ro::null(1,1,1); bReset = false; }
//    app.interface.touch(pnt); DRAW(pnt);
    
    vector<Pnt> vp;
    for (int i = 0; i < num; ++i){
        double u = 1.0 * i/num;

        for (int j = 0; j < res; ++j){
            double v = 1.0 * j/res;

            Cir tc = hf.cir(-1.0 + 2*u, v);
            Pnt tp = Ro::pnt_cir( tc, tphase * Ro::cur(tc) * phase);
            if(bDrawPnt)DRAW3(tp,u,v,1-u);
            if(bDrawCir)DRAW3(tc,v,u,1-u);
        
            vp.push_back( Ro::pnt_cir( tc, v * u * PI ) );
        }
    }
        
    pnt = Ro::loc( pnt.sp( Gen::bst(par * amt) ) );
    DRAW3(Ro::dls_pnt(pnt,.2),1,0,0);



class MyApp : public al::VsrApp {

    public:
    
    MyApp() : al::VsrApp() { 
        
       
    
    }

    virtual void onDraw(Graphics& gl){
        
        //Model Transform
        Rot t = Gen::aa( scene().model.rot() ); 
        GL::rotate( t.w() );
        
       hopf(*this);
    }

};

MyApp app;

int main(int argc, const char * argv[]){

    app.create(Window::Dim(800, 600), "Allovsr Demo: Hopf Fibration");
	
    MainLoop::start();
    
	return 0;

}
