//#include <vector>
//#include <map>
//#include <string>
#include <stdio.h>

#include "allocore/system/al_Printing.hpp"
#include "allocore/types/al_Array.hpp"
#include "allocore/graphics/al_Graphics.hpp"

namespace al{

Graphics::Graphics() : mInImmediateMode(false) {}
Graphics::~Graphics() {}


int Graphics::numComponents(Format v){
	switch(v){
		case BGRA:
		case RGBA:				return 4;
		case RGB:				return 3;
		case LUMINANCE_ALPHA:	return 2;
		case DEPTH_COMPONENT:
		case LUMINANCE:
		case RED:
		case GREEN:
		case BLUE:
		case ALPHA:				return 1;
		default:				return 0;
	};
}

int Graphics::numBytes(DataType v){
	#define CS(a,b) case a: return sizeof(b); 
	switch(v){
		CS(BYTE, char)
		CS(UBYTE, unsigned char)
		CS(INT, int)
		CS(UINT, unsigned int)
		CS(FLOAT, float)
		default: return 0;
	};
	#undef CS
}

const char * Graphics::errorString(bool verbose){
	GLenum err = glGetError();
	#define CS(GL_ERR, desc) case GL_ERR: return verbose ? #GL_ERR ", " desc : #GL_ERR;
	switch(err){
		case GL_NO_ERROR: return "";
		CS(GL_INVALID_ENUM, "An unacceptable value is specified for an enumerated argument.")
		CS(GL_INVALID_VALUE, "A numeric argument is out of range.")
		CS(GL_INVALID_OPERATION, "The specified operation is not allowed in the current state.")
	#ifdef GL_INVALID_FRAMEBUFFER_OPERATION
		CS(GL_INVALID_FRAMEBUFFER_OPERATION, "The framebuffer object is not complete.")
	#endif
		CS(GL_OUT_OF_MEMORY, "There is not enough memory left to execute the command.")
		CS(GL_STACK_OVERFLOW, "This command would cause a stack overflow.")
		CS(GL_STACK_UNDERFLOW, "This command would cause a stack underflow.")
	#ifdef GL_TABLE_TOO_LARGE
		CS(GL_TABLE_TOO_LARGE, "The specified table exceeds the implementation's maximum supported table size.")
	#endif
		default: return "Unknown error code.";
	}
	#undef CS
}

bool Graphics::error(const char * msg, int ID){
	const char * errStr = errorString();
	if(errStr[0]){
		if(ID>=0)	AL_WARN_ONCE("Error %s (id=%d): %s", msg, ID, errStr);
		else		AL_WARN_ONCE("Error %s: %s", msg, errStr);
		return true;
	}
	return false;
}

Graphics::DataType Graphics::toDataType(AlloTy v){
	switch(v){
		case AlloFloat32Ty: return FLOAT;
		case AlloFloat64Ty: return DOUBLE;
		case AlloSInt8Ty:	return BYTE;
		case AlloUInt8Ty:	return UBYTE;
		case AlloSInt16Ty:	return SHORT;
		case AlloUInt16Ty:	return USHORT;
		case AlloSInt32Ty:	return INT;
		case AlloUInt32Ty:	return UINT;
		default:			return BYTE;
	}
}

enum DataType {
		BYTE					= GL_BYTE,
		UBYTE					= GL_UNSIGNED_BYTE,
		SHORT					= GL_SHORT,
		USHORT					= GL_UNSIGNED_SHORT,
		INT						= GL_INT,
		UINT					= GL_UNSIGNED_INT,
		FLOAT					= GL_FLOAT,
		DOUBLE					= GL_DOUBLE
	};


AlloTy Graphics :: toAlloTy(Graphics::DataType v) {
	switch (v) {
		case BYTE:		return AlloSInt8Ty;
		case UBYTE:		return AlloUInt8Ty;
		case SHORT:		return AlloSInt16Ty;
		case USHORT:	return AlloUInt16Ty;
		case INT:		return AlloSInt32Ty;
		case UINT:		return AlloUInt32Ty;
		case FLOAT:		return AlloFloat32Ty;
		case DOUBLE:	return AlloFloat64Ty;
		default:		return AlloVoidTy;
	}
}

void Graphics::antialiasing(AntiAliasMode v){
	glHint(GL_POINT_SMOOTH_HINT, v);
	glHint(GL_LINE_SMOOTH_HINT, v);
	glHint(GL_POLYGON_SMOOTH_HINT, v);

	if (FASTEST != v) {
		glEnable(GL_POLYGON_SMOOTH);
		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_POINT_SMOOTH);
	} else {
		glDisable(GL_POLYGON_SMOOTH);
		glDisable(GL_LINE_SMOOTH);
		glDisable(GL_POINT_SMOOTH);
	}
}

void Graphics::fog(float end, float start, const Color& c){
	glEnable(GL_FOG);
	glFogi(GL_FOG_MODE, GL_LINEAR); 
	glFogf(GL_FOG_START, start); glFogf(GL_FOG_END, end);
	float fogColor[4] = {c.r, c.g, c.b, c.a};
	glFogfv(GL_FOG_COLOR, fogColor);
}

