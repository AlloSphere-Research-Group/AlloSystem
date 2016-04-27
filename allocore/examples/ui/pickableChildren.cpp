/*
Allocore Example: Pickable

Description:
This example demonstrates how associate a mesh with a pickable
and interact with it via the mouse

Author:
Tim Wood, April 2016
*/

#include "allocore/io/al_App.hpp"
#include "allocore/ui/al_BoundingBox.hpp"
#include "allocore/ui/al_Pickable.hpp"
using namespace al;

class MyApp : public App {
public:

  bool loadedFont;
  double t;
  Light light;      
  Material material; 
  
  Mesh mesh; 

  BoundingBox bb;
  TestPickable pickable;
  Font font;

  MyApp(){

    // try to load font
    loadedFont = font.load("allocore/share/fonts/VeraMono.ttf", 72);
    if(!loadedFont) std::cout << "Failed to load font. Not rendering labels.." << std::endl;

    // position camera, disable mouse to look
    nav().pos(0,0,10);
    navControl().useMouse(false);

    // Create a red spheres mesh
    addSphere(mesh,1);
    mesh.translate(-3,3,0);
    addSphere(mesh,2);
    mesh.translate(-2.7,2.7,0);
    addSphere(mesh,0.7);
    mesh.translate(2.7,-2.7,0);

    mesh.generateNormals();
    mesh.color(RGB(1,0,0));

    // Initialize BoundingBox
    // pickable.set(mesh);
    // pickable.bb.glUnitLength = 10; // ??? this behaves weird
    auto p = pickable.addChild(Pose(Vec3f(-0.5,0,0),Quatf()),0.5);
    p = p->addChild(Pose(),0.5);
    p = p->addChild(Pose(),0.5);
    p = p->addChild(Pose(),0.5);
    p = pickable.addChild(Pose(Vec3f(0.5,0,0),Quatf()),0.5);
    p = p->addChild(Pose(),0.5);
    p = p->addChild(Pose(),0.5);
    p = p->addChild(Pose(),0.5);

    // create window
    initWindow();
  }

  void onAnimate(double dt){
    // move light in a circle
    t += dt;
    // light.pos(10*cos(t),0,10*sin(t));
  }

  void onDraw(Graphics& g){

    pickable.draw(g, &pickable); 
    //pushMatrix(g);
    
    // // draw lit mesh
    // g.lighting(true);
    // light();
    // material();
    // g.draw(mesh);

    // // draw boundingbox
    // if(pickable.hover || pickable.selected){
    //   g.lighting(false);
    //   if(pickable.selected) g.color(0,1,1);
    //   else g.color(1,1,1);
    //   g.draw(pickable.bb.mesh);
    //   g.draw(pickable.bb.tics);

    //   if(loadedFont){
    //     g.blendAdd();
    //     g.color(1,1,1);
    //     pickable.bb.drawLabels(g, font, nav(), Pose(), 1);
    //     g.blendOff();
    //   }
    // }

    // pickable.popMatrix(g);


  }

  virtual void onMouseMove(const ViewpointWindow& w, const Mouse& m){
    // make a ray from mouse location
    Rayd r = getPickRay(w, m.x(), m.y());
    pickable.point(r);
  }
  virtual void onMouseDown(const ViewpointWindow& w, const Mouse& m){
    Rayd r = getPickRay(w, m.x(), m.y());
    pickable.pick(r);
  }
  virtual void onMouseDrag(const ViewpointWindow& w, const Mouse& m){
    Rayd r = getPickRay(w, m.x(), m.y());
    pickable.drag(r);
  }
  virtual void onMouseUp(const ViewpointWindow& w, const Mouse& m){
    Rayd r = getPickRay(w, m.x(), m.y());
    pickable.unpick(r);
  }
};

int main(){
  MyApp().start();
}
