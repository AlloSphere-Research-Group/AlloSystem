#ifndef INCLUDE_AL_GRAPHICS_MESH_HPP
#define INCLUDE_AL_GRAPHICS_MESH_HPP

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
	A generic mesh object for vertex-based geometry

	File author(s):
	Wesley Smith, 2010, wesley.hoke@gmail.com
	Lance Putnam, 2010, putnam.lance@gmail.com
	Graham Wakefield, 2010, grrrwaaa@gmail.com
*/

#include "allocore/math/al_Vec.hpp"
#include "allocore/math/al_Matrix4.hpp"
#include "allocore/types/al_Buffer.hpp"
#include "allocore/types/al_Color.hpp"

namespace al{

/// Stores buffers related to rendering graphical objects

/// A mesh is a collection of buffers storing vertices, colors, indices, etc.
/// that define the geometry and coloring/shading of a graphical object.
class Mesh {
public:

	typedef Vec3f			Vertex;
	typedef Vec3f			Normal;
	//typedef Vec4f			Color;
	typedef Vec2f			TexCoord2;
	typedef Vec3f			TexCoord3;
	typedef unsigned int	Index;
	typedef Vec3i			TriFace;
	typedef Vec4i			QuadFace;

	typedef Buffer<Vertex>		Vertices;
	typedef Buffer<Normal>		Normals;
	typedef Buffer<Color>		Colors;
	typedef Buffer<Colori>		Coloris;
	typedef Buffer<TexCoord2>	TexCoord2s;
	typedef Buffer<TexCoord3>	TexCoord3s;
	typedef Buffer<Index>		Indices;
	

	/// @param[in] primitive	renderer-dependent primitive number
	Mesh(int primitive=0): mPrimitive(primitive){}
	
	Mesh(const Mesh& cpy) : 
		mVertices(cpy.mVertices),
		mNormals(cpy.mNormals),
		mColors(cpy.mColors),
		mColoris(cpy.mColoris),
		mTexCoord2s(cpy.mTexCoord2s),
		mTexCoord3s(cpy.mTexCoord3s),
		mIndices(cpy.mIndices),
		mPrimitive(cpy.mPrimitive)
		{}


	/// Get corners of bounding box of vertices
	
	/// @param[out] min		minimum corner of bounding box
	/// @param[out] max		maximum corner of bounding box
	void getBounds(Vec3f& min, Vec3f& max) const;

	/// Get center of vertices
	Vertex getCenter() const;


	// destructive edits to internal vertices:

	/// Convert indices (if any) to flat vertex buffers
	void decompress();

	/// Extend buffers to match number of vertices
	
	/// This will resize all populated buffers to match the size of the vertex
	/// buffer. Buffers are extended by copying their last element.
	void equalizeBuffers();
	
	/// Append buffers from another mesh:
	void merge(const Mesh& src);

	/// Reset all buffers
	Mesh& reset();

	/// Scale all vertices to lie in [-1,1]
	void unitize(bool proportional=true);

	/// Scale all vertices
	Mesh& scale(float x, float y, float z);
	Mesh& scale(float s){ return scale(s,s,s); }

	template <class T>
	Mesh& scale(const Vec<3,T>& v){ return scale(v[0],v[1],v[2]); }

	/// Translate all vertices
	Mesh& translate(float x, float y, float z);
	
	template <class T>
	Mesh& translate(const Vec<3,T>& v){ return translate(v[0],v[1],v[2]); }
	
	/// Transform vertices by projective transform matrix
	
	/// @param[in] m		projective transform matrix
	/// @param[in] begin	beginning index of vertices
	/// @param[in] end		ending index of vertices, negative amount specify distance from one past last element
	template <class T>
	Mesh& transform(const Mat<4,T>& m, int begin=0, int end=-1);

	/// Generates indices for a set of vertices
	void compress();

	/// Generates normals for a set of vertices
	
	/// This method will generate a normal for each vertex in the buffer
	/// assuming the drawing primitive is a triangle. Face normals are generated
	/// if no indices are present, and averaged vertex normals are generated
	/// if indices are present. This will replace any normals currently in use.
	///
	/// @param[in] normalize	whether to normalize normals
	void generateNormals(bool normalize=true, bool equalWeightPerFace=false);
	
	/// Invert direction of normals
	void invertNormals();
	
	/// Creates a mesh filled with lines for each normal of the source
	
	/// @param[out] mesh		normal lines
	/// @param[in]  length		length of normals
	/// @param[in]  perFace		whether normals line should be generated per 
	///							face rather than per vertex
	void createNormalsMesh(Mesh& mesh, float length=0.1, bool perFace=false);

	/// Ribbonize curve
	
	/// This creates a two-dimensional ribbon from a one-dimensional space curve.
	/// The result is to be rendered with a triangle strip.
	/// @param[in] width			Width of ribbon
	/// @param[in] faceBinormal		If true, surface faces binormal vector of curve.
	///								If false, surface faces normal vector of curve.
	void ribbonize(float width=0.04, bool faceBinormal=false){
		ribbonize(&width, 0, faceBinormal);
	}

	/// Ribbonize curve

