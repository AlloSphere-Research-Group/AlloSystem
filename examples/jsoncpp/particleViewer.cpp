#include "alloutil/al_OmniApp.hpp"

#include "json/json.h"
#include <iostream>
#include <fstream>

#include <vector>

static Json::Value json;
static std::string errors;

using namespace al;


struct MyApp : OmniApp {
  struct Particle {
    std::string label;
    double x, y, z;
  };

  std::vector<Particle> particles;

  Mesh mesh;
  Light light;

  MyApp() {
    mesh.primitive(Graphics::TRIANGLES);
    addSphere(mesh, 0.125, 32, 32);
    for (int i = 0; i < mesh.vertices().size(); ++i) {
      float f = (float)i / mesh.vertices().size();
      mesh.color(Color(HSV(f, 1 - f, 1), 1));
    }
    mesh.generateNormals();
    light.ambient(Color(0.4, 0.4, 0.4, 1.0));
    light.pos(5, 5, 5);
    initAudio();


    std::string json_file_name = "particleList.json";

    static std::ifstream json_file(json_file_name.c_str(), std::ifstream::binary | std::ifstream::in);
    json_file >> json;
    json_file.close();

    if ( json.empty() ){
      std::cout  << "Failed to parse configuration." << std::endl;
      std::cout << errors << std::endl;
      std::cout << "Using default configuration." << std::endl;
      // return 1;
    } else {
      std::cout << "Parsed configuration file." << std::endl;
    }

    for (unsigned i=0; i<json["particles"].size(); i++) {
      particles.push_back(Particle());

      Json::Value& this_particle = json["particles"][i];
        particles.back().label = this_particle["label"].asString();
        particles.back().x = this_particle["x"].asDouble();
        particles.back().y = this_particle["y"].asDouble();
        particles.back().z = this_particle["z"].asDouble();
    }
  }

  virtual ~MyApp() {}

  virtual void onDraw(Graphics& g) {
    light();
    // say how much lighting you want
    shader().uniform("lighting", 1.0);

    g.translate(0,0,-0.5);


    for (std::vector<Particle>::iterator particle = particles.begin() ; particle != particles.end(); ++particle){
      g.pushMatrix();

      g.translate(particle->x, particle->y, particle->z);
      g.draw(mesh);

      g.popMatrix();
    }


  }

  virtual void onAnimate(al_sec dt) {
    // light.pos(nav().pos());
    pose = nav();
    // std::cout << dt << std::endl;
  }

  virtual void onSound(AudioIOData& io) {
    while (io()) {
      io.out(0) = rnd::uniformS() * 0.05;
    }
  }

  virtual void onMessage(osc::Message& m) {
    OmniApp::onMessage(m);
  }

  virtual bool onKeyDown(const Keyboard& k) { return true; }
};

int main(int argc, char* argv[]) {
  MyApp().start();
  return 0;
}
