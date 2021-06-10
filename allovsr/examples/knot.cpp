//
//  hopf.cpp
//  allovsr
//
//  Created by Pablo Colapinto on 11/28/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#define ALLOSPHERE 0

//vsr Includes
#include "vsr/vsr.h"
#include "vsr/vsr_op.h"
#include "vsr/vsr_draw.h"
#include "vsr/vsr_camera.h"
#include "vsr/vsr_fiber.h"
#include "vsr/vsr_stat.h"
//#include "vsr/vsr_gamma.h"

//allo includes
#include "allocore/al_Allocore.hpp"
#include "allocore/io/al_ControlNav.hpp"
#include "alloGLV/al_ControlGLV.hpp"

//gamma
//#include "Gamma/Oscillator.h"

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

    // 3-Sphere

    HopfFiber hf;

    // Gamma
//    static gam::LFO<> ramp(1);  GfxSync::master() << ramp;
//    double tramp = .5 + ramp.up2();


    /////////
    /* GUI */
    /////////

    static double m,n,amt,iter;
    static double theta, phi, gamma; //ANGLES of POLES and INITIAL CIRCLE
    static double ntrace, traceamt, bandres;
    static bool bReset, bCirFlow, bDrawCirStrip, bDrawStrip, bDrawSrc;

	//stereo controls
	static double eyesep, fovy;

    PRESET
        app.glv.gui(m,"m",0,10)(n,"n",0,10)(amt,"amt",-10,10)(iter,"iter",1,1000);
        app.glv.gui(theta,"theta",0,100)(phi,"phi",0,100);//(gamma);
        app.glv.gui(ntrace,"trace",0,1000)(traceamt,"trace_amt",0,10)(bandres,"band_res",0,100);
        app.glv.gui(bReset,"reset")(bCirFlow,"cirFlow")(bDrawCirStrip,"cir_strip")(bDrawStrip, "strip")(bDrawSrc, "src");
	app.glv.gui(eyesep,"eyesep",0,2)(fovy,"fovy",0,180);
        m = 1; n = 5; amt = .005; iter = 1000; ntrace = 10; traceamt = .1;
	eyesep = .2; fovy = 45;
    }

	app.lens.eyeSep(eyesep*-1); app.lens.fovy(fovy);

    /////////////////////////////////
    /* The Transformation Operator */
    /////////////////////////////////

    //HOPF LINKS at Poles (orthogonal)

    static double th = 0; th += theta;
    static double ph = 0; ph += phi;

    vector<Cir> cp = hf.poles(-1 + fmod(th,1) * 2,  fabs( sin(ph) ) );

//    vector<Cir> cp;
//    Cir cone = hf.fiber(0,-.5);
//    Cir ctwo = hf.fiber(0, .5);

