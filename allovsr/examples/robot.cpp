/*

    Demonstrates Robot Kinematics using Versor Library.
    
    Given a three joint robot, with certain constraints and a target, 
    finds positions and orientations of various joints.

*/

//vsr Includes
#include "VSR/vsr.h"
#include "VSR/vsr_op.h"
#include "VSR/vsr_draw.h"
//#include "VSR/vsr_camera.h"
#include "VSR/vsr_frame.h"
#include "VSR/vsr_chain.h"

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


    MyApp() : al::VsrApp() { }

    virtual void onDraw(Graphics& gl){
        
        //Model Transform (to enable rotation of model with Option + Arrow Keys)
        Rot t = Gen::aa( scene().model.rot() ); 
        GL::rotate( t.w() );
        
                
        Dll dll = interface.vd().ray; DRAW(dll);
                                        
        //SET UP GUI PARAMETERS
        static double distA;
        PRESET
        
            glv.gui(distA, "LinkLength", 1,10);
            glv.gui.colors().back.set(.3,.3,.3);
            distA = 1.0;
        ENDSET
        
        static Frame baseFrame(PAO, Rot(1,0,0,0) );
        interface.touch(baseFrame);
                
        Point target = interface.mouse.origin;
        DRAW3(target,1,0,0);  
        Frame targetFrame( target, Rot(1,0,0,0) );
        
        Frame secondFrame( Ro::null(0,distA,0) );
        
        
        DualSphere firstSphere = Ro::dls( secondFrame.pos(), distA);        
        DualSphere targetSphere = Ro::dls( target, distA );


        //Plane of Rotation formed by yaxis of base and target point
        vsr::Plane rplane = baseFrame.ly() ^ target;
        DRAW3(rplane,0,1,0);
        
        //XZ plane of Target
        DualPlane targetXZ = targetFrame.dxz();
        DRAW3(targetXZ,0,.5,1);
        
        //Line of Target
        DualLine tline = targetXZ ^ rplane.dual();
        DRAW3(tline,1,1,0);
        
        //Point Pairs of Final joint
        PointPair fjoint = ( tline ^ targetSphere ).dual();
        DRAW(fjoint);
        
        //Pick the one closest to the base frame
        Frame finalFrame ( Ro::split(fjoint,true), Rot(1,0,0,0) );
        
        //Sphere around fframe
        DualSphere ffsphere = Ro::dls( finalFrame.pos(), distA);
                
        //Circle of Possibilities
        Circle cir = ( ffsphere ^ firstSphere).dual();
        DRAW3(cir,.5,1,1);
        
        //TWo points where the middle joint could be
        PointPair fpair = ( rplane.dual() ^ cir.dual() ).dual();
        DRAW3(fpair, 1,.5,.5);
        
        //Pick One and put the middle frame there
        Frame middleFrame( Ro::split(fpair,true), Rot(1,0,0,0) );
        
        
        //We can store the frame positions in a chain class which will sort out relative orientations for us
        Chain k(5);
        k[0] = baseFrame;
        k[1] = secondFrame;
        k[2] = middleFrame;
        k[3] = finalFrame;
        k[4] = targetFrame;
        
        Rotor r1 =  Gen::ratio( Vector::z, rplane.dual() );
        k[0].rot( r1 );
        
        //calculate joint rotations and link lengths from current positions
        k.joints(0); 
        //k.links();
        
        for (int i = 0; i < 5; ++i){
            DRAW(k.linf(i));
            DRAW(k[i]);
        }
     
        
        DRAW4(ffsphere,1,0,0,.2);
        DRAW4(firstSphere,1,0,0,.2);

    }

};

MyApp app;

int main(int argc, const char * argv[]){

    app.create(Window::Dim(800, 600), "Allovsr Example: Robot Arm");
	
    //app.description = "hello";
    MainLoop::start();
    
	return 0;

}


