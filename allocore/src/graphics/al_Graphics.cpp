#include <map>
#include <stdio.h>
#include "allocore/graphics/al_Graphics.hpp"

namespace al{

Graphics::Graphics()
:	mRescaleNormal(0), mInImmediateMode(false)
{
	#ifdef AL_GRAPHICS_USE_PROG_PIPELINE
	for(auto& stack : mMatrixStacks){
		stack.emplace(1.f);
	}
	#endif
}

Graphics::~Graphics(){}

#define CS(t) case Graphics::t: return #t;
const char * toString(Graphics::DataType v){
	switch(v){
		CS(BYTE) CS(UBYTE) CS(SHORT) CS(USHORT) CS(FLOAT)
		#ifdef AL_GRAPHICS_SUPPORTS_INT32
		CS(INT) CS(UINT)
		#endif
		#ifdef AL_GRAPHICS_SUPPORTS_DOUBLE
		CS(DOUBLE)
		#endif
		default: return "";
	}
}

const char * toString(Graphics::Format v){
	switch(v){
		#ifdef AL_GRAPHICS_SUPPORTS_DEPTH_COMP
		CS(DEPTH_COMPONENT)
		#endif
		CS(LUMINANCE) CS(LUMINANCE_ALPHA)
		CS(ALPHA) CS(RGB) CS(RGBA)
		default: return "";
	}
}
#undef CS

int Graphics::numComponents(Format v){
	switch(v){
		case RGBA:				return 4;
		case RGB:				return 3;
		case LUMINANCE_ALPHA:	return 2;
		#ifdef AL_GRAPHICS_SUPPORTS_DEPTH_COMP
		case DEPTH_COMPONENT:
		#endif
		case LUMINANCE:
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
		CS(FLOAT, GLfloat)
		#ifdef AL_GRAPHICS_SUPPORTS_INT32
		CS(INT, GLint)
		CS(UINT, GLuint)
		#endif
		#ifdef AL_GRAPHICS_SUPPORTS_DOUBLE
		CS(DOUBLE, GLdouble)
		#endif
		default: return 0;
	};
	#undef CS
}

template<> Graphics::DataType Graphics::toDataType<char>(){ return BYTE; }
template<> Graphics::DataType Graphics::toDataType<unsigned char>(){ return UBYTE; }
template<> Graphics::DataType Graphics::toDataType<short>(){ return SHORT; }
template<> Graphics::DataType Graphics::toDataType<unsigned short>(){ return USHORT; }
#ifdef AL_GRAPHICS_SUPPORTS_INT32
template<> Graphics::DataType Graphics::toDataType<int>(){ return INT; }
template<> Graphics::DataType Graphics::toDataType<unsigned int>(){ return UINT; }
#endif
template<> Graphics::DataType Graphics::toDataType<float>(){ return FLOAT; }
#ifdef AL_GRAPHICS_SUPPORTS_DOUBLE
template<> Graphics::DataType Graphics::toDataType<double>(){ return DOUBLE; }
#endif

Graphics::DataType Graphics::toDataType(AlloTy v){
	switch(v){
		case AlloFloat32Ty: return FLOAT;
		#ifdef AL_GRAPHICS_SUPPORTS_DOUBLE
		case AlloFloat64Ty: return DOUBLE;
		#endif
		case AlloSInt8Ty:	return BYTE;
		case AlloUInt8Ty:	return UBYTE;
		case AlloSInt16Ty:	return SHORT;
		case AlloUInt16Ty:	return USHORT;
		#ifdef AL_GRAPHICS_SUPPORTS_INT32
		case AlloSInt32Ty:	return INT;
		case AlloUInt32Ty:	return UINT;
		#endif
		default:			return BYTE;
	}
}

AlloTy Graphics::toAlloTy(Graphics::DataType v) {
	switch(v){
		case BYTE:		return AlloSInt8Ty;
		case UBYTE:		return AlloUInt8Ty;
		case SHORT:		return AlloSInt16Ty;
		case USHORT:	return AlloUInt16Ty;
		#ifdef AL_GRAPHICS_SUPPORTS_INT32
		case INT:		return AlloSInt32Ty;
		case UINT:		return AlloUInt32Ty;
		#endif
		case FLOAT:		return AlloFloat32Ty;
		#ifdef AL_GRAPHICS_SUPPORTS_DOUBLE
		case DOUBLE:	return AlloFloat64Ty;
		#endif
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
		CS(GL_OUT_OF_MEMORY, "There is not enough memory left to execute the command.")
	#ifdef GL_INVALID_FRAMEBUFFER_OPERATION
		CS(GL_INVALID_FRAMEBUFFER_OPERATION, "The framebuffer object is not complete.")
	#endif
	#ifdef GL_TABLE_TOO_LARGE
		CS(GL_TABLE_TOO_LARGE, "The specified table exceeds the implementation's maximum supported table size.")
	#endif
	#ifdef GL_STACK_OVERFLOW
		CS(GL_STACK_OVERFLOW, "This command would cause a stack overflow.")
	#endif
	#ifdef GL_STACK_UNDERFLOW
		CS(GL_STACK_UNDERFLOW, "This command would cause a stack underflow.")
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
	#ifdef AL_GRAPHICS_SUPPORTS_STROKE_SMOOTH
	glHint(GL_POINT_SMOOTH_HINT, v);
	glHint(GL_LINE_SMOOTH_HINT, v);
	if(FASTEST != v){
		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_POINT_SMOOTH);
	} else {
		glDisable(GL_LINE_SMOOTH);
		glDisable(GL_POINT_SMOOTH);
	}
	#endif

	#ifdef AL_GRAPHICS_SUPPORTS_POLYGON_SMOOTH
	glHint(GL_POLYGON_SMOOTH_HINT, v);
	if(FASTEST != v){
		glEnable(GL_POLYGON_SMOOTH);
	} else {
		glDisable(GL_POLYGON_SMOOTH);
	}
	#endif
}

#ifdef AL_GRAPHICS_USE_FIXED_PIPELINE
void Graphics::fog(float end, float start, const Color& c){
	glEnable(GL_FOG);
	glFogf(GL_FOG_MODE, GL_LINEAR);
	glFogf(GL_FOG_START, start); glFogf(GL_FOG_END, end);
	float fogColor[4] = {c.r, c.g, c.b, c.a};
	glFogfv(GL_FOG_COLOR, fogColor);
}
#else
void Graphics::fog(float end, float start, const Color& c){}
#endif

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