void Graphics::viewport(int x, int y, int width, int height) {
	glViewport(x, y, width, height);
	enable(SCISSOR_TEST);
	glScissor(x, y, width, height);
}

Viewport Graphics::viewport() const {
	GLint vp[4];
	glGetIntegerv(GL_VIEWPORT, vp);
	return Viewport(vp[0], vp[1], vp[2], vp[3]);
}


// Immediate Mode
void Graphics::begin(Primitive v) {
	// clear buffers
	mMesh.reset();

	// start primitive drawing
	mMesh.primitive(v);
	mInImmediateMode = true;
}

void Graphics::end() {
	draw(mMesh);
	mInImmediateMode = false;
}

void Graphics::vertex(double x, double y, double z) {
	if(mInImmediateMode) {
		// make sure all buffers are the same size if > 0
		mMesh.vertex(x, y, z);
		mMesh.equalizeBuffers();
	}	
}

void Graphics::texCoord(double u, double v) {
	if(mInImmediateMode) {
		mMesh.texCoord(u, v);
	}
}

void Graphics::texCoord(double s, double t, double r) {
	if(mInImmediateMode) {
		mMesh.texCoord(s, t, r);
	}
}

void Graphics::normal(double x, double y, double z) {
	if(mInImmediateMode) {
		mMesh.normal(x, y, z);
	}
}

void Graphics::color(double r, double g, double b, double a) {
	if(mInImmediateMode) {
		mMesh.color(r, g, b, a);
	} else {
		currentColor(r, g, b, a);
	}
}

// Buffer drawing
void Graphics::draw(int num_vertices, const Mesh& m){
	draw(m, num_vertices);
}

void Graphics::draw(const Mesh& v, int count, int begin){

	const int Nv = v.vertices().size();
	if(0 == Nv) return; // nothing to draw, so just return...

	const int Ni = v.indices().size();

	const int Nmax = Ni ? Ni : Nv;

	// Adjust negative amounts
	if(count < 0) count += Nmax+1;
	if(begin < 0) begin += Nmax+1;

	// Safety checks...
	//if(count > Nmax) count = Nmax;
	//if(begin+count >= iend) return;

	if(begin >= Nmax) return;	// Begin index past end?
	if(begin + count > Nmax){	// If end index past end, then truncate it
		count = Nmax - begin;
	}

	const int Nc = v.colors().size();
	const int Nci= v.coloris().size();
	const int Nn = v.normals().size();
	const int Nt2= v.texCoord2s().size();
	const int Nt3= v.texCoord3s().size();
	
	//printf("client %d, GPU %d\n", clientSide, gpuSide);
	//printf("Nv %i Nc %i Nn %i Nt2 %i Nt3 %i Ni %i\n", Nv, Nc, Nn, Nt2, Nt3, Ni);

	// Enable arrays and set pointers...
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, &v.vertices()[0]);

	if(Nn >= Nv){
		glEnableClientState(GL_NORMAL_ARRAY);
		glNormalPointer(GL_FLOAT, 0, &v.normals()[0]);
	}
	
	if(Nc >= Nv){
		glEnableClientState(GL_COLOR_ARRAY);
		glColorPointer(4, GL_FLOAT, 0, &v.colors()[0]);
	}
	else if(Nci >= Nv){
		glEnableClientState(GL_COLOR_ARRAY);
		glColorPointer(4, GL_UNSIGNED_BYTE, 0, &v.coloris()[0]);
		//printf("using integer colors\n");	
	}
	else if(0 == Nc && 0 == Nci){
		// just use whatever the last glColor() call used!
	}
	else{
		if(Nc)
			//glColor4f(v.colors()[0][0], v.colors()[0][1], v.colors()[0][2], v.colors()[0][3]);
			glColor4fv(v.colors()[0].components);
		else
			glColor3ubv(v.coloris()[0].components);
	}
	
	if(Nt2 || Nt3){
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		if(Nt2 >= Nv) glTexCoordPointer(2, GL_FLOAT, 0, &v.texCoord2s()[0]);
		if(Nt3 >= Nv) glTexCoordPointer(3, GL_FLOAT, 0, &v.texCoord3s()[0]);
	}

	// Draw
	if(Ni){
		glDrawElements(
			((Graphics::Primitive)v.primitive()), 
			count, // number of indexed elements to render
			GL_UNSIGNED_INT, 
			&v.indices()[begin]
		);
	}
	else{
		glDrawArrays(
			((Graphics::Primitive)v.primitive()), 
			begin,
			count
		);
	}

	// Disable arrays
					glDisableClientState(GL_VERTEX_ARRAY);
	if(Nn)			glDisableClientState(GL_NORMAL_ARRAY);
	if(Nc || Nci)	glDisableClientState(GL_COLOR_ARRAY);
	if(Nt2 || Nt3)	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

} // al::
