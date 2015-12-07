#ifndef INCLUDE_AL_GRAPHICS_MESH_VBO_HPP
#define INCLUDE_AL_GRAPHICS_MESH_VBO_HPP

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
  An extension of Mesh which allows for easy vertex buffer object usage

  File author:
  Kurt Kaminski, 2015, kurtoon@gmail.com

  Usage:
  VBO's require a GL context to work, so init() should take place in OnCreate,
  or anytime after a window has been initialized. Adding vertices and anything else
  done with a regular Mesh may be done before initWindow().

  Q: What usage symbol should I use?
  A:  GL_STREAM_DRAW when the data will be modified once and used (i.e., drawn) at most a few times.
      GL_STATIC_DRAW when the data will be modified once and used many times.
      GL_DYNAMIC_DRAW when the data will be modified repeatedly and used many times.

  TO DO:
  [ ] test texture mapping
	[ ] test = operators and copyFrom
	[ ] better performance testing. fpsActual() maxes out at display's refresh rate (~60fps)

  QUESTIONS:
	[ ] BufferObject?
	[ ] Where to put enumerations? al_Graphics? take from BufferObject? Intended for users...
	[ ] is drawInstanced possible?
	[ ] Best way to make a "dirty" state for Mesh?
			true = mesh has changed; false = onDraw()?
*/

#include "allocore/graphics/al_Mesh.hpp"
#include "allocore/graphics/al_OpenGL.hpp"

namespace al{

class MeshVBO : public Mesh {
public:

  // /*
	// "Static" means the data in VBO will not be changed, "Dynamic" means the data
	// will be changed frequently, and "Stream" means the data will be changed every
	// frame. "Draw" means the data will be sent to the GPU from the application,
	// "Read" means the data will be read by the application from the GPU, and "Copy"
	// means the data will be copied internally on the GPU.
	// */
	enum BufferDataUsage{
		STREAM_DRAW		= GL_STREAM_DRAW,
		//STREAM_READ		= GL_STREAM_READ,
		//STREAM_COPY		= GL_STREAM_COPY,
		STATIC_DRAW		= GL_STATIC_DRAW,
		//STATIC_READ		= GL_STATIC_READ,
		//STATIC_COPY		= GL_STATIC_COPY,
		DYNAMIC_DRAW	= GL_DYNAMIC_DRAW,
		//DYNAMIC_READ	= GL_DYNAMIC_READ,
		//DYNAMIC_COPY	= GL_DYNAMIC_COPY
	};

	BufferDataUsage bufferUsage = STREAM_DRAW;

  uint vertId = 0;
  uint normalId = 0;
  uint colorId = 0;
  uint texCoordId = 0;
  uint indexId = 0;

  int num_vertices = 0;
  int num_indices = 0;
  int vertStride = sizeof(Vec3f);
  int normalStride = sizeof(Vec3f);
  int colorStride = sizeof(Color);
  int texCoordStride = sizeof(TexCoord2);
  int indexStride = sizeof(uint);

  bool isInit = false;
  bool isBound = false;

  bool hasNormals = false;
  bool hasColors = false;
  bool hasTexCoord2s = false;
  bool hasTexCoord3s = false;
  bool hasIndices = false;

	MeshVBO();
	MeshVBO(Mesh& cpy);

	void copyFrom(Mesh& cpy);
	void copyFrom(MeshVBO& cpy);

  void operator=(Mesh& cpy);
  void operator=(MeshVBO& cpy);

  void init(BufferDataUsage usage = STREAM_DRAW);
	void update();

	template <class T>
	void setData(const T * src, uint * bufferId, int total, BufferDataUsage usage, int bufferTarget);

	template <class T>
	void updateData(const T * src, uint * bufferId, int total, int bufferTarget);

	// void autoUpdate();

  // void enableColors();
  // void enableNormals();
  // void enableTexCoords();
  // void enableIndices();

  // void disableColors();
  // void disableNormals();
  // void disableTexCoords();
  // void disableIndices();

  void bind();
  void unbind();

  void clear();

};

} // al::

#endif