	auto prim = (Graphics::Primitive)m.primitive();

	if(m.stroke() > 0.f){
		switch(prim){
		case LINES: case LINE_STRIP: case LINE_LOOP:
			lineWidth(m.stroke());
			break;
		case POINTS:
			pointSize(m.stroke());
			break;
		default:;
		}
	}

	#ifdef AL_GRAPHICS_USE_PROG_PIPELINE

	if(mCompileShader){
		mCompileShader = false; // will only make one attempt

		mShader.compile(
		R"(
		uniform mat4 MVP;
		uniform vec4 singleColor;
		attribute vec3 posIn;
		attribute vec4 colorIn;
		attribute vec3 normalIn;
		attribute vec2 texCoord2In;
		varying vec4 color;
		varying vec3 normal;
		varying vec2 texCoord2;
		void main(){
			gl_Position = MVP * vec4(posIn,1.);
			color = singleColor.a==8192. ? colorIn : singleColor;
			normal = normalIn;
			texCoord2 = texCoord2In;
		}
		)",R"(
		precision mediump float; // yes, this is req'd by ES2
		varying vec4 color;
		varying vec3 normal;
		void main(){
			gl_FragColor = color;
			//gl_FragColor = vec4(1.,0.,0.,1.); //debug
		}
		)"
		);

