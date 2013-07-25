#include "alloutil/al_OmniApp.hpp"
using namespace al;

struct MyApp : OmniApp {
  Mesh mesh;
  Light light;

  MyApp() {
    mesh.primitive(Graphics::TRIANGLES);
    addSphere(mesh, 1.0, 32, 32);
    for (int i = 0; i < mesh.vertices().size(); ++i) {
      float f = (float)i / mesh.vertices().size();
      mesh.color(Color(HSV(f, 1 - f, 1), 1));
    }
    mesh.generateNormals();
    light.ambient(Color(0.4, 0.4, 0.4, 1.0));
    light.pos(5, 5, 5);
  }

  virtual ~MyApp() {}
  
  virtual void onDraw(Graphics& g) {
    light();
    // say how much lighting you want
    shader().uniform("lighting", 1.0);
    g.draw(mesh);
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
