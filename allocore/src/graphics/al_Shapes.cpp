#include <map>
#include <cmath>
#include <cstdint> // uint64_t
#include "allocore/math/al_Constants.hpp"
#include "allocore/graphics/al_Mesh.hpp"
#include "allocore/graphics/al_Shapes.hpp"

/*
Platonic solids code derived from:
Bourke, P. (1993). "Platonic Solids (Regular polytopes in 3D)",
Accessed from http://paulbourke.net/geometry/platonic/.
*/

namespace al{

const double phi = (1 + sqrt(5))/2; // the golden ratio


// Complex sinusoid used for fast circle generation
struct CSin{
	CSin(double freq, double amp=1.)
	:	r(amp), i(0.), dr(cos(freq)), di(sin(freq)){}
	void operator()(){
		double r_ = r*dr - i*di;
		i = r*di + i*dr;
		r = r_;
	}
	void ampPhase(double amp, double phs){
		r = amp*cos(phs);
		i = amp*sin(phs);
	}
	double r,i,dr,di;
};


// Scale last N vertices
static void scaleVerts(Mesh& m, float radius, int N){
	if(radius != 1.f){
		int Ne = m.vertices().size();
		int Nb = Ne - N;
		for(int i=Nb; i<Ne; ++i){
			m.vertices()[i] *= radius;
		}
	}
}

// Temporarily disable attribute hint(s)
template <class Func>
static void noAttribScope(Mesh& m, Mesh::Attrib attribs, const Func& f){
	auto a = m.attribHint();
	m.attribHint(a ^ attribs);
	f();
	m.attribHint(a);
}

int addCuboid(Mesh& m, float rx, float ry, float rz){

	m.triangles();

	/* 2 3      y  
	   0 1      |
	6 7         /--x
	4 5       z         */

	Mesh::Vertex v[8] = {
		{-rx,-ry,-rz}, { rx,-ry,-rz}, {-rx, ry,-rz}, { rx, ry,-rz},
		{-rx,-ry, rz}, { rx,-ry, rz}, {-rx, ry, rz}, { rx, ry, rz}
	};

	if(m.wants(Mesh::NORMAL | Mesh::TANGENT | Mesh::TEXCOORD)){
		addQuad(m, v[2],v[0],v[4],v[6]); // left
		addQuad(m, v[1],v[3],v[7],v[5]); // right
		addQuad(m, v[3],v[2],v[6],v[7]); // back
		addQuad(m, v[0],v[1],v[5],v[4]); // front
		addQuad(m, v[2],v[3],v[1],v[0]); // bottom
		addQuad(m, v[4],v[5],v[7],v[6]); // top
		return 4*6;
	}

	m.indexRel(
		2,0,4, 2,4,6, // left
		1,3,7, 1,7,5, // right
		3,2,6, 3,6,7, // back
		0,1,5, 0,5,4, // front
		2,3,1, 2,1,0, // bottom
		4,5,7, 4,7,6  // top
	);

	m.vertex(v, 8);

	return 8;
}


int addTetrahedron(Mesh& m, float radius){

	m.triangles();

	static const float l = sqrt(1./3);
	static const float vertices[] = {
		 l, l, l,
		-l, l,-l,
		 l,-l,-l,
		-l,-l, l
	};

	static const int indices[] = {0,2,1, 0,1,3, 1,2,3, 2,0,3};

	int Nv = sizeof(vertices)/sizeof(*vertices)/3;

	m.vertex(vertices, Nv);
	m.index(indices, sizeof(indices)/sizeof(*indices), m.vertices().size()-Nv);

	scaleVerts(m, radius, Nv);

	return Nv;
}


int addOctahedron(Mesh& m, float radius){

	m.triangles();

	static const float vertices[] = {
		 1,0,0, 0, 1,0, 0,0, 1,	// 0 1 2
		-1,0,0, 0,-1,0, 0,0,-1	// 3 4 5
	};

	static const int indices[] = {
		0,1,2, 1,3,2, 3,4,2, 4,0,2,
		1,0,5, 3,1,5, 4,3,5, 0,4,5
	};

	int Nv = sizeof(vertices)/sizeof(*vertices)/3;

	m.vertex(vertices, Nv);
	m.index(indices, sizeof(indices)/sizeof(*indices), m.vertices().size()-Nv);

	scaleVerts(m, radius, Nv);

	return Nv;
}


int addDodecahedron(Mesh& m, float radius){

	m.triangles();

	static const float b = sqrt(1./3);
	static const float a = (phi-1)*b;
	static const float c = sqrt(1-a*a);

	static const float vertices[] = {
		-b,-b, b,	 c, a, 0,	 c,-a, 0,	-c, a, 0,	-c,-a, 0,
		 0, c, a,	 0, c,-a,	 a, 0,-c,	-a, 0,-c,	 0,-c,-a,
		 0,-c, a,	 a, 0, c,	-a, 0, c,	 b, b,-b,	 b, b, b,
		-b, b,-b,	-b, b, b,	 b,-b,-b,	 b,-b, b,	-b,-b,-b
	};

	static const int indices[] = {
		18, 2, 1,	11,18, 1,	14,11, 1,
		 7,13, 1,	17, 7, 1,	 2,17, 1,
		19, 4, 3,	 8,19, 3,	15, 8, 3,
		12,16, 3,	 0,12, 3,	 4, 0, 3,
		 6,15, 3,	 5, 6, 3,	16, 5, 3,
		 5,14, 1,	 6, 5, 1,	13, 6, 1,
		 9,17, 2,	10, 9, 2,	18,10, 2,
		10, 0, 4,	 9,10, 4,	19, 9, 4,
		19, 8, 7,	 9,19, 7,	17, 9, 7,
		 8,15, 6,	 7, 8, 6,	13, 7, 6,
		11,14, 5,	12,11, 5,	16,12, 5,
		12, 0,10,	11,12,10,	18,11,10
	};

	int Nv = sizeof(vertices)/sizeof(*vertices)/3;

	m.vertex(vertices, Nv);
	m.index(indices, sizeof(indices)/sizeof(*indices), m.vertices().size()-Nv);

	scaleVerts(m, radius, Nv);

	return Nv;
}


int addIcosahedron(Mesh& m, float radius){

	m.triangles();

	static const float a = (0.5) / 0.587785;
	static const float b = (1. / (2 * phi)) / 0.587785;
	//printf("%f\n", sqrt(a*a + b*b));

	static const float vertices[] = {
		 0, b,-a,	 b, a, 0,	-b, a, 0,	//  0  1  2
		 0, b, a,	 0,-b, a,	-a, 0, b,	//  3  4  5
		 a, 0, b,	 0,-b,-a,	 a, 0,-b,	//  6  7  8
		-a, 0,-b,	 b,-a, 0,	-b,-a, 0	//  9 10 11
	};

	static const int indices[] = {
		 1, 0, 2,	 2, 3, 1,	 4, 3, 5,	 6, 3, 4,
		 7, 0, 8,	 9, 0, 7,	10, 4,11,	11, 7,10,
		 5, 2, 9,	 9,11, 5,	 8, 1, 6,	 6,10, 8,
		 5, 3, 2,	 1, 3, 6,	 2, 0, 9,	 8, 0, 1,
		 9, 7,11,	10, 7, 8,	11, 4, 5,	 6, 4,10
	};

	int Nv = sizeof(vertices)/sizeof(*vertices)/3;

	m.vertex(vertices, Nv);
	m.index(indices, sizeof(indices)/sizeof(*indices), m.vertices().size()-Nv);

	scaleVerts(m, radius, Nv);

	return Nv;
}


// Subdivides triangles

// This function subdivides each triangle in the mesh into four new triangles
// formed from the vertices and edge midpoints of the original triangle.
// TODO: add as method to Mesh?
void subdivide(Mesh& m, unsigned iterations, bool normalize){

	typedef std::map<uint64_t, unsigned> PointToIndex;

	if(!m.isTriangles()) return;

	for(unsigned k=0; k<iterations; ++k){

		PointToIndex middlePointIndexCache;

		Mesh::Index newIndex = m.vertices().size();
		Mesh::Indices oldIndices(m.indices());
		m.indices().reset();

		// Iterate through triangles
		for(unsigned j=0; j<(unsigned)oldIndices.size(); j+=3){

			//printf("%u %u\n", k, j);

			Mesh::Index * corner = &oldIndices[j];
			Mesh::Index mid[3];

			for(unsigned i=0; i<3; ++i){
				uint64_t i1 = corner[ i     ];
				uint64_t i2 = corner[(i+1)%3];
				uint64_t key = i1 < i2 ? (i1<<32) | i2 : (i2<<32) | i1;

				PointToIndex::iterator it = middlePointIndexCache.find(key);
				if(it != middlePointIndexCache.end()){
					mid[i] = it->second;
				}
				else{
					middlePointIndexCache.insert(std::make_pair(key, newIndex));
					auto v1 = m.vertices()[i1];
					auto v2 = m.vertices()[i2];
					decltype(v1) vm;
					if(normalize){
						vm = v1 + v2;
						//vm.normalize();
						// use average magnitude to keep smooth
						vm.normalize((v1.mag()+v2.mag())*0.5);
					}
					else{
						vm = (v1 + v2)*0.5;	
					}
					m.vertex(vm);
					// TODO: other attributes (colors, normals, etc.)
					mid[i] = newIndex;
					++newIndex;
				}
			}

			Mesh::Index newIndices[] = {
				corner[0], mid[0], mid[2],
				corner[1], mid[1], mid[0],
				corner[2], mid[2], mid[1],
				mid[0], mid[1], mid[2]
			};

			m.index(newIndices, 12);
		}
	}
}

int addIcosphere(Mesh& m, double radius, int divisions){
	int Nv = m.vertices().size();
	addIcosahedron(m, radius);
	subdivide(m, divisions, true);
	if(m.wants(Mesh::NORMAL) && m.normals().size() < m.vertices().size()){
		auto scale = radius>0. ? 1./radius : 1.;
		for(int i=Nv; i<m.vertices().size(); ++i){
			m.normal(m.vertices()[i]*scale);
		}
	}
	return m.vertices().size() - Nv;
}


// Stacks are circles cut perpendicular to the z axis while slices are circles
// cut through the z axis.
// The top is (0,0,radius) and the bottom is (0,0,-radius).
int addSphere(Mesh& m, double radius, int slices, int stacks){

	int Nv = m.vertices().size();

	CSin P( M_PI/stacks, radius);
	CSin T(M_2PI/slices);

	if(!m.wants(Mesh::TEXCOORD)){
		m.triangles();

		// Add top cap (as triangle fan)
		m.vertex(0,0,radius);
		for(int i=0; i<slices; ++i){
			m.index(Nv+1 + i);
			m.index(Nv+1 + ((i+1)%slices));
			m.index(Nv); // N pole
		}

		P(); // increment since we added point at N pole

		// Add rings
		for(int j=0; j<stacks-2; ++j){
			int jp1 = j+1;

			for(int i=0; i<slices; ++i){
				int ip1 = (i+1)%slices;

				int i00 = Nv+1 + j  *slices + i;
				int i10 = Nv+1 + j  *slices + ip1;
				int i01 = Nv+1 + jp1*slices + i;
				int i11 = Nv+1 + jp1*slices + ip1;

				m.vertex(T.r*P.i, T.i*P.i, P.r);
				m.index(i00);
				m.index(i01);
				m.index(i10);
				m.index(i10);
				m.index(i01);
				m.index(i11);
				T(); // rotate one step around pole
			}

			P(); // rotate one step from N to S pole
		}

		// Add bottom ring and cap
		int icap = m.vertices().size() + slices;
		for(int i=0; i<slices; ++i){
			m.vertex(T.r*P.i, T.i*P.i, P.r);
			m.index(icap - slices + ((i+1)%slices));
			m.index(icap - slices + i);
			m.index(icap);
			T();
		}
		m.vertex(0,0,-radius);

	} else { // for texturing: edges must have duplicate vertices
		noAttribScope(m, Mesh::NORMAL | Mesh::TANGENT, [&](){
			addSurface(m, slices+1,stacks+1, 1,1, 0.5,0.5);
		});

		for(int i=Nv; i<m.vertices().size(); ++i){
			auto& pos = m.vertices()[i];
			/* slow way
			auto t =     pos.x  * 2. * 355./113.;
			auto p = (1.-pos.y) * 1. * 355./113.; // go S to N pole
			float c1 = std::cos(t), s1 = std::sin(t);
			float c2 = radius * std::cos(p), s2 = radius * std::sin(p);
			pos.set(s2*c1, s2*s1, c2);
			//*/
			//* fast way
			pos.set(T.r*P.i, T.i*P.i, -P.r); // go S to N pole
			int j = i - Nv;
			if((j % (slices+1)) < slices) T();
			else P();
			//*/
		}
	}

	if(m.wants(Mesh::NORMAL)){
		float s = radius!=0. ? 1.f/radius : 0.f;
		for(int i=Nv; i<m.vertices().size(); ++i)
			m.normal(m.vertices()[i] * s);

		if(m.wants(Mesh::TANGENT)){
			for(int i=Nv; i<m.vertices().size(); ++i){
				auto B = m.vertices()[i].with<2>(0.f).normalize().rotate90();
				m.tangent(cross(m.normals()[i], B));
			}
		}
	}

	return m.vertices().size()-Nv;
}


int addWireBox(Mesh& m, const Vec3f& l, const Vec3f& h){
	m.lines();

	int Nv = m.vertices().size();

	/*		6 7
			4 5
		2 3
		0 1			*/

	m.vertex(l[0], l[1], l[2]);
	m.vertex(h[0], l[1], l[2]);
	m.vertex(l[0], h[1], l[2]);
	m.vertex(h[0], h[1], l[2]);
	m.vertex(l[0], l[1], h[2]);
	m.vertex(h[0], l[1], h[2]);
	m.vertex(l[0], h[1], h[2]);
	m.vertex(h[0], h[1], h[2]);

	static const int I[] = {
		0,1, 2,3, 4,5, 6,7,
		0,2, 1,3, 4,6, 5,7,
		0,4, 1,5, 2,6, 3,7
	};

	m.index(I, sizeof(I)/sizeof(*I), Nv);

	return m.vertices().size() - Nv;
}


int addWireBox(Mesh& m, float w, float h, float d){
	Vec3f p(w, h, d);
	p *= 0.5f;
	return addWireBox(m, -p,p);
}


int addCone(Mesh& m, float radius, const Vec3f& apex, unsigned slices, unsigned stacks, unsigned cycles){

	m.triangles();

	unsigned Nv = m.vertices().size();

	// Note: leaving base on xy plane makes it easy to construct a bicone
	m.vertex(apex);

	if(m.wants(Mesh::NORMAL)){
		m.normal(Vec3f(0,0,apex.z>=0.f ? 1.f : -1.f));
	}
	if(m.wants(Mesh::TEXCOORD)){
		m.texCoord(0.5, 0.5);
	}

	for(unsigned i=0; i<stacks; ++i){

		float h = float(i+1)/stacks;

		CSin csin(cycles * 2*M_PI/slices, h);

		for(unsigned j=0; j<slices; ++j){
			float x = csin.r;
			float y = csin.i;
			csin();

			bool lastSlice = j == (slices - 1);

			if(0 == i){ // first stack
				m.index(Nv);
				int v2 = 0;
				int v3 = 1; if(lastSlice) v3-=slices;
				m.indexRel(v2, v3);
			} else {
				int v1 = -int(slices);
				int v2 = 0;
				int v3 = v1+1;
				int v4 = 1;
				if(lastSlice){ v3-=slices; v4-=slices; }
				m.indexRel(v1,v2,v3, v3,v2,v4);
			}

			auto pos = Vec3f(x,y,0.)*radius + apex*(1.-h);

			m.vertex(pos);

			if(m.wants(Mesh::NORMAL)){
				auto t1 = pos - apex;
				auto t2 = Vec3f(-y,x,0.f);
				auto N = cross(t1,t2).dir();
				m.normal(N);
			}
			if(m.wants(Mesh::TEXCOORD)){
				m.texCoord(x*0.5f+0.5f, y*0.5f+0.5f);
			}
		}
	}

	return 1 + slices*stacks;
}

int addDisc(Mesh& m, float radius, unsigned slices, unsigned stacks){
	return addCone(m, radius, Vec3f(0,0,0), slices, stacks);
}

int addEllipse(Mesh& m, float radx, float rady, int N){
	m.lines();
	for(int i=0; i<N; ++i) m.indexRel(i, (i+1)%N);
	m.vertices().resize(m.vertices().size() + N);
	ellipse(&m.vertices().last() - N + 1, N, radx, rady);
	return N;
}

int addCircle(Mesh& m, float radius, int N){
	return addEllipse(m, radius, radius, N);
}

int addRect(Mesh& m, float width, float height, float x, float y, float z){
	float w_2 = width*0.5, h_2 = height*0.5;
	return addQuad(m,
		x-w_2, y-h_2, z,
		x+w_2, y-h_2, z,
		x+w_2, y+h_2, z,
		x-w_2, y+h_2, z
	);
}

int addFrame(Mesh& m, float w, float h, float x, float y, float z){
	m.lines();
	float l = x - w*0.5, r = x + w*0.5;
	float b = y - h*0.5, t = y + h*0.5;
	m.indexRel(0,1, 1,2, 2,3, 3,0);
	m.vertex(l,b,z);
	m.vertex(r,b,z);
	m.vertex(r,t,z);
	m.vertex(l,t,z);
	return 4;
}

int addQuad(Mesh& m,
	float x1, float y1, float z1,
	float x2, float y2, float z2,
	float x3, float y3, float z3,
	float x4, float y4, float z4
){
	m.triangles();
	m.indexRel(0,1,3, 3,1,2);
	Mesh::Vertex a(x1,y1,z1), b(x2,y2,z2), c(x3,y3,z3), d(x4,y4,z4);
	m.vertex(a).vertex(b).vertex(c).vertex(d);
	if(m.wants(Mesh::NORMAL)){
		auto N = cross(b-a, d-a).dir();
		m.normalFill(N);
	}
	if(m.wants(Mesh::TANGENT)){
		auto T = (d-a).dir();
		m.tangentFill(T);
	}
	if(m.wants(Mesh::TEXCOORD)){
		m.texCoord(0,0).texCoord(1,0).texCoord(1,1).texCoord(0,1);
	}
	return 4;
}

#define Spec_addWireGrid(Dim1,Dim2)\
template<>\
int addWireGrid<Dim1,Dim2>(Mesh& m, int n1, int n2, Vec2f radii, Vec2f center){\
	m.lines();\
	auto mn = center - radii;\
	auto mx = center + radii;\
	for(int i=0; i<n1+1; ++i){\
		float x = (float(i)/n1*2.-1.)*radii[0] + center[0];\
		m.vertex(Vec3f().template set<Dim1>(x).template set<Dim2>(mn[1]));\
		m.vertex(Vec3f().template set<Dim1>(x).template set<Dim2>(mx[1]));\
	}\
	for(int i=0; i<n2+1; ++i){\
		float y = (float(i)/n2*2.-1.)*radii[1] + center[1];\
		m.vertex(Vec3f().template set<Dim2>(y).template set<Dim1>(mn[0]));\
		m.vertex(Vec3f().template set<Dim2>(y).template set<Dim1>(mx[0]));\
	}\
	return (n1+1)*2 + (n2+1)*2;\
}

Spec_addWireGrid(0,1)
Spec_addWireGrid(1,2)
Spec_addWireGrid(2,1)

int addPrismOpen(Mesh& m, float rb, float rt, float h, unsigned slices, float twist){

	m.triangles();
	unsigned Nv = m.vertices().size();
	float h_2 = h*0.5f;

	double frq = 2*M_PI/slices;
	CSin csinb(frq, rb);
	CSin csint = csinb;
	csint.ampPhase(rt, twist*frq);

	// With texcoords, we must duplicate the seam. Incrementing the slices by
	// one here will do the trick---all the math below will still work.
	if(m.wants(Mesh::TEXCOORD)) ++slices;

	for(unsigned i=0; i<slices; ++i){
		auto pb = Mesh::Vertex(csinb.r, csinb.i, -h_2);
		auto pt = Mesh::Vertex(csint.r, csint.i,  h_2);
		csinb();
		csint();

		m.vertex(pb);
		m.vertex(pt);

		Mesh::Tangent T;
		if(m.wants(Mesh::TANGENT | Mesh::NORMAL)){
			T = (pt - pb).dir();
		}
		if(m.wants(Mesh::TANGENT)){
			m.tangent(T);
			m.tangent(T);
		}
		if(m.wants(Mesh::NORMAL)){
			Mesh::Normal N;
			if(h != 0.f){
				N = (rb != 0.f ? pb.xy()/rb : pt.xy()/rt).take<3>();
				N = N.rej1(T); // rotates N towards T to make orthonormal
			} else {
				N = Vec3f(0,0,1);
			}
			m.normal(N);
			m.normal(N);
		}

		if(m.wants(Mesh::TEXCOORD)){
			float u = float(i)/(slices-1);
			m.texCoord(u, 0.f);
			m.texCoord(u, 1.f);

			if(i){
				int ib1 = Nv + 2*i;
				int ib0 = ib1 - 2;
				int it0 = ib0 + 1;
				int it1 = ib1 + 1;
				m.index(ib0, ib1, it0);
				m.index(it0, ib1, it1);
			}

		} else {
			int j = (i+1)%slices; // next slice over
			int ib0 = Nv + 2*i;
			int ib1 = Nv + 2*j;
			int it0 = ib0 + 1;
			int it1 = ib1 + 1;
			m.index(ib0, ib1, it0);
			m.index(it0, ib1, it1);
		}
	}

	return 2*slices;
}

int addPrism(Mesh& m, float rb, float rt, float h, unsigned slices, float twist, bool caps){

	unsigned Nv = m.vertices().size();

	// tex coords not supported ATM...
	noAttribScope(m, Mesh::TEXCOORD, [&](){
		addPrismOpen(m, rb,rt,h, slices, twist);
	});

	if(caps){
		float h_2 = h*0.5f;
		m.vertex(0.,0.,-h_2);
		m.vertex(0.,0., h_2);

		if(m.wants(Mesh::TANGENT)){
			m.tangent(1.f,0.f,0.f);
			m.tangent(1.f,0.f,0.f);
		}
		if(m.wants(Mesh::NORMAL)){
			m.normal(0.f,0.f,-1.f);
			m.normal(0.f,0.f,+1.f);
		}

		int ib = m.vertices().size()-2;
		int it = m.vertices().size()-1;
		for(int i=0; i<slices; ++i){
			int j = (i+1)%slices; // next slice over
			int ib0 = Nv + 2*i;
			int ib1 = Nv + 2*j;
			m.index(ib, ib1, ib0);
			m.index(it, ib0+1, ib1+1);
		}
	}

	return 2*slices + 2*int(caps);
}


int addAnnulus(Mesh& m, float inRadius, float outRadius, unsigned slices, float twist){
	return addPrismOpen(m, outRadius, inRadius, 0, slices, twist);
}


int addCylinder(Mesh& m, float radius, float height, unsigned slices, float twist, bool caps){
	return addPrism(m, radius, radius, height, slices, twist, caps);
}

int addCylinderOpen(Mesh& m, float radius, float height, unsigned slices, float twist){
	return addPrismOpen(m, radius, radius, height, slices, twist);
}


int addSurface(
	Mesh& m, int Nx, int Ny,
	double width, double height, double cx, double cy
){
	m.triangleStrip();

	int Nv = m.vertices().size();

	// Generate positions
	for(int j=0; j<Ny; ++j){ float v = float(j)/(Ny-1);
	for(int i=0; i<Nx; ++i){ float u = float(i)/(Nx-1);
		if(m.wants(Mesh::TEXCOORD)) m.texCoord(u,v);
		if(m.wants(Mesh::NORMAL  )) m.normal (0,0,1);
		if(m.wants(Mesh::TANGENT )) m.tangent(0,1,0);
		m.vertex(
			cx + (u-0.5)*width,
			cy + (v-0.5)*height,
			0.
		);
	}}

	// Note: the start and end points of each row are duplicated to create
	// degenerate triangles.
	for(int j=0; j<Ny-1; ++j){
		m.index(j*Nx + Nv);
		for(int i=0; i<Nx; ++i){
			int idx = j*Nx + i + Nv;
			// First tri degenerate, so winding order seems reversed here
			m.index(idx);
			m.index(idx+Nx);
		}
		m.index((m.indices().last()));
	}

	return Nx*Ny;
}


int addSurfaceLoop(
	Mesh& m, int Nx, int Ny, int loopMode,
	double width, double height, double cx, double cy
){
	if(0 == loopMode) return addSurface(m ,Nx,Ny, width,height, cx,cy);

	m.triangleStrip();

	int Nv = m.vertices().size();

	// Number of cells along y
	int My = loopMode==1 ? Ny - 1 : Ny;

	double du = width/Nx;
	double dv = height/My;

	// Generate vertices
	double v = cy - height*0.5;
	for(int j=0; j<Ny; ++j){
		double u = cx - width*0.5;
		for(int i=0; i<Nx; ++i){
			if(m.wants(Mesh::NORMAL )) m.normal (0,0,1);
			if(m.wants(Mesh::TANGENT)) m.tangent(0,1,0);
			m.vertex(u, v);
			u += du;
		}
		v += dv;
	}

	// Generate indices
	// The first and last indices are duplicated to create degenerate triangles.
	m.index(Nv);

	for(int j=0; j<My; ++j){
		int j1 = j*Nx + Nv;
		int j2 = ((j+1)%Ny)*Nx + Nv;
		for(int i=0; i<Nx; ++i){
			m.index(j1 + i);
			m.index(j2 + i);
		}
		m.index(j1);
		m.index(j2);
	}

	m.index((m.indices().last()));

	return Nx*Ny;
}


int addWireSurface(
	Mesh& m, int Nx, int Ny,
	double width, double height, double x, double y
){
	m.lines();
	double mx=width /(Nx-1), ax=-0.5*width +x;
	double my=height/(Ny-1), ay=-0.5*height+y;
	for(int j=0; j<Ny; ++j){ float gy = j*my+ay;
	for(int i=0; i<Nx; ++i){ float gx = i*mx+ax;
		if(i<=(Nx-2)) m.indexRel(0, 1);
		if(j<=(Ny-2)) m.indexRel(0, Nx);
		m.vertex(gx,gy);
	}}
	return Nx*Ny;
}


int addTorus(
	Mesh& m, double minRadius, double majRadius, int Nmin, int Nmaj,
	double minPhase
){
	int beg = m.vertices().size();

	int Nv = addSurfaceLoop(m,
		Nmaj, Nmin,
		m.wants(Mesh::TEXCOORD) ? 0 : 2,
		2*M_PI, 2*M_PI, M_PI, M_PI - minPhase*2*M_PI/Nmin
	);

	for(int i=beg; i<beg+Nv; ++i){
		auto& p = m.vertices()[i];
		auto cs1 = std::cos(p.x), sn1 = std::sin(p.x);
		auto cs2 = std::cos(p.y), sn2 = std::sin(p.y);
		p = Mesh::Vertex(
			(majRadius + minRadius*cs2) * cs1,
			(majRadius + minRadius*cs2) * sn1,
			minRadius*sn2
		);
		if(m.wants(Mesh::NORMAL)){ // addSurface* added normals
			m.normals()[i] = { cs2*cs1, cs2*sn1, sn2 };
		}
		if(m.wants(Mesh::TANGENT)){ // addSurface* added tangents
			m.tangents()[i] = { -sn2*cs1, -sn2*sn1, cs2 };
		}
	}

	return Nv;
}


int addVoxels(
	Mesh& m,
	const std::function<float(int x, int y, int z)>& getVoxel,
	int Nx, int Ny, int Nz, float cellSize,
	const std::function<void(int vertex)>& onFace
){
	int numVertIn = m.vertices().size();
	m.triangles();
	float n = 1.; // 1 normals point out, -1 normals point in
	//bool wantsTan = m.wants(Mesh::TANGENT);
	//bool wantsTxc = m.wants(Mesh::TEXCOORD);
	for(int k=0; k<Nz+1; ++k){
	for(int j=0; j<Ny+1; ++j){
	for(int i=0; i<Nx+1; ++i){
		bool igood = i<=(Nx-1);
		bool jgood = j<=(Ny-1);
		bool kgood = k<=(Nz-1);
		auto v = igood && jgood && kgood ? getVoxel(i,j,k) : 0.f;
		auto vx = i>0 && jgood && kgood ? getVoxel(i-1,j,k) : 0.f;
		auto vy = j>0 && igood && kgood ? getVoxel(i,j-1,k) : 0.f;
		auto vz = k>0 && igood && jgood ? getVoxel(i,j,k-1) : 0.f;
		Vec3f pos(i,j,k);
		pos = pos * cellSize;
		float D = cellSize;
		if((v*vx)==0. && v!=vx){ // one value zero and the other non-zero
			int Nv = m.vertices().size();
			m.vertex(pos.x, pos.y  , pos.z  );
			m.vertex(pos.x, pos.y+D, pos.z  );
			m.vertex(pos.x, pos.y  , pos.z+D);
			m.vertex(pos.x, pos.y+D, pos.z+D);
			if(v<vx){
				for(int i=0; i<4; ++i){
					m.normal( n, 0., 0.);
					//if(wantsTan) m.tangent(0.,0.,1.);
					//if(wantsTxc) m.texCoord(0.,0.,1.);
				}
				m.index(Nv, Nv+1, Nv+2, Nv+2, Nv+1, Nv+3);
			} else {
				for(int i=0; i<4; ++i) m.normal(-n, 0., 0.);
				m.index(Nv+2, Nv+1, Nv, Nv+3, Nv+1, Nv+2);
			}
			onFace(Nv);
		}
		if((v*vy)==0. && v!=vy){
			int Nv = m.vertices().size();
			m.vertex(pos.x  , pos.y, pos.z  );
			m.vertex(pos.x+D, pos.y, pos.z  );
			m.vertex(pos.x  , pos.y, pos.z+D);
			m.vertex(pos.x+D, pos.y, pos.z+D);
			RGB col = HSV((v+vy-1.)*0.03, cos(v+vy)*0.2+0.8, sin(v+vy)*0.3+0.7);
			if(v<vy){
				for(int i=0; i<4; ++i) m.normal(0., n, 0.);
				m.index(Nv+2, Nv+1, Nv, Nv+3, Nv+1, Nv+2);
			} else {
				for(int i=0; i<4; ++i) m.normal(0.,-n, 0.);
				m.index(Nv, Nv+1, Nv+2, Nv+2, Nv+1, Nv+3);
			}
			onFace(Nv);
		}
		if((v*vz)==0. && v!=vz){
			int Nv = m.vertices().size();
			m.vertex(pos.x  , pos.y  , pos.z);
			m.vertex(pos.x+D, pos.y  , pos.z);
			m.vertex(pos.x  , pos.y+D, pos.z);
			m.vertex(pos.x+D, pos.y+D, pos.z);
			if(v<vz){
				for(int i=0; i<4; ++i) m.normal(0., 0., n);
				m.index(Nv, Nv+1, Nv+2, Nv+2, Nv+1, Nv+3);
			} else {
				for(int i=0; i<4; ++i) m.normal(0., 0.,-n);
				m.index(Nv+2, Nv+1, Nv, Nv+3, Nv+1, Nv+2);
			}
			onFace(Nv);
		}
	}}}

	return m.vertices().size() - numVertIn;
}

} // al::
