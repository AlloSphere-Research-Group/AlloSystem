#include <math.h>
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


int addCube(Mesh& m, bool withNormalsAndTexcoords, float l){

	m.primitive(Graphics::TRIANGLES);

	// This generates a cube with face-oriented normals and unit texture
	// coordinates per face. It should be rendered using a quad primitive.
	if(withNormalsAndTexcoords){

		// All six faces will have the same tex coords
		for(int i=0;i<6;++i){
			m.texCoord( 0, 0);
			m.texCoord( 1, 0);
			m.texCoord( 1, 1);
			m.texCoord( 0, 1);
		}

		// +x face
		for(int i=0;i<4;++i) m.normal( 1, 0, 0);
		m.vertex( l,-l, l);
		m.vertex( l,-l,-l);
		m.vertex( l, l,-l);
		m.vertex( l, l, l);

		// -x face
		for(int i=0;i<4;++i) m.normal(-1, 0, 0);
		m.vertex(-l, l, l);
		m.vertex(-l, l,-l);
		m.vertex(-l,-l,-l);
		m.vertex(-l,-l, l);

		// +y face
		for(int i=0;i<4;++i) m.normal( 0, 1, 0);
		m.vertex(-l, l, l);
		m.vertex( l, l, l);
		m.vertex( l, l,-l);
		m.vertex(-l, l,-l);

		// -y face
		for(int i=0;i<4;++i) m.normal( 0,-1, 0);
		m.vertex(-l,-l,-l);
		m.vertex( l,-l,-l);
		m.vertex( l,-l, l);
		m.vertex(-l,-l, l);

		// +z face
		for(int i=0;i<4;++i) m.normal( 0, 0, 1);
		m.vertex(-l,-l, l);
		m.vertex( l,-l, l);
		m.vertex( l, l, l);
		m.vertex(-l, l, l);

		// -z face
		for(int i=0;i<4;++i) m.normal( 0, 0,-1);
		m.vertex(-l, l,-l);
		m.vertex( l, l,-l);
		m.vertex( l,-l,-l);
		m.vertex(-l,-l,-l);

		return 6*4;
	}
	else{
		/*
				0	1

				2	3
		4	5

		6	7

			t	b
			| /
		l --+--	r
		  /	|
		f	b

		*/

		int Nv = 8;
		m.vertex(-l, l,-l);	m.vertex( l, l,-l);
		m.vertex(-l,-l,-l);	m.vertex( l,-l,-l);
		m.vertex(-l, l, l);	m.vertex( l, l, l);
		m.vertex(-l,-l, l);	m.vertex( l,-l, l);

		static const int indices[] = {
			6,5,4, 6,7,5, 7,1,5, 7,3,1,
			3,0,1, 3,2,0, 2,4,0, 2,6,4,
			4,1,0, 4,5,1, 2,3,6, 3,7,6
		};

		m.index(indices, sizeof(indices)/sizeof(*indices), m.vertices().size()-Nv);

		return Nv;
	}
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


int addWireBox(Mesh& m, float w, float h, float d){

	m.primitive(Graphics::LINES);

	int Nv = m.vertices().size();

	/*		6 7
			4 5
		2 3
		0 1			*/

	for(int k=-1; k<=1; k+=2){
	for(int j=-1; j<=1; j+=2){
	for(int i=-1; i<=1; i+=2){
		m.vertex(i*w, j*h, k*d);
	}}}

	static const int I[] = {
		0,1, 2,3, 4,5, 6,7,
		0,2, 1,3, 4,6, 5,7,
		0,4, 1,5, 2,6, 3,7
	};

	m.index(I, sizeof(I)/sizeof(*I), Nv);

	return m.vertices().size() - Nv;
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


int addPrism(Mesh& m, float btmRadius, float topRadius, float height, unsigned slices, float twist){

	m.primitive(Graphics::TRIANGLE_STRIP);
	unsigned Nv = m.vertices().size();
	float height_2 = height/2;

	if(twist == 0){
		CSin csin(2*M_PI/slices);
		for(unsigned i=0; i<slices; ++i){
			m.vertex(csin.r*btmRadius, csin.i*btmRadius,  height_2);
			m.vertex(csin.r*topRadius, csin.i*topRadius, -height_2);
			csin();
			m.index(Nv + 2*i);
			m.index(Nv + 2*i+1);
		}
	}
	else{
		double frq = 2*M_PI/slices;
		CSin csinb(frq, btmRadius);
		CSin csint = csinb;
		csint.ampPhase(topRadius, twist*frq);
		for(unsigned i=0; i<slices; ++i){
			m.vertex(csinb.r, csinb.i,  height_2);
			csinb();
			m.vertex(csint.r, csint.i, -height_2);
			csint();
			m.index(Nv + 2*i);
			m.index(Nv + 2*i+1);
		}
	}

	m.index(Nv);
	m.index(Nv+1);

	return 2*slices;
}


int addAnnulus(Mesh& m, float inRadius, float outRadius, unsigned slices, float twist){
	return addPrism(m, inRadius, outRadius, 0, slices, twist);
}


int addCylinder(Mesh& m, float radius, float height, unsigned slices, float twist){
	return addPrism(m, radius, radius, height, slices, twist);
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
			//m.texCoord(float(i)/(Nx-1), float(j)/(Ny-1)); //TODO: make Mesh method
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
			m.index(idx);
			m.index(idx+Nx);
		}
		int idx = m.indices().last();
		m.index(idx);
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

	m.index(m.indices().last());

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

}
