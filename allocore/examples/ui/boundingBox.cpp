/*
Allocore Example: Bounding Box

Description:
This example demonstrates how render a bounding box around a mesh
and render labels and tic marks 

Author:
Tim Wood, April 2016
*/

#include "allocore/io/al_App.hpp"
#include "allocore/ui/al_BoundingBox.hpp"
using namespace al;

class MyApp : public App {
public:
  bool loadedFont;
  double t;
  Light light;      
  Material material; 
  Mesh mesh; 

  BoundingBox bb;
  Font font;

  MyApp(){

    // try to load font
    loadedFont = font.load("allocore/share/fonts/VeraMono.ttf", 72);
    if(!loadedFont) std::cout << "Failed to load font. Not rendering labels.." << std::endl;

    // position camera
    nav().pos(0,0,4);

    // Create a red sphere mesh
    addSphere(mesh,7.156);
    mesh.generateNormals();
    mesh.color(RGB(1,0,0));

    // Initialize BoundingBox
    bb.set(mesh);
    bb.glUnitLength = 10;

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
    g.draw(mesh);

    // draw white boundingbox
    g.lighting(false);
    g.color(1,1,1);
    g.draw(bb.mesh);
    g.draw(bb.tics);

    if(loadedFont){
      g.blendAdd();
      bb.drawLabels(g, font, nav(), Pose(), 1);
      g.blendOff();
    }
  }
};

int main(){
  MyApp().start();
}
