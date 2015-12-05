/*
Test of MeshVBO in OmniStereoGraphicsRenderer
Based on Allocore Example: Many Shape VBO

Authors:
Lance Putnam, April 2011
Kurt Kaminski, December 2015
Karl Yerkes
*/

#include "alloutil/al_OmniStereoGraphicsRenderer.hpp"

using namespace al;
using namespace std;

struct MyApp : OmniStereoGraphicsRenderer {
  MeshVBO shapesVBO;
  Light light;
  Material material;

  int numShapes = 10000;
  float scatterSize = 1.0;

  MyApp() {
    // Choose a nice point of view
    //
    nav().set(Pose(Vec3d(0.000000, -0.338890, 1.059181),
                   Quatd(0.998629, -0.052338, 0.000000, 0.000000)));
  }

  double t = 0;
  void onAnimate(double dt) {
    t += dt;
    // update the pose with the nav. normally OmniStereoGraphicsRenderer takes
    // it's pose from Cuttlebone state. This lets us navigate around like an
    // al::App.
    //
    pose = nav();
  }

  void onContext() {
    for (int i = 0; i < numShapes; ++i) {
      int Nv = rnd::prob(0.5) ? (rnd::prob(0.5) ? addCube(shapesVBO)
                                                : addDodecahedron(shapesVBO))
                              : addIcosahedron(shapesVBO);

      // Scale and translate the newly added shape
      scatterSize = (float)numShapes * .001;
      Mat4f xfm;
      xfm.setIdentity();
      xfm.scale(Vec3f(rnd::uniform(1., 0.1), rnd::uniform(1., 0.1),
                      rnd::uniform(1., 0.1)));
      xfm.translate(Vec3f(rnd::uniformS(scatterSize),
                          rnd::uniformS(scatterSize),
                          rnd::uniformS(scatterSize)));
      shapesVBO.transform(xfm, shapesVBO.vertices().size() - Nv);

      // Color newly added vertices
      for (int i = 0; i < Nv; ++i) {
        float f = float(i) / Nv;
        shapesVBO.color(HSV(f * 0.1 + 0.2, 1, 1));
      }
    }

    // Convert to non-indexed triangles to get flat shading
    shapesVBO.decompress();
    shapesVBO.generateNormals();

    // Update the VBO after all calculations are completed
    shapesVBO.update();

    // set the far clipping plane out
    //
    lens().far(300);
    //
    // This stuff needs to have a valid OpenGL context to run
    //
    shapesVBO.init();
    shapesVBO.primitive(Graphics::TRIANGLES);
    shapesVBO.print();
  }

  void onDraw(Graphics& g) {
    // Run this only the first time
    //
    static bool hasRunOnce = false;
    if (!hasRunOnce) {
      hasRunOnce = true;
      onContext();
    }

    shader().uniform("lighting", 1.0);
    material();
    light();

    g.translate(0, -scatterSize, -scatterSize);
    g.rotate(t, 0, 1, 0);
    g.draw(shapesVBO);
  }
};

int main() { MyApp().start(); }
