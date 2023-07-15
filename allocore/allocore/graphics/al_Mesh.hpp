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

#include <functional>
#include <string>
#include "allocore/math/al_Vec.hpp"
#include "allocore/math/al_Mat.hpp"
#include "allocore/types/al_Buffer.hpp"
#include "allocore/types/al_Color.hpp"

namespace al{

// Ideally in Mesh, but some trickery here that doesn't work with nested classes
struct SVGOptions{
	#define AL_SVG_PROPERTY(T, name, ...)\
	protected: T m_##name{__VA_ARGS__};\
	public: auto name(const T& v) -> decltype(*this)& { m_##name = v; return *this; }\
	public: const T& name() const { return m_##name; }\
	public: T& name(){ return m_##name; }
	AL_SVG_PROPERTY(std::string, title);
	AL_SVG_PROPERTY(std::string, desc);
	AL_SVG_PROPERTY(int, width, 400);
	AL_SVG_PROPERTY(int, height, 400);
	AL_SVG_PROPERTY(float, scale, 0.95);
	#undef AL_SVG_PROPERTY
};

/// Stores buffers related to rendering graphical objects

/// A mesh is a collection of buffers storing vertices, colors, indices, etc.
/// that define the geometry and coloring/shading of a graphical object.
/// @ingroup allocore
class Mesh {
public:

	typedef Vec3f			Vertex;
	typedef Vec3f			UnitVector;
	typedef UnitVector		Normal;
	typedef UnitVector		Tangent;
	typedef float			TexCoord1;
	typedef Vec2f			TexCoord2;
	typedef Vec3f			TexCoord3;
	typedef unsigned int	Index;

	typedef Buffer<Vertex>		Vertices;
	typedef Buffer<Normal>		Normals;
	typedef Buffer<Tangent>		Tangents;
	typedef Buffer<Color>		Colors;
	typedef Buffer<Colori>		Coloris;
	typedef Buffer<TexCoord1>	TexCoord1s;
	typedef Buffer<TexCoord2>	TexCoord2s;
	typedef Buffer<TexCoord3>	TexCoord3s;
	typedef Buffer<Index>		Indices;


	enum Attrib{
		POSITION	= 1<<0,
		NORMAL		= 1<<1,
		TANGENT		= 1<<2,
		TEXCOORD	= 1<<3,
		COLOR		= 1<<4
	};


	/// @param[in] primitive	renderer-dependent primitive number
	Mesh(int primitive=0);

	/// Copy constructor
	Mesh(const Mesh& cpy);


	/// Set geometric primitive
	Mesh& primitive(int prim){ mPrimitive=prim; return *this; }

	int primitive() const { return mPrimitive; }

	Mesh& points();
	Mesh& points(float stroke);
	Mesh& lines();
	Mesh& lines(float stroke);
	Mesh& lineStrip();
	Mesh& lineStrip(float stroke);
	Mesh& lineLoop();
	Mesh& lineLoop(float stroke);
	Mesh& triangles();
	Mesh& triangleStrip();

	bool isPoints() const;
	bool isLines() const;
	bool isLineStrip() const;
	bool isLineLoop() const;
	bool isTriangles() const;
	bool isTriangleStrip() const;
	bool isTriangleType() const;

	/// Set stroke size

	/// For line primitives, this is the line width.
	/// For point primitives, this is the point diameter.
	/// A negative value uses the current value of the renderer.
	Mesh& stroke(float v){ mStroke=v; return *this; }

	float stroke() const { return mStroke; }

	/// Reset all buffers
	Mesh& reset();

	const Vertices& vertices() const { return mVertices; }
	Vertices& vertices(){ return mVertices; }
	const Normals& normals() const { return mNormals; }
	Normals& normals(){ return mNormals; }
	const Tangents& tangents() const { return mTangents; }
	Tangents& tangents(){ return mTangents; }
	const Colors& colors() const { return mColors; }
	Colors& colors(){ return mColors; }
	const Coloris& coloris() const { return mColoris; }
	Coloris& coloris(){ return mColoris; }
	const TexCoord1s& texCoord1s() const { return mTexCoord1s; }
	TexCoord1s& texCoord1s(){ return mTexCoord1s; }
	const TexCoord2s& texCoord2s() const { return mTexCoord2s; }
	TexCoord2s& texCoord2s(){ return mTexCoord2s; }
	const TexCoord3s& texCoord3s() const { return mTexCoord3s; }
	TexCoord3s& texCoord3s(){ return mTexCoord3s; }
	const Indices& indices() const { return mIndices; }
	Indices& indices(){ return mIndices; }

	/// Append index to index buffer
	Mesh& index(unsigned int i){ indices().append(i); return *this; }

	/// Append indices to index buffer
	template <class Tindex1, class Tindex2 = Index>
	Mesh& index(const Tindex1 * buf, int size, Tindex2 indexOffset=0){
		for(int i=0; i<size; ++i) index((Index)(buf[i] + indexOffset));
		return *this;
	}

	template <class...Indices>
	Mesh& index(unsigned i, Indices... indices){
		index(i);
		return index(indices...);
	}

	/// Append index to index buffer relative to current number of vertices

	/// This should be called BEFORE adding the relevant vertex positions.
	/// For example, if you want to add a triangle, call indexRel(0,1,2) then
	/// add the triangle vertex positions.
	Mesh& indexRel(unsigned int i){ return index(vertices().size()+i); }

	template <class...Indices>
	Mesh& indexRel(unsigned i, Indices... indices){
		indexRel(i);
		return indexRel(indices...);
	}


	/// Append vertex to vertex buffer
	Mesh& vertex(float x, float y, float z=0){ return vertex(Vertex(x,y,z)); }

	/// Append vertex to vertex buffer
	Mesh& vertex(const Vertex& v){ vertices().append(v); return *this; }

	/// Append vertex to vertex buffer
	template <class T>
	Mesh& vertex(const Vec<2,T>& v, float z=0){ return vertex(v[0], v[1], z); }

	/// Append vertices from flat array
	template <class T>
	Mesh& vertex(const T * src, int numVerts){
		for(int i=0; i<numVerts; ++i) vertex(src[3*i+0], src[3*i+1], src[3*i+2]);
		return *this;
	}

	/// Append vertices to vertex buffer
	template <class T>
	Mesh& vertex(const Vec<3,T> * src, int numVerts){
		for(int i=0; i<numVerts; ++i) vertex(src[i][0], src[i][1], src[i][2]);
		return *this;
	}

	/// Append 2D vertex
	template <class T1, class T2>
	Mesh& vertex2(T1 x1, T2 y1){ return vertex(x1, y1); }

	/// Append 2D vertices
	template <class T1, class T2, class... Ts>
	Mesh& vertex2(T1 x1, T2 y1, Ts... xnyn){ return vertex2(x1, y1).vertex2(xnyn...); }


	/// Append normal to normal buffer
	Mesh& normal(float x, float y, float z=0){ return normal(Normal(x,y,z)); }

	/// Append normal to normal buffer
	Mesh& normal(const Normal& v){ normals().append(v); return *this; }

	/// Append normal to normal buffer
	template <class T>
	Mesh& normal(const Vec<2,T>& v, float z=0){ return normal(v[0], v[1], z); }

	/// Append normals from flat array
	template <class T>
	Mesh& normal(const T * src, int len){
		for(int i=0; i<len; ++i) normal(src[3*i+0], src[3*i+1], src[3*i+2]);
		return *this;
	}

	/// Fill any deficit in normal buffer with value
	Mesh& normalFill(const Normal& v);


	/// Append tangent to tangent buffer
	Mesh& tangent(float x, float y, float z=0){ return tangent(Tangent(x,y,z)); }

	/// Append tangent to tangent buffer
	Mesh& tangent(const Tangent& v){ mTangents.append(v); return *this; }

	/// Append tangent to tangent buffer
	template <class T>
	Mesh& tangent(const Vec<2,T>& v, float z=0){ return tangent(v[0], v[1], z); }

	/// Append tangents from flat array
	template <class T>
	Mesh& tangent(const T * src, int len){
		for(int i=0; i<len; ++i) tangent(src[3*i+0], src[3*i+1], src[3*i+2]);
		return *this;
	}

	/// Fill any deficit in tangent buffer with value
	Mesh& tangentFill(const Tangent& v);


	/// Append color to color buffer
	Mesh& color(const Color& v) { colors().append(v); return *this; }

	/// Append color to color buffer
	Mesh& color(const Colori& v) { coloris().append(v); return *this; }

	/// Append color to color buffer
	Mesh& color(const HSV& v) { colors().append(v); return *this; }

	/// Append color to color buffer
	Mesh& color(const RGB& v) { colors().append(v); return *this; }

	/// Append color to color buffer
	Mesh& color(float r, float g, float b, float a=1){ return color(Color(r,g,b,a)); }

	/// Append color to color buffer
	template <class T>
	Mesh& color(const Vec<4,T>& v) { return color(v[0], v[1], v[2], v[3]); }

	/// Append colors from flat array
	template <class T>
	Mesh& color(const T * src, int numColors){
		for(int i=0; i<numColors; ++i) color(src[4*i+0], src[4*i+1], src[4*i+2], src[4*i+3]);
		return *this;
	}


	/// Append floating-point color to integer color buffer
	Mesh& colori(const Colori& v) { coloris().append(v); return *this; }

	/// Append integer colors from flat array
	template <class T>
	Mesh& colori(const T * src, int numColors){
		for(int i=0; i<numColors; ++i){
			coloris().append(Colori(src[4*i+0], src[4*i+1], src[4*i+2], src[4*i+3]));
		}
		return *this;
	}

	/// Append copy of last appended color
	Mesh& repeatColor(){
		if(colors().size()) colors().repeatLast();
		else if(coloris().size()) coloris().repeatLast();
		return *this;
	}

	/// Fill any deficit in color buffer with a color
	Mesh& colorFill(const Color& v);

	/// Fill any deficit in colori buffer with a color
	Mesh& coloriFill(const Colori& v);


	/// Append texture coordinate to 1D texture coordinate buffer
	Mesh& texCoord(float u){ texCoord1s().append(TexCoord1(u)); return *this; }

	/// Append texture coordinate to 2D texture coordinate buffer
	Mesh& texCoord(float u, float v){ texCoord2s().append(TexCoord2(u,v)); return *this; }

	/// Append texture coordinate to 2D texture coordinate buffer
	template <class T>
	Mesh& texCoord(const Vec<2,T>& v){ return texCoord(v[0], v[1]); }

	/// Append texture coordinate to 3D texture coordinate buffer
	Mesh& texCoord(float u, float v, float w){ texCoord3s().append(TexCoord3(u,v,w)); return *this; }

	/// Append texture coordinate to 3D texture coordinate buffer
	template <class T>
	Mesh& texCoord(const Vec<3,T>& v){ return texCoord(v[0], v[1], v[2]); }


	/// Returns whether mesh has valid data for rendering
	bool valid() const;

	/// Returns whether mesh is empty (has no vertex positions)
	bool empty() const;

	/// Repeat last vertex element(s)
	Mesh& repeatLast();

	/// Call function for each face

	/// If the primitive is points or lines, then the function is called for
	/// each point or line segment, respectively.
	const Mesh& forEachFace(const std::function<void(int v1, int v2, int v3)>& onFace) const;
	Mesh& forEachFace(const std::function<void(int v1, int v2, int v3)>& onFace);

	/// Call function for each vertex (position)

	/// A less error-prone way of writing
	/// 	for(int i=0; i<mesh.vertices().size(); ++i) ...
	const Mesh& forEachVertex(const std::function<void(int i)>& onVert) const;
	Mesh& forEachVertex(const std::function<void(int i)>& onVert);

	/// Get corners of bounding box of vertices

	/// @param[out] min		minimum corner of bounding box
	/// @param[out] max		maximum corner of bounding box
	void bounds(Vec3f& min, Vec3f& max) const;

	/// Get centroid (geometric mean) of vertices
	Vertex centroid() const;

	/// Scale all vertices
	Mesh& scale(float x, float y, float z=1.f);
	Mesh& scale(float s){ return scale(s,s,s); }

	template <class T>
	Mesh& scale(const Vec<3,T>& v){ return scale(v[0],v[1],v[2]); }

	/// Translate all vertices
	Mesh& translate(float x, float y, float z=0.f);

	template <class T>
	Mesh& translate(const Vec<3,T>& v){ return translate(v[0],v[1],v[2]); }

	template <class T>
	Mesh& translate(const Vec<2,T>& xy, float z=0.f){ return translate(xy[0],xy[1],z); }

	template <class T>
	Mesh& translate(const T& v){ return translate(v,v,v); }

	/// Rotate 90 degrees on specified plane
	template <unsigned Dim1=0, unsigned Dim2=1>
	Mesh& rotate90(){
		for(auto& v : vertices()) v.rotate90<Dim1,Dim2>();
		for(auto& v : normals() ) v.rotate90<Dim1,Dim2>();
		for(auto& v : tangents()) v.rotate90<Dim1,Dim2>();
		return *this;
	}

	/// Transform vertices by projective transform matrix

	/// @param[in] m		projective transform matrix
	/// @param[in] begin	beginning index of vertices; negative value specifies
	///						distance from last element
	/// @param[in] end		ending index of vertices; negative value specifies
	///						distance from one past last element
	template <class T>
	Mesh& transform(const Mat<4,T>& m, int begin=0, int end=-1);

	/// Scales and translates vertices to lie in sphere
	Mesh& fitToSphere(float radius=1);

	/// Scales and translates vertices to lie in cube
	Mesh& fitToCube(float radius=1, bool proportional=true);

	Mesh& fitToCubeTransform(Vec3f& center, Vec3f& scale, float radius=1, bool proportional=true);

	/// Scales and translates vertices to lie in cube with extrema [-1,1]
	Mesh& unitize(bool proportional=true);


	/// Generates indices for a set of vertices
	Mesh& compress();

	/// Convert indices (if any) to flat vertex buffers
	Mesh& decompress();

	/// Extend buffers to match number of vertices

	/// This will resize all populated buffers to match the size of the vertex
	/// buffer. Buffers are extended by copying their last element.
	Mesh& equalizeBuffers();

	/// Append buffers from another mesh:
	Mesh& merge(const Mesh& src);

	template <class T>
	Mesh& merge(const Mesh& src, const Mat<4,T>& xfm){
		return merge(src).transform(xfm, -src.vertices().size());
	}

	/// Convert triangle strip to triangles
	Mesh& toTriangles();

	/// Generates normals for a set of vertices

	/// This method will generate a normal for each vertex in the buffer
	/// assuming the drawing primitive is either triangles or a triangle strip.
	/// Averaged vertex normals are generated if indices are present and, for
	/// triangles only, face normals are generated if no indices are present.
	/// This will replace any normals currently in use.
	///
	/// @param[in] normalize			whether to normalize normals
	/// @param[in] equalWeightPerFace	whether to use an equal weighting of
	///									face normals rather than a weighting
	///									based on face areas
	Mesh& generateNormals(bool normalize=true, bool equalWeightPerFace=false);

	/// Generate normals if they do not exist
	Mesh& ensureNormals(bool equalWeightPerFace=false);

	/// Invert direction of normals
	Mesh& invertNormals();

	/// Creates a mesh filled with lines for each normal of the source

	/// @param[out] mesh		normal lines
	/// @param[in]  length		length of normals
	/// @param[in]  perFace		whether normals line should be generated per
	///							face rather than per vertex
	void createNormalsMesh(Mesh& mesh, float length=0.1, bool perFace=false) const;

	Mesh& normalsMesh(Mesh& mesh, float length=0.1, bool perFace=false) const {
		createNormalsMesh(mesh,length,perFace);
		return mesh;
	}

	/// Flip the winding order (of triangles)
	Mesh& flipWinding();

	/// Smooths a triangle mesh

	/// This smooths a triangle mesh using Laplacian (low-pass) filtering.
	/// New vertex positions are a weighted sum of their nearest neighbors. 
	/// The number of vertices is not changed.
	/// @param[in] amount		interpolation fraction between original and smoothed result
	/// @param[in] weighting	0 = equal weight, 1 = inverse distance weight
	Mesh& smooth(float amount=1, int weighting=0);

	/// Ribbonize curve

	/// This creates a two-dimensional ribbon from a one-dimensional space curve.
	/// The result is to be rendered with a triangle strip.
	/// @param[in] width			Width of ribbon
	/// @param[in] faceBinormal		If true, surface faces binormal vector of curve.
	///								If false, surface faces normal vector of curve.
	Mesh& ribbonize(float width=0.04, bool faceBinormal=false){
		return ribbonize(&width, 0, faceBinormal);
	}

	/// Ribbonize curve

	/// This creates a two-dimensional ribbon from a one-dimensional space curve.
	/// The result is to be rendered with a triangle strip.
	/// @param[in] widths			Array specifying width of ribbon at each point along curve
	/// @param[in] widthsStride		Stride factor of width array
	/// @param[in] faceBinormal		If true, surface faces binormal vector of curve.
	///								If false, surface faces normal vector of curve.
	Mesh& ribbonize(float * widths, int widthsStride=1, bool faceBinormal=false);


	/// Save mesh to file

	/// Currently supported file formats are FBX, PLY and STL.
	///
	/// @param[in] filePath		path of file to save to
	/// @param[in] solidName	solid name defined within the file (optional)
	/// @param[in] binary		write data in binary form as opposed to ASCII
	/// \returns true on successful save, otherwise false
	bool save(const std::string& filePath, const std::string& solidName = "", bool binary=true) const;

	/// Save mesh to FBX file
	bool saveFBX(const std::string& filePath, const std::string& solidName = "") const;

	/// Save mesh to PLY file

	/// This implementation saves an ASCII (as opposed to binary) PLY file.
	///
	/// @param[in] filePath		path of file to save to
	/// @param[in] solidName	solid name defined within the file (optional)
	/// @param[in] binary		write data in binary form as opposed to ASCII
	/// \returns true on successful save, otherwise false
	bool savePLY(const std::string& filePath, const std::string& solidName = "", bool binary=true) const;

	/// Save mesh to STL file

	/// STL (STereoLithography) is a file format used widely for
	/// rapid prototyping. It contains only surface geometry (vertices and
	/// normals) as a list of triangular facets.
	/// This implementation saves an ASCII (as opposed to binary) STL file.
	///
	/// @param[in] filePath		path of file to save to
	/// @param[in] solidName	solid name defined within the file (optional)
	/// \returns true on successful save, otherwise false
	bool saveSTL(const std::string& filePath, const std::string& solidName = "") const;

	/// Save mesh to SVG file

	/// This implementation supports only line and point primitives. Since SVG
	/// is a 2D format, only the XY projection is written out.
	///
	/// @param[in] filePath		path of file to save to
	/// @param[in] opt			options specific to SVG
	/// \returns true on successful save, otherwise false
	bool saveSVG(const std::string& filePath, const SVGOptions& opt = {}) const;


	/// Load mesh data from 3D object file

	/// Currently supported file formats are PLY and OBJ.
	///
	/// @param[in] filePath		path of object file to load
	bool load(const std::string& filePath);

	bool loadOBJ(const void * data, int numBytes);
	bool loadPLY(const void * data, int numBytes);


	/// Set hint for what attributes generators/loaders should produce
	Mesh& attribHint(Attrib v){ mAttribHint = v; return *this; }
	Attrib attribHint() const { return mAttribHint; }
	/// Get whether mesh wants attribute(s)
	bool wants(Attrib v) const { return mAttribHint & v; }

	/// Print information about Mesh
	void print(FILE * dst = stdout) const;

	/// Debug mesh

	/// @param[in] dst	file to print log to or nullptr for no logging
	/// \returns whether the mesh is well-formed
	bool debug(FILE * dst = stdout) const;

protected:

	// Only populated (size>0) buffers will be used
	Vertices mVertices;
	Normals mNormals;
	Tangents mTangents;
	Colors mColors;
	Coloris mColoris;
	TexCoord1s mTexCoord1s;
	TexCoord2s mTexCoord2s;
	TexCoord3s mTexCoord3s;
	Indices mIndices;

	int mPrimitive;
	Attrib mAttribHint = Attrib(0);
	float mStroke = -1.f;
};

inline Mesh::Attrib operator& (Mesh::Attrib a, Mesh::Attrib b){ return Mesh::Attrib(+a & +b); }
inline Mesh::Attrib operator| (Mesh::Attrib a, Mesh::Attrib b){ return Mesh::Attrib(+a | +b); }
inline Mesh::Attrib operator^ (Mesh::Attrib a, Mesh::Attrib b){ return Mesh::Attrib(+a ^ +b); }

} // al::

#endif
