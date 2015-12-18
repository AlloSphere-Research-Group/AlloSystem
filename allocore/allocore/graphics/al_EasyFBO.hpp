#ifndef __EASYFBO_HPP__
#define __EASYFBO_HPP__

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
  FBO helper, wrapper around fbo, rbo, texture, pose, projectionMatrix..

  File author(s):
  Tim Wood, 2015, fishuyo@gmail.com
*/

#include "allocore/graphics/al_Graphics.hpp"
#include "allocore/graphics/al_FBO.hpp"
#include "allocore/graphics/al_Texture.hpp"
#include "allocore/spatial/al_Pose.hpp"

namespace al {

/// Encapsulates FBO, depth buffer, and texture
/// @ingroup allocore
///
struct EasyFBO {

  int w, h;
  Texture texture;
  RBO rbo;
  FBO fbo;

  Pose pose;
  Matrix4d projectionMatrix;

  Color clearColor;

  Viewport savedViewport;

  EasyFBO(){}

  void init(int _w=512, int _h=512){
    // both depth and color attachees must be valid on the GPU before use:
    w = _w;
    h = _h;

    clearColor = Color(0,0,0,0);
    projectionMatrix = Matrix4d::ortho(-2, 2, -2, 2, 0.01, 100);

    rbo.resize(w, h);
    //texture.filterMin(Texture::LINEAR_MIPMAP_LINEAR);
    texture.resize(w,h);
    texture.validate();

    fbo.attachRBO(rbo, FBO::DEPTH_ATTACHMENT);
    fbo.attachTexture2D(texture.id(), FBO::COLOR_ATTACHMENT0);
    printf("fbo status %s\n", fbo.statusString());
  }

  void projection(Matrix4d proj){
    projectionMatrix.set(proj);
  }
  Matrix4d& projection(){ return projectionMatrix; }

  void begin(Graphics &gl){
    gl.pushMatrix(Graphics::PROJECTION);
    gl.pushMatrix(Graphics::MODELVIEW);
    savedViewport = gl.viewport();

    fbo.begin();
      gl.viewport(0, 0, w, h);
      gl.clearColor(clearColor);
      gl.clear(Graphics::COLOR_BUFFER_BIT | Graphics::DEPTH_BUFFER_BIT);

      // gl.projection(Matrix4d::ortho(-viewWidth, viewWidth, -viewHeight, viewHeight, viewNear, viewFar));
      // gl.projection(Matrix4d::perspective(10, 1, 0.001, 100));
      gl.projection(projectionMatrix);
      gl.modelView(Matrix4d::lookAt(pose.pos(), pose.uf(), pose.uu()));

  }

  void end(Graphics &gl){
    fbo.end();
    gl.popMatrix(Graphics::PROJECTION);
    gl.popMatrix(Graphics::MODELVIEW);
    gl.viewport(savedViewport);
    // texture.generateMipmap();
  }

};

} // al::

#endif
