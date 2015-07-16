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


int addTetrahedron(Mesh& m){

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

	return Nv;
}


int addOctahedron(Mesh& m){

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

	return Nv;
}


int addDodecahedron(Mesh& m){

	m.primitive(Graphics::TRIANGLES);

//	float b = 1. / phi;
//	float c = 2. - phi;
//	float vertices[] = {
//		 c,  0,  1,   -c,  0,  1,   -b,  b,  b,    0,  1,  c,    b,  b,  b,
//		-c,  0,  1,    c,  0,  1,    b, -b,  b,    0, -1,  c,   -b, -b,  b,
//		 c,  0, -1,   -c,  0, -1,   -b, -b, -b,    0, -1, -c,    b, -b, -b,
//		-c,  0, -1,    c,  0, -1,    b,  b, -b,    0,  1, -c,   -b,  b, -b,
//		 0,  1, -c,    0,  1,  c,    b,  b,  b,    1,  c,  0,    b,  b, -b,
//		 0,  1,  c,    0,  1, -c,   -b,  b, -b,   -1,  c,  0,   -b,  b,  b,
//		 0, -1, -c,    0, -1,  c,   -b, -b,  b,   -1, -c,  0,   -b, -b, -b,
//		 0, -1,  c,    0, -1, -c,    b, -b, -b,    1, -c,  0,    b, -b,  b,
//		 1,  c,  0,    1, -c,  0,    b, -b,  b,    c,  0,  1,    b,  b,  b,
//		 1, -c,  0,    1,  c,  0,    b,  b, -b,    c,  0, -1,    b, -b, -b,
//		-1,  c,  0,   -1, -c,  0,   -b, -b, -b,   -c,  0, -1,   -b,  b, -b,
//		-1, -c,  0,   -1,  c,  0,   -b,  b,  b,   -c,  0,  1,   -b, -b,  b
//	};
//
//	for(int i=0; i<Nv; i+=5){
//		Vec3f v1(vertices[3*i+ 0], vertices[3*i+ 1], vertices[3*i+ 2]);
//		Vec3f v2(vertices[3*i+ 3], vertices[3*i+ 4], vertices[3*i+ 5]);
//		Vec3f v3(vertices[3*i+ 6], vertices[3*i+ 7], vertices[3*i+ 8]);
//		Vec3f v4(vertices[3*i+ 9], vertices[3*i+10], vertices[3*i+11]);
//		Vec3f v5(vertices[3*i+12], vertices[3*i+13], vertices[3*i+14]);
//
//		Vec3f vc = (v1+v2+v3+v4+v5)/5;
//
//		plato5.vertex(v1);
//	}

	static const float vertices[] = {
		-0.57735, -0.57735, 0.57735,
		0.934172,  0.356822, 0,
		0.934172, -0.356822, 0,
		-0.934172, 0.356822, 0,
		-0.934172, -0.356822, 0,
		0,  0.934172,  0.356822,
		0,  0.934172,  -0.356822,
		0.356822,  0,  -0.934172,
		-0.356822,  0,  -0.934172,
		0,  -0.934172,  -0.356822,
		0,  -0.934172,  0.356822,
		0.356822,  0,  0.934172,
		-0.356822,  0,  0.934172,
		0.57735,  0.57735,  -0.57735,
		0.57735,  0.57735, 0.57735,
		-0.57735,  0.57735,  -0.57735,
		-0.57735,  0.57735,  0.57735,
		0.57735,  -0.57735,  -0.57735,
		0.57735,  -0.57735,  0.57735,
		-0.57735,  -0.57735,  -0.57735
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

	return Nv;
}


int addIcosahedron(Mesh& m){

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

	return Nv;
}


// Stacks are circles cut perpendicular to the z axis while slices are circles
// cut through the z axis.
// The top is (0,0,radius) and the bottom is (0,0,-radius).
int addSphere(Mesh& m, double radius, int slices, int stacks){

	m.primitive(Graphics::TRIANGLES);

	struct CSin{
		CSin(double frq, double radius=1.)
		:	r(radius), i(0.), dr(cos(frq)), di(sin(frq)){}
		void operator()(){
			double r_ = r*dr - i*di;
			i = r*di + i*dr;
			r = r_;
		}
		double r,i,dr,di;
	};

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

	double du = width/(Nx-1);
	double dv = height/(My-1);

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


void addCylinder(Mesh& m, double r1, double r2, double height, int vertCount){

  m.primitive(Graphics::TRIANGLE_STRIP);
    
  int indxCount = vertCount+2;
  double theta = 0.0;

  for (int j=0; j < vertCount; j++){
    double r = ((j % 2 == 0) ? r1 : r2);
    double x = cos(theta);
    double y = sin(theta);
    float u = ((j % 2 == 0) ? 1.0 : 0.0);
    float v = j*1.0 / vertCount;

    m.normal(x,y,(r1-r2)/2.0);
    // m.texCoord(u,v);
    m.vertex(r*x,r*y,((j % 2 == 0) ? 0.0 : height));

    theta += 2 * M_PI / (vertCount);
  }

  for(int i=0; i < indxCount; i++) m.index(i % vertCount);
}

}