	/// This creates a two-dimensional ribbon from a one-dimensional space curve.
	/// The result is to be rendered with a triangle strip.
	/// @param[in] widths			Array specifying width of ribbon at each point along curve
	/// @param[in] widthsStride		Stride factor of width array
	/// @param[in] faceBinormal		If true, surface faces binormal vector of curve.
	///								If false, surface faces normal vector of curve.	
	void ribbonize(float * widths, int widthsStride=1, bool faceBinormal=false);


	int primitive() const { return mPrimitive; }
	const Buffer<Vertex>& vertices() const { return mVertices; }
	const Buffer<Normal>& normals() const { return mNormals; }
	const Buffer<Color>& colors() const { return mColors; }
	const Buffer<Colori>& coloris() const { return mColoris; }
	const Buffer<TexCoord2>& texCoord2s() const { return mTexCoord2s; }
	const Buffer<TexCoord3>& texCoord3s() const { return mTexCoord3s; }
	const Buffer<Index>& indices() const { return mIndices; }


	/// Set geometric primitive
	Mesh& primitive(int prim){ mPrimitive=prim; return *this; }
	
	/// Repeat last vertex element(s)
	Mesh& repeatLast();

	/// Append index to index buffer
	void index(unsigned int i){ indices().append(i); }

	/// Append indices to index buffer	
	template <class Tindex>
	void index(const Tindex * buf, int size, Tindex indexOffset=0){
		for(int i=0; i<size; ++i) index((Index)(buf[i] + indexOffset)); }


	/// Append color to color buffer
	void color(const Color& v) { colors().append(v); }

	/// Append color to color buffer
	void color(const Colori& v) { coloris().append(v); }

	/// Append color to color buffer
	void color(const HSV& v) { colors().append(v); }

	/// Append color to color buffer
	void color(const RGB& v) { colors().append(v); }

	/// Append color to color buffer
	void color(float r, float g, float b, float a=1){ color(Color(r,g,b,a)); }
	
	/// Append color to color buffer
	template <class T>
	void color(const Vec<4,T>& v) { color(v[0], v[1], v[2], v[3]); }

	/// Append floating-point color to integer color buffer
	void colori(const Color& v) { coloris().append(Colori(v)); }


	/// Append normal to normal buffer
	void normal(float x, float y, float z=0){ normal(Normal(x,y,z)); }
	
	/// Append normal to normal buffer
	void normal(const Normal& v) { normals().append(v); }


	/// Append texture coordinate to 2D texture coordinate buffer
	void texCoord(float u, float v){ texCoord(TexCoord2(u,v)); }
	
	/// Append texture coordinate to 2D texture coordinate buffer
	void texCoord(const TexCoord2& v){ texCoord2s().append(v); }

	/// Append texture coordinate to 3D texture coordinate buffer
	void texCoord(float u, float v, float w){ texCoord(TexCoord3(u,v,w)); }
	
	/// Append texture coordinate to 3D texture coordinate buffer
	void texCoord(const TexCoord3& v){ texCoord3s().append(v); }


	/// Append vertex to vertex buffer
	void vertex(float x, float y, float z=0){ vertex(Vertex(x,y,z)); }
	
	/// Append vertex to vertex buffer
	void vertex(const Vertex& v){ vertices().append(v); }

	/// Append vertices to vertex buffer
	template <class T>
	void vertex(const T * buf, int size){
		for(int i=0; i<size; ++i) vertex(buf[3*i+0], buf[3*i+1], buf[3*i+2]);
	}

	/// Append vertices to vertex buffer
	template <class T>
	void vertex(const Vec<3,T> * buf, int size){
		for(int i=0; i<size; ++i) vertex(buf[i][0], buf[i][1], buf[i][2]);
	}


	/// Get number of faces (assumes triangles or quads)
//	int numFaces() const { return mIndices.size() / ( ( mPrimitive == Graphics::TRIANGLES ) ? 3 : 4 ); }
	/// Get indices as triangles
//	TriFace& indexAsTri(){ return (TriFace*) indices(); }
	/// Get indices as quads
//	QuadFace& indexAsQuad(){ return (QuadFace*) indices(); }

	Vertices& vertices(){ return mVertices; }
	Normals& normals(){ return mNormals; }
	Colors& colors(){ return mColors; }
	Coloris& coloris(){ return mColoris; }
	TexCoord2s& texCoord2s(){ return mTexCoord2s; }
	TexCoord3s& texCoord3s(){ return mTexCoord3s; }
	Indices& indices(){ return mIndices; }
	
protected:

	// Only populated (size>0) buffers will be used
	Vertices mVertices;
	Normals mNormals;
	Colors mColors;
	Coloris mColoris;
	TexCoord2s mTexCoord2s;
	TexCoord3s mTexCoord3s;
	Indices mIndices;
	
	int mPrimitive;
};




template <class T>
Mesh& Mesh::transform(const Mat<4,T>& m, int begin, int end){
	if(end<0) end += vertices().size()+1; // negative index wraps to end of array
	for(int i=begin; i<end; ++i){
		Vertex& v = vertices()[i];
		v.set(m * Vec<4,T>(v, 1));
	}
	return *this;
}

} // al::

#endif
