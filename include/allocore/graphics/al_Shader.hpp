#ifndef INCLUDE_AL_GRAPHICS_SHADER_HPP
#define INCLUDE_AL_GRAPHICS_SHADER_HPP

#include <string>
#include "allocore/graphics/al_GPUObject.hpp"

namespace al{

/// Shader abstract base class
class ShaderBase : public GPUObject{
public:
	ShaderBase(): mLog(0){}
	
	virtual ~ShaderBase(){ deleteLog(); }

	/// Returns info log or 0 if none.
	const char * log();

protected:
	char * mLog;
	void deleteLog(){ delete[] mLog; }
	void newLog(int size){ deleteLog(); mLog=new char[size]; }

	//virtual void get(int pname, GLint * params) = 0;
	virtual void get(int pname, void * params) const = 0;
};



/// Shader object
/// A shader object represents your source code. You are able to pass your source code to a shader object and compile the shader object. 
class Shader : public ShaderBase{
public:

	enum Type{
		VERTEX,
		FRAGMENT
	};

	Shader(const std::string& source="", Shader::Type type=FRAGMENT);
	
	/// This will automatically delete the shader object when it is no longer 
	/// attached to any program object.
	virtual ~Shader(){ destroy(); }

	Shader& source(const std::string& v);
	Shader& source(const std::string& v, Shader::Type type);
	Shader& compile();
	bool compiled() const;

private:
	std::string mSource;
	Shader::Type mType;
	void sendSource();

	virtual void get(int pname, void * params) const;
	
	virtual void onCreate();
	virtual void onDestroy();
};



/// Shader program object
/// A program object represents a useable part of render pipeline. 
/// Links shaders to one program object
class ShaderProgram : public ShaderBase{
public:
	ShaderProgram(){}
	
	/// Any attached shaders will automatically be detached, but not deleted.
	virtual ~ShaderProgram(){ destroy(); }
	
	/// input Shader s will be compiled if necessary:
	const ShaderProgram& attach(Shader& s);
	const ShaderProgram& detach(const Shader& s) const;
	
	const ShaderProgram& link() const;
	const ShaderProgram& use() const;
	
	void begin() const;
	void end() const;

	/// Returns whether program linked successfully.
	bool linked() const;

	const ShaderProgram& uniform(const char * name, int v0);
	const ShaderProgram& uniform(const char * name, float v0);
	int uniformLocation(const char * name) const;

protected:
	virtual void get(int pname, void * params) const;

	virtual void onCreate();
	virtual void onDestroy();
};

} // ::al

#endif
