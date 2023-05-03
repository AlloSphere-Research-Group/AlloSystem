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

int addCuboid(Mesh& m, float rx, float ry, float rz){

	m.triangles();
	/* 0 1      t  b
	   2 3      | /
	4 5       l +--r
	6 7       f b       */

	int Nv = 8;
	m.vertex(-rx, ry,-rz);	m.vertex( rx, ry,-rz);
	m.vertex(-rx,-ry,-rz);	m.vertex( rx,-ry,-rz);
	m.vertex(-rx, ry, rz);	m.vertex( rx, ry, rz);
	m.vertex(-rx,-ry, rz);	m.vertex( rx,-ry, rz);

	static const int indices[] = {
		6,5,4, 6,7,5, // front
		7,1,5, 7,3,1, // right
		3,0,1, 3,2,0, // back
		2,4,0, 2,6,4, // left
		4,1,0, 4,5,1, // top
		2,3,6, 3,7,6  // bottom
	};

	m.index(indices, sizeof(indices)/sizeof(*indices), m.vertices().size()-Nv);

	return Nv;
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

		P.r *= P.dr; P.i = P.di; // due to cap

		// Add top cap
		// Triangles have one vertex at the north pole and the others on the first
		// ring down.
		m.vertex(0,0,radius);
		for(int i=0; i<slices; ++i){
			m.index(Nv+1 + i);
			m.index(Nv+1 + ((i+1)%slices));
			m.index(Nv);	// the north pole
		}

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
				T();
			}
			P();
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
		addSurface(m, slices+1,stacks+1, 1,1, 0.5,0.5);

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

	if((m.attribHint() & Mesh::NORMAL) && (radius != 0.)){
		float s = 1./radius;
		for(auto& p : m.vertices()) m.normal(p * s);
	}

	return m.vertices().size()-Nv;
}


int addSphereWithTexcoords(Mesh& m, double radius, int bands){

	m.triangles();

	double r = radius;

	// calculate vertex data with closing duplicate vertices for texturing
	for ( int lat=0; lat <= bands; lat++ ){
		double theta = lat * M_PI / bands;
		double sinTheta = sin(theta);
		double cosTheta = cos(theta);

		for (int lon=0; lon <= bands; lon++ ){
			double phi = lon * 2.0 * M_PI / bands;
			double sinPhi = sin(phi);
			double cosPhi = cos(phi);
			double x = cosPhi * sinTheta;
			double y = cosTheta;
			double z = sinPhi * sinTheta;
			double u = 1.0 - ((double)lon / bands);
			double v = (double)lat / bands;
			m.vertex(r*x, r*y, r*z);
			m.texCoord(u,v);
			m.normal(x,y,z);
		}
	}

  	// add indices
	for ( int lat=0; lat < bands; lat++ ){
		for (int lon=0; lon < bands; lon++ ){
			int first = (lat * (bands + 1)) + lon;
			int second = first + bands + 1;
			m.index( first );
			m.index( second );
			m.index( (first + 1) );
			m.index( second );
			m.index( (second + 1) );
			m.index( (first + 1) );
		}
	}

	return m.vertices().size();
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

	for(unsigned i=0; i<stacks; ++i){

		float h = float(i+1)/stacks;

		CSin csin(cycles * 2*M_PI/slices, h*radius);

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
			m.vertex(Vec3f(x,y,0.) + apex*(1.-h));
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
	m.vertex(x1,y1,z1).vertex(x2,y2,z2).vertex(x3,y3,z3).vertex(x4,y4,z4);
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

int addPrism(Mesh& m, float btmRadius, float topRadius, float height, unsigned slices, float twist, bool caps){

	m.triangles();
	unsigned Nv = m.vertices().size();
	float height_2 = height/2;

	double frq = 2*M_PI/slices;
	CSin csinb(frq, btmRadius);
	CSin csint = csinb;
	csint.ampPhase(topRadius, twist*frq);
	for(unsigned i=0; i<slices; ++i){
		m.vertex(csinb.r, csinb.i, -height_2);
		csinb();
		m.vertex(csint.r, csint.i,  height_2);
		csint();
		
		int j = (i+1)%slices; // next slice over
		int ib0 = Nv + 2*i;
		int ib1 = Nv + 2*j;
		int it0 = ib0 + 1;
		int it1 = ib1 + 1;
		m.index(ib0, ib1, it0);
		m.index(it0, ib1, it1);
	}

	if(caps){
		m.vertex(0.,0.,-height_2);
		m.vertex(0.,0., height_2);
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

	return 2*slices + 2*caps;
}


int addAnnulus(Mesh& m, float inRadius, float outRadius, unsigned slices, float twist){
	return addPrism(m, outRadius, inRadius, 0, slices, twist, false);
}


int addCylinder(Mesh& m, float radius, float height, unsigned slices, float twist, bool caps){
	return addPrism(m, radius, radius, height, slices, twist, caps);
}


int addSurface(
	Mesh& m, int Nx, int Ny,
	double width, double height, double x, double y
){
	m.triangleStrip();

	int Nv = m.vertices().size();

	// Generate positions
	for(int j=0; j<Ny; ++j){ float v = float(j)/(Ny-1);
	for(int i=0; i<Nx; ++i){ float u = float(i)/(Nx-1);
		if(m.attribHint() & Mesh::TEXCOORD) m.texCoord(u,v);
		if(m.attribHint() & Mesh::NORMAL  ) m.normal (0,0,1);
		if(m.attribHint() & Mesh::TANGENT ) m.tangent(0,1,0);
		m.vertex(
			x + (u-0.5)*width,
			y + (v-0.5)*height,
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
	double width, double height, double x, double y
){
	m.triangleStrip();

	int Nv = m.vertices().size();

	// Number of cells along y
	int My = loopMode==1 ? Ny - 1 : Ny;

	double du = width/Nx;
	double dv = height/My;

	// Generate positions
	double v = y - height*0.5;
	for(int j=0; j<Ny; ++j){
		double u = x - width*0.5;
		for(int i=0; i<Nx; ++i){
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
	int Nv = addSurfaceLoop(
		m, Nmaj, Nmin, 2, 2*M_PI, 2*M_PI, M_PI, M_PI - minPhase*2*M_PI/Nmin
	);

	for(int i=beg; i<beg+Nv; ++i){
		auto& p = m.vertices()[i];
		p = Mesh::Vertex(
			(majRadius + minRadius*::cos(p.y)) * ::cos(p.x),
			(majRadius + minRadius*::cos(p.y)) * ::sin(p.x),
			minRadius*::sin(p.y)
		);
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
	bool wantsTan = m.attribHint() & Mesh::TANGENT;
	bool wantsTxc = m.attribHint() & Mesh::TEXCOORD;
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
