#include "allocore/graphics/al_GraphicsOpenGL.hpp"
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
	if(s) printf("%s\n", s);
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
	GraphicsGL::gl_error("glerror compile0");
	validate(); 
	GraphicsGL::gl_error("glerror compile1");
	return *this; 
}

bool Shader::compiled() const {
	GLint v;
	GLhandleARB h = (GLhandleARB)id();
	glGetObjectParameterivARB(h, GL_COMPILE_STATUS, &v);
	//glGetProgramiv(id(), GL_COMPILE_STATUS, &v);
	GraphicsGL::gl_error("Shader::compiled()");
	return v;
}

void Shader::get(int pname, void * params) const { glGetShaderiv(id(), pname, (GLint *)params); }

void Shader::onCreate(){
	mID = glCreateShader(gl_shader_type(mType));
	GraphicsGL::gl_error("Shader::onCreate0");
	//mHandle = glCreateShaderObjectARB(gl_shader_type(mType));
	//mID = (long)handle();
	if(mSource[0]){
		sendSource(); 
		GraphicsGL::gl_error("Shader::onCreate1");
		glCompileShader(id());
		GraphicsGL::gl_error("Shader::onCreate2");
	}
}

void Shader::onDestroy(){ 
	//glDeleteObjectARB((GLhandleARB)handle());
	glDeleteShader(id()); 
}

void Shader::sendSource(){
	const char * s = mSource.c_str();
	glShaderSource(id(), 1, &s, NULL);
	//glShaderSourceARB((GLhandleARB)handle(), 1, &s, NULL);
}

Shader& Shader::source(const std::string& v){
	mSource = v;
	if(created()){
		sendSource();
		compile();
	}
	return *this;
}

Shader& Shader::source(const std::string& src, Shader::Type type){
	mType=type; return source(src);
}
	




ShaderProgram& ShaderProgram::attach(Shader& s){
	validate();
	s.compile();
	glAttachObjectARB((GLhandleARB)id(), (GLhandleARB)s.id());
	//glAttachShader(id(), s.id()); 
	
	if (s.type() == Shader::GEOMETRY) {
		glProgramParameteriEXT(id(),GL_GEOMETRY_INPUT_TYPE_EXT,GraphicsGL::gl_primitive(inPrim));
		glProgramParameteriEXT(id(),GL_GEOMETRY_OUTPUT_TYPE_EXT,GraphicsGL::gl_primitive(outPrim));
		glProgramParameteriEXT(id(),GL_GEOMETRY_VERTICES_OUT_EXT,outVertices);
	}
	
	return *this; 
}
const ShaderProgram& ShaderProgram::detach(const Shader& s) const { 
	glDetachShader(id(), s.id()); 
	//glDetachObjectARB((GLhandleARB)handle(), (GLhandleARB)s.handle());
	return *this; 
}
const ShaderProgram& ShaderProgram::link() const { 
	glLinkProgram(id()); 
	//glLinkProgramARB((GLhandleARB)handle());
	
	int isValid;
	glValidateProgram(id());
	//glValidateProgramARB((GLhandleARB)handle());
	glGetProgramiv(id(), GL_VALIDATE_STATUS, &isValid);
	if (!isValid) {
		GraphicsGL::gl_error("ShaderProgram::link");
	}
		
	return *this; 
}

void ShaderProgram::onCreate(){ 
	//mHandle = glCreateProgramObjectARB();
	//mID = (long)handle();
	mID = glCreateProgram(); 
}
void ShaderProgram::onDestroy(){ 
	glDeleteProgram(id()); 
	//glDeleteObjectARB((GLhandleARB)handle()); 
}

const ShaderProgram& ShaderProgram::use() const { 
	glUseProgram(id()); 
	//glUseProgramObjectARB((GLhandleARB)handle());
	return *this; 
}
void ShaderProgram::begin() const { 
	use(); 
}
void ShaderProgram::end() const { 
	glUseProgram(0); 
	//glUseProgramObjectARB(0);
}
bool ShaderProgram::linked() const { 
	GLint v; 
	get(GL_LINK_STATUS, &v); 
	return v; 
}
// GLint v; glGetProgramiv(id(), GL_LINK_STATUS, &v); return v; }

const ShaderProgram& ShaderProgram::uniform(const char * name, int v0) const{
	glUniform1i(uniformLocation(name), v0);	return *this;
}
const ShaderProgram& ShaderProgram::uniform(const char * name, float v0) const{
	glUniform1f(uniformLocation(name), v0);	return *this;
}
const ShaderProgram& ShaderProgram::uniform(const char * name, float v0, float v1) const{
	glUniform2f(uniformLocation(name), v0,v1); return *this;	
}
const ShaderProgram& ShaderProgram::uniform(const char * name, float v0, float v1, float v2) const{
	glUniform3f(uniformLocation(name), v0,v1,v2); return *this;	
}
const ShaderProgram& ShaderProgram::uniform(const char * name, float v0, float v1, float v2, float v3) const{
	glUniform4f(uniformLocation(name), v0,v1,v2,v3); return *this;	
}
const ShaderProgram& ShaderProgram::uniform1(const char * name, const float * v, int count) const{
	glUniform1fv(uniformLocation(name), count, v); return *this;
}
const ShaderProgram& ShaderProgram::uniform2(const char * name, const float * v, int count) const{
	glUniform2fv(uniformLocation(name), count, v); return *this;
}
const ShaderProgram& ShaderProgram::uniform3(const char * name, const float * v, int count) const{
	glUniform3fv(uniformLocation(name), count, v); return *this;
}
const ShaderProgram& ShaderProgram::uniform4(const char * name, const float * v, int count) const{
	glUniform4fv(uniformLocation(name), count, v); return *this;
}

