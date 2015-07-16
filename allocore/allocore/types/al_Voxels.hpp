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
        Kon Hyong Kim, 2014, konhyong@gmail.com

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
#define INCLUDE_ALLO_VOXELS_HPP

//#include <stdlib.h>
#include <string>
#include <sstream>  
#include <dirent.h>
#include <cassert>
#include <iostream>
#include <fstream>
#include "allocore/types/al_Array.hpp"
#include "allocore/al_Allocore.hpp"

using namespace al;
using namespace std;




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

/// OBJECT-oriented interface to AlloArray
class Voxels : public Array {
public:
  Voxels() :
      Array() {
    init(1,1,1, METERS);
  }

  /// Construct dimx x dimy x dimz voxel grid giving 3D size of each voxel cuboid with units
  Voxels(AlloTy ty, uint32_t dimx, uint32_t dimy, uint32_t dimz, float sizex, float sizey, float sizez, UnitsTy units) :
       Array(1, ty, dimx, dimy, dimz) {
    init(sizex, sizey, sizez, units);
  }


  /// Construct dimx x dimy x dimz voxel grid giving 3D size of each voxel cuboid in meters
  Voxels(AlloTy ty, uint32_t dimx, uint32_t dimy, uint32_t dimz, float sizex, float sizey, float sizez) :
      Array(1, ty, dimx, dimy, dimz) {
    init(sizex, sizey, sizez, METERS);
  }

  /// Construct dimx x dimy x dimz voxel grid giving dimension of each voxel cube with units
  Voxels(AlloTy ty, uint32_t dimx, uint32_t dimy, uint32_t dimz, float voxelsize, UnitsTy units) :
      Array(1, ty, dimx, dimy, dimz) {
    init(voxelsize, voxelsize, voxelsize, units);
  }

  /// Construct dimx x dimy x dimz voxel grid with every voxel 1m x 1m x 1m
  Voxels(AlloTy ty, uint32_t dimx, uint32_t dimy, uint32_t dimz) :
      Array(1, ty, dimx, dimy, dimz) {
    init(1, 1, 1, METERS);
  }

  int getdir(string dir, vector<string> &files) {
    DIR *dp;
    struct dirent * dirp;
    if((dp = opendir(dir.c_str())) == NULL)	{
      //cout << "Error(" << errno << ") opening " << dir << endl;
      return 1;
    }
    while((dirp = readdir(dp)) != NULL) {
      char *name = dirp->d_name;
      if (strcmp(name, ".") && strcmp(name, "..") && strcmp(name, "info.txt")) {
	cout << name << endl;
	files.push_back(dir + "/" + string(name));
      }
    }
    closedir(dp);
    return 0;
  }
  
    
  int parseInfo(string dir, vector<string> &data){
    string file = dir + "/info.txt";
    cout << file << endl;
    ifstream infile(file.c_str());
    if (!infile.good())
      return 1; // exit if file not found

    string strOneLine;

    while (infile)
    {
      getline(infile, strOneLine);
      if (strOneLine.length() > 0){
	data.push_back(strOneLine.substr(strOneLine.find(":")+2,strOneLine.length()));
      }
    }

    infile.close();

    return 0;
  }


