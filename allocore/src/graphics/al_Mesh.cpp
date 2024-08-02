#include <algorithm> // transform
#include <cctype> // tolower
#include <cmath> // min
#include <cstdio>
#include <map>
#include <set>
#include <string>
#include <sstream> // stringstream
#include <utility> // swap
#include <vector>
#include <fstream> // filebuf
#include <iostream> // istream
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

Mesh::Mesh(const std::function<void(Mesh&)>& f)
:	Mesh()
{
	f(*this);
}

Mesh& Mesh::reset() {
	mVertices.reset();
	mNormals.reset();
	mTangents.reset();
	mColors.reset();
	mColoris.reset();
	mTexCoord1s.reset();
	mTexCoord2s.reset();
	mTexCoord3s.reset();
	mIndices.reset();
	return *this;
}

Mesh& Mesh::points(){ return primitive(Graphics::POINTS); }
Mesh& Mesh::points(float stroke){ return points().stroke(stroke); }
Mesh& Mesh::lines(){ return primitive(Graphics::LINES); }
Mesh& Mesh::lines(float stroke){ return lines().stroke(stroke); }
Mesh& Mesh::lineStrip(){ return primitive(Graphics::LINE_STRIP); }
Mesh& Mesh::lineStrip(float stroke){ return lineStrip().stroke(stroke); }
Mesh& Mesh::lineLoop(){ return primitive(Graphics::LINE_LOOP); }
Mesh& Mesh::lineLoop(float stroke){ return lineLoop().stroke(stroke); }
Mesh& Mesh::triangles(){ return primitive(Graphics::TRIANGLES); }
Mesh& Mesh::triangleStrip(){ return primitive(Graphics::TRIANGLE_STRIP); }

bool Mesh::isPoints() const { return primitive() == Graphics::POINTS; }
bool Mesh::isLines() const { return primitive() == Graphics::LINES; }
bool Mesh::isLineStrip() const { return primitive() == Graphics::LINE_STRIP; }
bool Mesh::isLineLoop() const { return primitive() == Graphics::LINE_LOOP; }
bool Mesh::isTriangles() const { return primitive() == Graphics::TRIANGLES; }
bool Mesh::isTriangleStrip() const { return primitive() == Graphics::TRIANGLE_STRIP; }
bool Mesh::isTriangleType() const { return isTriangles() || isTriangleStrip(); }

Mesh& Mesh::decompress(){
	if(indexed()){ // only makes sense for indexed mesh
		toTriangles();
		int Ni = mIndices.size();

		#define DECOMPRESS(buf){\
			int N = buf.size();\
			if(N > 1){\
				auto old(buf);\
				buf.resize(Ni);\
				for(int i=0; i<Ni; ++i)	buf[i] = old[mIndices[i]];\
			}\
		}
		DECOMPRESS(mVertices)
		DECOMPRESS(mNormals)
		DECOMPRESS(mTangents)
		DECOMPRESS(mColors)
		DECOMPRESS(mColoris)
		DECOMPRESS(mTexCoord1s)
		DECOMPRESS(mTexCoord2s)
		DECOMPRESS(mTexCoord3s)
		#undef DECOMPRESS

		mIndices.reset();
	}
	return *this;
}

template <class T>
void equalize(Buffer<T>& b, int N){
	auto M = b.size();
	if(M){
		for(int i=M; i<N; ++i) b.append(b[M-1]);
	}
}

Mesh& Mesh::equalizeBuffers() {
	auto Nv = mVertices.size();
	equalize(mNormals, Nv);
	equalize(mTangents, Nv);
	equalize(mColors, Nv);
	equalize(mColoris, Nv);
	equalize(mTexCoord1s, Nv);
	equalize(mTexCoord2s, Nv);
	equalize(mTexCoord3s, Nv);
	return *this;
}

void Mesh::createNormalsMesh(Mesh& mesh, float length, bool perFace) const {

	auto initMesh = [](Mesh& m, int numNormals){
		m.mVertices.resize(numNormals*2);
		m.reset();
		m.lines();
	};

	// Generate normal lines based on faces (ignoring existing normals)
	if(perFace){
		if(isTriangleType()){
			int Nn = std::max(mVertices.size(), mIndices.size())/3;
			initMesh(mesh, Nn);

			forEachFace([&](int i1, int i2, int i3){
				if(i1==i2 || i2==i3 || i3==i1) return; // skip degenerate

				auto v1 = mVertices[i1];
				auto v2 = mVertices[i2];
				auto v3 = mVertices[i3];

				auto ctr = (v1 + v2 + v3)*0.33333f;
				auto dN = cross(v2-v1, v3-v1).normalize(length);

				mesh.vertex(ctr);
				mesh.vertex(ctr + dN);
			});
		} else {
			AL_WARN_ONCE("createNormalsMesh: per-face option only valid for triangle meshes");
		}

	// Generate normal lines based on existing normals in mesh (if any)
	} else {
		int Nn = std::min(mVertices.size(), mNormals.size());
		initMesh(mesh, Nn);

		for(int i=0; i<Nn; ++i){
			auto& v = mVertices[i];
			mesh.vertex(v);
			mesh.vertex(v + mNormals[i]*length);
		}
	}
}

Mesh& Mesh::compress(){

	int Ni = mIndices.size();
	int Nv = mVertices.size();
	if(Ni){
		AL_WARN_ONCE("cannot compress Mesh with indices");
		return *this;
	}
	if(0 == Nv){
		AL_WARN_ONCE("cannot compress Mesh with no vertices");
		return *this;
	}

	int Nc = mColors.size();
	int Nci = mColoris.size();
	int Nn = mNormals.size();
	int Nt = mTangents.size();
	int Nt1 = mTexCoord1s.size();
	int Nt2 = mTexCoord2s.size();
	int Nt3 = mTexCoord3s.size();

	// map tree to uniquely ID vertices with same values:
	typedef std::map<float, int> Zmap;
	typedef std::map<float, Zmap> Ymap;
	typedef std::map<float, Ymap> Xmap;
	Xmap xmap;

	// copy current values:
	Mesh old(*this);

	// walk backward through the vertex list
	// create a ID for each one
	for(int i=mVertices.size()-1; i>=0; i--){
		auto v = mVertices[i];
		xmap[v.x][v.y][v.z] = i;
	}

	// map of old vertex index to new vertex index:
	std::map<int, int> imap;

	reset();

	// walk forward, inserting if
	for(int i=0; i<old.mVertices.size(); i++){
		auto v = old.mVertices[i];
		int idx = xmap[v.x][v.y][v.z];
		auto it = imap.find(idx);
		if(it != imap.end()){
			index(it->second); // use existing
		} else {
			// create new
			int newidx = mVertices.size();
			vertex(v);
			if(Nc) color(old.mColors[i]);
			if(Nci) colori(old.mColoris[i]);
			if(Nn) normal(old.mNormals[i]);
			if(Nt) tangent(old.mTangents[i]);
			if(Nt1) texCoord(old.mTexCoord1s[i]);
			if(Nt2) texCoord(old.mTexCoord2s[i]);
			if(Nt3) texCoord(old.mTexCoord3s[i]);
			imap[idx] = newidx; // store new index
			index(newidx); // use new index
		}
	}

	return *this;
}

