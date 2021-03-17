#include <map>
#include <math.h>
#include "allocore/math/al_Constants.hpp"
#include "allocore/graphics/al_Shapes.hpp"
#include "allocore/graphics/al_Graphics.hpp"

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

	m.primitive(Graphics::TRIANGLES);
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

	m.primitive(Graphics::TRIANGLES);

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

	m.primitive(Graphics::TRIANGLES);

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

	m.primitive(Graphics::TRIANGLES);

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

	m.primitive(Graphics::TRIANGLES);

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

	if(m.primitive() != Graphics::TRIANGLES) return;

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
					Mesh::Vertex v1 = m.vertices()[i1];
					Mesh::Vertex v2 = m.vertices()[i2];
					Mesh::Vertex vm;
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

	m.primitive(Graphics::TRIANGLES);

	int Nv = m.vertices().size();

	CSin P( M_PI/stacks); P.r = P.dr*radius; P.i = P.di*radius;
	CSin T(M_2PI/slices);

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

	return m.vertices().size()-Nv;
}


int addSphereWithTexcoords(Mesh& m, double radius, int bands ){

	m.primitive(Graphics::TRIANGLES);

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
	m.primitive(Graphics::LINES);

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


int addCone(Mesh& m, float radius, const Vec3f& apex, unsigned slices, unsigned cycles){

	m.primitive(Graphics::TRIANGLES);

	unsigned Nv = m.vertices().size();

	// Note: leaving base on xy plane makes it easy to construct a bicone
	m.vertex(apex);

	CSin csin(cycles * 2*M_PI/slices, radius);
	for(unsigned i=Nv+1; i<=(Nv+slices); ++i){
		float x = csin.r;
		float y = csin.i;
		csin();
		m.vertex(x,y);
		m.index(Nv);
		m.index(i);
		m.index(i+1);
	}

	m.indices().last() = Nv+1;

	return 1 + slices;
}


int addDisc(Mesh& m, float radius, unsigned slices){
	return addCone(m, radius, Vec3f(0,0,0), slices);
}

int addRect(Mesh& m, float width, float height, float x, float y){
	float w_2 = width*0.5, h_2 = height*0.5;
	m.primitive(Graphics::TRIANGLES);
	m.vertex(x-w_2, y-h_2);
	m.vertex(x+w_2, y-h_2);
	m.vertex(x-w_2, y+h_2);
	m.vertex(x+w_2, y+h_2);
	m.index(0,1,2, 2,1,3);
	return 4;
}

int addQuad(Mesh& m,
	float x1, float y1, float z1,
	float x2, float y2, float z2,
	float x3, float y3, float z3,
	float x4, float y4, float z4
){
	m.primitive(Graphics::TRIANGLES);
	auto i = m.vertices().size();
	m.vertex(x1,y1,z1).vertex(x2,y2,z2).vertex(x3,y3,z3).vertex(x4,y4,z4);
	m.index(i,i+1,i+2, i,i+2,i+3);
	return 4;
}


int addPrism(Mesh& m, float btmRadius, float topRadius, float height, unsigned slices, float twist, bool caps){

	m.primitive(Graphics::TRIANGLES);
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
	m.primitive(Graphics::TRIANGLE_STRIP);

	int Nv = m.vertices().size();

	double du = width/(Nx-1);
	double dv = height/(Ny-1);

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
	m.primitive(Graphics::TRIANGLE_STRIP);

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


int addTorus(
	Mesh& m, double minRadius, double majRadius, int Nmin, int Nmaj,
	double minPhase
){
	int beg = m.vertices().size();
	int Nv = addSurfaceLoop(
		m, Nmaj, Nmin, 2, 2*M_PI, 2*M_PI, M_PI, M_PI - minPhase*2*M_PI/Nmin
	);

	for(int i=beg; i<beg+Nv; ++i){
		Mesh::Vertex& v = m.vertices()[i];
		v = Mesh::Vertex(
			(majRadius + minRadius*::cos(v.y)) * ::cos(v.x),
			(majRadius + minRadius*::cos(v.y)) * ::sin(v.x),
			minRadius*::sin(v.y)
		);
	}

	return Nv;
}


int addVoxels(
	Mesh& m, std::function<float(int x, int y, int z)> field,
	int Nx, int Ny, int Nz, float cellSize
){
	int numVertIn = m.vertices().size();
	m.primitive(Graphics::TRIANGLES);
	float n = -1.; // 1 normals point out, -1 normals point in
	for(int k=0; k<Nz+1; ++k){
	for(int j=0; j<Ny+1; ++j){
	for(int i=0; i<Nx+1; ++i){
		bool igood = i<=(Nx-1);
		bool jgood = j<=(Ny-1);
		bool kgood = k<=(Nz-1);
		auto v = igood && jgood && kgood ? field(i,j,k) : 0.f;
		auto vx = i>0 && jgood && kgood ? field(i-1,j,k) : 0.f;
		auto vy = j>0 && igood && kgood ? field(i,j-1,k) : 0.f;
		auto vz = k>0 && igood && jgood ? field(i,j,k-1) : 0.f;
		Vec3f pos(i,j,k);
		float ds = cellSize;
		float scale = 1.;
		pos = (pos * ds - 1.*0.)*scale;
		float D = ds*scale;
		if((v*vx)==0. && v!=vx){
			int Nv = m.vertices().size();
			m.vertex(pos.x, pos.y  , pos.z  );
			m.vertex(pos.x, pos.y+D, pos.z  );
			m.vertex(pos.x, pos.y  , pos.z+D);
			m.vertex(pos.x, pos.y+D, pos.z+D);
			//for(int i=0; i<4; ++i) m.colori(RGB(1.));
			if(v<vx){
				for(int i=0; i<4; ++i) m.normal( n, 0., 0.);
				m.index(Nv, Nv+1, Nv+2, Nv+2, Nv+1, Nv+3);
			} else {
				for(int i=0; i<4; ++i) m.normal(-n, 0., 0.);
				m.index(Nv+2, Nv+1, Nv, Nv+3, Nv+1, Nv+2);
			}
		}
		if((v*vy)==0. && v!=vy){
			int Nv = m.vertices().size();
			m.vertex(pos.x  , pos.y, pos.z   );
			m.vertex(pos.x+D, pos.y, pos.z   );
			m.vertex(pos.x  , pos.y, pos.z+D);
			m.vertex(pos.x+D, pos.y, pos.z+D);
			RGB col = HSV((v+vy-1.)*0.03, cos(v+vy)*0.2+0.8, sin(v+vy)*0.3+0.7);
			//for(int i=0; i<4; ++i) m.colori(col);
			if(v<vy){
				for(int i=0; i<4; ++i) m.normal(0., n, 0.);
				m.index(Nv+2, Nv+1, Nv, Nv+3, Nv+1, Nv+2);
			} else {
				for(int i=0; i<4; ++i) m.normal(0.,-n, 0.);
				m.index(Nv, Nv+1, Nv+2, Nv+2, Nv+1, Nv+3);
			}
		}
		if((v*vz)==0. && v!=vz){
			int Nv = m.vertices().size();
			m.vertex(pos.x  , pos.y  , pos.z);
			m.vertex(pos.x+D, pos.y  , pos.z);
			m.vertex(pos.x  , pos.y+D, pos.z);
			m.vertex(pos.x+D, pos.y+D, pos.z);
			//for(int i=0; i<4; ++i) m.colori(RGB(1.));
			if(v<vz){
				for(int i=0; i<4; ++i) m.normal(0., 0., n);
				m.index(Nv, Nv+1, Nv+2, Nv+2, Nv+1, Nv+3);
			} else {
				for(int i=0; i<4; ++i) m.normal(0., 0.,-n);
				m.index(Nv+2, Nv+1, Nv, Nv+3, Nv+1, Nv+2);
			}
		}
	}}}

	return m.vertices().size() - numVertIn;
}

} // al::