const ShaderProgram& ShaderProgram::attribute(const char * name, float v0) const{
	glVertexAttrib1f(attributeLocation(name), v0);	return *this;
}
const ShaderProgram& ShaderProgram::attribute(const char * name, float v0, float v1) const{
	glVertexAttrib2f(attributeLocation(name), v0,v1); return *this;	
}
const ShaderProgram& ShaderProgram::attribute(const char * name, float v0, float v1, float v2) const{
	glVertexAttrib3f(attributeLocation(name), v0,v1,v2); return *this;	
}
const ShaderProgram& ShaderProgram::attribute(const char * name, float v0, float v1, float v2, float v3) const{
	glVertexAttrib4f(attributeLocation(name), v0,v1,v2,v3); return *this;	
}
const ShaderProgram& ShaderProgram::attribute1(const char * name, const float * v) const{
	glVertexAttrib1fv(attributeLocation(name), v); return *this;
}
const ShaderProgram& ShaderProgram::attribute2(const char * name, const float * v) const{
	glVertexAttrib2fv(attributeLocation(name), v); return *this;
}
const ShaderProgram& ShaderProgram::attribute3(const char * name, const float * v) const{
	glVertexAttrib3fv(attributeLocation(name), v); return *this;
}
const ShaderProgram& ShaderProgram::attribute4(const char * name, const float * v) const{
	glVertexAttrib4fv(attributeLocation(name), v); return *this;
}

int ShaderProgram::uniformLocation(const char * name) const { 
	//GLint loc = glGetUniformLocationARB((GLhandleARB)handle(), name);
	GLint loc = glGetUniformLocation(id(), name);
	if (loc == -1)
        printf("No such uniform named \"%s\"\n", name);
	return loc; 
}

int ShaderProgram::attributeLocation(const char * name) const { 
	//GLint loc = glGetAttribLocationARB((GLhandleARB)handle(), name);
	GLint loc = glGetAttribLocation(id(), name);
	if (loc == -1)
        printf("No such attribute named \"%s\"\n", name);
	return loc;  
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
							256,
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
							256,
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

ShaderProgram::Type ShaderProgram :: param_type_from_gltype(GLenum gltype) {
	ShaderProgram::Type type = ShaderProgram::NONE;

	switch(gltype) {
		case GL_FLOAT:			type = ShaderProgram::FLOAT;	break;
		case GL_FLOAT_VEC2:		type = ShaderProgram::VEC2;	break;
		case GL_FLOAT_VEC3:		type = ShaderProgram::VEC3;	break;
		case GL_FLOAT_VEC4:		type = ShaderProgram::VEC4;	break;

		case GL_INT:			type = ShaderProgram::INT;	break;
		case GL_INT_VEC2:		type = ShaderProgram::INT2;	break;
		case GL_INT_VEC3:		type = ShaderProgram::INT3;	break;
		case GL_INT_VEC4:		type = ShaderProgram::INT4;	break;

		case GL_BOOL:			type = ShaderProgram::BOOL;	break;
		case GL_BOOL_VEC2:		type = ShaderProgram::BOOL2;	break;
		case GL_BOOL_VEC3:		type = ShaderProgram::BOOL3;	break;
		case GL_BOOL_VEC4:		type = ShaderProgram::BOOL4;	break;

		case GL_FLOAT_MAT2:		type = ShaderProgram::MAT22;	break;
		case GL_FLOAT_MAT3:		type = ShaderProgram::MAT33;	break;
		case GL_FLOAT_MAT4:		type = ShaderProgram::MAT44;	break;

		case GL_SAMPLER_1D:		type = ShaderProgram::SAMPLER_1D;	break;
		case GL_SAMPLER_2D:		type = ShaderProgram::SAMPLER_2D;	break;
		case GL_SAMPLER_2D_RECT_ARB: type = ShaderProgram::SAMPLER_RECT; break;
		case GL_SAMPLER_3D:		type = ShaderProgram::SAMPLER_3D;	break;
		case GL_SAMPLER_CUBE:	type = ShaderProgram::SAMPLER_CUBE; break;
		case GL_SAMPLER_1D_SHADOW: type = ShaderProgram::SAMPLER_1D_SHADOW; break;
		case GL_SAMPLER_2D_SHADOW: type = ShaderProgram::SAMPLER_2D_SHADOW; break;
	}

	return type;
}

} // ::al


//Shader shader1;
//shader1.set(shaderBuf, GL_FRAGMENT_SHADER);
//
//ShaderProgram shaderProgram;
//
//shaderProgram.attach(shader1);
//shaderProgram.link().use();

/*

	char *vs = NULL,*fs = NULL,*fs2 = NULL;

	v = glCreateShader(GL_VERTEX_SHADER);
	f = glCreateShader(GL_FRAGMENT_SHADER);
	f2 = glCreateShader(GL_FRAGMENT_SHADER);

	vs = textFileRead("toon.vert");
	fs = textFileRead("toon.frag");
	fs2 = textFileRead("toon2.frag");

	const char * ff = fs;
	const char * ff2 = fs2;
	const char * vv = vs;

	glShaderSource(v, 1, &vv,NULL);
	glShaderSource(f, 1, &ff,NULL);
	glShaderSource(f2, 1, &ff2,NULL);

	free(vs);free(fs);

	glCompileShader(v);
	glCompileShader(f);
	glCompileShader(f2);

	p = glCreateProgram();
	glAttachShader(p,f);
	glAttachShader(p,f2);
	glAttachShader(p,v);

	glLinkProgram(p);
	glUseProgram(p);
*/
