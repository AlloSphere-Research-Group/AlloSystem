#include <vector>
#include <map>
#include <string>

#include "allocore/system/al_Config.h"
#include "allocore/graphics/al_Mesh.hpp"

namespace al{

void Mesh::reset() {
	vertices().clear();
	normals().clear();
	colors().clear();
	texCoord2s().clear();
	texCoord3s().clear();
	indices().clear();
}

void Mesh::equalizeBuffers() {
	int VS = vertices().size();
	int NS = normals().size();
	int CS = colors().size();
	int T2S = texCoord2s().size();
	int T3S = texCoord3s().size();

	if(NS > 0) {
		for(int i=NS; i < VS; i++) {
			normals().append(normals()[NS-1]);
		}
	}
	if(CS > 0) {
		for(int i=CS; i < VS; i++) {
			colors().append(colors()[CS-1]);
		}
	}
	if(T2S > 0) {
		for(int i=T2S; i < VS; i++) {
			texCoord2s().append(texCoord2s()[T2S-1]);
		}
	}
	if(T3S > 0) {
		for(int i=T3S; i < VS; i++) {
			texCoord3s().append(texCoord3s()[T3S-1]);
		}
	}
}

class TriFace {
public:
	Mesh::Vertex vertices[3];
	Vec3f norm;

	TriFace(const TriFace& cpy)
	: norm(cpy.norm) {
		vertices[0] = cpy.vertices[0];
		vertices[1] = cpy.vertices[1];
		vertices[2] = cpy.vertices[2];
	}
	TriFace(Mesh::Vertex p0, Mesh::Vertex p1, Mesh::Vertex p2)
	{
		vertices[0] = p0;
		vertices[1] = p1;
		vertices[2] = p2;
		// calculate norm for this face:
		normal<float>(norm, p0, p1, p2);
	}
};

void Mesh::generateNormals(float angle) {
//	/*
//		Multi-pass algorithm:
//			generate a list of faces (assume triangles?)
//				(vary according to whether mIndices is used)
//			calculate normal per face (use normal<float>(dst, p0, p1, p2))
//			vertices may be used in multiple faces; their norm should be an average of the uses
//				easy enough if indices is being used; not so easy otherwise.
//					create a lookup table by hashing on vertex x,y,z
//
//
//			write avg into corresponding normals for each vertex
//				EXCEPT: if edge is sharper than @angle, just use the face normal directly
//	*/
//	std::vector<TriFace> faces;
//
//	std::map<std::string, int> vertexHash;
//
//	int Ni = indices().size();
//	int Nv = vertices().size();
//	if (Ni) {
//		for (int i=0; i<Ni;) {
//			TriFace face(
//				mVertices[mIndices[i++]],
//				mVertices[mIndices[i++]],
//				mVertices[mIndices[i++]]
//			);
//			faces.push_back(face);
//		}
//	} else {
//		for (int i=0; i<Nv;) {
//			TriFace face(
//				mVertices[i++],
//				mVertices[i++],
//				mVertices[i++]
//			);
//			faces.push_back(face);
//		}
//	}

	int Nv = vertices().size();

	// same number of normals as vertices
	normals().size(Nv);

	for(int i=0; i<Nv; ++i) normals()[i].set(0,0,0);

	// compute per face normals
	if(indices().size()){
		int Ni = indices().size();

//		if(primitive() == TRIANGLES){
			Ni = Ni - (Ni%3); // must be multiple of 3

			for(int i=0; i<Ni; i+=3){
				Index i1 = indices()[i+0];
				Index i2 = indices()[i+1];
				Index i3 = indices()[i+2];
				const Vertex& v1 = vertices()[i1];
				const Vertex& v2 = vertices()[i2];
				const Vertex& v3 = vertices()[i3];
				
				Vertex vn = cross(v2-v1, v3-v1);
				
				normals()[i1] += vn;
				normals()[i2] += vn;
				normals()[i3] += vn;
			}
//		}
//		else if(primitive() == TRIANGLE_STRIP){
//			for(int i=2; i<Ni; ++i){
//				Index i1 = indices()[i-2];
//				Index i2 = indices()[i-1];
//				Index i3 = indices()[i-0];
//				const Vertex& v1 = vertices()[i1];
//				const Vertex& v2 = vertices()[i2];
//				const Vertex& v3 = vertices()[i3];
//				
//				Vertex vn = cross(v2-v1, v3-v1);
//				
//				normals()[i1] += vn;
//				normals()[i2] += vn;
//				normals()[i3] += vn;
//			}
//		}

	}

	// normalize the normals
	for(int i=0; i<Nv; ++i) normals()[i].normalize();
}

void Mesh::getBounds(Vertex& min, Vertex& max) {
	if (mVertices.size() > 0) {
		min.set(mVertices[0][0], mVertices[0][1], mVertices[0][2]);
		min.set(mVertices[0][0], mVertices[0][1], mVertices[0][2]);
		for (int v=1; v<mVertices.size(); v++) {
			Vertex& vt = mVertices[v];
			for (int i=0; i<3; i++) {
				min[i] = MIN(min[i], vt[i]);
				max[i] = MAX(max[i], vt[i]);
			}
		}
	}
}

void Mesh::unitize() {
	Vertex min(0), max(0);
	getBounds(min, max);
	Vertex avg = (max-min)*0.5;
	for (int v=0; v<mVertices.size(); v++) {
		Vertex& vt = mVertices[v];
		for (int i=0; i<3; i++) {
			vt[i] = -1. + (vt[i]-min[i])/avg[i];
		}
	}
}

Vec3f Mesh::getCenter() {
	Vertex min(0), max(0);
	getBounds(min, max);
	return min+(max-min)*0.5;
}

void Mesh::translate(double x, double y, double z) {
	for (int v=0; v<mVertices.size(); v++) {
		Vertex& vt = mVertices[v];
		vt[0] += x;
		vt[1] += y;
		vt[2] += z;
	}
}

void Mesh::scale(double x, double y, double z) {
	for (int v=0; v<mVertices.size(); v++) {
		Vertex& vt = mVertices[v];
		vt[0] *= x;
		vt[1] *= y;
		vt[2] *= z;
	}
}

} // ::al