Mesh& Mesh::generateNormals(bool normalize, bool equalWeightPerFace) {

	if(!isTriangleType()) return *this;

	unsigned Nv = mVertices.size();

	// need at least one triangle
	if(Nv < 3) return *this;

	// make same number of normals as vertices
	mNormals.resize(Nv);

	// per-vertex case
	if(indexed() || isTriangleStrip()){
		for(auto& n : mNormals) n = 0.;

		auto calcNormal = [](const Vertex& p1, const Vertex& p2, const Vertex& p3, bool MWE){
			// MWAAT (mean weighted by areas of adjacent triangles)
			auto n = perp(p1, p2, p3);

			// MWE (mean weighted equally)
			if(MWE) n.normalize();

			// MWA (mean weighted by angle)
			// This doesn't work well with dynamic marching cubes- normals
			// pop in and out for small triangles.
			/*auto p12 = p2-p1;
			auto p13 = p3-p1;
			auto n = cross(p12, p13).normalize();
			n *= angle(p12, p13) / M_PI;*/

			return n;
		};

		forEachFace([&](int i1, int i2, int i3){
			auto n = calcNormal(
				mVertices[i1], mVertices[i2], mVertices[i3],
				equalWeightPerFace
			);
			mNormals[i1] += n;
			mNormals[i2] += n;
			mNormals[i3] += n;
		});
	}
	// per-face case (non-indexed triangles)
	else {
		forEachFace([&](int i1, int i2, int i3){
			const auto& p1 = mVertices[i1];
			const auto& p2 = mVertices[i2];
			const auto& p3 = mVertices[i3];
			auto n = perp(p1, p2, p3);
			mNormals[i1] = n;
			mNormals[i2] = n;
			mNormals[i3] = n;
		});
	}

	// normalize the normals?
	if(normalize) for(auto& n : mNormals) n.normalize();

	return *this;
}

Mesh& Mesh::ensureNormals(bool equalWeightPerFace){
	if(mNormals.empty()) generateNormals(true, equalWeightPerFace);
	return *this;
}

Mesh& Mesh::invertNormals(){
	for(auto& v : mNormals) v = -v;
	for(auto& v : mTangents) v = -v;
	return *this;
}

Mesh& Mesh::repeatLast(){
	if(indexed()){
		index(mIndices.last());
	}
	else{
		if(mVertices.size()) mVertices.repeatLast();
		if(mNormals.size()) mNormals.repeatLast();
		if(mTangents.size()) mTangents.repeatLast();
		if(mColors.size()) mColors.repeatLast();
		else if(mColoris.size()) mColoris.repeatLast();
		if(mTexCoord2s.size()) mTexCoord2s.repeatLast();
		else if(mTexCoord3s.size()) mTexCoord3s.repeatLast();
		else if(mTexCoord1s.size()) mTexCoord1s.repeatLast();
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

	mVertices.resize(N*2);
	mNormals.resize(N*2);

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

	int Ni = mIndices.size();

	// Build adjacency map
	for(int i=0; i<Ni; i+=3){
		int i0 = mIndices[i  ];
		int i1 = mIndices[i+1];
		int i2 = mIndices[i+2];
		nodes[i0].insert(i1);
		nodes[i0].insert(i2);
		nodes[i1].insert(i2);
		nodes[i1].insert(i0);
		nodes[i2].insert(i0);
		nodes[i2].insert(i1);
	}

	auto vertsCopy = mVertices;

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

		auto& orig = mVertices[node.first];
		orig = (sum-orig)*amount + orig;
	}

	return *this;
}


