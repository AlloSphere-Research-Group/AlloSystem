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

#include <iostream> // FILE
#include <string>
#include <vector>
#include "allocore/types/al_Array.hpp"


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

  float   xlen;       /* cell dimensions in angstroms   */
  float   ylen;       /* - MRC2014 standard             */
  float   zlen;

  float   alpha;      /* cell angles                    */
  float   beta;
  float   gamma;

  int32_t   mapx;       /* map coloumn 1=x,2=y,3=z.       */
  int32_t   mapy;       /* map row     1=x,2=y,3=z.       */
  int32_t   mapz;       /* map section 1=x,2=y,3=z.       */

  float   amin;       /* Minimum pixel value            */
  float   amax;       /* Maximum pixel value            */
  float   amean;      /* Mean pixel value            */

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
  float   min2;
  float   max2;
  float   min3;
  float   max3;
  int32_t   imodStamp;
  int32_t   imodFlags;
  int16_t   idtype;
  int16_t   lens;
  int16_t   nd1;     /* Devide by 100 to get float value. */
  int16_t   nd2;
  int16_t   vd1;
  int16_t   vd2;
  float   tiltangles[6];  /* 0,1,2 = original:  3,4,5 = current */

  /* MRC 2000 standard */
  float   origin[3];
  char    cmap[4];          /* Contains "MAP " for LE, " PAM" for BE */
  char    machinestamp[4];  /* Little Endian : 68 65 17 17 // Big Endian : 17 17 65 68 */
  float   rms;            /* RMS deviation of densities from mean density */

  int32_t nlabl;  // number of labels
  char  labels[10][80];
};

/// OBJECT-oriented interface to AlloArray
///
/// @ingroup allocore
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
  


  void init(float voxWidthX, float voxWidthY, float voxWidthZ, UnitsTy units) {
    m_voxWidth[0] = voxWidthX;
    m_voxWidth[1] = voxWidthY;
    m_voxWidth[2] = voxWidthZ;
    m_units = units;
  }

  float getVoxWidth(unsigned int axis) const { return m_voxWidth[axis]; }
  void setVoxWidth(unsigned int axis, float voxWidth) { m_voxWidth[axis] = voxWidth; }

  std::string printVoxWidth(unsigned int axis) const;

  UnitsTy getUnits() const { return m_units; }
  void setUnits(UnitsTy units) { m_units = units; }

  std::string printUnits() const {
    return printUnits(m_units);
  }
  
  std::string printUnits(UnitsTy t) const;

  // functions to support MRC
  MRCHeader& parseMRC(const char * data);

  bool loadFromMRC(std::string filename, bool update = false);

  bool loadFromMRC(std::string filename, UnitsTy ty, float voxWidth);

  bool loadFromMRC(std::string filename, UnitsTy ty, float voxWidthX, float voxWidthY, float voxWidthZ);

  //functions for loading from images
  bool getdir(std::string dir, std::vector<std::string> &files);

  bool parseInfo(std::string dir, std::vector<std::string> &data);

  bool loadFromDirectory(std::string dir);
  
  //functions for slicing
  bool linePlaneIntersection(const Vec3f &P0, const Vec3f &P1, const Vec3f &planeCenter, const Vec3f &planeNormal, Vec3f* intersection);
  
  std::vector<Vec3f> linspace( Vec3f a, Vec3f b, int n);
  
  Vec3f point2Dto3D(Vec3f Q, Vec3f H, Vec3f K, float u, float v);
  
  bool parallelLinespace(
	Vec3f p0, Vec3f p1, Vec3f p2, Vec3f p3,
	std::vector<Vec3f> &list, std::vector<Vec3f> &list2,
	float aDirection, float oDirection,
	std::vector<Vec3f> &points
  );
  
  Array slice(Vec3f planeCenter, Vec3f planeNormal, std::vector<Vec3f> &finalPointList);

  // mostly for saving partial changes into mrc header.
  bool writeToMRC(std::string filename, MRCHeader& header);

  // write/read files for voxel class
  bool writeToFile(std::string filename);
  
  bool loadFromFile(std::string filename);
  
  void print(FILE * fp = stdout) const;

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
