#ifndef INCLUDE_AL_GRAPHICS_MESH_HPP
#define INCLUDE_AL_GRAPHICS_MESH_HPP

/*
	Copyright (C) 2006-2008. The Regents of the University of California (REGENTS).
	All Rights Reserved.

	Permission to use, copy, modify, distribute, and distribute modified versions
	of this software and its documentation without fee and without a signed
	licensing agreement, is hereby granted, provided that the above copyright
	notice, the list of contributors, this paragraph and the following two paragraphs
	appear in all copies, modifications, and distributions.

	IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
	SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING
	OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF REGENTS HAS
	BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
	THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
	PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED
	HEREUNDER IS PROVIDED "AS IS". REGENTS HAS  NO OBLIGATION TO PROVIDE
	MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
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

	/// @param[in] primitive	renderer-dependent primitive number
	Mesh(int primitive=0): mPrimitive(primitive){}

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
	void merge(Mesh& src) {
		if (indices().size() || src.indices().size()) {
			printf("error: Mesh merging with indexed meshes not yet supported\n");
			return;
		}
		//equalizeBuffers(); << TODO: must do this if we are using indices.
		vertices().append(src.vertices());
		normals().append(src.normals());
		colors().append(src.colors());
		texCoord2s().append(src.texCoord2s());
		texCoord3s().append(src.texCoord3s());
		// TODO: indices are more complex, since the offsets may have changed.
		// we'd have to add indices.size() to all of the src.indices before adding.
		// also, both src & dst should either use or not use indices
		// tricky if src is empty...
		//indices().append(src.indices());
	}

	/// Reset all buffers
	Mesh& reset();

	/// Scale all vertices to lie in [-1,1]
	void unitize();

	void scale(double x, double y, double z);
	void scale(Vec3f v) { scale(v[0], v[1], v[2]); }
	void scale(double s) { scale(s, s, s); }
	void translate(double x, double y, double z);
	void translate(Vec3f v) { translate(v[0], v[1], v[2]); }
	
	/// Transform vertices by projective transform matrix
	
	/// @param[in] m		projective transform matrix
	/// @param[in] begin	beginning index of vertices
	/// @param[in] end		ending index of vertices, negative amount specify distance from one past last element
	template <class T>
	void transform(const Mat<4,T>& m, int begin=0, int end=-1);

	/// Generates normals for a set of vertices
	
	/// This method will generate a normal for each vertex in the buffer
	/// assuming the drawing primitive is a triangle. Face normals are generated
	/// if no indices are present, and averaged vertex normals are generated
	/// if indices are present. This will replace any normals currently in use.
	///
	/// @param[in] normalize	whether to normalize normals
	void generateNormals(bool normalize=true, bool equalWeightPerFace=false);
	
	void invertNormals();
	
	/// Creates a mesh filled with lines for each normal of the source
	
	/// @param[out] mesh		normal lines
	/// @param[in]  length		length of normals
	/// @param[in]  perFace		whether normals line should be generated per 
	///							face rather than per vertex
	void createNormalsMesh(Mesh& mesh, float length=0.1, bool perFace=false);

	int primitive() const { return mPrimitive; }
	const Buffer<Vertex>& vertices() const { return mVertices; }
	const Buffer<Normal>& normals() const { return mNormals; }
	const Buffer<Color>& colors() const { return mColors; }
	const Buffer<TexCoord2>& texCoord2s() const { return mTexCoord2s; }
	const Buffer<TexCoord3>& texCoord3s() const { return mTexCoord3s; }
	const Buffer<Index>& indices() const { return mIndices; }


	/// Append index to index buffer
	void index(unsigned int i){ indices().append(i); }

	/// Append indices to index buffer	
	template <class Tindex>
	void index(const Tindex * buf, int size, int indexOffset=0){
		for(int i=0; i<size; ++i) index(buf[i] + indexOffset); }


	/// Append color to color buffer
	void color(const Color& v) { colors().append(v); }

	/// Append color to color buffer
	void color(float r, float g, float b, float a=1){ color(Color(r,g,b,a)); }
	
	/// Append color to color buffer
	template <class T>
	void color(const Vec<4,T>& v) { color(v[0], v[1], v[2], v[3]); }


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
		for(int i=0; i<size; ++i) vertices().append(buf[i][0], buf[i][1], buf[i][2]);
	}

	/// Set geometric primitive
	Mesh& primitive(int prim){ mPrimitive=prim; return *this; }
	/// Get number of faces (assumes triangles or quads)
//	int numFaces() const { return mIndices.size() / ( ( mPrimitive == Graphics::TRIANGLES ) ? 3 : 4 ); }
	/// Get indices as triangles
//	TriFace& indexAsTri(){ return (TriFace*) indices(); }
	/// Get indices as quads
//	QuadFace& indexAsQuad(){ return (QuadFace*) indices(); }

	Buffer<Vertex>& vertices(){ return mVertices; }
	Buffer<Normal>& normals(){ return mNormals; }
	Buffer<Color>& colors(){ return mColors; }
	Buffer<TexCoord2>& texCoord2s(){ return mTexCoord2s; }
	Buffer<TexCoord3>& texCoord3s(){ return mTexCoord3s; }
	Buffer<Index>& indices(){ return mIndices; }
	
protected:

	// Only populated (size>0) buffers will be used
	Buffer<Vertex> mVertices;
	Buffer<Normal> mNormals;
	Buffer<Color> mColors;
	Buffer<TexCoord2> mTexCoord2s;
	Buffer<TexCoord3> mTexCoord3s;
	Buffer<Index> mIndices;
	
	int mPrimitive;
};



template <class T>
void Mesh::transform(const Mat<4,T>& m, int begin, int end){
	if(end<0) end += vertices().size()+1; // negative index wraps to end of array
	for(int i=begin; i<end; ++i){
		Vertex& v = vertices()[i];
		v.set(m * Vec<4,T>(v, 1));
	}
}

} // al::

#endif
