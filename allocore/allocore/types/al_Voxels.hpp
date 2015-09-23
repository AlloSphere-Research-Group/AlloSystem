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
        Voxel Array Class for scientific volumetric data, containing
        physical metadata

        Base structure utilizes 3D AlloArray class, with support for
        MRC file format

        File author(s):
        Matt Wright, 2015, matt@create.ucsb.edu
        Kon Hyong Kim, 2014, konhyong@gmail.com
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

typedef int UnitsTy;

enum VoxelUnits {
  VOX_PICOMETERS = -12,
  VOX_ANGSTROMS = -10,
  VOX_NANOMETERS = -9,
  VOX_MICROMETERS = -6,
  VOX_MILLIMETERS = -3,
  VOX_CENTIMETERS = -2,
  VOX_METERS = 0,
  VOX_KILOMETERS = 3
};

enum MRCMode {
  MRC_IMAGE_SINT8 = 0,    //image : signed 8-bit bytes range -128 to 127
  MRC_IMAGE_SINT16 = 1,   //image : 16-bit halfwords
  MRC_IMAGE_FLOAT32 = 2,    //image : 32-bit reals
  MRC_TRANSFORM_INT16 = 3,  //transform : complex 16-bit integers
  MRC_TRANSFORM_FLOAT32 = 4,  //transform : complex 32-bit reals
  MRC_IMAGE_UINT16 = 6        //image : unsigned 16-bit range 0 to 65535
};

struct MRCHeader {
  // @see http://bio3d.colorado.edu/imod/doc/mrc_format.txt

  int32_t   nx;         /*  # of Columns                  */
  int32_t   ny;         /*  # of Rows                     */
  int32_t   nz;         /*  # of Sections.                */
  int32_t   mode;       /*  given by #define MRC_MODE...  */

  int32_t   nxstart;    /*  Starting point of sub image.  */
  int32_t   nystart;
  int32_t   nzstart;

  int32_t   mx;         /* Number of intervals along x,y,z*/
  int32_t   my;
  int32_t   mz;

  float_t   xlen;       /* cell dimensions in angstroms   */
  float_t   ylen;       /* - MRC2014 standard             */
  float_t   zlen;

  float_t   alpha;      /* cell angles                    */
  float_t   beta;
  float_t   gamma;

  int32_t   mapx;       /* map coloumn 1=x,2=y,3=z.       */
  int32_t   mapy;       /* map row     1=x,2=y,3=z.       */
  int32_t   mapz;       /* map section 1=x,2=y,3=z.       */

  float_t   amin;       /* Minimum pixel value            */
  float_t   amax;       /* Maximum pixel value            */
  float_t   amean;      /* Mean pixel value            */

  int16_t   ispg;       /* image type */
  int16_t   nsymbt;     /* space group number */

  /* IMOD-SPECIFIC */
  int32_t   next;
  int16_t   creatid;  /* Used to be creator id, hvem = 1000, now 0 */
  char    blank[30];
  int16_t   nint;
  int16_t   nreal;
  int16_t   sub;
  int16_t   zfac;
  float_t   min2;
  float_t   max2;
  float_t   min3;
  float_t   max3;
  int32_t   imodStamp;
  int32_t   imodFlags;
  int16_t   idtype;
  int16_t   lens;
  int16_t   nd1;     /* Devide by 100 to get float value. */
  int16_t   nd2;
  int16_t   vd1;
  int16_t   vd2;
  float_t   tiltangles[6];  /* 0,1,2 = original:  3,4,5 = current */

  /* MRC 2000 standard */
  float_t   origin[3];
  char    cmap[4];          /* Contains "MAP " for LE, " PAM" for BE */
  char    machinestamp[4];  /* Little Endian : 68 65 17 17 // Big Endian : 17 17 65 68 */
  float_t   rms;            /* RMS deviation of densities from mean density */

  int32_t nlabl;  // number of labels
  char  labels[10][80];
};

/// OBJECT-oriented interface to AlloArray
class Voxels : public Array {
public:
  Voxels() :
      Array() {
    init(1,1,1, VOX_METERS);
  }

  /// Construct dimx x dimy x dimz voxel grid giving 3D size of each voxel cuboid with units
  Voxels(AlloTy ty, uint32_t dimx, uint32_t dimy, uint32_t dimz, float sizex, float sizey, float sizez, UnitsTy units) :
       Array(1, ty, dimx, dimy, dimz) {
    init(sizex, sizey, sizez, units);
  }


