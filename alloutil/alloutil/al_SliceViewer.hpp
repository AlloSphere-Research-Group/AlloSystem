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
        Support for viewing slices of volumetric data

        File author(s):
        Hannah Wolfe, 2017
        Kon Hyong Kim, 2017, konhyong@gmail.com
*/

#ifndef INCLUDE_ALLO_SLICEVIEWER_HPP
#define INCLUDE_ALLO_SLICEVIEWER_HPP

#include <cmath>
#include <iostream>
#include <vector>
#include "allocore/types/al_Array.hpp"
#include "allocore/io/al_MRC.hpp"

namespace al {

class SliceViewer {
public:
  SliceViewer() {}

  void init(MRC& mrc) {
    m_voxWidth[0] = mrc.header().cella[0];
    m_voxWidth[1] = mrc.header().cella[1];
    m_voxWidth[2] = mrc.header().cella[2];
    m_array = &mrc.array();
  }

  bool valid() {
    if (m_array) return true;
    else return false;
  }

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

  ~SliceViewer() {
  }

protected:
  float m_voxWidth[3];
  Array *m_array;
};

} // namespace al

#endif /* INCLUDE_ALLO_SLICEVIEWER_HPP */