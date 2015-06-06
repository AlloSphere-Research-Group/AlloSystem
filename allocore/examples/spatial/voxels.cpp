
/*
AlloCore Example: Voxels

Description:
This shows basic usage of the Voxel class which represents a 3-dimensional vector with physical size units

Matt Wright, April 2015, matt@create.ucsb.edu

*/

#include "allocore/types/al_Voxels.hpp"
//#include "al_Allocore.hpp"
//#include "al_Voxels.hpp"

#include <iostream>


// function used to initialize array data (from allocore/examples/graphics/texture3d.cpp)

void arrayfiller(float * values, double normx, double normy, double normz) {
	double snormx = normx-0.5;
	double snormy = normy-0.5;
	double snormz = normz-0.5;
	double snorm3 = snormx*snormx + snormy*snormy + snormz*snormz;

        double offset = 0.1234;

	values[0] = sin((snorm3 + offset) * M_PI);
	values[1] = cos((snorm3 + offset) * M_2PI);
	values[2] = sin((snorm3 + offset) * M_PI_2);
}


int main() {
  al::Voxels v(AlloFloat32Ty, 32, 32, 32, 0.1234, 0.1234, 8.5, NANOMETERS);


  // fill array:
  v.fill(arrayfiller);


  std::cout << "One voxel is " << v.sizexname() << " by " << v.sizeyname() << " by " << 
    v.sizezname() << " so its volume is " << (v.sizex() * v.sizey() * v.sizez()) <<
    " cubic " << v.unitsname() << std::endl;


  std::cout << "the upper left 12x12 of the 7th plane in Z looks like this: " << std::endl;

  int z = 7;
  for (int y = 0; y<11; y++) {
    for (int x = 0; x < 11; x++) {
      std::cout << v.elem<float>(1,x,y,z) << " ";
    }
    std::cout << std::endl;
  }
}

    
