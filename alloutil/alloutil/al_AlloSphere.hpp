#ifndef INC_AL_UTIL_ALLOSPHERE_HPP
#define INC_AL_UTIL_ALLOSPHERE_HPP

#include "allocore/math/al_Constants.hpp"
#include "allocore/graphics/al_GraphicsOpenGL.hpp"

/*!
	A model of the AlloSphere courtesy of Domagoj
*/

namespace al {

class AlloSphereModel : public GPUObject
{
public:

	AlloSphereModel(float size = 1.0, int slices = 32, int stacks = 32)
	: GPUObject(), mSize(size), mSlices(slices), mStacks(stacks)
	{
		if( mStacks % 2 != 0) {
			mStacks = mStacks - 1;
		}
	}

	virtual ~AlloSphereModel() {}

	virtual void onCreate() {
		if (mVboSolid ==0) initSolid();
		if (mVboWireframe ==0) initWireframe();
	}

	virtual void onDestroy() {
		glDeleteBuffers(1, &mVboSolid);
		glDeleteBuffers(1, &mVboWireframe);
		mVboSolid = mVboWireframe = 0;
	}

	void drawSolid();
	void drawWireframe();

private:
	float mSize;
	int mSlices;
	int mStacks;

	GLuint mVboSolid;
	GLuint mVboWireframe;

