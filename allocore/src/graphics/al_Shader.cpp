#include "allocore/graphics/al_Graphics.hpp"

#ifdef AL_GRAPHICS_SUPPORTS_SHADER

#include <cstdio>
#include <cstring> // strlen
#include "allocore/math/al_Mat.hpp"
#include "allocore/math/al_Quat.hpp"
#include "allocore/math/al_Vec.hpp"
#include "allocore/types/al_Color.hpp"
#include "allocore/system/al_Printing.hpp"
#include "allocore/graphics/al_Shader.hpp"

namespace al{

// Backend specific
#ifdef AL_GRAPHICS_USE_OPENGL
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

#else
const ShaderProgram& ShaderProgram::attribute1(int loc, const double * v) const{
	AL_WARN_ONCE("Attribute not set (glVertexAttrib1dv not supported)"); return *this;
}
const ShaderProgram& ShaderProgram::attribute2(int loc, const double * v) const{
	AL_WARN_ONCE("Attribute not set (glVertexAttrib2dv not supported)"); return *this;
}
const ShaderProgram& ShaderProgram::attribute3(int loc, const double * v) const{
	AL_WARN_ONCE("Attribute not set (glVertexAttrib3dv not supported)"); return *this;
}
const ShaderProgram& ShaderProgram::attribute4(int loc, const double * v) const{
	AL_WARN_ONCE("Attribute not set (glVertexAttrib4dv not supported)"); return *this;
}

#endif


GLenum gl_shader_type(Shader::Type v) {
	switch(v){
		case Shader::FRAGMENT:	return GL_FRAGMENT_SHADER;
		case Shader::VERTEX:	return GL_VERTEX_SHADER;
		#ifdef AL_GRAPHICS_SUPPORTS_GEOMETRY_SHADER
		case Shader::GEOMETRY:	return GL_GEOMETRY_SHADER_EXT;
		#endif
		default: return 0;
	}
}

const char * ShaderBase::log() const {
	GLint lsize; get(GL_INFO_LOG_LENGTH, &lsize);
	if(0==lsize) return NULL;
	static char buf[AL_SHADER_MAX_LOG_SIZE];
	getLog(buf);
	return buf;
}

void ShaderBase::printLog(const char * prepend) const {
	const char * s = log();
	if(s && s[0]) printf("%s\n%s\n", prepend, s);
}

void Shader::getLog(char * buf) const {
	glGetShaderInfoLog(id(), AL_SHADER_MAX_LOG_SIZE, NULL, buf);
}
void ShaderProgram::getLog(char * buf) const {
	glGetProgramInfoLog(id(), AL_SHADER_MAX_LOG_SIZE, NULL, buf);
}


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
	glGetShaderiv(id(), GL_COMPILE_STATUS, &v);
	AL_GRAPHICS_ERROR("Shader::compiled()", id());
	return v;
}

void Shader::get(int pname, void * params) const {
	glGetShaderiv(id(), pname, (GLint *)params);
}

void Shader::onCreate(){
	AL_GRAPHICS_ERROR("(before Shader::onCreate)", id());
	auto glType = gl_shader_type(mType);
	if(glType){ // guard against unsupported shader types
		mID = glCreateShader(glType);
	}
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
	glDeleteShader(id());
	AL_GRAPHICS_ERROR("glDeleteShader", id());
}

void Shader::sendSource(){
	validate();
	const char * s = mSource.c_str();
	glShaderSource(id(), 1, &s, NULL);
}

Shader& Shader::source(const std::string& v){
	mSource = v;
	invalidate();
	return *this;
}

Shader& Shader::source(const std::string& src, Shader::Type type){
	mType=type;
	return source(src);
}


ShaderProgram::ShaderProgram()
:	mInPrim(Graphics::TRIANGLES), mOutPrim(Graphics::TRIANGLES)
{}

ShaderProgram::ShaderProgram(
	const std::string& preambleCode,
	const std::string& vertexCode,
	const std::string& fragmentCode
) : ShaderProgram()
{
	preamble(preambleCode);
	compile(vertexCode, fragmentCode);
}

