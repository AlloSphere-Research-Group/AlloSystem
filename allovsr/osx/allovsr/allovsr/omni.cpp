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

struct MyApp : OmniApp {
  Light light;

  MyApp() {
  
    mOmni.mode( OmniStereo::DUAL ).stereo(true);

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
    // say how much lighting you want
    shader().uniform("lighting", 1.0);

    static double ls = -10;
//    ls += .01;
//    if (ls > 10 ) ls = 0;
    mLens.eyeSep(-ls);


    GL::push();
    DRAW3(CXY(.3),1,0,0);
    DRAW3(Vector(0,1,0),0,1,0);
    GL::pop();
    //g.draw(mesh);
  }

  virtual void onAnimate(al_sec dt) {
    //light.pos(nav().pos());
    //std::cout << dt << std::endl;
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
