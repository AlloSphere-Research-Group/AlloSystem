/*
Allocore Example: PickableTranslateHandle

Description:
This example shows how to add a rotation handle as a child 
to interact with its parent pickable

Author:
Tim Wood, April 2016
*/

#include "allocore/io/al_App.hpp"
#include "allocore/ui/al_Pickable.hpp"
#include "allocore/ui/al_TranslateHandle.hpp"
#include "allocore/ui/al_RotateHandle.hpp"

using namespace al;

class MyApp : public App {
public:

  bool loadedFont;
  double t;
  Light light;      
  Material material; 
  
  Mesh mesh; 

  Pickable pickable;
  TranslateHandle th;
  RotateHandle rh;

  Font font;

  MyApp(){

    // try to load font
    loadedFont = font.load("allocore/share/fonts/VeraMono.ttf", 72);
    if(!loadedFont) std::cout << "Failed to load font. Not rendering labels.." << std::endl;

    // position camera, disable mouse to look
    nav().pos(0,0,10);
    navControl().useMouse(false);

    // Create a red spheres mesh
    addTorus(mesh);
    // mesh.translate(-3,3,0);
    // addSphere(mesh,2);
    // mesh.translate(-2.7,2.7,0);
    // addSphere(mesh,0.7);
    // mesh.translate(2.7,-2.7,0);
    mesh.generateNormals();

    // Initialize Pickable
    pickable.set(mesh);
    // pickable.bb.glUnitLength = 10; // ??? this behaves weird
    
    // add translate handle as child
    pickable.addChild(rh);
    pickable.addChild(th);

    // pickable.bb.cen.print();
    // std::cout << std::endl; 

    // create window
    initWindow();
  }

  void onAnimate(double dt){
    // move light in a circle
    t += dt;
    light.pos(10*cos(t),0,10*sin(t));
  }

  void onDraw(Graphics& g){

    g.lighting(true);
    light();
    material();
    g.color(1,1,1);
    // p.drawMesh(g);
    // p.drawChildren(g);
    // p.drawBB(g, true)
    // p.drawChildren(g);

    pickable.pushMatrix(g);
    
    // draw mesh
    g.draw(mesh);

    // draw boundingbox
    if(pickable.hover || pickable.selected){
      g.lighting(false);
      if(pickable.selected) g.color(0,1,1);
      else g.color(1,1,1);
      g.draw(pickable.bb.mesh);
      g.draw(pickable.bb.tics);

      if(loadedFont){
        g.blendAdd();
        g.color(1,1,1);
        pickable.bb.drawLabels(g, font, nav(), Pose(), 1);
        g.blendOff();
      }
    }

    g.lighting(false);
    g.depthTesting(false);
    rh.draw(g);    
    th.draw(g,nav());
    g.depthTesting(true);


    pickable.popMatrix(g);


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