//    cp.push_back( cone); cp.push_back(ctwo);
    //A Point Pair "Boost" Generator . . .
    PointPair tp = cp[0].dual() * PI/m + cp[1].dual() * PI/n;

    //A Boost
    Bst bst = Gen::bst( tp*amt ) ;
    //Another Boost
    Bst bst2 = Gen::bst( tp*amt*traceamt );


    //POINTS ALONG KNOT

    //A Point you can Touch
    static Point pt = PT(1,0,0);
    app.interface.touch(pt);

    //dp/dt
    vector<Pnt> vp;
    Point np = pt;
    for (int i = 0; i < iter; ++i){
        np = Ro::loc( np.sp( bst ) );           //Extracts Location Each time with Ro::loc
        vp.push_back(np);
    }

    //CIRCLES ALONG THE KNOT

    //A Circle at point
    static Circle ct = Ro::cir( pt, Biv::yz  + Biv::xy * gamma * PI, .2);
    app.interface.touch(ct);


    vector<Cir> cirp;
    Circle ncirp = ct;
    for (int i = 0; i < iter; ++i){
        ncirp = ncirp.sp( Gen::bst( tp*amt ) ); // Does not Extract Location (so is not stable)
        cirp.push_back(ncirp);
    }

    //////////////////////
    // FLOW //////////////
    //////////////////////

    //Generate Random points, Random Circles

    static vector<Pnt> rp;  //  Points Flow
    static vector<Cir> scp; //  Circle Flow
    static vector<Pnt> bp;  //  Band of Points Flow

    if (bReset){

        rp.clear(); scp.clear(); bp.clear();
        Rand::Seed(10);
        for (int i = 0; i < iter; ++i){

            //Random Point
            double x = Rand::Num(-1,1);
            double y = Rand::Num(-.5,.5);
            double z = Rand::Num(-1,1);
            Pnt p = Vector(x,y,z).null();
            rp.push_back( p );

            //Random Circle
            Biv biv( Rand::Num(), Rand::Num(), Rand::Num());
            Cir tcir = Ro::cir(p, biv.unit(), .1);
            scp.push_back( tcir );

            for (int j = 0; j < bandres; ++j){
                double t = 1.0 * j/bandres;
                bp.push_back( Ro::pnt_cir( tcir, t * PI) );
            }
        }
    }


    //////////////////////
    // Draw Flow    //////
    //////////////////////

    //Modular Flow of Points
    for (int i = 0; i < rp.size(); ++i){
        double t = 1.0 * i/rp.size();
        Pnt np =  Ro::loc( rp[i].sp( bst ) ) ;
        rp[i] = np;
        for (int j = 0; j < ntrace; ++j){
            double jt = 1.0 * j/ntrace;
            np =  Ro::loc( np.sp( bst2 ) ) ;
            DRAW4(np,t,jt,1-t,1-jt);
        }
    }

    //Switch Circular or Band of Points

    if (bCirFlow){
        //Modular Flow of Circles
        for (int i = 0; i < scp.size(); ++i){
            double t = 1.0 * i/ cirp.size();
            Cir np =  scp[i].sp( bst ) ;
            scp[i] = np;
            for (int j = 0; j < ntrace; ++j){
                double jt = 1.0 * j/ntrace;
                np =  np.sp( bst2 ) ;
                DRAW4(np,t,jt,1-t,1-jt);
            }
        }
    } else {
        //Modular Flow of Band of points
        for (int i = 0; i < bp.size(); ++i){
            double t = 1.0 * i / cirp.size();
            Pnt np =  Ro::loc( bp[i].sp( bst ) );
            bp[i] = np;
            for (int j = 0; j < ntrace; ++j){
                double jt = 1.0 * j/ntrace;
                np =  Ro::loc( np.sp( bst2 ) ) ;
                DRAW4(np,t,jt,1-t,1-jt);
            }
        }
    }

    if (bDrawSrc){
        //Draw the Point
        DRAW3(pt,1,0,0);
        //Draw Polar Circles (Hopf Links)
        DRAW(cp[0]); DRAW(cp[1]);
    }

    if (bDrawStrip){
        //DRAW the Knot Strip
        glBegin(GL_LINE_STRIP);
            for (int i = 0; i < vp.size(); ++i){
                GL::vertex(vp[i].w());
            }
        glEnd();
    }

    if (bDrawCirStrip){
        //DRAW the Circle Knot Strip
        for (int i = 0; i < cirp.size(); ++i){
            DRAW3( cirp[i],0,1,0 );
        }
    }

}



class MyApp : public al::VsrApp {

    public:

    MyApp() : al::VsrApp() {

        glv.gui.colors().back.set(.3,.3,.3);

        if (ALLOSPHERE){
            stereo.stereo(true);
            stereo.mode( Stereoscopic::ACTIVE );

            lens.fovy(45);
            lens.eyeSep(lens.eyeSepAuto() *-1);

        }
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

    Window::DisplayMode DM = ALLOSPHERE ? Window::DEFAULT_BUF : Window::DEFAULT_BUF | Window::STEREO_BUF;

    app.create(Window::Dim(800, 600), "Hopf Fibration and Knots", 60, DM);
    //app.lens.eyeSep( app.lens.eyeSepAuto() );
    //cout << app.lens.eyeSep() << endl;
    MainLoop::start();

	return 0;

}