	void initSolid();
	void initWireframe();
};


inline void AlloSphereModel::initSolid()
{
	int slices = mSlices;
	int stacks = mStacks;
	float size = mSize;

	GLdouble radius = size;
	GLdouble gap = radius * 7 / 16.0;

	int count = 2 * (slices + 2) + 2 * (stacks / 2 - 1) * (slices + 1) * 2 + (slices + 1) * 2;

	GLfloat *vertex = new GLfloat[count * 3];
	GLfloat *normal = new GLfloat[count * 3];

	const double angleSlice = 2 * M_PI / double(slices);
	const double angleStack = M_PI / double(stacks);

	int p = 0;

	// cap
	// triangle fan, points: slices + 2

	double x0 = 1.0;
	double x1 = cos(angleStack);
	double r0 = 0.0;
	double r1 = sin(angleStack);

	normal[p*3 + 0] = x0;
	normal[p*3 + 1] = 0;
	normal[p*3 + 2] = 0;

	vertex[p*3 + 0] = x0*radius + gap/2;
	vertex[p*3 + 1] = 0;
	vertex[p*3 + 2] = 0;

	p++;

	for ( int j = slices; j >= 0; j--, p++ ) {
		double beta = M_PI + j * angleSlice;

		normal[p*3 + 0] = x1;
		normal[p*3 + 1] = sin(beta)*r1;
		normal[p*3 + 2] = cos(beta)*r1;

		vertex[p*3 + 0] = x1*radius + gap/2;
		vertex[p*3 + 1] = sin(beta)*r1*radius;
		vertex[p*3 + 2] = cos(beta)*r1*radius;
	}


	// hemisphere

	for (int i = 1; i < stacks / 2; i++ )
	{
		double alpha = i * angleStack;

		x0 = x1; x1 = cos(alpha + angleStack);
		r0 = r1; r1 = sin(alpha + angleStack);

		// quad strip, points: (slices + 1) * 2

		for (int j = 0; j <= slices; j++)
		{
			double beta = M_PI + j * angleSlice;
			double sinb = sin(beta);
			double cosb = cos(beta);

			normal[p*3 + 0] = x0;
			normal[p*3 + 1] = sinb*r0;
			normal[p*3 + 2] = cosb*r0;

			vertex[p*3 + 0] = x0*radius + gap/2;
			vertex[p*3 + 1] = sinb*r0*radius;
			vertex[p*3 + 2] = cosb*r0*radius;

			p++;

			normal[p*3 + 0] = x1;
			normal[p*3 + 1] = sinb*r1;
			normal[p*3 + 2] = cosb*r1;

			vertex[p*3 + 0] = x1*radius + gap/2;
			vertex[p*3 + 1] = sinb*r1*radius;
			vertex[p*3 + 2] = cosb*r1*radius;

			p++;
		}
	}


	// cylinder
	// quad strip, points: (slices + 1) * 2

	for (int j = 0; j <= slices; j++)
	{
		double beta = M_PI + j * angleSlice;
		double sinb = sin(beta);
		double cosb = cos(beta);

		normal[p*3 + 0] = 0;
		normal[p*3 + 1] = sinb;
		normal[p*3 + 2] = cosb;

		vertex[p*3 + 0] = +gap/2;
		vertex[p*3 + 1] = sinb*radius;
		vertex[p*3 + 2] = cosb*radius;

		p++;

		normal[p*3 + 0] = 0;
		normal[p*3 + 1] = sinb;
		normal[p*3 + 2] = cosb;

		vertex[p*3 + 0] = -gap/2;
		vertex[p*3 + 1] = sinb*radius;
		vertex[p*3 + 2] = cosb*radius;

		p++;
	}


	// hemisphere

	x1 = cos(M_PI - angleStack);
	r1 = sin(M_PI - angleStack);

	for (int i = stacks - 1; i > stacks / 2; i-- )
	{
		double alpha = i * angleStack;

		x0 = x1; x1 = cos(alpha - angleStack);
		r0 = r1; r1 = sin(alpha - angleStack);

		// quad strip, points: (slices + 1) * 2

		for (int j = 0; j <= slices; j++)
		{
			double beta = M_PI + j * angleSlice;
			double sinb = sin(beta);
			double cosb = cos(beta);

			normal[p*3 + 0] = x0;
			normal[p*3 + 1] = sinb*r0;
			normal[p*3 + 2] = cosb*r0;

			vertex[p*3 + 0] = x0*radius - gap/2;
			vertex[p*3 + 1] = sinb*r0*radius;
			vertex[p*3 + 2] = cosb*r0*radius;

			p++;

			normal[p*3 + 0] = x1;
			normal[p*3 + 1] = sinb*r1;
			normal[p*3 + 2] = cosb*r1;

			vertex[p*3 + 0] = x1*radius - gap/2;
			vertex[p*3 + 1] = sinb*r1*radius;
			vertex[p*3 + 2] = cosb*r1*radius;

			p++;
		}
	}


	// cap
	// triangle fan, points: slices + 2

	x1 = cos(M_PI - angleStack);
	r1 = sin(M_PI - angleStack);
	x0 = -1.0;

	normal[p*3 + 0] = x0;
	normal[p*3 + 1] = 0;
	normal[p*3 + 2] = 0;

	vertex[p*3 + 0] = x0*radius - gap/2;
	vertex[p*3 + 1] = 0;
	vertex[p*3 + 2] = 0;

	p++;

	for ( int j = slices; j >= 0; j--, p++ )
	{
		double beta = M_PI + j * angleSlice;

		normal[p*3 + 0] = x1;
		normal[p*3 + 1] = sin(beta)*r1;
		normal[p*3 + 2] = cos(beta)*r1;

		vertex[p*3 + 0] = x1*radius - gap/2;
		vertex[p*3 + 1] = sin(beta)*r1*radius;
		vertex[p*3 + 2] = cos(beta)*r1*radius;
	}


	GLuint vbo;

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	int asize = count * 3 * sizeof(GLfloat);

	glBufferData(GL_ARRAY_BUFFER, asize + asize, 0, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, asize, vertex);
	glBufferSubData(GL_ARRAY_BUFFER, asize, asize, normal);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	mVboSolid = vbo;
}

inline void AlloSphereModel::initWireframe()
{
	int slices = mSlices;
	int stacks = mStacks;
	float size = mSize;

	GLdouble radius = size;
	GLdouble gap = radius * 7 / 16.0;

	int count = stacks * slices + slices * (stacks + 2);

	GLfloat *vertex = new GLfloat[count * 3];

	const double angleSlice = 2 * M_PI / double(slices);
	const double angleStack = M_PI / double(stacks);

	int p = 0;


	// parallels
	for ( int i = 1; i <= stacks / 2; i++ )
	{
		double x = cos(i*angleStack) * radius + gap / 2;
		double r = sin(i*angleStack) * radius;

		// line loop, points: slices
		for (int j = 0; j < slices; j++, p++)
		{
			double y = sin(j*angleSlice);
			double z = cos(j*angleSlice);

			vertex[p*3 + 0] = x;
			vertex[p*3 + 1] = y * r;
			vertex[p*3 + 2] = z * r;
		}
	}
	for ( int i = stacks / 2; i < stacks; i++ )
	{
		double x = cos(i*angleStack) * radius - gap / 2;
		double r = sin(i*angleStack) * radius;

		// line loop, points: slices
		for (int j = 0; j < slices; j++, p++)
		{
			double y = sin(j*angleSlice);
			double z = cos(j*angleSlice);

			vertex[p*3 + 0] = x;
			vertex[p*3 + 1] = y * r;
			vertex[p*3 + 2] = z * r;
		}
	}


	// meridians
	for ( int i = 0; i < slices; i++ )
	{
		double alpha = i*angleSlice;
		double sina = sin(alpha);
		double cosa = cos(alpha);

		// line strip, points: stacks + 2

		for ( int j = 0; j <= stacks / 2; j++, p++ )
		{
			double x = cos(j*angleStack);
			double y = sina * sin(j*angleStack);
			double z = cosa * sin(j*angleStack);

			vertex[p*3 + 0] = x * radius + gap / 2;
			vertex[p*3 + 1] = y * radius;
			vertex[p*3 + 2] = z * radius;
		}
		for ( int j = stacks / 2; j <= stacks; j++, p++ )
		{
			double x = cos(j*angleStack);
			double y = sina * sin(j*angleStack);
			double z = cosa * sin(j*angleStack);

			vertex[p*3 + 0] = x * radius - gap / 2;
			vertex[p*3 + 1] = y * radius;
			vertex[p*3 + 2] = z * radius;
		}
	}


	GLuint vbo;

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	int asize = count * 3 * sizeof(GLfloat);

	glBufferData(GL_ARRAY_BUFFER, asize, 0, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, asize, vertex);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	mVboWireframe = vbo;
}

inline void AlloSphereModel::drawSolid()
{
	int slices = mSlices;
	int stacks = mStacks;
	int countFan = slices + 2;
	int countAll = 2 * (slices + 2) + 2 * (stacks / 2 - 1) * (slices + 1) * 2 + (slices + 1) * 2;

	glBindBuffer(GL_ARRAY_BUFFER, mVboSolid);

	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);

