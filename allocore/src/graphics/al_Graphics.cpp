#include <map>
#include <stdio.h>
#include "allocore/graphics/al_Graphics.hpp"

namespace al{

Graphics::Graphics()
:	mRescaleNormal(0), mInImmediateMode(false)
{}

Graphics::~Graphics(){}

#define CS(t) case Graphics::t: return #t;
const char * toString(Graphics::DataType v){
	switch(v){
		CS(BYTE) CS(UBYTE) CS(SHORT) CS(USHORT) CS(INT) CS(UINT)
		CS(BYTES_2) CS(BYTES_3) CS(BYTES_4)
		CS(FLOAT) CS(DOUBLE)
		default: return "";
	}
}

const char * toString(Graphics::Format v){
	switch(v){
		CS(DEPTH_COMPONENT) CS(LUMINANCE) CS(LUMINANCE_ALPHA)
		CS(RED) CS(GREEN) CS(BLUE) CS(ALPHA)
		CS(RGB) CS(BGR) CS(RGBA) CS(BGRA)
		default: return "";
	}
}
#undef CS

int Graphics::numComponents(Format v){
	switch(v){
		case RGBA:
		case BGRA:				return 4;
		case RGB:
		case BGR:				return 3;
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
		CS(BYTE, GLbyte)
		CS(UBYTE, GLubyte)
		CS(SHORT, GLshort)
		CS(USHORT, GLushort)
		CS(INT, GLint)
		CS(UINT, GLuint)
		CS(BYTES_2, char[2])
		CS(BYTES_3, char[3])
		CS(BYTES_4, char[4])
		CS(FLOAT, GLfloat)
		CS(DOUBLE, GLdouble)
		default: return 0;
	};
	#undef CS
}

template<> Graphics::DataType Graphics::toDataType<char>(){ return BYTE; }
template<> Graphics::DataType Graphics::toDataType<unsigned char>(){ return UBYTE; }
template<> Graphics::DataType Graphics::toDataType<short>(){ return SHORT; }
template<> Graphics::DataType Graphics::toDataType<unsigned short>(){ return USHORT; }
template<> Graphics::DataType Graphics::toDataType<int>(){ return INT; }
template<> Graphics::DataType Graphics::toDataType<unsigned int>(){ return UINT; }
template<> Graphics::DataType Graphics::toDataType<float>(){ return FLOAT; }
template<> Graphics::DataType Graphics::toDataType<double>(){ return DOUBLE; }

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

/*static*/ const std::string& Graphics::extensions(){
	static bool getExts = true;
	static std::string str;
	if(getExts){ // Get extensions string (once)
		str = (const char *)glGetString(GL_EXTENSIONS);
		str += " ";
		getExts = false;
	}
	return str;
}

/*static*/ bool Graphics::extensionSupported(const std::string& name, bool exactMatch){
	static std::map<std::string, bool> extMap;

	// First check for extension in the map...
	const auto searchTerm = name + (exactMatch?" ":"");
	auto it = extMap.find(searchTerm);
	if(it != extMap.end()){
		return it->second;
	}

	// Extension not in map, so search string and cache the result
	else{
		if(extensions().find(searchTerm) != std::string::npos){
			extMap[searchTerm] = true;
			return true;
		} else {
			extMap[searchTerm] = false;
			return false;
		}
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

void Graphics::draw(const Mesh& m, int count, int begin){

	const int Nv = m.vertices().size();
	if(0 == Nv) return; // nothing to draw, so just return...

	const int Ni = m.indices().size();

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

	const int Nc = m.colors().size();
	const int Nci= m.coloris().size();
	const int Nn = m.normals().size();
	const int Nt1= m.texCoord1s().size();
	const int Nt2= m.texCoord2s().size();
	const int Nt3= m.texCoord3s().size();

	//printf("client %d, GPU %d\n", clientSide, gpuSide);
	//printf("Nv %i Nc %i Nn %i Nt2 %i Nt3 %i Ni %i\n", Nv, Nc, Nn, Nt2, Nt3, Ni);

	// Enable arrays and set pointers...
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, &m.vertices()[0]);

	if(Nn >= Nv){
		glEnableClientState(GL_NORMAL_ARRAY);
		glNormalPointer(GL_FLOAT, 0, &m.normals()[0]);
	}

	if(Nc >= Nv){
		glEnableClientState(GL_COLOR_ARRAY);
		glColorPointer(4, GL_FLOAT, 0, &m.colors()[0]);
	}
	else if(Nci >= Nv){
		glEnableClientState(GL_COLOR_ARRAY);
		glColorPointer(4, GL_UNSIGNED_BYTE, 0, &m.coloris()[0]);
		//printf("using integer colors\n");
	}
	else if(0 == Nc && 0 == Nci){
		// just use whatever the last glColor() call used!
	}
	else{
		if(Nc)
			//glColor4f(m.colors()[0][0], m.colors()[0][1], m.colors()[0][2], m.colors()[0][3]);
			glColor4fv(m.colors()[0].components);
		else
			glColor3ubv(m.coloris()[0].components);
	}

	if(Nt1 >= Nv){
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(1, GL_FLOAT, 0, &m.texCoord1s()[0]);
	}
	else if(Nt2 >= Nv){
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2, GL_FLOAT, 0, &m.texCoord2s()[0]);
	}
	else if(Nt3 >= Nv){
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(3, GL_FLOAT, 0, &m.texCoord3s()[0]);
	}

	auto prim = (Graphics::Primitive)m.primitive();

	if(m.stroke() > 0.f){
		switch(prim){
		case LINES: case LINE_STRIP: case LINE_LOOP:
			glLineWidth(m.stroke());
			break;
		case POINTS:
			glPointSize(m.stroke());
			break;
		default:;
		}
	}

	// Draw
	if(Ni){
		// Here, 'count' is the number of indexed elements to render
		glDrawElements(prim, count, GL_UNSIGNED_INT, &m.indices()[begin]);
	}
	else{
		glDrawArrays(prim, begin, count);
	}

	// Disable arrays
	glDisableClientState(GL_VERTEX_ARRAY);
	if(Nn)					glDisableClientState(GL_NORMAL_ARRAY);
	if(Nc || Nci)			glDisableClientState(GL_COLOR_ARRAY);
	if(Nt1 || Nt2 || Nt3)	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

// draw a MeshVBO
void Graphics::draw(MeshVBO& meshVBO) {
	if (!meshVBO.isBound()) meshVBO.bind();

	if (meshVBO.hasIndices()) glDrawElements(meshVBO.primitive(), meshVBO.getNumIndices(), GL_UNSIGNED_INT, NULL);
	else glDrawArrays(meshVBO.primitive(), 0, meshVBO.getNumVertices());

	meshVBO.unbind();
}


void Graphics::onCreate(){
	GPUObject::mID = 1; // must be non-zero to flag creation
	mRescaleNormal = 0;
}

void Graphics::onDestroy(){
}

} // al::
