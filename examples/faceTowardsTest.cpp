#include "allocore/io/al_App.hpp"

using namespace al;


struct AlloApp : App {

  AlloApp() {
    nav().pos(0, 0, 0);
    Vec3f whatIWantToLookAt = Vec3f(1,0,0);

    for (int i = 0; i<10; ++i) {
          std::cout << "i is " << i << std::endl;

          std::cout << "nav().pos() is " << nav().pos() << std::endl;
          std::cout << "nav().smooth() is " << nav().smooth() << std::endl;

          std::cout << "whatIWantToLookAt is " << whatIWantToLookAt << std::endl;
          
          nav().faceToward(whatIWantToLookAt, .5);
          
          std::cout << "now my forward vector is " << nav().uf() << std::endl;
          std::cout << "calling nav().step(0)..." << std::endl;
          
          nav().step(0);
          
          std::cout << "now my forward vector is " << nav().uf() << std::endl;
    }
  }
};

int main(int argc, char* argv[]) {
  AlloApp app;
  app.start();
  return 0;
}
