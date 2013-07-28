
//vsr Includes
#include "VSR/vsr.h"
#include "VSR/vsr_op.h"
#include "VSR/vsr_draw.h"
#include "VSR/vsr_camera.h"

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


struct MyApp : public al::VsrApp {


    MyApp() : al::VsrApp() { }

    virtual void onDraw(Graphics& gl){
        
        //Model Transform
        Rot t = Gen::aa( scene().model.rot() ); 
        GL::rotate( t.w() );
        
        //A Circle in the XY plane with radius 1.5
        static Cir c = CXY(1.5);
        DRAW(c);                    //<-- DRAW is a macro that draws an element
        interface.touch(c);
        
        //A Plane (with normal along y axis, at distance -.5 units from the origin)
        static Dlp d(0,1,0,-.5);
        DRAW3(d,1,0,0);             //<-- DRAW3 is a macro that draws an element a certain color (red here)
        interface.touch(d);
        
        //Intersection of Circle and Plane (two points)
        DRAW3( (d ^ c.dual() ).dual(), 0,1,0 );
        
    }

};

MyApp app;

int main(int argc, const char * argv[]){

    app.create(Window::Dim(800, 600), "Allovsr Example: Circle - Plane Intersection");
	
    //app.description = "hello";
    MainLoop::start();
    
	return 0;

}