  Voxels(string dir) : Array() {
     
    vector<string> files;
    vector<string> info;

    // Image and Texture handle reading and displaying image files.
    Image RGBImage; // for reading into
    
    if (getdir(dir,files) != 0) {
      cout << "Problem reading directory " << dir << endl;
      exit(-1);
    }

    if (files.size() == 0) {
      cout << "Read zero files from directory " << dir << endl;
      exit(-2);
    }

    cout << "Judging by " << dir << " there are " << files.size() << " images (or at least files)" << endl;


    // Try reading the first one just to get the size
    if (!RGBImage.load(files[0])) {
      cout << "Couldn't read file " << files[0] << endl;
      exit(-3);
    }

    int nx = RGBImage.width();
    int ny = RGBImage.height();
    int nz = files.size();
    float vx = 1.;
    float vy = 1.;
    float vz = 1.;
    float type = NANOMETERS;

    if (parseInfo(dir,info) == 0){
      if (info.size() == 4){
	type = atoi(info[0].c_str());
	vx = atof(info[1].c_str());
	vy = atof(info[2].c_str());
	vz = atof(info[3].c_str());
	cout << "imported values from info.txt: " << type << ", " << vx << ", " << vy << ", " << vz << endl;
      } else {
	cout << info.size() << " info.txt doesn't have enough info, using default data" << endl;
      }
    } else {
      cout << "no info.txt, using default data" << endl;
    }

    cout << "Judging by " << files[0] << " each image should be " << nx << " by " << ny << endl;

    // For now assume 8-bit with 1 nm cube voxels
    format(1, AlloUInt8Ty, nx, ny, nz);
    init(vx,vy,vz,type);


    // Iterate through entire directory
    int slice = 0;
    for (vector<string>::iterator it = files.begin(); it != files.end(); ++it, ++slice) {
      string &filename = *it;

      if (RGBImage.load(filename)) {
	cout << "loaded " << filename << 
	  " (" << slice+1 << " of " << files.size() << ")" << endl;
      } else {
	cout << "Failed to read image from " << filename << endl;
	exit(-4);
      }

      // Verify XY resolution
      if (RGBImage.width() != nx || RGBImage.height() != ny) {
	cout << "Error:  resolution mismatch!" << endl;
	cout << "   " << files[0] << ": " << nx << " by " << ny << endl;
	cout << "   " << filename << ": " << RGBImage.width() << " by " << RGBImage.height() << endl;
	exit(-5);
      }
	

      // Access the read-in image data
      Array& array(RGBImage.array());
      
      // For now assume 8-bit RGBA
      Image::RGBAPix<uint8_t> pixel;

      // Copy it out pixel-by-pixel:
      for (size_t row = 0; row < array.height(); ++row) {
	for (size_t col = 0; col < array.width(); ++col) {
	  array.read(&pixel, col, row);

	  // For now we'll take only the red and put it in the single component; that's lame.
	  elem<char>(0, col, row, slice) = (char) pixel.r;
	}
      }
    }
    // v is ready

   

  }

  
  void init(float sizex, float sizey, float sizez, UnitsTy units) {
    m_sizex = sizex;
    m_sizey = sizey;
    m_sizez = sizez;
    m_units = units;
  }

  float sizex() { return m_sizex;}
  float sizey() { return m_sizey;}
  float sizez() { return m_sizez;}

  std::string sizexname() {
    std::ostringstream ss;
    ss << m_sizex << " " << UnitsName(m_units);

    return ss.str();
    //    return std::to_string(m_sizex) + " " + UnitsName(m_units);
  }

  std::string sizeyname() {
    std::ostringstream ss;
    ss << m_sizey << " " << UnitsName(m_units);
    return ss.str();
    //    return std::to_string(m_sizey) + " " + UnitsName(m_units);
  }

  std::string sizezname() {
    std::ostringstream ss;
    ss << m_sizez << " " << UnitsName(m_units);
    return ss.str();

    //    return std::to_string(m_sizez) + " " + UnitsName(m_units);
  }

  std::string unitsname() {
    return UnitsName(m_units);
  }
  
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
      std::ostringstream ss;
      ss << "(m*10^" << t << ")";
      return ss.str();
      //      return "(m*10^" + std::to_string(t) + ")";
    }
  }
  
  bool writeToFile(std::string filename);
  
  bool loadFromFile(std::string filename);
  
  void print(FILE * fp = stdout);

  ~Voxels() {
  }

protected:

  UnitsTy m_units;
  float m_sizex, m_sizey, m_sizez;

};

} // ::al::

#endif /* INCLUDE_ALLO_VOXELS_HPP */
