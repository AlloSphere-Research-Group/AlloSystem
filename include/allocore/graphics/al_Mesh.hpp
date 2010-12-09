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
class Mesh {
public:

	typedef Vec3f			Vertex;
	typedef Vec3f			Normal;
	typedef Vec4f			Color;
	typedef Vec2f			TexCoord2;
	typedef Vec3f			TexCoord3;
	typedef unsigned int	Index;

	Mesh(): mPrimitive(0){}

	/// Reset all buffers
	void reset();
	void equalizeBuffers();
	void getBounds(Vec3f& min, Vec3f& max);
	Vec3f getCenter(); // center at 0,0,0

	// destructive edits to internal vertices:
	void unitize();	/// scale to -1..1
	void scale(double x, double y, double z);
	void scale(Vec3f v) { scale(v[0], v[1], v[2]); }
	void scale(double s) { scale(s, s, s); }
	void translate(double x, double y, double z);
	void translate(Vec3f v) { translate(v[0], v[1], v[2]); }

	// generates smoothed normals for a set of vertices
	// will replace any normals currently in use
	// angle - maximum angle (in degrees) to smooth across
	void generateNormals(float angle);

	int primitive() const { return mPrimitive; }
	const Buffer<Vertex>& vertices() const { return mVertices; }
	const Buffer<Normal>& normals() const { return mNormals; }
	const Buffer<Color>& colors() const { return mColors; }
	const Buffer<TexCoord2>& texCoord2s() const { return mTexCoord2s; }
	const Buffer<TexCoord3>& texCoord3s() const { return mTexCoord3s; }
	const Buffer<Index>& indices() const { return mIndices; }

	void index(unsigned int i){ indices().append(i); }

	void color(float r, float g, float b, float a=1){ color(Color(r,g,b,a)); }
	void color(const Color& v) { colors().append(v); }
	void color(const al::Color& v) { color(v.r, v.g, v.b, v.a); }

	void normal(float x, float y, float z=0){ normal(Normal(x,y,z)); }
	void normal(const Normal& v) { normals().append(v); }

	void texCoord(float u, float v){ texCoord(TexCoord2(u,v)); }
	void texCoord(const TexCoord2& v){ texCoord2s().append(v); }
	
	void texCoord(float u, float v, float w){ texCoord(TexCoord3(u,v,w)); }
	void texCoord(const TexCoord3& v){ texCoord3s().append(v); }

	void vertex(float x, float y, float z=0){ vertex(Vertex(x,y,z)); }
	void vertex(const Vertex& v){ vertices().append(v); }

	void primitive(int prim){ mPrimitive=prim; }

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

} // ::al


#endif	/* include guard */

