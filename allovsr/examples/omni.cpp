//
//  omni.cpp
//  allovsr
//
//  Created by Pablo Colapinto on 2/26/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include "vsr/vsr.h"
#include "vsr/vsr_op.h"
#include "vsr/vsr_draw.h"

#include "alloutil/al_OmniApp.hpp"

using namespace al;
using namespace vsr;

#define PRESET \
    static bool bSet = 0;\
    if (!bSet) { \
        bSet = 1; 

#define ENDSET }

void cmcheck(){
    //CUBE MAP SANITY CHECK!
    double t = 3;
    //X Direction Facing a Sphere
    DRAW3( Ro::dls(t,0.0,0.0), 1, 0, 0 );
    //NX Direction Facing a Circle with a LINE BEHIND IT
    DRAW( CYZ(1).trs(-t,0,0) );
    DRAW3( DLN(0,1,0).trs(-t*1.1,0,0), 1,1,0 );
    //Y Direction Facing A Small Red Circle
    DRAW3( CXZ(.3).trs(0,t,0), 1,0,0 );
    //NY Direction Facing A Pair of Green Points 
    DRAW3( PAIR(1,0,0).trs(0,-t,0), 0,1,0 );
    //Z Direction an Arrow Pointing to WORLD SPACE -X, Y (tricky one . . .)
    GL::push(); GL::translate(0.,0.,t); DRAW3( Vector(-1,1,0),0,0,1 ) ; GL::pop();
    //NZ Direction a Plane (FRONT) with a blue circle BEHIND it
    DRAW( Dlp(0,0,1,0).trs(0,0,-t) );
    DRAW3( CXY(.5).trs(0,0,-t*1.1),0,0,1 );

}

struct MyApp : OmniApp {
  Light light;

  MyApp() {
  
    mOmni.mode( OmniStereo::ACTIVE ).stereo(true);

     //   omniEnable( false );

//    mesh.primitive(Graphics::TRIANGLES);
//    addSphere(mesh, 1.0, 32, 32);
//
//    for (int i = 0; i < mesh.vertices().size(); ++i) {
//      float f = (float)i / mesh.vertices().size();
//      mesh.color(Color(HSV(f, 1 - f, 1), 1));
//    }
//    mesh.generateNormals();
//    
//    light.ambient(Color(0.4, 0.4, 0.4, 1.0));
//    light.pos(5, 5, 5);
  
  }

  virtual ~MyApp() {
    
  
  }
  
  virtual void onDraw(Graphics& g) {
  
    light();
    shader().uniform("lighting", 1.0);    
    
    //arrows should point away from each other
    cmcheck();
    
  }

virtual void onAnimate(al_sec dt) {
    osc::Packet p;
    p.beginMessage("/nav");
    p << nav().pos().x << nav().pos().y << nav().pos().z << nav().quat().x << nav().quat().y << nav().quat().z << nav().quat().w;
    p.endMessage();
    
    osc::Send(12001, "192.168.0.26").send(p);
    osc::Send(12001, "192.168.0.27").send(p);
    osc::Send(12001, "192.168.0.28").send(p);
    osc::Send(12001, "192.168.0.29").send(p);
}

  virtual void onSound(AudioIOData& io) {
    while (io()) {
      io.out(0) = rnd::uniformS() * 0.05;
    }
  }

  virtual void onMessage(osc::Message& m) {
    OmniApp::onMessage(m);
  }

  virtual bool onKeyDown(const Keyboard& k){
    return true;
  }
};

int main(int argc, char * argv[]) {
  MyApp().start();
  return 0;
}