		if(mShader.linked()){
			mLocPos       = mShader.attribute("posIn");
			mLocColor     = mShader.attribute("colorIn");
			mLocNormal    = mShader.attribute("normalIn");
			mLocTexCoord2 = mShader.attribute("texCoord2In");
		} else {
			printf("Critical error: al::Graphics failed to compile shader\n");
		}
	}

	// glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid * pointer)

	//printf("%d %d %d %d\n", mLocPos, mLocColor, mLocNormal, mLocTexCoord2);
	// Return if shader didn't compile
	if(mLocPos < 0) return;

	glEnableVertexAttribArray(mLocPos);
	glVertexAttribPointer(mLocPos, 3, GL_FLOAT, 0, 0, &m.vertices()[0]);

	if(Nn >= Nv){
		glEnableVertexAttribArray(mLocNormal);
		glVertexAttribPointer(mLocNormal, 3, GL_FLOAT, 0, 0, &m.normals()[0]);
	}

	Color singleColor(0,0,0,8192); // if unchanged, triggers read from array

	if(Nc >= Nv){
		glEnableVertexAttribArray(mLocColor);
		glVertexAttribPointer(mLocColor, 4, GL_FLOAT, 0, 0, &m.colors()[0]);
	}
	else if(Nci >= Nv){
		glEnableVertexAttribArray(mLocColor);
		glVertexAttribPointer(mLocColor, 4, GL_UNSIGNED_BYTE, 0, 0, &m.coloris()[0]);
	}
	else if(0 == Nc && 0 == Nci){
		singleColor = mCurrentColor;
	}
	else{
		singleColor = Nc ? m.colors()[0] : Color(m.coloris()[0]);
	}

	if(Nt2 >= Nv){
		glEnableVertexAttribArray(mLocTexCoord2);
		glVertexAttribPointer(mLocTexCoord2, 2, GL_FLOAT, 0, 0, &m.texCoord2s()[0]);
	}

	mShader.begin();
		mShader.uniform("MVP", projection()*modelView());
		mShader.uniform("singleColor", singleColor);
		if(Ni){
			// Here, 'count' is the number of indices to render
			glDrawElements(prim, count, GL_UNSIGNED_INT, &m.indices()[begin]);
		}
		else{
			glDrawArrays(prim, begin, count);
		}
	mShader.end();

	glDisableVertexAttribArray(mLocPos);
	if(Nc) glDisableVertexAttribArray(mLocColor);
	if(Nn) glDisableVertexAttribArray(mLocNormal);
	if(Nt2)glDisableVertexAttribArray(mLocTexCoord2);


	//---- FIXED PIPELINE
	#else

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
			glColor4f(m.colors()[0].r, m.colors()[0].g, m.colors()[0].b, m.colors()[0].a);
		else
			glColor4ub(m.coloris()[0].r, m.coloris()[0].g, m.coloris()[0].b, m.coloris()[0].a);
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

	// Draw
	if(Ni){
		#ifdef AL_GRAPHICS_SUPPORTS_INT32
			// Here, 'count' is the number of indices to render
			glDrawElements(prim, count, GL_UNSIGNED_INT, &m.indices()[begin]);
		#else
			mIndices16.clear();
			for(int i=begin; i<begin+count; ++i){
				auto idx = m.indices()[i];
				if(idx > 65535) AL_WARN_ONCE("Mesh index value out of range (> 65535)");
				mIndices16.push_back(idx);
			}
			glDrawElements(prim, count, GL_UNSIGNED_SHORT, &mIndices16[0]);
		#endif
	}
	else{
		glDrawArrays(prim, begin, count);
	}

	// Disable arrays
	glDisableClientState(GL_VERTEX_ARRAY);
	if(Nn)					glDisableClientState(GL_NORMAL_ARRAY);
	if(Nc || Nci)			glDisableClientState(GL_COLOR_ARRAY);
	if(Nt1 || Nt2 || Nt3)	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	#endif
}


void Graphics::onCreate(){
	GPUObject::mID = 1; // must be non-zero to flag creation
	mRescaleNormal = 0;
}

void Graphics::onDestroy(){
}

} // al::