ShaderProgram::~ShaderProgram(){
	destroy();
}

ShaderProgram& ShaderProgram::attach(Shader& s){
	validate();
	s.compile();
	glAttachShader(id(), s.id());

	/* TODO: check for geometry shader extensions
	#ifdef GL_EXT_geometry_shader4
		printf("GL_EXT_geometry_shader4 defined\n");
	#endif
	#ifdef GL_ARB_geometry_shader4
		printf("GL_ARB_geometry_shader4 defined\n");
	#endif
	//*/

	#ifdef AL_GRAPHICS_SUPPORTS_GEOMETRY_SHADER
	if (s.type() == Shader::GEOMETRY) {
		glProgramParameteriEXT(id(),GL_GEOMETRY_INPUT_TYPE_EXT, mInPrim);
		glProgramParameteriEXT(id(),GL_GEOMETRY_OUTPUT_TYPE_EXT, mOutPrim);
		glProgramParameteriEXT(id(),GL_GEOMETRY_VERTICES_OUT_EXT,mOutVertices);
	}
	#endif

	return *this;
}
const ShaderProgram& ShaderProgram::detach(const Shader& s) const {
	glDetachShader(id(), s.id());
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

std::string ShaderProgram::idString() const {
	return mName.empty() ? std::to_string(id()) : "\""+mName+"\"";
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

	mUniformLocs.clear();
	mAttribLocs.clear();

	std::string ver = mVersion.empty() ? "" : "#version " + mVersion + "\n";
	auto pre = ver + mPreamble;

	Shader mShaderV, mShaderF, mShaderG;
	mShaderV.source(pre + vertSource, al::Shader::VERTEX);
	attach(mShaderV);
	mShaderF.source(pre + fragSource, al::Shader::FRAGMENT);
	attach(mShaderF);
	
	bool bGeom = geomSource[0];
	if(bGeom){
		mShaderG.source(pre + geomSource, al::Shader::GEOMETRY);
		attach(mShaderG);
	}
	link(false);
	auto idStr = "ShaderProgram " + idString();
	mShaderV.printLog((idStr + " vertex log:").c_str());
	mShaderF.printLog((idStr + " fragment log:").c_str());
	if(bGeom) mShaderG.printLog((idStr + " geometry log:").c_str());
	//printLog(); // program log is usually not helpful or is redundant

	// OpenGL.org says to detach shaders after linking:
	//   https://www.opengl.org/wiki/Shader_Compilation
	detach(mShaderV);
	detach(mShaderF);
	if(bGeom) detach(mShaderG);

	mOnce = true;

	return linked();
}

bool ShaderProgram::compileUnified(
	const std::string& source,
	const std::string& vertMacro,
	const std::string& fragMacro,
	const std::string& geomMacro
){
	bool hasGeom = source.find(geomMacro) != std::string::npos;
	return compile(
		"#define " + vertMacro + "\n" + source,
		"#define " + fragMacro + "\n" + source,
		hasGeom ? "#define " + geomMacro + "\n" + source : ""
	);
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

void ShaderProgram::end(){
	if(active()){
		glUseProgram(0);
		mOnce = false;
	}
}

bool ShaderProgram::linked() const {
	GLint v;
	get(GL_LINK_STATUS, &v);
	return (v == GL_TRUE);
}

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
	if(warnings() && -1 == loc)
		AL_WARN_ONCE("ShaderProgram %s has no uniform \"%s\"", idString().c_str(), name);
	return loc;
}

int ShaderProgram::attribute(const char * name) const {
	GET_LOC(mAttribLocs, glGetAttribLocation);
	if(warnings() && -1 == loc)
        AL_WARN_ONCE("ShaderProgram %s has no attribute \"%s\"", idString().c_str(), name);
	return loc;
}

#define DEF_UNIFORM(T, suf)\
template<> const ShaderProgram& ShaderProgram::uniform<T>(int loc, const T& v) const {\
	glUniform1##suf(loc, v); return *this;\
}

DEF_UNIFORM(float, f)
DEF_UNIFORM(double, f)
DEF_UNIFORM(int, i)
DEF_UNIFORM(bool, i)

template<> const ShaderProgram& ShaderProgram::uniform<Vec2f>(int loc, const Vec2f& v) const {
	return uniformVec(loc, v);
}
template<> const ShaderProgram& ShaderProgram::uniform<Vec3f>(int loc, const Vec3f& v) const {
	return uniformVec(loc, v);
}
template<> const ShaderProgram& ShaderProgram::uniform<Vec4f>(int loc, const Vec4f& v) const {
	return uniformVec(loc, v);
}
template<> const ShaderProgram& ShaderProgram::uniform<Vec2d>(int loc, const Vec2d& v) const {
	return uniform(loc, Vec2f(v));
}
template<> const ShaderProgram& ShaderProgram::uniform<Vec3d>(int loc, const Vec3d& v) const {
	return uniform(loc, Vec3f(v));
}
template<> const ShaderProgram& ShaderProgram::uniform<Vec4d>(int loc, const Vec4d& v) const {
	return uniform(loc, Vec4f(v));
}
template<> const ShaderProgram& ShaderProgram::uniform<Color>(int loc, const Color& v) const {
	return uniformVec(loc, v);
}
template<> const ShaderProgram& ShaderProgram::uniform<RGB>(int loc, const RGB& v) const {
	return uniformVec(loc, v);
}

template<> const ShaderProgram& ShaderProgram::uniform<Mat3f>(int loc, const Mat3f& v) const {
	return uniformMatrix3(loc, &v[0]);
}
template<> const ShaderProgram& ShaderProgram::uniform<Mat3d>(int loc, const Mat3d& v) const {
	return uniform(loc, Mat3f(v));
}

template<> const ShaderProgram& ShaderProgram::uniform<Mat4f>(int loc, const Mat4f& v) const {
	return uniformMatrix4(loc, &v[0]);
}
template<> const ShaderProgram& ShaderProgram::uniform<Mat4d>(int loc, const Mat4d& v) const {
	return uniform(loc, Mat4f(v));
}
template<> const ShaderProgram& ShaderProgram::uniform<Matrix4f>(int loc, const Matrix4f& v) const {
	return uniform(loc, Mat4f(v));
}
template<> const ShaderProgram& ShaderProgram::uniform<Matrix4d>(int loc, const Matrix4d& v) const {
	return uniform(loc, Mat4f(v));
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

void ShaderProgram::get(int pname, void * params) const {
	glGetProgramiv(id(), pname, (GLint *)params);
}

void ShaderProgram::listParams() const {
	GLuint program = id();
	GLint numActiveUniforms = 0;
	GLint numActiveAttributes = 0;

	glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &numActiveUniforms);
	glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &numActiveAttributes);

	char name[256];	// could query for max char length

	printf("ShaderProgram::listParams()\n");

	for(int j=0; j < numActiveUniforms; j++){
		GLsizei length;
		GLint size;
		GLenum gltype;

		glGetActiveUniform(program,	j, sizeof(name), &length, &size, &gltype, name);

		// check for array names
		int nameLen = strlen(name);
		if(name[nameLen-3] == '[' && name[nameLen-1] == ']') {
			name[nameLen-3] = '\0';
		}

		printf("uniform %d(%s): type %d size %d length %d\n",
			j, name, gltype, size, length);
	}

	for(int j=0; j < numActiveAttributes; j++) {
		GLsizei length;
		GLint size;
		GLenum gltype;

		glGetActiveAttrib(program, j, sizeof(name), &length, &size, &gltype, name);

		printf("attribute %d(%s): type %d size %d length %d\n",
			j, name, gltype, size, length);
	}
}

} // al::

#endif //AL_GRAPHICS_SUPPORTS_SHADER
