#include "allocore/graphics/al_Graphics.hpp"
#include "allocore/graphics/al_Shader.hpp"

#include <stdio.h>
#include <map>
#include <string>

using std::map;
using std::string;

namespace al{

GLenum gl_shader_type(Shader::Type v) {
	switch(v){
		case Shader::FRAGMENT:	return GL_FRAGMENT_SHADER;
		case Shader::VERTEX:	return GL_VERTEX_SHADER;
		case Shader::GEOMETRY:	return GL_GEOMETRY_SHADER_EXT;
		default: return 0;
	}
}

const char * ShaderBase::log() const {
//	GLint lsize; get(GL_INFO_LOG_LENGTH, &lsize);
//	if(0==lsize) return NULL;
//	newLog(lsize);
//	glGetShaderInfoLog(id(), 4096, NULL, mLog);
//	//glGetInfoLogARB((GLhandleARB)handle(), 4096, NULL, mLog);
//	return mLog;

	GLint lsize; get(GL_INFO_LOG_LENGTH, &lsize);
	if(0==lsize) return NULL;

	static char buf[AL_SHADER_MAX_LOG_SIZE];
	getLog(buf);
	return buf;
}

void ShaderBase::printLog() const {
	const char * s = log();
	if(s && s[0]) printf("%s\n", s);
}

void Shader::getLog(char * buf) const {
	glGetShaderInfoLog(id(), AL_SHADER_MAX_LOG_SIZE, NULL, buf);
}
void ShaderProgram::getLog(char * buf) const {
	glGetProgramInfoLog(id(), AL_SHADER_MAX_LOG_SIZE, NULL, buf);
}

/*
GLuint glCreateProgram (void);
GLuint glCreateShader (GLenum type);
void glDeleteShader (GLuint shader);
void glDeleteProgram (GLuint program);
void glDetachShader(GLuint program, GLuint shader);
*/

Shader::Shader(const std::string& source, Shader::Type type)
:	mSource(source), mType(type){}

Shader& Shader::compile(){
	AL_GRAPHICS_ERROR("(before Shader::compile)", id());
	validate(); // triggers a call to onCreate, if not created
	AL_GRAPHICS_ERROR("Shader::compile", id());
	return *this;
}

bool Shader::compiled() const {
	GLint v;
	GLhandleARB h = (GLhandleARB)id();
	glGetObjectParameterivARB(h, GL_COMPILE_STATUS, &v);
	//glGetProgramiv(id(), GL_COMPILE_STATUS, &v);
	AL_GRAPHICS_ERROR("Shader::compiled()", id());
	return v;
}

void Shader::get(int pname, void * params) const { glGetShaderiv(id(), pname, (GLint *)params); }

void Shader::onCreate(){
	AL_GRAPHICS_ERROR("(before Shader::onCreate)", id());
	mID = glCreateShader(gl_shader_type(mType));
	//printf("Create shader %lu\n",id());
	if(0 == id()){
		AL_WARN("Error creating shader object");
		return;
	}
	AL_GRAPHICS_ERROR("glCreateShader", id());

	if(mSource[0]){
		sendSource();
		AL_GRAPHICS_ERROR("Shader::sendSource", id());
		glCompileShader(id());
		AL_GRAPHICS_ERROR("glCompileShader", id());
	}
}

void Shader::onDestroy(){
	AL_GRAPHICS_ERROR("(before Shader::onDestroy)", id());
//printf("Destroy shader %lu\n", id());
	//glDeleteObjectARB((GLhandleARB)handle());
	glDeleteShader(id());
	AL_GRAPHICS_ERROR("glDeleteShader", id());
}

void Shader::sendSource(){
	validate();
	const char * s = mSource.c_str();
	glShaderSource(id(), 1, &s, NULL);
	//glShaderSourceARB((GLhandleARB)handle(), 1, &s, NULL);
}

Shader& Shader::source(const std::string& v){
	mSource = v;
	invalidate();
	return *this;
}

Shader& Shader::source(const std::string& src, Shader::Type type){
	mType=type; return source(src);
}


static ShaderProgram::Type param_type_from_gltype(GLenum gltype) {
	switch(gltype) {
		case GL_FLOAT:				return ShaderProgram::FLOAT;
		case GL_FLOAT_VEC2:			return ShaderProgram::VEC2;
		case GL_FLOAT_VEC3:			return ShaderProgram::VEC3;
		case GL_FLOAT_VEC4:			return ShaderProgram::VEC4;

		case GL_INT:				return ShaderProgram::INT;
		case GL_INT_VEC2:			return ShaderProgram::INT2;
		case GL_INT_VEC3:			return ShaderProgram::INT3;
		case GL_INT_VEC4:			return ShaderProgram::INT4;

		case GL_BOOL:				return ShaderProgram::BOOL;
		case GL_BOOL_VEC2:			return ShaderProgram::BOOL2;
		case GL_BOOL_VEC3:			return ShaderProgram::BOOL3;
		case GL_BOOL_VEC4:			return ShaderProgram::BOOL4;

		case GL_FLOAT_MAT2:			return ShaderProgram::MAT22;
		case GL_FLOAT_MAT3:			return ShaderProgram::MAT33;
		case GL_FLOAT_MAT4:			return ShaderProgram::MAT44;

		case GL_SAMPLER_1D:			return ShaderProgram::SAMPLER_1D;
		case GL_SAMPLER_2D:			return ShaderProgram::SAMPLER_2D;
		case GL_SAMPLER_2D_RECT_ARB:return ShaderProgram::SAMPLER_RECT;
		case GL_SAMPLER_3D:			return ShaderProgram::SAMPLER_3D;
		case GL_SAMPLER_CUBE:		return ShaderProgram::SAMPLER_CUBE;
		case GL_SAMPLER_1D_SHADOW:	return ShaderProgram::SAMPLER_1D_SHADOW;
		case GL_SAMPLER_2D_SHADOW:	return ShaderProgram::SAMPLER_2D_SHADOW;
		default:					return ShaderProgram::NONE;
	}
}

ShaderProgram::ShaderProgram()
:	mInPrim(Graphics::TRIANGLES), mOutPrim(Graphics::TRIANGLES), mOutVertices(3),
	mActive(true)
{}

ShaderProgram::~ShaderProgram(){
	destroy();
}

ShaderProgram& ShaderProgram::attach(Shader& s){
	validate();
	s.compile();
	glAttachObjectARB((GLhandleARB)id(), (GLhandleARB)s.id());
	//glAttachShader(id(), s.id());

	// TODO: check for geometry shader extensions
//#ifdef GL_EXT_geometry_shader4
//	printf("GL_EXT_geometry_shader4 defined\n");
//#endif
//#ifdef GL_ARB_geometry_shader4
//	printf("GL_ARB_geometry_shader4 defined\n");
//#endif

	if (s.type() == Shader::GEOMETRY) {
		glProgramParameteriEXT(id(),GL_GEOMETRY_INPUT_TYPE_EXT, mInPrim);
		glProgramParameteriEXT(id(),GL_GEOMETRY_OUTPUT_TYPE_EXT, mOutPrim);
		glProgramParameteriEXT(id(),GL_GEOMETRY_VERTICES_OUT_EXT,mOutVertices);
	}

	return *this;
}
const ShaderProgram& ShaderProgram::detach(const Shader& s) const {
	glDetachShader(id(), s.id());
	//glDetachObjectARB((GLhandleARB)handle(), (GLhandleARB)s.handle());
	return *this;
}
const ShaderProgram& ShaderProgram::link(bool doValidate) const {
	#if defined(GL_ARB_transform_feedback) || defined(GL_ARB_transform_feedback2) || defined(GL_ARB_transform_feedback3)
		#define TRANSFORM_FEEDBACK_EXT "GL_ARB_transform_feedback"
		#define transformFeedbackVaryings glTransformFeedbackVaryings
		#define INTERLEAVED_ATTRIBS GL_INTERLEAVED_ATTRIBS

	#elif defined(GL_EXT_transform_feedback) || defined(GL_EXT_transform_feedback2) || defined(GL_EXT_transform_feedback3)
		#define TRANSFORM_FEEDBACK_EXT "GL_EXT_transform_feedback"
		#define transformFeedbackVaryings glTransformFeedbackVaryingsEXT
		#define INTERLEAVED_ATTRIBS GL_INTERLEAVED_ATTRIBS_EXT
	#endif

	if(!mTFVaryings.empty()){
		#ifdef TRANSFORM_FEEDBACK_EXT
			if(Graphics::extensionSupported(TRANSFORM_FEEDBACK_EXT)){
				std::vector<const GLchar *> varyings;
				for(unsigned i=0; i<mTFVaryings.size(); ++i)
					varyings.push_back(mTFVaryings[i].c_str());
				transformFeedbackVaryings(
					id(), mTFVaryings.size(), &varyings[0], INTERLEAVED_ATTRIBS
				);
			} else {
				AL_WARN_ONCE("Platform does not support transform feedback (" TRANSFORM_FEEDBACK_EXT ")");
			}
		#else
			AL_WARN_ONCE("al::ShaderProgram built without support for transform feedback");
		#endif
	}

	glLinkProgram(id());
	if(doValidate) validateProgram();
	return *this;
}

bool ShaderProgram::validateProgram(bool doPrintLog) const {
	GLint isValid;
	glValidateProgram(id());
	glGetProgramiv(id(), GL_VALIDATE_STATUS, &isValid);
	if(GL_FALSE == isValid){
		AL_GRAPHICS_ERROR("ShaderProgram::link", id());
		if(doPrintLog) printLog();
		return false;
	}
	return true;
}

bool ShaderProgram::compile(
	const std::string& vertSource,
	const std::string& fragSource,
	const std::string& geomSource
){
	mVertSource = vertSource;
	mFragSource = fragSource;
	mGeomSource = geomSource;

	if(!created()) return false;

	Shader mShaderV, mShaderF, mShaderG;
	mShaderV.source(vertSource, al::Shader::VERTEX);
	attach(mShaderV);
	mShaderF.source(fragSource, al::Shader::FRAGMENT);
	attach(mShaderF);
	
	bool bGeom = geomSource[0];
	if(bGeom){
		mShaderG.source(geomSource, al::Shader::GEOMETRY);
		attach(mShaderG);
	}
	link(false);
	mShaderV.printLog();
	mShaderF.printLog();
	if(bGeom) mShaderG.printLog();
	printLog();

	// OpenGL.org says to detach shaders after linking:
	//   https://www.opengl.org/wiki/Shader_Compilation
	detach(mShaderV);
	detach(mShaderF);
	if(bGeom) detach(mShaderG);

	return linked();
}

void ShaderProgram::onCreate(){
	//mHandle = glCreateProgramObjectARB();
	//mID = (long)handle();
	mID = glCreateProgram();

	// Automatically compile any code set with ShaderProgram::compile
	if(!mVertSource.empty()){
		compile(mVertSource, mFragSource, mGeomSource);
	}
}
void ShaderProgram::onDestroy(){
	glDeleteProgram(id());
	//glDeleteObjectARB((GLhandleARB)handle());
}

void ShaderProgram::use(unsigned programID){
	glUseProgram(programID);
}

const ShaderProgram& ShaderProgram::use(){
	//if(active()){
		validate();
		use(id());
	//}
	//glUseProgramObjectARB((GLhandleARB)handle());
	return *this;
}

bool ShaderProgram::begin(){
	if(active()){
		use();
		return true;
	}
	return false;
}

void ShaderProgram::end() const {
	if(active()) glUseProgram(0);
	//glUseProgramObjectARB(0);
}

bool ShaderProgram::linked() const {
	GLint v;
	get(GL_LINK_STATUS, &v);
	return (v == GL_TRUE);
}
// GLint v; glGetProgramiv(id(), GL_LINK_STATUS, &v); return v; }

#define GET_LOC(map, glGetFunc)\
	int loc;\
	auto it = map.find(name);\
	if(it != map.end()){\
		loc = it->second;\
	}\
	else{\
		loc = glGetFunc(id(), name);\
		map[name] = loc;\
	}

int ShaderProgram::uniform(const char * name) const {
	GET_LOC(mUniformLocs, glGetUniformLocation);
	if(-1 == loc)
		AL_WARN_ONCE("No such uniform named \"%s\"", name);
	return loc;
}

int ShaderProgram::attribute(const char * name) const {
	GET_LOC(mAttribLocs, glGetAttribLocation);
	if(-1 == loc)
        AL_WARN_ONCE("No such attribute named \"%s\"", name);
	return loc;
}

const ShaderProgram& ShaderProgram::uniform(int loc, int v) const{
	glUniform1i(loc, v); return *this;
}
const ShaderProgram& ShaderProgram::uniform(int loc, float v) const{
	glUniform1f(loc, v); return *this;
}
const ShaderProgram& ShaderProgram::uniform(int loc, float v0, float v1) const{
	glUniform2f(loc, v0,v1); return *this;
}
const ShaderProgram& ShaderProgram::uniform(int loc, float v0, float v1, float v2) const{
	glUniform3f(loc, v0,v1,v2); return *this;
}
const ShaderProgram& ShaderProgram::uniform(int loc, float v0, float v1, float v2, float v3) const{
	glUniform4f(loc, v0,v1,v2,v3); return *this;
}
const ShaderProgram& ShaderProgram::uniformMatrix3(int loc, const float * v, bool transpose) const {
	glUniformMatrix3fv(loc, 1, transpose, v); return *this;
}
const ShaderProgram& ShaderProgram::uniformMatrix4(int loc, const float * v, bool transpose) const {
	glUniformMatrix4fv(loc, 1, transpose, v); return *this;
}
const ShaderProgram& ShaderProgram::uniform1(const char * name, const float * v, int count) const{
	glUniform1fv(uniform(name), count, v); return *this;
}
const ShaderProgram& ShaderProgram::uniform2(const char * name, const float * v, int count) const{
	glUniform2fv(uniform(name), count, v); return *this;
}
const ShaderProgram& ShaderProgram::uniform3(const char * name, const float * v, int count) const{
	glUniform3fv(uniform(name), count, v); return *this;
}
const ShaderProgram& ShaderProgram::uniform4(const char * name, const float * v, int count) const{
	glUniform4fv(uniform(name), count, v); return *this;
}
const ShaderProgram& ShaderProgram::uniform(const char * name, int v) const{
	return uniform(uniform(name), v);
}
const ShaderProgram& ShaderProgram::uniform(const char * name, float v) const{
	return uniform(uniform(name), v);
}
const ShaderProgram& ShaderProgram::uniform(const char * name, float v0, float v1) const{
	return uniform(uniform(name), v0,v1);
}
const ShaderProgram& ShaderProgram::uniform(const char * name, float v0, float v1, float v2) const{
	return uniform(uniform(name), v0,v1,v2);
}
const ShaderProgram& ShaderProgram::uniform(const char * name, float v0, float v1, float v2, float v3) const{
	return uniform(uniform(name), v0,v1,v2,v3);
}
const ShaderProgram& ShaderProgram::uniformMatrix3(const char * name, const float * v, bool transpose) const{
	return uniformMatrix3(uniform(name), v, transpose);
}
const ShaderProgram& ShaderProgram::uniformMatrix4(const char * name, const float * v, bool transpose) const{
	return uniformMatrix4(uniform(name), v, transpose);
}

const ShaderProgram& ShaderProgram::attribute(int loc, float v) const{
	glVertexAttrib1f(loc, v);	return *this;
}
const ShaderProgram& ShaderProgram::attribute(int loc, float v0, float v1) const{
	glVertexAttrib2f(loc, v0,v1); return *this;
}
const ShaderProgram& ShaderProgram::attribute(int loc, float v0, float v1, float v2) const{
	glVertexAttrib3f(loc, v0,v1,v2); return *this;
}
const ShaderProgram& ShaderProgram::attribute(int loc, float v0, float v1, float v2, float v3) const{
	glVertexAttrib4f(loc, v0,v1,v2,v3); return *this;
}
const ShaderProgram& ShaderProgram::attribute(const char * name, float v) const{
	return attribute(attribute(name), v);
}
const ShaderProgram& ShaderProgram::attribute(const char * name, float v0, float v1) const{
	return attribute(attribute(name), v0,v1);
}
const ShaderProgram& ShaderProgram::attribute(const char * name, float v0, float v1, float v2) const{
	return attribute(attribute(name), v0,v1,v2);
}
const ShaderProgram& ShaderProgram::attribute(const char * name, float v0, float v1, float v2, float v3) const{
	return attribute(attribute(name), v0,v1,v2,v3);
}
const ShaderProgram& ShaderProgram::attribute1(const char * name, const float * v) const{
	glVertexAttrib1fv(attribute(name), v); return *this;
}
const ShaderProgram& ShaderProgram::attribute2(const char * name, const float * v) const{
	glVertexAttrib2fv(attribute(name), v); return *this;
}
const ShaderProgram& ShaderProgram::attribute3(const char * name, const float * v) const{
	glVertexAttrib3fv(attribute(name), v); return *this;
}
const ShaderProgram& ShaderProgram::attribute4(const char * name, const float * v) const{
	glVertexAttrib4fv(attribute(name), v); return *this;
}
const ShaderProgram& ShaderProgram::attribute1(int loc, const double * v) const{
	glVertexAttrib1dv(loc, v); return *this;
}
const ShaderProgram& ShaderProgram::attribute2(int loc, const double * v) const{
	glVertexAttrib2dv(loc, v); return *this;
}
const ShaderProgram& ShaderProgram::attribute3(int loc, const double * v) const{
	glVertexAttrib3dv(loc, v); return *this;
}
const ShaderProgram& ShaderProgram::attribute4(int loc, const double * v) const{
	glVertexAttrib4dv(loc, v); return *this;
}


void ShaderProgram::get(int pname, void * params) const {
	glGetProgramiv(id(), pname, (GLint *)params);
}

void ShaderProgram::listParams() const {
	GLuint program = id();
	GLint numActiveUniforms = 0;
	GLint numActiveAttributes = 0;

	glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &numActiveUniforms);
	glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &numActiveAttributes);

	printf("ShaderProgram::listParams()\n");

	for(int j=0; j < numActiveUniforms; j++)
	{
		GLsizei length;
		GLint size;
		GLenum gltype;
		char name[256];

		glGetActiveUniform(program,
							j,
							sizeof(name),
							&length,
							&size,
							&gltype,
							name);

		// check for array names
		if(name[ strlen(name)-3 ] == '[' && name[ strlen(name)-1 ] == ']') {
			name[ strlen(name)-3 ] = '\0';
		}

		printf("uniform %d(%s): type %d size %d length %d\n",
			j, name, param_type_from_gltype(gltype), size, length);

//		//could already have a param if the user set some values before compiling
//		map<string, ShaderProgram *>::iterator it = mParameters.find(name);
//		if(it != mParameters.end()) {
//			ShaderProgram::Type type = param_type_from_gltype(gltype);
//			ShaderProgram &p = *(it->second);
//			p.set_active(true);
//			p.set_location(j);
//			p.set_type(type);
//			p.set_count(size);
//		}
		/*
		Only use params defined in shader file
		else
		{
			ShaderProgram *p = new ShaderProgram(name, j, type, size);
			mParameters[ name ] = p;
		}*/
	}

	for(int j=0; j < numActiveAttributes; j++) {
		GLsizei length;
		GLint size;
		GLenum gltype;
		char name[256];	// could query for max char length

		glGetActiveAttrib(program,
							j,
							sizeof(name),
							&length,
							&size,
							&gltype,
							name);

		printf("attribute %d(%s): type %d size %d length %d\n",
			j, name, param_type_from_gltype(gltype), size, length);

		//map<string, ShaderAttribute *>::iterator it = mAttributes.find(name);
//		if(it != mAttributes.end()) {
//			// TODO: FIX THIS HACK
//			#if defined(MURO_LINUX_VERSION)
//			int loc = (j < 0) ? 1 : j+1;
//			#else
//			int loc = (j <= 0) ? 1 : j;
//			#endif
//			ShaderProgram::Type type = param_type_from_gltype(gltype);
//			ShaderAttribute &a = *(it->second);
//			a.realize_location(loc);
//			a.set_type(type);
//			//a.setCount(size);
//		}
	}
}

} // al::
