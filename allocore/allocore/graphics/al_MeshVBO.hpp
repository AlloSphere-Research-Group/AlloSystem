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
	[ ] method for deleting underlying Mesh buffers (Andres)
	[ ] think about making all Mesh methods into shaders or some GPU solution (Andres)
	[ ] is drawInstanced possible?
	[ ] autoUpdate? dirty state for Mesh

*/

/// MeshVBO
///
/// @ingroup allocore
#include "allocore/graphics/al_Mesh.hpp"
#include "allocore/graphics/al_OpenGL.hpp"
#include <cstdint>

namespace al{

/// MeshVBO object

/// MeshVBO is simply a Mesh with the ability to store its data on the GPU
/// for faster drawing. Its usage is exactly the same as Mesh with the exception
/// of two new methods: allocate() and update().
///
/// allocate() creates new buffers on the GPU and pushes any data attached to
/// MeshVBO. A graphics context is required for allocate() to work, so it must
/// be called from within onCreate(), onAnimate() or onDraw().
///
/// update() should be called whenever the data has changed and is ready to be
/// rendered.
///
/// In practice, the user is only responsible for update(). An automatic check
/// is performed in Graphics::draw() and calls allocate() if necessary.
///
/// See allocore/examples/graphics/VBOManyShape.cpp for a usage example.
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

	/// Default constructor
	MeshVBO();

	/// Constructor given a Mesh allocates a VBO using data from Mesh
	MeshVBO(Mesh& cpy);

	/// Copy from Mesh and allocate GPU memory.

	/// Set allocate to false if no graphics context exists yet
	void copyFrom(Mesh& cpy, bool _allocate=true);

	/// Copy from MeshVBO
	void copyFrom(MeshVBO& cpy);

  void operator=(Mesh& cpy);
  void operator=(MeshVBO& cpy);

	/// Create new VBO buffer IDs, bindings, and data stores.

	/// Requires a graphics context to work.
  void allocate(BufferDataUsage usage = STREAM_DRAW);

	/// Update old VBO buffer data with new information.

	/// Does not create new buffer IDs unless the size of the new vertex buffer
	/// is larger than the one previously allocated.
	void update();

	/// Resets member variables and buffer IDs, unbinds buffers.
	void clear();

	/// Binds available buffers and gets appropriate pointers before rendering.

	/// Called by Graphics::draw()
	void bind();

	/// Unbinds buffers.
  void unbind();

	/// Get vertex buffer ID
	uint32_t getVertId();
	/// Get normal buffer ID
	uint32_t getNormalId();
	/// Get color buffer ID
	uint32_t getColorId();
	uint32_t getColoriId();
	/// Get texture coordinates buffer ID
	uint32_t getTexCoordId();
	/// Get index buffer ID
	uint32_t getIndexId();

	/// Get number of vertices
	int getNumVertices();
	/// Get number of indices
	int getNumIndices();

	/// Check if MeshVBO has been allocated
	bool isAllocated();
	/// Check if MeshVBO has been bound
	bool isBound();

	/// Check if normals exist
	bool hasNormals();
	/// Check if colors exist
	bool hasColors();
	bool hasColoris();
	/// Check if 2d texture coordinates exist
	bool hasTexCoord2s();
	/// Check if 3d texture coordinates exist
	bool hasTexCoord3s();
	/// Check if indices exist
	bool hasIndices();

protected:
	BufferDataUsage mBufferUsage = STREAM_DRAW;

	uint32_t mVertId = 0;
	uint32_t mNormalId = 0;
	uint32_t mColorId = 0;
	uint32_t mColoriId = 0;
	uint32_t mTexCoordId = 0;
	uint32_t mIndexId = 0;

	int mNumVertices = 0;
	int mNumIndices = 0;
	int mVertStride = sizeof(Vec3f);
	int mNormalStride = sizeof(Vec3f);
	int mColorStride = sizeof(Color);
	int mColoriStride = sizeof(Colori);
	int mTexCoordStride = sizeof(TexCoord2);
	int mIndexStride = sizeof(uint32_t);

	bool mAllocated = false;
	bool mBound = false;

	bool mHasNormals = false;
	bool mHasColors = false;
	bool mHasColoris = false;
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
