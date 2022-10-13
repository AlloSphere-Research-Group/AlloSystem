/*
  Karl Yerkes 2012.08.05

  Shows how to query OpenGL state.  See 'man glGet' for more information.

*/

#include <iostream>
#include "allocore/io/al_Window.hpp"
#include "allocore/graphics/al_Graphics.hpp"

struct Foo : al::Window {

  virtual bool onCreate() override {

    // We know there's a valid OpenGL context now
    //
    GLint bits;

    glGetIntegerv(GL_RED_BITS, &bits);
    std::cout << bits << " bits of red" << std::endl;

    glGetIntegerv(GL_GREEN_BITS, &bits);
    std::cout << bits << " bits of green" << std::endl;

    glGetIntegerv(GL_BLUE_BITS, &bits);
    std::cout << bits << " bits of blue" << std::endl;

    glGetIntegerv(GL_ALPHA_BITS, &bits);
    std::cout << bits << " bits of alpha" << std::endl;

    glGetIntegerv(GL_DEPTH_BITS, &bits);
    std::cout << bits << " bits of depth" << std::endl;

    return true;
  }
};

int main(int argc, char* argv[]) {
  Foo foo;
	foo.create();
  return 0;
}