Mesh& Mesh::flipWinding(){
	if(isTriangles()){
		if(indexed()){
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
	// TODO: only do merge if source and dest are well-formed

	if(src.mVertices.empty()) return *this;

	// Inherit primitive if no verts yet
	if(mVertices.empty()){
		primitive(src.primitive());
	}

	const int Nv = mVertices.size();

	// Source has indices, and I either do or don't.
	// After this block, I will have indices.
	if(src.indexed()){
		// If no indices, must create
		if(!indexed()){
			for(int i=0; i<Nv; ++i) index(i);
		}
		// Add source indices offset by my number of vertices
		index(src.mIndices.data(), src.mIndices.size(), (unsigned int)Nv);
	}

	// Source doesn't have indices, but I do
	else if(indexed()){
		for(int i=Nv; i<Nv+src.mVertices.size(); ++i) index(i);
	}

	// From here, everything is indice invariant
	mVertices.append(src.mVertices);
	mNormals.append(src.mNormals);
	mTangents.append(src.mTangents);

	#define COPY_COL(dcolor)\
	if(dcolor##s().size() == Nv){\
		if(src.mColors.size() >= src.mVertices.size()){\
			for(auto& c : src.mColors) dcolor(c);\
		} else if(src.mColors.size() > 0){\
			dcolor##Fill(src.mColors[0]);\
		} else if(src.mColoris.size() >= src.mVertices.size()){\
			for(auto& c : src.mColoris) dcolor(c);\
		} else if(src.mColoris.size() > 0){\
			dcolor##Fill(src.mColoris[0]);\
		}\
	}
	COPY_COL(color)
	else COPY_COL(colori)

	mTexCoord1s.append(src.mTexCoord1s);
	mTexCoord2s.append(src.mTexCoord2s);
	mTexCoord3s.append(src.mTexCoord3s);

	return *this;
}


void Mesh::bounds(Vertex& min, Vertex& max) const {
	if(mVertices.size()){
		min = mVertices[0];
		max = min;
		for(int v=1; v<mVertices.size(); ++v){
			const Vertex& vt = mVertices[v];
			for(int i=0; i<3; ++i){
				min[i] = std::min(min[i], vt[i]);
				max[i] = std::max(max[i], vt[i]);
			}
		}
	}
}

Mesh::Vertex Mesh::centroid() const {
	Vertex min(0), max(0);
	bounds(min, max);
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

Mesh& Mesh::fitToCubeTransform(Vec3f& center, Vec3f& scale, float radius, bool proportional){
	Vertex min(0), max(0);
	bounds(min, max);
	// span of each axis:
	auto span = max-min;	// positive only
	// center of each axis:
	center = min + (span * 0.5);
	// axis scalar:
	scale = (2.f*radius)/span; // positive only

	// adjust to use scale of largest axis:
	if(proportional){
		scale = std::min({scale.x, scale.y, scale.z});
	}
	return *this;
}

Mesh& Mesh::fitToCube(float radius, bool proportional){
	Vec3f center, scale;
	fitToCubeTransform(center, scale, radius, proportional);
	for(auto& v : mVertices) v = (v-center)*scale;
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

template <class T>
Mesh& Mesh::transform(const Mat<4,T>& m, int begin, int end){
	if(begin<0) begin += mVertices.size();
	if(  end<0)   end += mVertices.size()+1; // negative index wraps to end of array

	#define LOOP for(int i=begin; i<end; ++i)

	LOOP mVertices[i] = m.transformPoint(mVertices[i]);

	if(mTangents.size() >= mVertices.size()){
		LOOP mTangents[i] = m.transformVector(mTangents[i]).normalize();
	}

	// normals must be transformed by (A^-1)^T
	if(mNormals.size() >= mVertices.size()){
		auto n = normalMatrix(m);
		LOOP mNormals[i] = (n * mNormals[i]).normalize();
	}

	#undef LOOP

	return *this;
}

template Mesh& Mesh::transform<float>(const Mat<4,float>&, int, int);
template Mesh& Mesh::transform<double>(const Mat<4,double>&, int, int);


bool Mesh::valid() const { return mVertices.size(); }
bool Mesh::empty() const { return 0==mVertices.size(); }

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
	buf.resize(j);
}

template <class T>
static void stripToTri(Buffer<T>& buf){
	int N = buf.size();
	int Ntri = N-2;
	buf.resize(Ntri*3);

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
		int Nv = mVertices.size();
		int Ni = mIndices.size();

		// indexed:
		if(Ni > 3){
			stripToTri(mIndices);
			removeDegenerates(mIndices);
		}
		// non-indexed:
		// TODO: remove degenerate triangles
		else if(Ni == 0 && Nv > 3){
			stripToTri(mVertices);
			if(mNormals.size() >= Nv) stripToTri(mNormals);
			if(mTangents.size() >= Nv) stripToTri(mTangents);
			if(mColors.size() >= Nv) stripToTri(mColors);
			if(mColoris.size() >= Nv) stripToTri(mColoris);
			if(mTexCoord1s.size() >= Nv) stripToTri(mTexCoord1s);
			if(mTexCoord2s.size() >= Nv) stripToTri(mTexCoord2s);
			if(mTexCoord3s.size() >= Nv) stripToTri(mTexCoord3s);
		}
	}

	return *this;
}

#define DEF_FILL(name, Type)\
Mesh& Mesh::name##Fill(const Type& v){\
	int N = mVertices.size() - name##s().size();\
	for(int i=0; i<N; ++i) name(v);\
	return *this;\
}

DEF_FILL(normal, Normal)
DEF_FILL(tangent, Tangent)
DEF_FILL(color, Color)
DEF_FILL(colori, Colori)

Mesh& Mesh::forEachFace(const std::function<void(int v1, int v2, int v3)>& onFace){
	if(indexed()){
		if(isTriangles()){
			for(int i=2; i<mIndices.size(); i+=3){
				onFace(mIndices[i-2], mIndices[i-1], mIndices[i]);
			}
		} else if(isTriangleStrip()){
			for(int i=0; i<mIndices.size()-2; i++){
				int w = i&1; // winding: 0=ccw, 1=cw
				onFace(mIndices[i+w], mIndices[i+1-w], mIndices[i+2]);
			}
		} else if(isLines()){
			for(int i=1; i<mIndices.size(); i+=2){
				onFace(mIndices[i-1], mIndices[i], mIndices[i-1]);
			}
		} else if(isLineStrip() || isLineLoop()){
			for(int i=1; i<mIndices.size(); i++){
				onFace(mIndices[i-1], mIndices[i], mIndices[i-1]);
			}
			if(isLineLoop())
				onFace(mIndices.last(), mIndices[0], mIndices.last());
		} else if(isPoints()){
			for(int i=0; i<mIndices.size(); i++){
				onFace(mIndices[i], mIndices[i], mIndices[i]);
			}
		}
	} else {
		if(isTriangles()){
			for(int i=2; i<mVertices.size(); i+=3){
				onFace(i-2, i-1, i);
			}
		} else if(isTriangleStrip()){
			for(int i=0; i<mVertices.size()-2; i++){
				int w = i&1; // winding: 0=ccw, 1=cw
				onFace(i+w, i+1-w, i+2);
			}
		} else if(isLines()){
			for(int i=1; i<mVertices.size(); i+=2){
				onFace(i-1, i, i-1);
			}
		} else if(isLineStrip() || isLineLoop()){
			for(int i=1; i<mVertices.size(); i++){
				onFace(i-1, i, i-1);
			}
			if(isLineLoop())
				onFace(mVertices.size()-1, 0, mVertices.size()-1);
		} else if(isPoints()){
			for(int i=0; i<mVertices.size(); i++){
				onFace(i, i, i);
			}
		}
	}
	return *this;
}

const Mesh& Mesh::forEachFace(const std::function<void(int v1, int v2, int v3)>& onFace) const {
	return const_cast<Mesh*>(this)->forEachFace(onFace);
}

Mesh& Mesh::forEachVertex(const std::function<void(int i)>& onVert){
	for(int i=0; i<mVertices.size(); ++i) onVert(i);
	return *this;
}

const Mesh& Mesh::forEachVertex(const std::function<void(int i)>& onVert) const {
	return const_cast<Mesh*>(this)->forEachVertex(onVert);
}

Mesh& Mesh::forEachVertex(Mesh::Group g, const std::function<void(int i)>& onVert){
	for(int i=g.begin; i<g.end; ++i) onVert(i);
	return *this;
}

const Mesh& Mesh::forEachVertex(Mesh::Group g, const std::function<void(int i)>& onVert) const {
	return const_cast<Mesh*>(this)->forEachVertex(g, onVert);
}

bool Mesh::saveFBX(const std::string& filePath, const std::string& solidName) const {

	if(!(isTriangles() || isTriangleStrip())){
		AL_WARN("Unsupported primitive type. Must be either triangles or triangle strip.");
		return false;
	}

	if(!mVertices.size()) return false;

	const bool binary = false;
	std::ofstream fs;
	fs.open(filePath, binary ? (std::ios::out | std::ios::binary) : std::ios::out);
	if(fs.fail()) return false;

	// Use a copy to handle triangle strip
	Mesh copy;
	if(isTriangleStrip()){
		copy = *this;
		copy.toTriangles();
	}
	const auto& m = isTriangleStrip() ? copy : *this;

	const auto Nv = m.mVertices.size();
	const auto Nc = m.mColors.size();
	const auto Nci = m.mColoris.size();

	fs <<
R"(; FBX 7.4.0 project file
FBXHeaderExtension: {
	FBXHeaderVersion: 1003
	FBXVersion: 7400
	Creator: "AlloSystem"
}
Definitions: {
	Version: 100
	Count: 3
	ObjectType: "GlobalSettings" {
		Count: 1
	}
	ObjectType: "Model" {
		Count: 1
	}
	ObjectType: "Geometry" {
		Count: 1
	}
}
GlobalSettings:  {
	Version: 100
	Properties70:  {
		P: "UpAxis", "int", "Integer", "",1
		P: "UpAxisSign", "int", "Integer", "",1
		P: "FrontAxis", "int", "Integer", "",2
		P: "FrontAxisSign", "int", "Integer", "",1
		P: "CoordAxis", "int", "Integer", "",0
		P: "CoordAxisSign", "int", "Integer", "",1
	}
}
Objects: {
	Model: 75, "Model::model", "Mesh" {
		Version: 232
		Properties70: {
			P: "DefaultAttributeIndex", "int", "Integer", "",0
		}
	}
	Geometry: 50, "Geometry::", "Mesh" {
)";
	auto writeFloat = [&](float v, int precision=6){
		fs.precision(precision);
		if(std::abs(v) < 1e-7) v=0;
		fs << v << ',';
	};

	fs << "\t\tVertices: *" << Nv*3 << " {\n";
	fs << "\t\t\ta: ";
	for(auto& p : m.mVertices){
		writeFloat(p[0]);
		writeFloat(p[1]);
		writeFloat(p[2]);
	}
	fs.seekp(-1, std::ios::cur); // erase last comma
	fs << "\n\t\t}\n";

	if(m.indexed()){
		fs << "\t\tPolygonVertexIndex: *" << m.mIndices.size() << " {\n\t\t\ta: ";
		for(int i=0; i<m.mIndices.size(); i+=3){
			fs << (int)(m.mIndices[i  ]) << ',';
			fs << (int)(m.mIndices[i+1]) << ',';
			fs <<-(int)(m.mIndices[i+2]+1) << ','; // negate to close polygon (yes, needs +1!!!)
		}
		fs.seekp(-1, std::ios::cur); // erase last comma
		fs << "\n\t\t}\n";
	}

	fs << "\t\tGeometryVersion: 100\n";

	bool hasNormals = m.mNormals.size()>=Nv;
	bool hasColors = m.mColors.size()>=Nv || m.mColoris.size()>=Nv;

	if(hasNormals){
		fs << R"(
		LayerElementNormal: 0 {
			Version: 100
			Name: ""
			MappingInformationType: "ByVertice"
			ReferenceInformationType: "Direct"
			Normals: *)" << m.mNormals.size()*3 << " {\n\t\t\t\ta: ";
		for(auto& n : m.mNormals){
			writeFloat(n[0], 4);
			writeFloat(n[1], 4);
			writeFloat(n[2], 4);
		}
		fs.seekp(-1, std::ios::cur); // erase last comma
		fs << "\n\t\t\t}\n\t\t}\n";
	}

	if(hasColors){
		auto Nc = m.mColors.size();
		auto Nci = m.mColoris.size();
		fs << R"(
		LayerElementColor: 0 {
			Version: 100
			Name: ""
			MappingInformationType: "ByVertice"
			ReferenceInformationType: "Direct"
			Colors: *)" << (Nc?m.mColors.size():m.mColoris.size())*4 << " {\n\t\t\t\ta: ";
		for(int i=0; i<Nv; ++i){
			auto col = Nc ? Color(m.mColors[i]) : Color(m.mColoris[i]);
			writeFloat(col.r, 4);
			writeFloat(col.g, 4);
			writeFloat(col.b, 4);
			writeFloat(col.a, 4);
		}
		fs.seekp(-1, std::ios::cur); // erase last comma
		fs << "\n\t\t\t}\n\t\t}\n";
	}

	if(hasNormals || hasColors){
		fs << R"(
		Layer: 0 {
			Version: 100)";

		if(hasNormals){
			fs << R"(
			LayerElement: {
				Type: "LayerElementNormal"
				TypedIndex: 0
			})";
		}
		if(hasColors){
			fs << R"(
			LayerElement: {
				Type: "LayerElementColor"
				TypedIndex: 0
			})";
		}

		fs << "\n\t\t}";
	}

	fs << R"(
	}
}
Connections: {
	C: "OO",50,75
	C: "OO",75,0
})";

	return true;
}

