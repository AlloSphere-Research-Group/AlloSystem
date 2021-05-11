#include <algorithm> // transform
#include <cctype> // tolower
#include <map>
#include <set>
#include <string>
#include <utility> // swap
#include <vector>
#include <fstream>
#include "allocore/graphics/al_Mesh.hpp"
#include "allocore/system/al_Printing.hpp"
#include "allocore/graphics/al_Graphics.hpp"

namespace al{

Mesh::Mesh(int primitive)
:	mPrimitive(primitive)
{}

Mesh::Mesh(const Mesh& cpy)
:	mVertices(cpy.mVertices),
	mNormals(cpy.mNormals),
	mColors(cpy.mColors),
	mColoris(cpy.mColoris),
	mTexCoord1s(cpy.mTexCoord1s),
	mTexCoord2s(cpy.mTexCoord2s),
	mTexCoord3s(cpy.mTexCoord3s),
	mIndices(cpy.mIndices),
	mPrimitive(cpy.mPrimitive)
{}

Mesh& Mesh::reset() {
	vertices().reset();
	normals().reset();
	colors().reset();
	coloris().reset();
	texCoord1s().reset();
	texCoord2s().reset();
	texCoord3s().reset();
	indices().reset();
	return *this;
}

bool Mesh::isTriangles() const { return primitive() == Graphics::TRIANGLES; }
bool Mesh::isTriangleStrip() const { return primitive() == Graphics::TRIANGLE_STRIP; }

Mesh& Mesh::decompress(){
	if(indices().size()){ // only makes sense for indexed mesh
		toTriangles();
		int Ni = indices().size();

		#define DECOMPRESS(buf, Type)\
		{\
			int N = buf.size();\
			if(N > 1){\
				std::vector<Type> old(N);\
				std::copy(&buf[0], (&buf[0]) + N, old.begin());\
				buf.size(Ni);\
				for(int i=0; i<Ni; ++i)	buf[i] = old[indices()[i]];\
			}\
		}
		DECOMPRESS(vertices(), Vertex)
		DECOMPRESS(colors(), Color)
		DECOMPRESS(coloris(), Colori)
		DECOMPRESS(normals(), Normal)
		DECOMPRESS(texCoord1s(), TexCoord1)
		DECOMPRESS(texCoord2s(), TexCoord2)
		DECOMPRESS(texCoord3s(), TexCoord3)
		#undef DECOMPRESS

		indices().reset();
	}
	return *this;
}

Mesh& Mesh::equalizeBuffers() {
	const int Nv = vertices().size();
	const int Nn = normals().size();
	const int Nc = colors().size();
	const int Nci= coloris().size();
	const int Nt1= texCoord1s().size();
	const int Nt2= texCoord2s().size();
	const int Nt3= texCoord3s().size();

	if(Nn){
		for(int i=Nn; i<Nv; ++i){
			normals().append(normals()[Nn-1]);
		}
	}
	if(Nc){
		for(int i=Nc; i<Nv; ++i){
			colors().append(colors()[Nc-1]);
		}
	}
	else if(Nci){
		for(int i=Nci; i<Nv; ++i){
			coloris().append(coloris()[Nci-1]);
		}
	}
	if(Nt1){
		for(int i=Nt1; i<Nv; ++i){
			texCoord1s().append(texCoord1s()[Nt1-1]);
		}
	}
	if(Nt2){
		for(int i=Nt2; i<Nv; ++i){
			texCoord2s().append(texCoord2s()[Nt2-1]);
		}
	}
	if(Nt3){
		for(int i=Nt3; i<Nv; ++i){
			texCoord3s().append(texCoord3s()[Nt3-1]);
		}
	}
	return *this;
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
void Mesh::createNormalsMesh(Mesh& mesh, float length, bool perFace){

	auto initMesh = [](Mesh& m, int n){
		m.vertices().size(n*2);
		m.reset();
		m.primitive(Graphics::LINES);
	};

	if (perFace) {
		// compute vertex based normals
		if(indices().size()){

			int Ni = indices().size();
			Ni = Ni - (Ni%3); // must be multiple of 3
			initMesh(mesh, (Ni/3)*2);

			for(int i=0; i<Ni; i+=3){
				Index i1 = indices()[i+0];
				Index i2 = indices()[i+1];
				Index i3 = indices()[i+2];
				const Vertex& v1 = vertices()[i1];
				const Vertex& v2 = vertices()[i2];
				const Vertex& v3 = vertices()[i3];

				// get mean:
				const Vertex mean = (v1 + v2 + v3)/3.f;

				// get face normal:
				Vertex facenormal = cross(v2-v1, v3-v1);
				facenormal.normalize();

				mesh.vertex(mean);
				mesh.vertex(mean + (facenormal*length));
			}
		} else {
			AL_WARN_ONCE("createNormalsMesh only valid for indexed meshes");
		}
	} else {
		int Ni = al::min(vertices().size(), normals().size());
		initMesh(mesh, Ni*2);

		for(int i=0; i<Ni; ++i){
			const Vertex& v = vertices()[i];
			mesh.vertex(v);
			mesh.vertex(v + normals()[i]*length);
		}
	}
}

Mesh& Mesh::invertNormals() {
	for(auto& v : normals()) v = -v;
	return *this;
}

Mesh& Mesh::compress() {

	int Ni = indices().size();
	int Nv = vertices().size();
	if (Ni) {
		AL_WARN_ONCE("cannot compress Mesh with indices");
		return *this;
	}
	if (Nv == 0) {
		AL_WARN_ONCE("cannot compress Mesh with no vertices");
		return *this;
	}

	int Nc = colors().size();
	int Nci = coloris().size();
	int Nn = normals().size();
	int Nt1 = texCoord1s().size();
	int Nt2 = texCoord2s().size();
	int Nt3 = texCoord3s().size();

	// map tree to uniquely ID vertices with same values:
	typedef std::map<float, int> Zmap;
	typedef std::map<float, Zmap> Ymap;
	typedef std::map<float, Ymap> Xmap;
	Xmap xmap;

	// copy current values:
	Mesh old(*this);

	// walk backward through the vertex list
	// create a ID for each one
	for (int i=vertices().size()-1; i>=0; i--) {
		Vertex& v = vertices()[i];
		xmap[v.x][v.y][v.z] = i;
	}

	// map of old vertex index to new vertex index:
	typedef std::map<int, int> Imap;
	Imap imap;

	// reset current mesh:
	reset();

	// walk forward, inserting if
	for (int i=0; i<old.vertices().size(); i++) {
		Vertex& v = old.vertices()[i];
		int idx = xmap[v.x][v.y][v.z];
		Imap::iterator it = imap.find(idx);
		if (it != imap.end()) {
			// use existing
			index(it->second);
		} else {
			// create new
			int newidx = vertices().size();
			vertex(v);
			if (Nc) color(old.colors()[i]);
			if (Nci) colori(old.coloris()[i]);
			if (Nn) normal(old.normals()[i]);
			if (Nt1) texCoord(old.texCoord1s()[i]);
			if (Nt2) texCoord(old.texCoord2s()[i]);
			if (Nt3) texCoord(old.texCoord3s()[i]);
			// store new index:
			imap[idx] = newidx;
			// use new index:
			index(newidx);
		}
	}

	return *this;
}

Mesh& Mesh::generateNormals(bool normalize, bool equalWeightPerFace) {
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

	auto calcNormal = [](const Vertex& v1, const Vertex& v2, const Vertex& v3, bool MWE){
		// MWAAT (mean weighted by areas of adjacent triangles)
		auto vn = cross(v2-v1, v3-v1);

		// MWE (mean weighted equally)
		if(MWE) vn.normalize();

		// MWA (mean weighted by angle)
		// This doesn't work well with dynamic marching cubes- normals
		// pop in and out for small triangles.
		/*auto v12= v2-v1;
		auto v13= v3-v1;
		auto vn = cross(v12, v13).normalize();
		vn *= angle(v12, v13) / M_PI;*/

		return vn;
	};

	unsigned Nv = vertices().size();

	// need at least one triangle
	if(Nv < 3) return *this;

	// make same number of normals as vertices
	normals().size(Nv);

	// compute vertex based normals
	if(indices().size()){

		for(auto& n : normals()) n = 0.;

		unsigned Ni = indices().size();

		if(isTriangles()){
			Ni = Ni - (Ni%3); // must be multiple of 3

			for(unsigned i=0; i<Ni; i+=3){
				auto i1 = indices()[i  ];
				auto i2 = indices()[i+1];
				auto i3 = indices()[i+2];

				auto vn = calcNormal(
					vertices()[i1], vertices()[i2], vertices()[i3],
					equalWeightPerFace
				);

				normals()[i1] += vn;
				normals()[i2] += vn;
				normals()[i3] += vn;
			}
		}
		else if(isTriangleStrip()){
			for(unsigned i=0; i<Ni-2; ++i){

				// Flip every other normal due to change in winding direction
				auto odd = i & 1;

				auto i1 = indices()[i];
				auto i2 = indices()[i+1+odd];
				auto i3 = indices()[i+2-odd];

				auto vn = calcNormal(
					vertices()[i1], vertices()[i2], vertices()[i3],
					equalWeightPerFace
				);

				normals()[i1] += vn;
				normals()[i2] += vn;
				normals()[i3] += vn;
			}
		}

		// normalize the normals
		if(normalize) for(auto& n : normals()) n.normalize();
	}

	// non-indexed case
	else{
		// compute face based normals
		if(isTriangles()){
			auto N = Nv - (Nv % 3);

			for(unsigned i=0; i<N; i+=3){
				auto i1 = i+0;
				auto i2 = i+1;
				auto i3 = i+2;
				const auto& v1 = vertices()[i1];
				const auto& v2 = vertices()[i2];
				const auto& v3 = vertices()[i3];

				auto vn = cross(v2-v1, v3-v1);
				if(normalize) vn.normalize();

				normals()[i1] = vn;
				normals()[i2] = vn;
				normals()[i3] = vn;
			}
		}
		// compute vertex based normals
		else if(isTriangleStrip()){

			for(auto& n : normals()) n = 0.;

			for(unsigned i=0; i<Nv-2; ++i){

				// Flip every other normal due to change in winding direction
				auto odd = i & 1;

				auto vn = calcNormal(
					vertices()[i], vertices()[i+1+odd], vertices()[i+2-odd],
					equalWeightPerFace
				);

				normals()[i  ] += vn;
				normals()[i+1] += vn;
				normals()[i+2] += vn;
			}

			// normalize the normals
			if(normalize) for(auto& n : normals()) n.normalize();
		}
	}

	return *this;
}



Mesh& Mesh::repeatLast(){
	if(indices().size()){
		index(indices().last());
	}
	else{
		if(colors().size()) colors().repeatLast();
		else if(coloris().size()) coloris().repeatLast();
		if(vertices().size()) vertices().repeatLast();
		if(normals().size()) normals().repeatLast();
		if(texCoord2s().size()) texCoord2s().repeatLast();
		else if(texCoord3s().size()) texCoord3s().repeatLast();
		else if(texCoord1s().size()) texCoord1s().repeatLast();
	}
	return *this;
}


Mesh& Mesh::ribbonize(float * widths, int widthsStride, bool faceBinormal){

	auto frenet = [](
		Vertex * f, const Vertex& v0, const Vertex& v1, const Vertex& v2
	){
		const auto vf = v2 - v1; // forward difference
		const auto vb = v1 - v0; // backward difference
		const auto d1 = vf + vb; // first difference (x 2)
		f[2] = cross(vb,  vf).normalized(); // binormal
		f[1] = cross(d1,f[2]).normalized();	// normal (T x B)
		//f[0] = d1.normalized(); // tangent (not used)
	};

	const int N = mVertices.size();

	if(0 == N) return *this;

	mVertices.size(N*2);
	mNormals.size(N*2);

	int in = faceBinormal ? 2 : 1;
	int ib = faceBinormal ? 1 : 2;

	Vertex ff[3]; // T,N,B

	// Compute second and second to last Frenet frames used later to ribbonize
	// the first and last vertices.
	frenet(ff, mVertices[0], mVertices[1], mVertices[2]);
	const auto n1 = ff[in];
	const auto b1 = ff[ib] * widths[0];
	frenet(ff, mVertices[N-3], mVertices[N-2], mVertices[N-1]);
	const auto nN = ff[in];
	const auto bN = ff[ib] * widths[(N-1)*widthsStride];

	// Store last vertex since it will be overwritten eventually
	const auto last = mVertices[N-1];

	// Go backwards through vertices since we are processing in place
	for(int i=N-2; i>=1; --i){
		int i0 = i-1;
		int i1 = i;
		int i2 = i+1;
		const auto& v0 = mVertices[i0];
		const auto& v1 = mVertices[i1];
		const auto& v2 = mVertices[i2];

		// Compute Frenet frame
		frenet(ff, v0,v1,v2);

		// Scale binormal by ribbon width
		ff[ib] *= widths[i1*widthsStride];

		int i12 = i1<<1;
		// v1 is ref, so we must write in reverse to properly handle i=0
		mVertices[i12+1] = v1+ff[ib];
		mVertices[i12  ] = v1-ff[ib];

		mNormals [i12  ].set(ff[in][0], ff[in][1], ff[in][2]);
		mNormals [i12+1] = mNormals[i12];
	}

	// Ribbonize first and last vertices
	mVertices[1] = mVertices[0] + b1;
	mVertices[0] = mVertices[0] - b1;
	mNormals[0] = mNormals[1] = n1;
	int iN = (N-1)*2;
	mVertices[iN+1] = last + bN;
	mVertices[iN+0] = last - bN;
	mNormals[iN+0] = mNormals[iN+1] = nN;

	if(mColors.size()) mColors.expand<2,true>();
	if(mColoris.size()) mColoris.expand<2,true>();

	return *this;
}


Mesh& Mesh::smooth(float amount, int weighting){
	std::map<int, std::set<int>> nodes;

	int Ni = indices().size();

	// Build adjacency map
	for(int i=0; i<Ni; i+=3){
		int i0 = indices()[i  ];
		int i1 = indices()[i+1];
		int i2 = indices()[i+2];
		nodes[i0].insert(i1);
		nodes[i0].insert(i2);
		nodes[i1].insert(i2);
		nodes[i1].insert(i0);
		nodes[i2].insert(i0);
		nodes[i2].insert(i1);
	}

	Mesh::Vertices vertsCopy(vertices());

	for(const auto& node: nodes){
		Mesh::Vertex sum(0,0,0);
		const auto& adjs = node.second;

		switch(weighting){
		case 0: { // equal weighting
			for(auto adj : adjs){
				sum += vertsCopy[adj];
			}
			sum /= adjs.size();
		} break;

		case 1: { // inverse distance weights; reduces vertex sliding
			float sumw = 0;
			for(auto adj : adjs){
				const auto& v = vertsCopy[adj];
				const auto& c = vertsCopy[node.first];
				float dist = (v-c).mag();
				float w = 1./dist;
				sumw += w;
				sum += v * w;
			}
			sum /= sumw;
		} break;
		}

		auto& orig = vertices()[node.first];
		orig = (sum-orig)*amount + orig;
	}

	return *this;
}


Mesh& Mesh::flipWinding(){
	if(isTriangles()){
		if(mIndices.size()){
			for(int i=0; i<mIndices.size(); i+=3)
				std::swap(mIndices[i], mIndices[i+2]);
		} else {
			for(int i=0; i<mVertices.size(); i+=3)
				std::swap(mVertices[i], mVertices[i+2]);
		}
	}
	return *this;
}



Mesh& Mesh::merge(const Mesh& src){
//	if (indices().size() || src.indices().size()) {
//		fprintf(stderr, "error: Mesh merging with indexed meshes not yet supported\n");
//		return *this;
//	}

	// TODO: only do merge if source and dest are well-formed
	// TODO: what to do when mixing float and integer colors? promote or demote?

	// TODO: indices are more complex, since the offsets may have changed.
	// we'd have to add indices.size() to all of the src.indices before adding.
	// also, both src & dst should either use or not use indices
	// tricky if src is empty...
	//indices().append(src.indices());

	// Source has indices, and I either do or don't.
	// After this block, I will have indices.
	if(src.indices().size()){
		Index Nv = vertices().size();
		Index Ni = indices().size();
		// If no indices, must create
		if(0 == Ni){
			for(Index i=0; i<Nv; ++i) index(i);
		}
		// Add source indices offset by my number of vertices
		index(src.indices().elems(), src.indices().size(), (unsigned int)Nv);
	}

	// Source doesn't have indices, but I do
	else if(indices().size()){
		int Nv = vertices().size();
		for(int i=Nv; i<Nv+src.vertices().size(); ++i) index(i);
	}

	// From here, the game is indice invariant

	//equalizeBuffers(); << TODO: must do this if we are using indices.
	vertices().append(src.vertices());
	normals().append(src.normals());
	colors().append(src.colors());
	texCoord1s().append(src.texCoord1s());
	texCoord2s().append(src.texCoord2s());
	texCoord3s().append(src.texCoord3s());

	return *this;
}


void Mesh::getBounds(Vertex& min, Vertex& max) const {
	if(vertices().size()){
		min = vertices()[0];
		max = min;
		for(int v=1; v<vertices().size(); ++v){
			const Vertex& vt = vertices()[v];
			for(int i=0; i<3; ++i){
				min[i] = std::min(min[i], vt[i]);
				max[i] = std::max(max[i], vt[i]);
			}
		}
	}
}

Mesh::Vertex Mesh::getCenter() const {
	Vertex min(0), max(0);
	getBounds(min, max);
	return min+(max-min)*0.5;
}

Mesh& Mesh::fitToSphere(float radius){
	double maxMag = 0.;
	for(auto& v : mVertices){
		auto mm = v.dot(v);
		if(mm > maxMag) maxMag=mm;
	}
	if(maxMag > 0.){
		auto nrm = radius/sqrt(maxMag);
		for(auto& v : mVertices){
			v *= nrm;
		}
	}
	return *this;
}

Mesh& Mesh::fitToCube(float radius, bool proportional){
	Vertex min(0), max(0);
	getBounds(min, max);
	// span of each axis:
	auto span = max-min;	// positive only
	// center of each axis:
	auto mid = min + (span * 0.5);
	// axis scalar:
	auto scale = (2.f*radius)/span; // positive only

	// adjust to use scale of largest axis:
	if (proportional) {
		scale = al::min(scale.x, scale.y, scale.z);
	}

	for (auto& v : mVertices) {
		v = (v-mid)*scale;
	}

	return *this;
}

Mesh& Mesh::unitize(bool proportional){
	return fitToCube(1.f, proportional);
}

Mesh& Mesh::translate(float x, float y, float z){
	const Vertex xfm(x,y,z);
	for(auto& v : mVertices) v += xfm;
	return *this;
}

Mesh& Mesh::scale(float x, float y, float z){
	const Vertex xfm(x,y,z);
	for(auto& v : mVertices) v *= xfm;
	return *this;
}

bool Mesh::valid() const { return mVertices.size(); }

// removes triplets with two matching values
template <class T>
static void removeDegenerates(Buffer<T>& buf){
	unsigned N = buf.size();
	unsigned j=0;
	for(unsigned i=0; i<N; i+=3){
		T v1 = buf[i  ];
		T v2 = buf[i+1];
		T v3 = buf[i+2];
		buf[j  ] = v1;
		buf[j+1] = v2;
		buf[j+2] = v3;
		if((v1 != v2) && (v2 != v3) && (v3 != v1)){
			j+=3;
		}
	}
	buf.size(j);
}

template <class T>
static void stripToTri(Buffer<T>& buf){
	int N = buf.size();
	int Ntri = N-2;
	buf.size(Ntri*3);

	// Iterate backwards through elements so we can operate in place
	// strip (i): 0 1 2 3 4 5 6 7 8 ...
	//  tris (j): 0 1 2 3 2 1 2 3 4 ...
	for(int i=N-3, j=Ntri*3-3; i>0; i--, j-=3){
		// Odd numbered triangles must have orientation flipped
		if(i & 1){
			buf[j  ] = buf[i+2];
			buf[j+1] = buf[i+1];
			buf[j+2] = buf[i  ];
		}
		else{
			buf[j  ] = buf[i  ];
			buf[j+1] = buf[i+1];
			buf[j+2] = buf[i+2];
		}
	}
}

Mesh& Mesh::toTriangles(){

	if(isTriangleStrip()){
		primitive(Graphics::TRIANGLES);
		int Nv = vertices().size();
		int Ni = indices().size();

		// indexed:
		if(Ni > 3){
			stripToTri(indices());
			removeDegenerates(indices());
		}
		// non-indexed:
		// TODO: remove degenerate triangles
		else if(Ni == 0 && Nv > 3){
			stripToTri(vertices());
			if(normals().size() >= Nv) stripToTri(normals());
			if(colors().size() >= Nv) stripToTri(colors());
			if(coloris().size() >= Nv) stripToTri(coloris());
			if(texCoord1s().size() >= Nv) stripToTri(texCoord1s());
			if(texCoord2s().size() >= Nv) stripToTri(texCoord2s());
			if(texCoord3s().size() >= Nv) stripToTri(texCoord3s());
		}
	}

	return *this;
}

Mesh& Mesh::colorFill(const Color& v){
	int N = vertices().size() - colors().size();
	for(int i=0; i<N; ++i) color(v);
	return *this;
}

Mesh& Mesh::coloriFill(const Colori& v){
	int N = vertices().size() - coloris().size();
	for(int i=0; i<N; ++i) colori(v);
	return *this;
}

bool Mesh::saveSTL(const std::string& filePath, const std::string& solidName) const {

	if(!(isTriangles() || isTriangleStrip())){
		AL_WARN("Unsupported primitive type. Must be either triangles or triangle strip.");
		return false;
	}

	if(!vertices().size()) return false;

	// Create a copy since we must convert to non-indexed triangles
	Mesh m(*this);

	// Convert mesh to non-indexed triangles
	m.toTriangles();
	m.decompress();
	m.generateNormals();

	// STL vertices must be in positive octant
	Vec3f vmin, vmax;
	m.getBounds(vmin, vmax);

	std::ofstream s(filePath);
	if(s.fail()) return false;
	s.flags(std::ios::scientific);

	s << "solid " << solidName << "\n";
	for(int i=0; i<m.vertices().size(); i+=3){
		s << "facet normal";
		for(int j=0; j<3; j++) s << " " << m.normals()[i][j];
		s << "\n";
		s << "outer loop\n";
		for(int j=0; j<3; ++j){
			s << "vertex";
			for(int k=0; k<3; k++) s << " " << m.vertices()[i+j][k] - vmin[k];
			s << "\n";
		}
		s << "endloop\n";
		s << "endfacet\n";
	}
	s << "endsolid " << solidName;

	return true;
}

bool Mesh::savePLY(const std::string& filePath, const std::string& solidName, bool binary) const {

	if(!(isTriangles() || isTriangleStrip())){
		AL_WARN("Unsupported primitive type. Must be either triangles or triangle strip.");
		return false;
	}

	if(!vertices().size()) return false;

	std::ofstream s;
	s.open(filePath, binary ? (std::ios::out | std::ios::binary) : std::ios::out);
	if(s.fail()) return false;

	// Use a copy to handle triangle strip;
	// not ideal if already triangles!
	Mesh m(*this);
	m.toTriangles();

	const unsigned Nv = m.vertices().size();
	const unsigned Nn = m.normals().size();
	const unsigned Nc = m.colors().size();
	const unsigned Nci= m.coloris().size();
	const unsigned Ni = m.indices().size();
	//const unsigned Bi = Nv<=65536 ? 2 : 4; // max bytes/index
	const unsigned Bi = Nv<=32768 ? 2 : 4; // changed since assimp import not working with full ushort range up to 65536

	// Ref: http://paulbourke.net/dataformats/ply/

	int bigEndian = 1;
	if(1 == *(char *)&bigEndian) bigEndian = 0;

	// Header
	s << "ply\n";
	s << "format " << (binary ? (bigEndian ? "binary_big_endian" : "binary_little_endian") : "ascii") << " 1.0\n";
	s << "comment AlloSystem\n";

	if(solidName[0]){
		s << "comment " << solidName << "\n";
	}

	s <<
	"element vertex " << Nv << "\n"
	"property float x\n"
	"property float y\n"
	"property float z\n"
	;

	bool hasNormals = Nn >= Nv;
	if(hasNormals){
		const char * type = binary ? "short" : "float";
		s <<
		"property " << type << " nx\n"
		"property " << type << " ny\n"
		"property " << type << " nz\n"
		;
	}

	bool hasColors = Nc >= Nv || Nci >= Nv;
	if(hasColors){
		s <<
		"property uchar red\n"
		"property uchar green\n"
		"property uchar blue\n"
		"property uchar alpha\n"
		;
	}

	// TODO: texcoords (s,t)

	if(Ni){
		s << "element face " << Ni/3 << "\n";
		s << "property list uchar " << (Bi==4?"uint":"ushort") << " vertex_indices\n";
	}

	s << "end_header\n";

	if(binary){
		// Vertex data
		for(unsigned i=0; i<Nv; ++i){
			s.write(reinterpret_cast<const char*>(&m.vertices()[i][0]), sizeof(Mesh::Vertex));
			if(hasNormals){
				for(int k=0; k<3; ++k){
					short v=m.normals()[i][k]*32767.;
					s.write(reinterpret_cast<const char*>(&v), 2);
				}
			}
			if(hasColors){
				auto col = Nci >= Nv ? m.coloris()[i] : Colori(m.colors()[i]);
				s << col.r << col.g << col.b << col.a;
			}
		}
		// Face data
		if(Ni){
			for(unsigned i=0; i<Ni; i+=3){
				s << char(3); // 3 indices/face
				if(sizeof(Mesh::Index) == Bi){
					s.write(reinterpret_cast<const char*>(&m.indices()[i]), Bi*3);
				}
				else{
					unsigned short idx[3];
					idx[0] = m.indices()[i  ];
					idx[1] = m.indices()[i+1];
					idx[2] = m.indices()[i+2];
					s.write(reinterpret_cast<const char*>(idx), Bi*3);
					//printf("%u %u %u\n", idx[0], idx[1], idx[2]);
					/*for(int i=0; i<Bi*3; ++i){
						printf("%d ", reinterpret_cast<char*>(idx)[i]);
					}printf("\n");*/
				}
			}
		}
	}
	else {
		// Vertex data
		for(unsigned i = 0; i < Nv; ++i){
			auto vrt = m.vertices()[i];
			s << vrt.x << " " << vrt.y << " " << vrt.z;
			if(hasNormals){
				auto nrm = m.normals()[i];
				s << " " << nrm.x << " " << nrm.y << " " << nrm.z;
			}
			if(hasColors){
				auto col = Nci >= Nv ? m.coloris()[i] : Colori(m.colors()[i]);
				s << " " << int(col.r) << " " << int(col.g) << " " << int(col.b) << " " << int(col.a);
			}
			s << "\n";
		}
		// Face data
		if(Ni){
			for(unsigned i = 0; i < Ni; i+=3){
				auto i1 = m.indices()[i  ];
				auto i2 = m.indices()[i+1];
				auto i3 = m.indices()[i+2];
				s << "3 " << i1 << " " << i2 << " " << i3 << "\n";
			}
		}
	}

	return true;
}

bool Mesh::save(const std::string& filePath, const std::string& solidName, bool binary) const {

	auto pos = filePath.find_last_of(".");
	if(std::string::npos == pos) return false;
	auto ext = filePath.substr(pos+1);
	std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

	if("ply" == ext){
		return savePLY(filePath, solidName, binary);
	}
	else if("stl" == ext){
		return saveSTL(filePath, solidName);
	}

	return false;
}

void Mesh::print(FILE * dst) const {
	fprintf(dst, "Mesh %p (prim = %d) has:\n", this, mPrimitive);
	if(vertices().size())	fprintf(dst, "%8d Vertices\n", vertices().size());
	if(colors().size())		fprintf(dst, "%8d Colors\n", colors().size());
	if(coloris().size())	fprintf(dst, "%8d Coloris\n", coloris().size());
	if(normals().size())	fprintf(dst, "%8d Normals\n", normals().size());
	if(texCoord1s().size())	fprintf(dst, "%8d TexCoord1s\n", texCoord1s().size());
	if(texCoord2s().size())	fprintf(dst, "%8d TexCoord2s\n", texCoord2s().size());
	if(texCoord3s().size())	fprintf(dst, "%8d TexCoord3s\n", texCoord3s().size());
	if(indices().size())	fprintf(dst, "%8d Indices\n", indices().size());

	unsigned bytes
		= vertices().size()*sizeof(Vertex)
		+ colors().size()*sizeof(Color)
		+ coloris().size()*sizeof(Colori)
		+ normals().size()*sizeof(Normal)
		+ texCoord1s().size()*sizeof(TexCoord1)
		+ texCoord2s().size()*sizeof(TexCoord2)
		+ texCoord3s().size()*sizeof(TexCoord3)
		+ indices().size()*sizeof(Index)
		;
	fprintf(dst, "%8d bytes (%.1f kB)\n", bytes, double(bytes)/1000);
}

bool Mesh::debug(FILE * dst) const {

	#define DPRINTF(...) if(dst){ fprintf(dst, __VA_ARGS__); }

	bool ok = true;
	int Nv = vertices().size();
	if(!Nv){ DPRINTF("No vertices\n"); ok=false; }

	#define CHECK_ARR(arr, oneOkay)\
	if(arr().size() && arr().size() != Nv){\
		if(!oneOkay || arr().size()!=1){\
			DPRINTF("%d " #arr ", but %d vertices\n", arr().size(), Nv);\
			ok=false;\
		}\
	}
	CHECK_ARR(normals, false);
	CHECK_ARR(colors, true);
	CHECK_ARR(coloris, true);
	CHECK_ARR(texCoord1s, false);
	CHECK_ARR(texCoord2s, false);
	CHECK_ARR(texCoord3s, false);
	#undef CHECK_ARR

	if(colors().size() && coloris().size()){
		DPRINTF("More than one color array populated\n");
		ok=false;
	}

	if(texCoord1s().size() && texCoord2s().size() && texCoord2s().size()){
		DPRINTF("More than one texture coordinate array populated\n");
		ok=false;
	}

	if(indices().size()){
		for(auto i : indices()){
			if(i > Nv){
				DPRINTF("Index out of bounds: %d (%d max)\n", i, Nv);
				ok=false;
				break;
			}
		}
	}

	#undef DPRINTF

	return ok;
}

} // al::
