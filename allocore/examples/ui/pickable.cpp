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

  Pickable pickable;
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
    pickable.set(mesh);
    pickable.bb.glUnitLength = 10; // ??? this behaves weird

    // create window
    initWindow();
  }

  void onAnimate(double dt){
    // move light in a circle
    t += dt;
    light.pos(10*cos(t),0,10*sin(t));
  }

  void onDraw(Graphics& g){

    // draw lit mesh
    g.lighting(true);
    light();
    material();
    pickable.drawMesh(g);
    
    g.lighting(false);
    pickable.drawBB(g);

    if(loadedFont){
      g.blendAdd();
      g.color(1,1,1);
      pickable.drawLabels(g, font, nav());
      g.blendOff();
    }
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
