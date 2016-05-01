/*
Allocore Example: PickableChildren

Description:
This example demonstrates how to write your own implementation of PickableBase
and add children pickables and interact with them

Author:
Tim Wood, April 2016
*/

#include "allocore/io/al_App.hpp"
#include "allocore/ui/al_BoundingBox.hpp"
#include "allocore/ui/al_Pickable.hpp"
using namespace al;


// Make a new pickable implmentation
struct TestPickable : PickableBase {

  bool childHover;
  
  Vec3f selectOffset;
  float selectDist;

  TestPickable(){}

  // this method is used for all intersection tests
  double intersect(Rayd &r){
    return r.intersectBox(pose.pos(), Vec3f(2)*scale);
  }

  TestPickable* addChild(Pose p, Vec3f scl){
    TestPickable *pick = new TestPickable();
    pick->pose.set(p);
    pick->scale.set(scl);
    children.push_back(pick);
    return pick;
  }

  void draw(Graphics &g){
    Mesh &m = g.mesh();
    m.reset();
    addWireBox(g.mesh());
    m.primitive(g.LINES);
    
    if(hover) g.color(1,0,0);
    else if(childHover) g.color(0,1,0);
    else if(selected) g.color(0,0,1);
    else g.color(1,1,1);
    pushMatrix(g);
    g.draw(m);
    popMatrix(g);
    drawChildren(g);
  }

  bool onPoint(Rayd &r, double t, bool child){
    if(t > 0.0){
      if(child){
        childHover = true;
        hover = false;
      } else {
        hover = true;
        childHover = false;
      }
    } else hover = false;
    return hover || childHover;
  }

  bool onPick(Rayd &r, double t, bool child){
    if(t > 0.0){
      if(child){
        selected = false;
        hover = false;
      } else {
        selectDist = t;
        selectOffset = pose.pos() - r(t);
        selected = true;
      }
    } else selected = false;
    return selected || child;
  }
  
  bool onDrag(Rayd &r, double t, bool child){
    // if(t > 0.0){
      if(child){
        return true;
      } else if(selected){
        Vec3f newPos = r(selectDist) + selectOffset;
        pose.pos().set(newPos);
        return true;
      }
    // }
    return false;
  }
};


// App
class MyApp : public App {
public:

  bool loadedFont;
  
  TestPickable pickable;
  Font font;

  MyApp(){

    // try to load font
    loadedFont = font.load("allocore/share/fonts/VeraMono.ttf", 72);
    if(!loadedFont) std::cout << "Failed to load font. Not rendering labels.." << std::endl;

    // position camera, disable mouse to look
    nav().pos(0,0,10);
    navControl().useMouse(false);

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

  void onAnimate(double dt){}

  void onDraw(Graphics& g){
    pickable.draw(g); 
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
