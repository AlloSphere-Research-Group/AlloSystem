/*
Allocore Example: pickRay

Description:
The example demonstrates how to interact with objects using ray intersection tests.

Author:
Tim Wood, 9/19/2014
*/

#include "allocore/io/al_App.hpp"
#include "allocore/math/al_Ray.hpp"

using namespace al;

#define N 10

struct PickRayDemo : App {
  Material material;
  Light light;
  Mesh m;

  Vec3f pos[N];
  Vec3f offset[N];  // difference from intersection to center of sphere
  float dist[N];    // distance of intersection
  bool hover[N];    // mouse is hovering over sphere
  bool selected[N]; // mouse is down over sphere

  PickRayDemo() {
    nav().pos(0, 0, 10);
    light.pos(0, 0, 10);
    addSphere(m, 0.5);
    m.generateNormals();

    for(int i=0; i<N; i++){
      pos[i] = Vec3f(rnd::uniformS(),rnd::uniformS(),0.f) * 2.f;
      offset[i] = Vec3f();
      dist[i] = 0.f;
      hover[i] = false;
      selected[i] = false;
    }

    initWindow();

    // disabel nav control mouse drag to look
    navControl().useMouse(false);

  }

  virtual void onDraw(Graphics& g, const Viewpoint& v) {
    material();
    light();

    for(int i=0; i<N; i++){
      g.pushMatrix();
      g.translate(pos[i]);
      if(selected[i]) g.color(1,0,1);
      else if(hover[i]) g.color(0,1,1);
      else g.color(1,1,1);
      g.draw(m);
      g.popMatrix();
    }
  }

  virtual void onMouseMove(const ViewpointWindow& w, const Mouse& m){
    // make a ray from mouse location
    Rayd r = getPickRay(w, m.x(), m.y());

    // intersect ray with each sphere in scene
    for(int i=0; i<N; i++){
      // intersect sphere at center pos[i] and radius 0.5f
      // returns the distance of the intersection otherwise -1
      float t = r.intersectSphere(pos[i], 0.5f);
      hover[i] = t > 0.f;
    }
  }
  virtual void onMouseDown(const ViewpointWindow& w, const Mouse& m){
    Rayd r = getPickRay(w, m.x(), m.y());

    for(int i=0; i<N; i++){
      float t = r.intersectSphere(pos[i], 0.5f);
      selected[i] = t > 0.f;

      // if intersection occured store and offset and distance for moving the sphere
      if(t > 0.f){
        offset[i] = pos[i] - r(t);
        dist[i] = t;
      }
    }
  }
  virtual void onMouseDrag(const ViewpointWindow& w, const Mouse& m){
    Rayd r = getPickRay(w, m.x(), m.y());

    for(int i=0; i<N; i++){
      // if sphere previously selected move sphere
      if(selected[i]){
        Vec3f newPos = r(dist[i]) + offset[i];
        pos[i] = newPos;
      }
    }
  }
  virtual void onMouseUp(const ViewpointWindow& w, const Mouse& m){
    // deselect all spheres
    for(int i=0; i<N; i++) selected[i] = false;
  }

};
int main(int argc, char* argv[]) {
  PickRayDemo app;
  app.start();
  return 0;
}

