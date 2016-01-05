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
  VBO's require a GL window context to work, so init() should take place in OnCreate,
  onAnimate, or onDraw. After that, update() should be used when data changes.
	BufferDataUsage must be set during init() as it's only used once when first setting
	the buffers.

  TO DO:
  [ ] test texture mapping
	[v] test = operators and copyFrom
	[ ] better performance testing. fpsActual() maxes out at display's refresh rate (~60fps)
	[v] throw exception if no vertices exist during allocate; or use asserts (Andres)
	[*] move things to protected; *- changed "init" to "allocate", added get..() (Andres)
	[v] replace uint with uint32_t and bring in cstdint (Andres)
	[v] prefix member variables with 'm' (Andres)
	[ ] Doxygen (Andres)
	[ ] method for deleting underlying Mesh buffers (Andres)
	[ ] think about making all Mesh methods into shaders or some GPU solution (Andres)
	[ ] is drawInstanced possible?
	[ ] autoUpdate? dirty state for Mesh

*/

#include "allocore/graphics/al_Mesh.hpp"
#include "allocore/graphics/al_OpenGL.hpp"
#include <cstdint>

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

	MeshVBO();
	MeshVBO(Mesh& cpy);

	void copyFrom(Mesh& cpy);
	void copyFrom(MeshVBO& cpy);

  void operator=(Mesh& cpy);
  void operator=(MeshVBO& cpy);

  void allocate(BufferDataUsage usage = STREAM_DRAW);
	void update();
	void clear();

	void bind();
  void unbind();

	uint32_t getVertId();
	uint32_t getNormalId();
	uint32_t getColorId();
	uint32_t getTexCoordId();
	uint32_t getIndexId();

	int getNumVertices();
	int getNumIndices();

	bool isAllocated();
	bool isBound();

	bool hasNormals();
	bool hasColors();
	bool hasTexCoord2s();
	bool hasTexCoord3s();
	bool hasIndices();

protected:
	BufferDataUsage mBufferUsage = STREAM_DRAW;

	uint32_t mVertId = 0;
	uint32_t mNormalId = 0;
	uint32_t mColorId = 0;
	uint32_t mTexCoordId = 0;
	uint32_t mIndexId = 0;

	int mNumVertices = 0;
	int mNumIndices = 0;
	int mVertStride = sizeof(Vec3f);
	int mNormalStride = sizeof(Vec3f);
	int mColorStride = sizeof(Color);
	int mTexCoordStride = sizeof(TexCoord2);
	int mIndexStride = sizeof(uint32_t);

	bool mAllocated = false;
	bool mBound = false;

	bool mHasNormals = false;
	bool mHasColors = false;
	bool mHasTexCoord2s = false;
	bool mHasTexCoord3s = false;
	bool mHasIndices = false;

	template <class T>
	void setData(const T *src, uint32_t *bufferId, int total, BufferDataUsage usage, int bufferTarget);

	template <class T>
	void updateData(const T *src, uint32_t *bufferId, int total, int bufferTarget);

	// void autoUpdate();

  // void enableColors();
  // void enableNormals();
  // void enableTexCoords();
  // void enableIndices();

  // void disableColors();
  // void disableNormals();
  // void disableTexCoords();
  // void disableIndices();
};

} // al::

#endif