  /// Construct dimx x dimy x dimz voxel grid giving 3D size of each voxel cuboid in meters
  Voxels(AlloTy ty, uint32_t dimx, uint32_t dimy, uint32_t dimz, float sizex, float sizey, float sizez) :
      Array(1, ty, dimx, dimy, dimz) {
    init(sizex, sizey, sizez, VOX_METERS);
  }

  /// Construct dimx x dimy x dimz voxel grid giving dimension of each voxel cube with units
  Voxels(AlloTy ty, uint32_t dimx, uint32_t dimy, uint32_t dimz, float voxelsize, UnitsTy units) :
      Array(1, ty, dimx, dimy, dimz) {
    init(voxelsize, voxelsize, voxelsize, units);
  }

  /// Construct dimx x dimy x dimz voxel grid with every voxel 1m x 1m x 1m
  Voxels(AlloTy ty, uint32_t dimx, uint32_t dimy, uint32_t dimz) :
      Array(1, ty, dimx, dimy, dimz) {
    init(1, 1, 1, VOX_METERS);
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

/*
   sliceassembler

   Voxel data import aka slice assembler aka volume reconstructor 

   by Matt Wright, April 2015
   and Hannah Wolfe, July 2015

   based on tiExporter.cpp from images2raw.cpp by Coby Kaufer
   <cobykaufer@bluejayke.com>, Karl Yerkes <karl.yerkes@gmail.com>,
   and Matt Wright <matt@create.ucsb.edu

   Read in a directory full of 2D image files with some naming
   convention, assemble them all into an al::Array or al::Voxels
   and write the result as one huge fast-to-load raw binary data file.


   Limitations:

   - Images must contain 8-bit RGB pixels

   - Ignores all but the red channel

   - Chokes if directory contains anything besides "info.txt" and image files

   - Creates a voxel from the data
*/

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
    float type = VOX_NANOMETERS;

    if (parseInfo(dir,info) == 0) {
      if (info.size() == 4) {
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

  
  void init(float voxWidthX, float voxWidthY, float voxWidthZ, UnitsTy units) {
    m_voxWidth[0] = voxWidthX;
    m_voxWidth[1] = voxWidthY;
    m_voxWidth[2] = voxWidthZ;
    m_units = units;
  }

  float getVoxWidth(unsigned int axis) const { return m_voxWidth[axis]; }
  void setVoxWidth(unsigned int axis, float voxWidth) { m_voxWidth[axis] = voxWidth; }

  std::string printVoxWidth(unsigned int axis) {
    std::ostringstream ss;
    ss << m_voxWidth[axis] << " " << printUnits(m_units);

    return ss.str();
  }

  UnitsTy getUnits() const { return m_units; }
  void setUnits(UnitsTy units) { m_units = units; }

  std::string printUnits() {
    return printUnits(m_units);
  }
  
  const std::string printUnits(UnitsTy t) {
    if (t == VOX_ANGSTROMS) {
      return "angstroms";
    } else if (t == VOX_NANOMETERS) {
      return "nm";
    } else if (t == VOX_MICROMETERS) {
      return "µm";
    } else if (t == VOX_MILLIMETERS) {
      return "mm";
    } else {
      std::ostringstream ss;
      ss << "(m*10^" << t << ")";
      return ss.str();
    }
  }

  // functions to support MRC
  MRCHeader& parseMRC(const char * data);

  bool loadFromMRC(std::string filename, bool update = false);

  bool loadFromMRC(std::string filename, UnitsTy ty, float voxWidth);

  bool loadFromMRC(std::string filename, UnitsTy ty, float voxWidthX, float voxWidthY, float voxWidthZ);

  // mostly for saving partial changes into mrc header.
  bool writeToMRC(std::string filename, MRCHeader& header);

  // write/read files for voxel class
  bool writeToFile(std::string filename);
  
  bool loadFromFile(std::string filename);
  
  void print(FILE * fp = stdout);

  float min() const { return m_min; }

  float max() const { return m_max; }

  float mean() const { return m_mean; }

  float rms() const { return m_rms; }

  ~Voxels() {
  }

protected:

  UnitsTy m_units;
  float m_voxWidth[3];
  float m_min, m_max, m_mean, m_rms;
};

} // namespace al

#endif /* INCLUDE_ALLO_VOXELS_HPP */
