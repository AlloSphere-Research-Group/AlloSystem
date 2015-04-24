/*	Allocore --
	Multimedia / virtual environment application class library

	Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology, UCSB.
	Copyright (C) 2012. The Regents of the University of California.
	All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

		Redistributions of source code must retain the above copyright notice,
		this list of conditions and the following disclaimer.

		Redistributions in binary form must reproduce the above copyright
		notice, this list of conditions and the following disclaimer in the
		documentation and/or other materials provided with the distribution.

		Neither the name of the University of California nor the names of its
		contributors may be used to endorse or promote products derived from
		this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
	IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
	ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
	LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
	CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
	POSSIBILITY OF SUCH DAMAGE.


        File description:
        Volume of voxels with physical size

        Voxels are basically just 3D AlloArrays with some size info

        File author(s):
        Matt Wright, 2015, matt@create.ucsb.edu

        Terminology suggestions for differentiating the number of
        pixels in each dimension (an integer) versus the physical
        length scale of a pixel in each dimension (e.g., a number of
        nanometers):

        dimx
        xnumcells
        xcellwidth
        xcellcount
        xcellsize
        xcount
        numx
        nx

        Vec3f voxelsize

*/

#ifndef INCLUDE_ALLO_VOXELS_HPP
#define INCLUDE_ALLO_VOXELS_HPP 1

//#include <stdlib.h>
#include <string>
#include "allocore/types/al_Array.hpp"


namespace al {

typedef float UnitsTy;

#define PICOMETERS -12
#define ANGSTROMS -10
#define NANOMETERS -9
#define MICROMETERS -6
#define MILLIMETERS -3
#define CENTIMETERS -2
#define METERS 0
#define KILOMETERS 3

const std::string UnitsName(UnitsTy t) {
    if (t == ANGSTROMS) {
      return "angstroms";
    } else if (t == NANOMETERS) {
      return "nm";
    } else if (t == MICROMETERS) {
      return "µm";
    } else if (t == MILLIMETERS) {
      return "mm";
    } else {
      std::string s = "";
      s += t;
      return "(m*10^" + s + ")";
      // The C++11 way:
      //      return "(m*10^" + std::to_string(t) + ")";
    }
}

/// OBJECT-oriented interface to AlloArray
class Voxels : public Array {
public:

  /// Construct dimx x dimy x dimz voxel grid giving 3D size of each voxel cuboid with units
  Voxels(AlloTy ty, uint32_t dimx, uint32_t dimy, uint32_t dimz, float sizex, float sizey, float sizez, UnitsTy units) :
       Array(1, ty, dimx, dimy, dimz) {
    m_sizex = sizex;
    m_sizey = sizey;
    m_sizez = sizez;
    m_units = units;
  }


  /// Construct dimx x dimy x dimz voxel grid giving 3D size of each voxel cuboid in meters
  Voxels(AlloTy ty, uint32_t dimx, uint32_t dimy, uint32_t dimz, float sizex, float sizey, float sizez) {
    Voxels(ty, dimx, dimy, dimz, sizex, sizey, sizez, METERS);
  }

  /// Construct dimx x dimy x dimz voxel grid giving dimension of each voxel cube with units
  Voxels(AlloTy ty, uint32_t dimx, uint32_t dimy, uint32_t dimz, float voxelsize, UnitsTy units) {
    Voxels(ty, dimx, dimy, dimz, voxelsize, voxelsize, voxelsize, units);
  }

  /// Construct dimx x dimy x dimz voxel grid with every voxel 1m x 1m x 1m
  Voxels(AlloTy ty, uint32_t dimx, uint32_t dimy, uint32_t dimz) {
    Voxels(ty, dimx, dimy, dimz, 1, 1, 1, METERS);
  }

  float sizex() { return m_sizex;}
  float sizey() { return m_sizey;}
  float sizez() { return m_sizez;}

  std::string sizexname() {
    return std::to_string(m_sizex) + " " + UnitsName(m_units);
  }

  std::string sizeyname() {
    return std::to_string(m_sizey) + " " + UnitsName(m_units);
  }

  std::string sizezname() {
    return std::to_string(m_sizez) + " " + UnitsName(m_units);
  }

  std::string unitsname() {
    return UnitsName(m_units);
  }

  ~Voxels() {
  }

protected:

  UnitsTy m_units;
  float m_sizex, m_sizey, m_sizez;

};




} // ::al::

#endif /* INCLUDE_ALLO_VOXELS_HPP */
