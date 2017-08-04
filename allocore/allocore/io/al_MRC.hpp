/*  Allocore --
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
        Support for reading/writing MRC files and updating header data

        File author(s):
        Kon Hyong Kim, 2017, konhyong@gmail.com
*/

#ifndef INCLUDE_ALLO_MRC_HPP
#define INCLUDE_ALLO_MRC_HPP

#include <cmath>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>
#include "allocore/types/al_Array.hpp"
#include "allocore/types/al_Conversion.hpp"


namespace al {

typedef int UnitsTy;

enum LengthUnits {
  AL_PICOMETERS = -12,
  AL_ANGSTROMS = -10,
  AL_NANOMETERS = -9,
  AL_MICROMETERS = -6,
  AL_MILLIMETERS = -3,
  AL_CENTIMETERS = -2,
  AL_METERS = 0,
  AL_KILOMETERS = 3
};

enum MRCMode {
  MRC_IMAGE_SINT8 = 0,    //image : signed 8-bit bytes range -128 to 127
  MRC_IMAGE_SINT16 = 1,   //image : 16-bit halfwords
  MRC_IMAGE_FLOAT32 = 2,    //image : 32-bit reals
  MRC_TRANSFORM_INT16 = 3,  //transform : complex 16-bit integers
  MRC_TRANSFORM_FLOAT32 = 4,  //transform : complex 32-bit reals
  MRC_IMAGE_UINT16 = 6        //image : unsigned 16-bit range 0 to 65535
};

// http://www.ccpem.ac.uk/mrc_format/mrc2014.php
struct MRCHeader {
  int32_t   nx;         /*  # of Columns (fast axis)      */
  int32_t   ny;         /*  # of Rows (medium axis)       */
  int32_t   nz;         /*  # of Sections (slow axis)     */
  int32_t   mode;       /*  given by #define MRC_MODE...  */

  int32_t   nxstart;    /*  location of first col/row/sec in unit cell  */
  int32_t   nystart;
  int32_t   nzstart;

  int32_t   mx;         /* sampling along axis of unit cell */
  int32_t   my;
  int32_t   mz;

  float     cella[3];   /* cell dimensions in angstroms   */
  float     cellb[3];   /* cell angles in degrees         */

  int32_t   mapc;       /* column axis  1=x,2=y,3=z.     */
  int32_t   mapr;       /* row axis     1=x,2=y,3=z.     */
  int32_t   maps;       /* section axis 1=x,2=y,3=z.     */

  // may not be kept up-to-date:
  //   max < min, or mean < (max,min), or rms < 0 means data is not current
  float     dmin;       /* Minimum density value         */
  float     dmax;       /* Maximum density value         */
  float     dmean;      /* Mean density value            */

  int32_t   ispg;       /* image type */
  int32_t   nsymbt;     /* space group number 24 */

  int32_t   extra[25];  /* 0 by default. might contain extended header info */

  float     origin[3];        /* phase origin(pixels) or origin of subvolume */
  char      cmap[4];          /* Contains "MAP " for LE, " PAM" for BE */
  char      machinestamp[4];  /* LE: 68 68 00 00 / BE : 17 17 00 00 */
  float     rms;              /* RMS deviation of densities from mean density */

  int32_t   nlabl;            /* number of labels */
  char      labels[10][80];   /* text labels */
};


class MRC {
public:
  // constructors
  MRC() {}
  MRC(std::string filename);

  // read/write MRC file
  bool loadFromMRC(std::string filename);
  bool writeToMRC(std::string filename);

  // read binary file
  // bool loadFromBIN(std::string filename);

  // read image directory
  // bool getdir(std::string dir, std::vector<std::string> &files);
  // bool parseInfo(std::string dir, std::vector<std::string> &data);
  // bool loadFromDirectory(std::string dir);
  
  // header functions
  MRCHeader& header() { return m_header; }
  MRCHeader& parseMRC(const char * data);

  Array& array() { return m_array; }
  char* dataPtr() { return m_array.data.ptr; }

  // utility functions
  void print(FILE * fp = stdout);
  float min() const { return m_header.dmin; }
  float max() const { return m_header.dmax; }
  float mean() const { return m_header.dmean; }
  float rms() const { return m_header.rms; }

  // deconstructor
  ~MRC() {}

protected:
  MRCHeader m_header;
  Array m_array;
};

} // namespace al

#endif /* INCLUDE_ALLO_MRC_HPP */