	glNormalPointer( GL_FLOAT, 0, (void*) (countAll * 3 * sizeof(GLfloat)) );
	glVertexPointer(3, GL_FLOAT, 0, 0);

	glDrawArrays(GL_TRIANGLE_FAN, 0, countFan);

	for ( int i = 0; i < stacks - 1; i++ )
	{
		//glDrawArrays( GL_QUAD_STRIP, countFan + i * (slices + 1) * 2, (slices + 1) * 2 );
		glDrawArrays( GL_TRIANGLE_STRIP, countFan + i * (slices + 1) * 2, (slices + 1) * 2 );
	}

	glDrawArrays(GL_TRIANGLE_FAN, countAll - countFan, countFan);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void AlloSphereModel::drawWireframe()
{
	int slices = mSlices;
	int stacks = mStacks;

	glBindBuffer(GL_ARRAY_BUFFER, mVboWireframe);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, 0);

	// parallels
	for ( int i = 0; i < stacks; i++ )
	{
		glDrawArrays( GL_LINE_LOOP, i * slices, slices );
	}

	// meridians
	for ( int i = 0; i < slices; i++ )
	{
		glDrawArrays( GL_LINE_STRIP, stacks * slices + i * (stacks + 2), stacks + 2 );
	}

	glDisableClientState(GL_VERTEX_ARRAY);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

} // al::

#endif
