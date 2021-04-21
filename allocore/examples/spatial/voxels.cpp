
/*
AlloCore Example: Voxels

Description:
This shows basic usage of the Voxel class which represents a 3-dimensional vector with physical size units

Matt Wright, April 2015, matt@create.ucsb.edu

*/

#include <iostream>
#include "allocore/types/al_Voxels.hpp"

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

  //  loading voxel from image collection
  //  al::Voxels v("../../spherical_harmonic_generator/sample10_a");
  
  al::Voxels v(AlloFloat32Ty, 32, 32, 32, 0.1234, 0.1234, 8.5, al::VOX_NANOMETERS);


  // fill array:
  v.fill(arrayfiller);


  std::cout << "One voxel is " << v.printVoxWidth(0) << " by " << v.printVoxWidth(1) << " by " << 
    v.printVoxWidth(2) << " so its volume is " << (v.getVoxWidth(0) * v.getVoxWidth(1) * v.getVoxWidth(2)) <<
    " cubic " << v.printUnits() << std::endl;


  std::cout << "the upper left 12x12 of the 7th plane in Z looks like this: " << std::endl;

  int z = 7;
  for (int y = 0; y<11; y++) {
    for (int x = 0; x < 11; x++) {
      std::cout << v.elem<float>(1,x,y,z) << " ";
    }
    std::cout << std::endl;
  }
}

    
