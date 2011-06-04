#ifndef INCLUDE_AL_GRAPHICS_SHADER_HPP
#define INCLUDE_AL_GRAPHICS_SHADER_HPP

#include <string>
#include "allocore/graphics/al_GPUObject.hpp"
#include "allocore/graphics/al_Graphics.hpp"

#define AL_SHADER_MAX_LOG_SIZE	4096

namespace al{

/// Shader abstract base class
class ShaderBase : public GPUObject{
public:

	virtual ~ShaderBase(){}

	/// Returns info log or 0 if none
	const char * log() const;

	/// Prints info log, if any
	void printLog() const;

protected:
	virtual void get(int pname, void * params) const = 0;
	virtual void getLog(char * buf) const = 0;
};



/// Shader object

/// A shader object represents your source code. You are able to pass your 
/// source code to a shader object and compile the shader object. 
class Shader : public ShaderBase{
public:

	enum Type {
		VERTEX,
		GEOMETRY,
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
	
	Shader::Type type() const { return mType; }

private:
	std::string mSource;
	Shader::Type mType;
	void sendSource();

	virtual void get(int pname, void * params) const;
	virtual void getLog(char * buf) const;

	virtual void onCreate();
	virtual void onDestroy();	
};



/// Shader program object

/// A program object represents a useable part of render pipeline. 
/// Links shaders to one program object
class ShaderProgram : public ShaderBase{
public:

	/*!
		The basic parameter types
	*/
	enum Type {
		NONE = 0,	//uninitialized type

		FLOAT,		///< A single float value
		VEC2,		///< Two float values
		VEC3,		///< Three float values
		VEC4,		///< Four float values

		INT,		///< A single int value
		INT2,		///< Two int values
		INT3,		///< Three int values
		INT4,		///< Four int values

		BOOL,		///< A single bool value
		BOOL2,		///< Two bool values
		BOOL3,		///< Three bool values
		BOOL4,		///< Four bool values

		MAT22,		///< A 2x2 matrix
		MAT33,		///< A 3x3 matrix
		MAT44,		///< A 4x4 matrix

		SAMPLER_1D,			///< A 1D texture
		SAMPLER_2D,			///< A 2D texture
		SAMPLER_RECT,		///< A rectangular texture
		SAMPLER_3D,			///< A 3D texture
		SAMPLER_CUBE,		///< A cubemap texture
		SAMPLER_1D_SHADOW,	///< A 1D depth texture
		SAMPLER_2D_SHADOW	///< A 2D depth texture

		//textures? non square matrices? attributes?
	};

	ShaderProgram()
	:	mInPrim(Graphics::TRIANGLES), mOutPrim(Graphics::TRIANGLES), mOutVertices(3),
		mActive(true)
	{}
	
	/// Any attached shaders will automatically be detached, but not deleted.
	virtual ~ShaderProgram(){ destroy(); }
	
	/// input Shader s will be compiled if necessary:
	ShaderProgram& attach(Shader& s);
	const ShaderProgram& detach(const Shader& s) const;
	
	// These parameters must be set before attaching geometry shaders
	void setGeometryInputPrimitive(Graphics::Primitive prim) { mInPrim = prim; }
	void setGeometryOutputPrimitive(Graphics::Primitive prim) { mOutPrim = prim; }
	void setGeometryOutputVertices(unsigned int i) { mOutVertices = i; }

	const ShaderProgram& link() const;
	const ShaderProgram& use() const;

	/// Get whether program is active
	bool active() const { return mActive; }

	/// Set whether program is active
	ShaderProgram& active(bool v){ mActive=v; return *this; }	

	/// Toggle active state
	ShaderProgram& toggleActive(){ mActive^=true; return *this; }

	void begin() const;
	void end() const;

	/// Returns whether program linked successfully.
	bool linked() const;

	void listParams() const;
	const ShaderProgram& uniform(const char * name, int v0) const;
	const ShaderProgram& uniform(const char * name, float v0) const;
	const ShaderProgram& uniform(const char * name, double v0) const { return uniform(name, (float)v0); }
	const ShaderProgram& uniform(const char * name, float v0, float v1) const;
	const ShaderProgram& uniform(const char * name, float v0, float v1, float v2) const;
	const ShaderProgram& uniform(const char * name, float v0, float v1, float v2, float v3) const;

	const ShaderProgram& uniform1(const char * name, const float * v, int count=1) const;
	const ShaderProgram& uniform2(const char * name, const float * v, int count=1) const;
	const ShaderProgram& uniform3(const char * name, const float * v, int count=1) const;
	const ShaderProgram& uniform4(const char * name, const float * v, int count=1) const;

	const ShaderProgram& attribute(const char * name, float v0) const;
	const ShaderProgram& attribute(const char * name, float v0, float v1) const;
	const ShaderProgram& attribute(const char * name, float v0, float v1, float v2) const;
	const ShaderProgram& attribute(const char * name, float v0, float v1, float v2, float v3) const;

	const ShaderProgram& attribute1(const char * name, const float * v) const;
	const ShaderProgram& attribute2(const char * name, const float * v) const;
	const ShaderProgram& attribute3(const char * name, const float * v) const;
	const ShaderProgram& attribute4(const char * name, const float * v) const;
	
	int uniformLocation(const char * name) const;
	int attributeLocation(const char * name) const;

protected:
	Graphics::Primitive mInPrim, mOutPrim;	// IO primitives for geometry shaders
	unsigned int mOutVertices;
	bool mActive;
	
	virtual void get(int pname, void * params) const;
	virtual void getLog(char * buf) const;

	virtual void onCreate();
	virtual void onDestroy();
};

} // ::al

#endif