bool Mesh::savePLY(const std::string& filePath, const std::string& solidName, bool binary) const {

	if(!(isTriangles() || isTriangleStrip())){
		AL_WARN("Unsupported primitive type. Must be either triangles or triangle strip.");
		return false;
	}

	if(!mVertices.size()) return false;

	std::ofstream s;
	s.open(filePath, binary ? (std::ios::out | std::ios::binary) : std::ios::out);
	if(s.fail()) return false;

	// Use a copy to handle triangle strip
	Mesh copy;
	if(isTriangleStrip()){
		copy = *this;
		copy.toTriangles();
	}
	const auto& m = isTriangleStrip() ? copy : *this;

	const unsigned Nv = m.mVertices.size();
	const unsigned Nn = m.mNormals.size();
	const unsigned Nc = m.mColors.size();
	const unsigned Nci= m.mColoris.size();
	const unsigned Nt = m.mTexCoord2s.size();
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

	bool hasTexCoords = Nt >= Nv;
	if(hasTexCoords){
		s <<
		"property float s\n"
		"property float t\n"
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

	if(Ni){
		s << "element face " << Ni/3 << "\n";
		// Annoyingly, some software like MeshLab does not support unsigned int, short, etc.
		s << "property list uchar " << (Bi==4?"uint":"ushort") << " vertex_indices\n";
	}

	s << "end_header\n";

	if(binary){
		// Vertex data
		for(unsigned i=0; i<Nv; ++i){
			s.write(reinterpret_cast<const char*>(&m.mVertices[i][0]), sizeof(Mesh::Vertex));
			if(hasNormals){
				for(int k=0; k<3; ++k){
					short v=m.mNormals[i][k]*32767.;
					s.write(reinterpret_cast<const char*>(&v), 2);
				}
			}
			if(hasTexCoords){
				s.write(reinterpret_cast<const char*>(&m.mTexCoord2s[i][0]), sizeof(Mesh::TexCoord2));
			}
			if(hasColors){
				auto col = Nci >= Nv ? m.mColoris[i] : Colori(m.mColors[i]);
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
			auto vrt = m.mVertices[i];
			s << vrt.x << " " << vrt.y << " " << vrt.z;
			if(hasNormals){
				auto nrm = m.mNormals[i];
				s << " " << nrm.x << " " << nrm.y << " " << nrm.z;
			}
			if(hasTexCoords){
				auto txc = m.mTexCoord2s[i];
				s << " " << txc.x << " " << txc.y;
			}
			if(hasColors){
				auto col = Nci >= Nv ? m.mColoris[i] : Colori(m.mColors[i]);
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

bool Mesh::saveSTL(const std::string& filePath, const std::string& solidName) const {

	if(!(isTriangles() || isTriangleStrip())){
		AL_WARN("Unsupported primitive type. Must be either triangles or triangle strip.");
		return false;
	}

	if(!mVertices.size()) return false;

	// Create a copy since we must convert to non-indexed triangles
	Mesh m(*this);

	// Convert mesh to non-indexed triangles
	m.toTriangles();
	m.decompress();
	m.generateNormals();

	// STL vertices must be in positive octant
	Vec3f vmin, vmax;
	m.bounds(vmin, vmax);

	std::ofstream s(filePath);
	if(s.fail()) return false;
	s.flags(std::ios::scientific);

	s << "solid " << solidName << "\n";
	for(int i=0; i<m.mVertices.size(); i+=3){
		s << "facet normal";
		for(int j=0; j<3; j++) s << " " << m.mNormals[i][j];
		s << "\n";
		s << "outer loop\n";
		for(int j=0; j<3; ++j){
			s << "vertex";
			for(int k=0; k<3; k++) s << " " << m.mVertices[i+j][k] - vmin[k];
			s << "\n";
		}
		s << "endloop\n";
		s << "endfacet\n";
	}
	s << "endsolid " << solidName;

	return true;
}

bool Mesh::saveSVG(const std::string& filePath, const SVGOptions& opt) const {
	if(!isLines() && !isLineStrip() && !isLineLoop() && !isPoints()) return false;

	std::ofstream fs;
	fs.open(filePath);
	if(fs.fail()) return false;

	int w = opt.width(), h = opt.height();

	fs << "<?xml version=\"1.0\"?>\n";
	//fs << "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 " << w << " " << h << "\">\n";
	fs << "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"" << -w/2 << " " << -h/2 << " " << w << " " << h << "\">\n";
	if(opt.title().size()) fs << "<title>" << opt.title() << "</title>\n";
	if(opt.desc().size() ) fs << "<desc>\n" << opt.desc() << "\n</desc>\n";

	auto toHexString = [](float v01){
		auto clamp = [](float v){ return v<0.f ? 0.f : (v>1.f ? 1.f : v); };
		unsigned char vbyte = clamp(v01)*255.f + 0.5f;
		char hex[2];
		for(int i=0; i<2; ++i)
			hex[i] = "0123456789ABCDEF"[(vbyte>>((1-i)*4))&0xF];
		return std::string(hex,2);
	};
	auto rgbToHexString = [&toHexString](float rgb[3]){
		return "#" + toHexString(rgb[0]) + toHexString(rgb[1]) + toHexString(rgb[2]);
	};

	// SVG typically has transparent background, but this is how we can color it:
	//fs << "<rect x=\"-50%\" y=\"-50%\" width=\"100%\" height=\"100%\" fill=\"#ff8888\"/>\n";

	RGB col(0);

	if(colors().size()) col = colors()[0].rgb();
	else if(coloris().size()) col = coloris()[0];

	std::string style = "style=\"fill:none";
	style += ";stroke-width:" + std::to_string(int((stroke()>0.?stroke():1.f)+0.5));
	style += ";stroke:" + rgbToHexString(col.components);
	style += ";stroke-linecap:round";
	style += ";stroke-linejoin:round";
	style += "\" ";

	// (0,0) is left-top (y is flipped) in SVG
	std::string xfm = "transform=\"";
	xfm += "scale(1,-1)";
	/* Probably right thing to do, but affects stroke-width
	xfm += "scale(" + al::toString(w/2) + "," + al::toString(-h/2) + ")";
	//*/
	xfm += "\" ";

	// projection plane
	const int e1 = 0;
	const int e2 = 1;

	Vec3f mn, mx;
	bounds(mn,mx);
	float vmul = w * 0.5 * opt.scale()/(max(mn.absVec(), mx.absVec()).get(e1,e2).max());

	auto encodePos = [&](Vec3f pos){
		auto xy = pos.get(e1,e2) * vmul;
		for(auto& v : xy){
			auto a = std::abs(v);
			if(a < 1e-10) v = 0.;
		}
		return xy;
	};

	// https://developer.mozilla.org/en-US/docs/Web/SVG/Attribute/d#path_commands
	fs << "<path " << style << xfm << "d=\"";	
	
	if(isLines()){
		Vec2f prevPos{1e38}; // to detect line strips
		forEachFace([&](int i1, int i2, int i3){
			auto v1 = encodePos(mVertices[i1]);
			auto v2 = encodePos(mVertices[i2]);
			if(v1 != prevPos){
				fs << "M" << v1.x << "," << v1.y << " ";
			}
			fs << "L" << v2.x << "," << v2.y << " ";
			prevPos = v2;
		});
	} else if(isLineStrip() || isLineLoop()){
		forEachFace([&](int i1, int i2, int i3){
			if(0==i1){
				auto v1 = encodePos(mVertices[i1]);
				fs << "M" << v1.x << "," << v1.y << " ";
			}
			auto v2 = encodePos(mVertices[i2]);
			fs << "L" << v2.x << "," << v2.y << " ";
		});
	} else if(isPoints()){
		forEachFace([&](int i1, int i2, int i3){
			auto v1 = encodePos(mVertices[i1]);
			fs << "M" << v1.x << "," << v1.y << " ";
			fs << "h0 "; // dup; points are degenerate lines
		});
	}

	fs << "\"/>\n"; // end path
	fs << "</svg>";

	return true;
}

std::string extension(const std::string& path){
	std::string ext;
	auto pos = path.find_last_of('.');
	if(pos != std::string::npos){
		ext = path.substr(pos+1);
		for(auto& c : ext) c = std::tolower(c);
	}
	return ext;
}

bool Mesh::save(const std::string& filePath, const std::string& solidName, bool binary) const {
	auto ext = extension(filePath);
	if("fbx" == ext)		return saveFBX(filePath, solidName);
	else if("ply" == ext)	return savePLY(filePath, solidName, binary);
	else if("stl" == ext)	return saveSTL(filePath, solidName);
	else if("svg" == ext)	return saveSVG(filePath, SVGOptions().title(solidName));
	return false;
}


std::string strip(const std::string& s, const char * stripChars="\r"){
	using namespace std;
	auto beg = s.find_first_not_of(stripChars);
	auto end = s.find_last_not_of(stripChars);
	if(string::npos != beg || string::npos != end){
		if(string::npos == beg) beg = 0;
		end = std::string::npos == end ? s.size() : end+1;
		return s.substr(beg, end-beg);
		//printf("%s\n", s.c_str());
	}
	return s;
}

// getline that strips given chars from ends of string
void getLineTrim(std::istream& is, std::string& s, const char * stripChars="\r"){
	getline(is, s);
	s = strip(s, stripChars);
}

void getTokens(std::vector<std::string>& tokens, const std::string& src, char delim=' ', bool eraseEmpty=true){
	tokens.clear();
	std::stringstream ss(src);
	std::string token;
	char delimStr[] = {delim, 0}; 
	while(getline(ss, token, delim)){
		//tokens.push_back(strip(token, delimStr));
		tokens.push_back(token);
	}
	if(eraseEmpty){
		tokens.erase(
			std::remove_if(tokens.begin(), tokens.end(), [](const std::string& s){ return s.empty(); }),
			tokens.end()
		);
	}
};


bool loadPLY(Mesh& mesh, std::istream& is){
	using namespace std;

	string buf;

	getLineTrim(is, buf);
	//printf("buf: %s\n", buf.c_str());
	if("ply" != buf) return false; // not a PLY file

	enum Enc{BINARY_LE, BINARY_BE, ASCII, UNKNOWN_ENC};
	enum Elem{VERTEX, FACE, EDGE, UNKNOWN_ELEM};
	enum Type{INT8=0, UINT8, INT16, UINT16, INT32, UINT32, FLOAT32, FLOAT64, UNKNOWN_TYPE};
	enum Attrib{PX,PY,PZ, NX,NY,NZ, CR,CG,CB,CA, TX,TY, UNKNOWN_ATTRIB};

	static int typeSize[] = { 1,1, 2,2, 4,4, 4, 8, 0 };

	static auto str2type = [](const std::string& s){
		if("char"==s || "int8"==s) return INT8;
		if("uchar"==s || "uint8"==s) return UINT8;
		if("short"==s || "int16"==s) return INT16;
		if("ushort"==s || "uint16"==s) return UINT16;
		if("int"==s || "int32"==s) return INT32;
		if("uint"==s || "uint32"==s) return UINT32;
		if("float"==s || "float32"==s) return FLOAT32;
		if("double"==s || "float64"==s) return FLOAT64;
		return UNKNOWN_TYPE;
	};

	static auto str2attrib = [](const std::string& s){
		switch(s[0]){
		case 'x': return PX;
		case 'y': return PY;
		case 'z': return PZ;
		case 's': case 'u': return TX;
		case 't': case 'v': return TY;
		default:
			if("nx"==s) return NX;
			if("ny"==s) return NY;
			if("nz"==s) return NZ;
			if("red"==s) return CR;
			if("green"==s) return CG;
			if("blue"==s) return CB;
			if("alpha"==s) return CA;
		}
		return UNKNOWN_ATTRIB;
	};

	struct AttribDef{
		Type type;
		Attrib attrib;
	};

	struct ListDef{
		Type countType;
		Type valueType;
	};

	struct PropDef{
		enum PropType{ATTRIB, LIST};
		union{
			AttribDef attribDef;
			ListDef listDef;
		};
		PropType type;

		PropDef(const AttribDef& v){ attribDef=v; type=ATTRIB; }
		PropDef(const ListDef& v){ listDef=v; type=LIST; }
		unsigned size() const { return type==ATTRIB ? typeSize[attribDef.type] : typeSize[listDef.countType] + typeSize[listDef.valueType]; }
	};

	struct ElemDef{
		ElemDef(int cnt=0): count(cnt){}
		int count;
		std::vector<PropDef> propDefs;

		unsigned size() const {
			unsigned n=0;
			for(const auto& p : propDefs) n += p.size();
			return n*count;
		}
		ElemDef& addAttrib(Type t, Attrib a){ propDefs.push_back(AttribDef{t,a}); return *this; }
		ElemDef& addAttrib(const std::string& t, const std::string& a){ return addAttrib(str2type(t), str2attrib(a)); }
		ElemDef& addList(Type c, Type v){ propDefs.push_back(ListDef{c,v}); return *this; }
		ElemDef& addList(const std::string& c, const std::string& v){ return addList(str2type(c), str2type(v)); }
	};

	Enc dataEnc = UNKNOWN_ENC;
	Elem currElem = UNKNOWN_ELEM;
	ElemDef vertElem;
	ElemDef faceElem;
	vector<ElemDef> otherElems; // unsupported data following verts and faces

	vector<string> tokens;
	auto getLineTokens = [&](){
		getLineTrim(is, buf);
		getTokens(tokens, buf);
	};

	// Get header info
	while(is){
		getLineTokens();

		const auto& tag = tokens[0];
		//printf("%s\n", tag.c_str());
		if("format" == tag && tokens.size()>=2){
			if("binary_little_endian" == tokens[1]){
				dataEnc = BINARY_LE;
			} else if("binary_big_endian" == tokens[1]){
				dataEnc = BINARY_BE;
			} else if("ascii" == tokens[1]){
				dataEnc = ASCII;
			}
		} else if("element" == tag && tokens.size()>=3){
			unsigned count = std::stoi(tokens[2]);
			//printf("%d %s elements\n", count, tokens[1].c_str());
			if("vertex" == tokens[1]){ currElem = VERTEX; vertElem.count = count; }
			else if("face" == tokens[1]){ currElem = FACE; faceElem.count = count; }
			else{ currElem = UNKNOWN_ELEM; otherElems.emplace_back(count); }
		} else if("property" == tag && tokens.size()>=3){
			switch(currElem){
			case VERTEX:
				vertElem.addAttrib(tokens[1], tokens[2]);
				break;
			case FACE:
				if("list"==tokens[1] && tokens.size()>=4){
					faceElem.addList(tokens[2], tokens[3]);
				}
				break;
			case EDGE: break;
			case UNKNOWN_ELEM:
				if("list"==tokens[1] && tokens.size()>=4){
					otherElems.back().addList(tokens[2], tokens[3]);
				} else {
					otherElems.back().addAttrib(tokens[1], tokens[2]);
				}
			}
		} else if("end_header" == tag){
			break;
		}
	}

	//for(auto& v : vertElem.propDefs) printf("%d %d\n", v.attribDef.type, v.attribDef.attrib);
	//for(auto& v : faceElem.propDefs) printf("%d %d\n", v.listDef.countType, v.listDef.valueType);

	if(dataEnc == UNKNOWN_ENC) return false;

	int bigEndian = 1;
	if(1 == *(char *)&bigEndian) bigEndian = 0;
	auto machineEndian = bigEndian ? BINARY_BE : BINARY_LE;

	auto toDouble = [&](const char * buf, Type type){
		double v = 0.;
		switch(type){
		case INT8:    v = *buf; break;
		case UINT8:   v = *(unsigned char*)(buf); break;
		case INT16:   v = *(short*)(buf); break;
		case UINT16:  v = *(unsigned short*)(buf); break;
		case INT32:   v = *(int*)(buf); break;
		case UINT32:  v = *(unsigned int*)(buf); break;
		case FLOAT32: v = *(float*)(buf); break;
		case FLOAT64: v = *(double*)(buf); break;
		default:;
		}
		return v;
	};

	auto normalize = [](double v, Type type){
		switch(type){
		case INT8:   v /= 127.; break;
		case UINT8:  v /= 255.; break;
		case INT16:  v /= 32767.; break;
		case UINT16: v /= 65535.; break;
		case INT32:  v /= 2147483647.; break;
		case UINT32: v /= 4294967295.; break;
		default:;
		}
		return v;
	};

	bool ascii = (dataEnc == ASCII);

	auto readNextVal = [&](Type type){
		char b[8]; auto Nbytes = typeSize[type];
		is.read(b, Nbytes);
		if(machineEndian != dataEnc){
			for(int i=0; i<Nbytes/2; ++i) std::swap(b[i],b[Nbytes-1-i]);
		}
		return toDouble(b, type);
	};

	mesh.reset();

	for(int k=0; k<vertElem.count; ++k){
		bool Bp=false, Bn=false, Bc=false, Bt1=false, Bt2=false;
		Vec3f pos, nrm;
		Color col;
		Vec2f tex;
		if(ascii) getLineTokens();
		int i=0;
		for(auto& p : vertElem.propDefs){
			if(p.type == PropDef::ATTRIB){
				const auto& attrib = p.attribDef;
				double v = ascii ? std::stod(tokens[i++]) : readNextVal(attrib.type);
				v = normalize(v, attrib.type);
				switch(attrib.attrib){
				case PX: Bp=true; pos[0]=v; break;
				case PY: Bp=true; pos[1]=v; break;
				case PZ: Bp=true; pos[2]=v; break;
				case NX: Bn=true; nrm[0]=v; break;
				case NY: Bn=true; nrm[1]=v; break;
				case NZ: Bn=true; nrm[2]=v; break;
				case CR: Bc=true; col[0]=v; break;
				case CG: Bc=true; col[1]=v; break;
				case CB: Bc=true; col[2]=v; break;
				case CA: Bc=true; col[3]=v; break;
				case TX: Bt1=true;tex[0]=v; break;
				case TY: Bt2=true;tex[1]=v; break;
				default:;
				}
			} else { // skip over lists
				if(ascii){
				} else {
					is.seekg(p.size(), ios_base::cur);
				}
			}
		}
		//printf("pos: %g %g %g\n", pos.x, pos.y, pos.z);
		//printf("col: %g %g %g\n", col.r, col.g, col.b);
		if(Bp) mesh.vertex(pos);
		if(Bn) mesh.normal(nrm);
		if(Bc) mesh.color(col);
		if(Bt2) mesh.texCoord(tex[0], tex[1]);
		else if(Bt1) mesh.texCoord(tex[0]);
	}

	std::vector<int> face;

	for(int i=0; i<faceElem.count; ++i){
		if(ascii) getLineTokens();
		for(auto& p : faceElem.propDefs){
			if(p.type == PropDef::LIST){
				const auto& list = p.listDef;
				unsigned count = ascii ? std::stod(tokens[0]) : readNextVal(list.countType);
				//printf("count:%u\n", count);
				if(count>=3){ // tri, quad or other poly
					mesh.triangles();
					face.clear();
					for(unsigned i=0; i<count; ++i)
						face.push_back(ascii ? std::stod(tokens[i+1]) : readNextVal(list.valueType));
					//bool good = true;
					//for(auto i:face){ if(i<0 || i>=vertElem.count) good=false; } // bad index
					//for(auto i:face) printf("%d ",i); printf("\n");

					for(unsigned i=1; i<count-1; ++i){ // add tri fan
						mesh.index(face[0]);
						mesh.index(face[i]);
						mesh.index(face[i+1]);
					}
				}
			} else { // skip over attribs
				if(ascii){
				} else {
					is.seekg(p.size(), ios_base::cur);
				}
			}
		}
	}

	return true;
}


bool loadOBJ(Mesh& mesh, std::istream& is){
	using namespace std;

	mesh.reset();
	string buf;
	vector<string> tokens;
	vector<string> faceTokens;

	struct Index{ int p=-1, t=-1, n=-1; };

	// temp buffers
	std::vector<Index> face; // face indices (of convex polygon)
	std::vector<Vec3f> Ps;
	std::vector<Vec2f> Ts;
	std::vector<Vec3f> Ns;
	char attr = 0; // 'p', 'n', 't', 'f'

	while(is){
		getLineTrim(is, buf, " \r\t");
		if(buf.empty() || '#'==buf[0]) continue;
		//printf("%s\n", buf.c_str());
		getTokens(tokens, buf);
		if(tokens.empty()) continue;
		//for(auto& s : tokens){ printf("%s,", s.c_str()); } printf("\n");

		if("v"==tokens[0] && tokens.size()>=4){
			if('p' != attr){
				//TODO: new mesh started
			}
			attr = 'p';
			auto x = std::stod(tokens[1]);
			auto y = std::stod(tokens[2]);
			auto z = std::stod(tokens[3]);
			Ps.emplace_back(x,y,z);
		} else if("vt"==tokens[0] && tokens.size()>=3){
			attr = 't';
			auto u = std::stod(tokens[1]);
			auto v = std::stod(tokens[2]);
			Ts.emplace_back(u,v);
		}  else if("vn"==tokens[0] && tokens.size()>=4){
			attr = 'n';
			auto nx = std::stod(tokens[1]);
			auto ny = std::stod(tokens[2]);
			auto nz = std::stod(tokens[3]);
			Ns.emplace_back(nx,ny,nz);
		} else if("f"==tokens[0]){
			attr = 'f';
			auto numVerts = tokens.size()-1;
			face.clear();
			if(numVerts>=3){ // tris, quads and other polygons
				for(int j=1; j<tokens.size(); ++j){
					getTokens(faceTokens, tokens[j], '/', false);
					// face types: v, v/t, v/t/n, v//n
					Index idx;
					idx.p = std::stoi(faceTokens[0]) - 1;
					if(faceTokens.size()>=2 && !faceTokens[1].empty()){
						auto t = std::stoi(faceTokens[1]) - 1;
						if(t < Ts.size()) idx.t = t;
					}
					if(faceTokens.size()>=3 && !faceTokens[2].empty()){
						auto n = std::stoi(faceTokens[2]) - 1;
						if(n < Ns.size()) idx.n = n;
					}
					face.push_back(idx);
				}
				// add triangles as tri fan
				mesh.triangles();
				for(int i=1; i<face.size()-1; ++i){
					auto i0 = face[0];
					auto i1 = face[i];
					auto i2 = face[i+1];
					mesh.vertex(Ps[i0.p]);
					mesh.vertex(Ps[i1.p]);
					mesh.vertex(Ps[i2.p]);
					if(i0.n>=0 && i1.n>=0 && i2.n>=0){
						mesh.normal(Ns[i0.n]);
						mesh.normal(Ns[i1.n]);
						mesh.normal(Ns[i2.n]);
					}
					if(i0.t>=0 && i1.t>=0 && i2.t>=0){
						mesh.texCoord(Ts[i0.t]);
						mesh.texCoord(Ts[i1.t]);
						mesh.texCoord(Ts[i2.t]);
					}
					// We can't easily use indices since attribute arrays can have different lengths
					//mesh.index(i0.p, i1.p, i2.p);
				}
			}
		} else if("g"==tokens[0]){
			// object name
		}
	}

	return true;
}


bool Mesh::load(const std::string& filePath){
	std::filebuf fb;
	if(!fb.open(filePath, std::ios::in | std::ios::binary)) return false;
	std::istream is(&fb);
	auto ext = extension(filePath);
	bool res = false;
	if("ply" == ext)		res = al::loadPLY(*this, is);
	else if("obj" == ext)	res = al::loadOBJ(*this, is);
	if(res){
		if(mAttribHint & Mesh::NORMAL) ensureNormals();
	}
	return res;
}

// Must subclass streambuf for reading raw bytes
struct imembuf : public std::streambuf {
	imembuf(const void * data, int len){
		auto * ptr = static_cast<char *>(const_cast<void *>(data));
		setg(ptr, ptr, ptr+len);
	}
};

#define LOAD_MEM(TYPE)\
bool Mesh::load##TYPE(const void * data, int numBytes){\
	imembuf buf(data, numBytes);\
	std::istream is(&buf);\
	return al::load##TYPE(*this, is);\
}

LOAD_MEM(OBJ)
LOAD_MEM(PLY)


template <class Array>
int sizeBytes(const Array& a){
	return a.size() * sizeof(typename Array::value_type);
}

void Mesh::print(FILE * dst) const {

	fprintf(dst, "Mesh %p (prim = %d) has:\n", this, mPrimitive);

	auto niceByteString = [](unsigned bytes, double& printVal){
		if(bytes <    1000){ printVal=bytes; return "B"; }
		if(bytes < 1000000){ printVal=bytes*1e-3; return "kB"; }
		else               { printVal=bytes*1e-6; return "MB"; }
	};

	#define QUERY_ATTRIB(attrib, name)\
	if(attrib.size()){\
		auto B = sizeBytes(attrib);\
		Btot += B;\
		double byteVal;\
		const char * byteUnits = niceByteString(B, byteVal);\
		fprintf(dst, "%8d " #name " (%.1f %s)\n", (int)attrib.size(), byteVal, byteUnits);\
	}

	unsigned Btot = 0;
	QUERY_ATTRIB(mVertices, Vertices)
	QUERY_ATTRIB(mColors, Colors)
	QUERY_ATTRIB(mColoris, Coloris)
	QUERY_ATTRIB(mNormals, Normals)
	QUERY_ATTRIB(mTangents, Tangents)
	QUERY_ATTRIB(mTexCoord1s, TexCoord1s)
	QUERY_ATTRIB(mTexCoord2s, TexCoord2s)
	QUERY_ATTRIB(mTexCoord3s, TexCoord3s)
	QUERY_ATTRIB(mIndices, Indices)

	{
		double byteVal;
		const char * byteUnits = niceByteString(Btot, byteVal);
		fprintf(dst, "%8d bytes (%.1f %s)\n", Btot, byteVal, byteUnits);
	}
}

bool Mesh::debug(FILE * dst) const {

	bool ok = true;
	#define ALERT(...) if(dst){ fprintf(dst, __VA_ARGS__); ok=false; }

	const int Nv = mVertices.size();
	if(!Nv){ ALERT("No vertices\n"); }

	const int Ni = mIndices.size();

	#define CHECK_ARR(arr, oneOkay)\
	if(arr.size() && arr.size() != Nv){\
		if(!oneOkay || arr.size()!=1){\
			ALERT("%d " #arr ", but %d vertices\n", (int)arr.size(), Nv);\
		}\
	}
	CHECK_ARR(mNormals, false);
	CHECK_ARR(mTangents, false);
	CHECK_ARR(mColors, true);
	CHECK_ARR(mColoris, true);
	CHECK_ARR(mTexCoord1s, false);
	CHECK_ARR(mTexCoord2s, false);
	CHECK_ARR(mTexCoord3s, false);
	#undef CHECK_ARR

	// Check for clashing attributes
	if(mColors.size() && mColoris.size()){
		ALERT("More than one color array populated\n");
	}
	{
		int Nt1 = mTexCoord1s.size();
		int Nt2 = mTexCoord2s.size();
		int Nt3 = mTexCoord3s.size();
		int n = (Nt1?1:0) + (Nt2?1:0) + (Nt3?1:0);
		if(n > 1){
			ALERT("More than one tex coord array populated: %d %d %d\n", Nt1, Nt2, Nt3);
		}
	}

	// Check integrity of any indices
	if(Ni){
		for(auto i : mIndices){
			if(i >= Nv){
				ALERT("Index out of bounds: %d (must be in [0,%d))\n", i, Nv);
				break;
			}
		}
		// Check for unused verts? This could be intentional for LOD, multi-meshes, etc.
	}

	// Misc primitive specific checks
	if(isPoints()){
		if(Ni){
			ALERT("Using indexed points: did you forget to set the primitive?\n");
		}
	} else if(isLines()){
		auto extra = (Ni ? Ni : Nv) % 2;
		if(extra){
			ALERT("%s count not multiple of 2 for lines\n", Ni ? "Index" : "Vertex");
		}
	} else if(isTriangles()){
		auto extra = (Ni ? Ni : Nv) % 3;
		if(extra){
			ALERT("%s count not multiple of 3 for triangles (%d extra)\n", Ni ? "Index" : "Vertex", extra);
		}
	}

	// Geometric checks
	#define CHECK_UNIT_VEC(arr)\
	if(arr.size()){\
		static constexpr float eps = 0.001;\
		static constexpr auto lo = (1.-eps)*(1.-eps);\
		static constexpr auto hi = (1.+eps)*(1.+eps);\
		for(int i=0; i<arr.size(); ++i){\
			auto mm = arr[i].magSqr();\
			if(mm < lo || mm > hi){\
				ALERT(#arr " vector not unit magnitude: %d is %f\n", i, std::sqrt(mm));\
				break;\
			}\
		}\
	}
	CHECK_UNIT_VEC(mNormals)
	CHECK_UNIT_VEC(mTangents)
	#undef CHECK_UNIT_VEC

	if(mNormals.size() == mTangents.size()){
		for(int i=0; i<mNormals.size(); ++i){
			auto d = std::abs(dot(mNormals[i], mTangents[i]));
			if(d > 0.001){
				ALERT("Normal and tangent not orthogonal: %d\n", i);
				break;
			}
		}
	}

	#undef DPRINTF

	if(dst) fflush(dst);

	return ok;
}

} // al::
