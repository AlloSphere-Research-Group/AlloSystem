#include <iostream>
#include "allocore/math/al_Vec.hpp"
using namespace al;

int main() {
  Vec3f f(1,2,3);
  std::cout << f;
  Vec<6,float> v;
  std::cout << v;
  Vec<2,unsigned char> u;
  u.x = 'x';
  u.y = 'y';
  std::cout << u;
  Vec<12,long double> w;
  for (int i = 0; i < w.size(); ++i)
    w.elems()[i] = 1.3f / i;
  std::cout << w;
}
