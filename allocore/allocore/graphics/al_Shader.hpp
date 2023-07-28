#ifndef INCLUDE_AL_GRAPHICS_SHADER_HPP
#define INCLUDE_AL_GRAPHICS_SHADER_HPP

/*	Allocore --
	Multimedia / virtual environment application class library

	Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology, UCSB.
	Copyright (C) 2012. The Regents of the University of California.
	All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

		Redistributions of source code must retain the above copyright notice,
		this list of conditions and the following disclaimer.

		Redistributions in binary form must reproduce the above copyright
		notice, this list of conditions and the following disclaimer in the
		documentation and/or other materials provided with the distribution.

		Neither the name of the University of California nor the names of its
		contributors may be used to endorse or promote products derived from
		this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
	IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
	ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
	LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
	CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
	POSSIBILITY OF SUCH DAMAGE.


	File description:
	Wrappers to OpenGL GLSL shaders

	File author(s):
	Lance Putnam, 2010, putnam.lance@gmail.com
	Graham Wakefield, 2010, grrrwaaa@gmail.com
	Wesley Smith, 2010, wesley.hoke@gmail.com
*/


#include "allocore/graphics/al_OpenGL.hpp"

#ifdef AL_GRAPHICS_SUPPORTS_SHADER

#include <string>
#include <unordered_map>
#include <vector>
#include "allocore/graphics/al_GPUObject.hpp"

#define AL_SHADER_MAX_LOG_SIZE	4096

namespace al{

template <int N, class T> class Vec;
template <class T> class Quat;

/// Shader abstract base class
/// @ingroup allocore
class ShaderBase : public GPUObject{
public:

	virtual ~ShaderBase(){}

	/// Returns info log or 0 if none
	const char * log() const;

	/// Prints info log, if any
	void printLog(const char * prepend = "") const;

protected:
	virtual void get(int pname, void * params) const = 0;
	virtual void getLog(char * buf) const = 0;
};



/// Shader object

/// A shader object represents your source code. You are able to pass your
/// source code to a shader object and compile the shader object.
/// @ingroup allocore
class Shader : public ShaderBase{
public:

	enum Type {
		VERTEX,
		FRAGMENT,
		GEOMETRY
	};

	Shader(const std::string& source="", Shader::Type type=FRAGMENT);

	/// This will automatically delete the shader object when it is no longer
	/// attached to any program object.
	virtual ~Shader(){ destroy(); }

	Shader& source(const std::string& v);
	Shader& source(const std::string& v, Shader::Type type);

	const std::string& source() const { return mSource; }

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
/// It links one or more shader units into a single program object.
/// @ingroup allocore
class ShaderProgram : public ShaderBase{
public:

	ShaderProgram();

	/** Construct with shader code
	
	Example pass-thru GLSL shader:

		\code{.cpp}
		ShaderProgram shader{
		R"(
			varying vec3 vcol;
		)", R"(
			uniform mat4 MVP;
			void main(){
				vcol = gl_Color.rgb;
				gl_Position = MVP * gl_Vertex;
			}
		)", R"(
			void main(){
				vec3 col = vcol;
				gl_FragColor = vec4(col, 1.);
			}
		)"};
		\endcode
	*/
	ShaderProgram(
		const std::string& preambleCode,
		const std::string& vertexCode,
		const std::string& fragmentCode
	);
	

	/// Any attached shaders will automatically be detached, but not deleted.
	virtual ~ShaderProgram();


	/// Attach shader to program

	/// The input shader will be compiled if necessary.
	///
	ShaderProgram& attach(Shader& s);

	/// Detach shader from program
	const ShaderProgram& detach(const Shader& s) const;

	/// Link attached shaders

	/// @param[in] doValidate	validate program after linking;
	///		You might not want to do this if you need to set uniforms before
	///		validating, e.g., when using different texture sampler types in the
	///		same shader.
	const ShaderProgram& link(bool doValidate=true) const;


	/// Set preamble to be inserted before sources specified by compile

	/// This is useful for inserting code that is used across all shader stages,
	/// e.g. version, common uniforms, etc.
	ShaderProgram& preamble(const std::string& s){ mPreamble=s; return *this; }

	/// Add #version directive
	ShaderProgram& version(const std::string& v){ mVersion=v; return *this; }
	ShaderProgram& version(int n){ return version(std::to_string(n)); }

	/// Name of shader program (for debugging)
	ShaderProgram& name(const std::string& v){ mName=v; return *this; }
	const std::string& name() const { return mName; }

	/// Compile and link shader sources

	/// If called outside of a graphics context, compiling and linking will
	/// occur once the first time the shader is bound.
	bool compile(
		const std::string& vertSource,
		const std::string& fragSource,
		const std::string& geomSource=""
	);

	/// Compile and link shader source
	
	/// This version is the same as compile(), but takes a single string with 
	/// different shader stages separated by preprocessor conditionals.
	bool compileUnified(
		const std::string& source,
		const std::string& vertMacro = "VERT_PROG",
		const std::string& fragMacro = "FRAG_PROG",
		const std::string& geomMacro = "GEOM_PROG"
	);

	const std::string& sourceVert() const { return mVertSource; }
	const std::string& sourceFrag() const { return mFragSource; }
	const std::string& sourceGeom() const { return mGeomSource; }

	const ShaderProgram& use();

	/// Get whether program is active
	bool active() const { return mActive; }

	/// Set whether program is active
	ShaderProgram& active(bool v){ mActive=v; return *this; }

	/// Toggle active state
	ShaderProgram& toggleActive(){ mActive^=true; return *this; }

	/// Begin use of shader program
	bool begin();

	/// End use of shader program
	void end();

	/// Call a function wrapped inside begin/end calls

	/// The function should have one argument---a mutable reference to a 
	/// ShaderProgram (which will receive *this).
	template <class Func>
	ShaderProgram& scope(const Func& f){
		begin(); f(*this); end();
		return *this;
	}


	/// Returns whether program linked successfully
	bool linked() const;

	/// Returns whether linked program can execute in current graphics state
	bool validateProgram(bool printLog=false) const;

	/// Returns true on first call to begin after compile, otherwise false

	/// This is useful for initializing uniforms.
	///
	bool once() const { return mOnce; }

	/// Call a function wrapped inside begin/end calls once after compile

	/// The function should have one argument---a mutable reference to a 
	/// ShaderProgram (which will receive *this).
	template <class Func>
	ShaderProgram& scopeOnce(const Func& f){
		if(once()){ scope([&f](auto& s){ f(s); }); }
		return *this;
	}


	/// Set parameters for geometry shader

	/// @param[in] inPrim	Input primitive to geometry shader
	/// @param[in] outPrim	Output primitive from geometry shader
	/// @param[in] outVert	Number of vertices output from geometry shader 
	///						(note the number of input vertices is always 1)
	///
	/// This must be called before attaching the geometry shader.
	/// For GLSL, the geometry shader must include the line:
	///		#extension GL_EXT_geometry_shader4 : enable
	ShaderProgram& setGeometry(int inPrim, int outPrim, unsigned outVert){
		mInPrim=inPrim; mOutPrim=outPrim; mOutVertices=outVert; return *this;
	}

	// These parameters must be set before attaching geometry shaders
	ShaderProgram& setGeometryInputPrimitive(int prim){ mInPrim = prim; return *this; }
	ShaderProgram& setGeometryOutputPrimitive(int prim){ mOutPrim = prim; return *this; }
	ShaderProgram& setGeometryOutputVertices(unsigned i){ mOutVertices = i; return *this; }

	std::vector<std::string>& transformFeedbackVaryings(){ return mTFVaryings; }
	const std::vector<std::string>& transformFeedbackVaryings() const { return mTFVaryings; }

	/// Print out all the input parameters to the shader
	void listParams() const;

	/// Get location of uniform
	int uniform(const char * name) const;

	/// Get location of attribute
	int attribute(const char * name) const;


	/// Set uniform via location
	template <typename T>
	const ShaderProgram& uniform(int loc, const T& v) const;

	/// Set uniform via name
	template <typename T>
	const ShaderProgram& uniform(const char * name, const T& v) const {
		return uniform(uniform(name), v);
	}

	// Terminal case for below...
	const ShaderProgram& uniform() const { return *this; }

	/// Set multiple uniforms via location or name
	template <class Loc, class Val, class... Rest>
	const ShaderProgram& uniform(Loc l, const Val& v, Rest&&... rest) const {
		return uniform(l,v).uniform(std::forward<Rest>(rest)...);
	}


	/// Set uniform via location from fixed-sized vector/array
	template <typename VecN>
	const ShaderProgram& uniformVec(int loc, const VecN& v) const {
		constexpr auto N = sizeof(v)/sizeof(v[0]);
		static_assert(1<=N && N<=4, "Vector must have 1-4 elements");
		switch(N){
		case 1: return uniform<typename VecN::value_type>(loc, v[0]); 
		case 2: return uniform(loc, v[0], v[1]); 
		case 3: return uniform(loc, v[0], v[1], v[2]); 
		case 4: return uniform(loc, v[0], v[1], v[2], v[3]);
		default:;
		}
		return *this;
	}

	/// Set uniform via name from fixed-sized vector/array
	template <typename VecN>
	const ShaderProgram& uniformVec(const char * name, const VecN& v) const {
		return uniformVec(uniform(name), v); }


	const ShaderProgram& uniform(int loc, float v0, float v1) const;
	const ShaderProgram& uniform(int loc, float v0, float v1, float v2) const;
	const ShaderProgram& uniform(int loc, float v0, float v1, float v2, float v3) const;

	const ShaderProgram& uniformMatrix3(int loc, const float * v, bool transpose=false) const;
	const ShaderProgram& uniformMatrix4(int loc, const float * v, bool transpose=false) const;

	const ShaderProgram& uniform(const char * name, float v0, float v1) const;
	const ShaderProgram& uniform(const char * name, float v0, float v1, float v2) const;
	const ShaderProgram& uniform(const char * name, float v0, float v1, float v2, float v3) const;

	const ShaderProgram& uniform1(const char * name, const float * v, int count=1) const;
	const ShaderProgram& uniform2(const char * name, const float * v, int count=1) const;
	const ShaderProgram& uniform3(const char * name, const float * v, int count=1) const;
	const ShaderProgram& uniform4(const char * name, const float * v, int count=1) const;

	const ShaderProgram& uniformMatrix3(const char * name, const float * v, bool transpose=false) const;
	const ShaderProgram& uniformMatrix4(const char * name, const float * v, bool transpose=false) const;


	const ShaderProgram& attribute(int loc, float v) const;
	const ShaderProgram& attribute(int loc, float v0, float v1) const;
	const ShaderProgram& attribute(int loc, float v0, float v1, float v2) const;
	const ShaderProgram& attribute(int loc, float v0, float v1, float v2, float v3) const;

	const ShaderProgram& attribute(const char * name, float v) const;
	const ShaderProgram& attribute(const char * name, float v0, float v1) const;
	const ShaderProgram& attribute(const char * name, float v0, float v1, float v2) const;
	const ShaderProgram& attribute(const char * name, float v0, float v1, float v2, float v3) const;

	const ShaderProgram& attribute1(const char * name, const float * v) const;
	const ShaderProgram& attribute2(const char * name, const float * v) const;
	const ShaderProgram& attribute3(const char * name, const float * v) const;
	const ShaderProgram& attribute4(const char * name, const float * v) const;
	const ShaderProgram& attribute1(int loc, const double * v) const;
	const ShaderProgram& attribute2(int loc, const double * v) const;
	const ShaderProgram& attribute3(int loc, const double * v) const;
	const ShaderProgram& attribute4(int loc, const double * v) const;

	template<typename T>
	const ShaderProgram& attribute(int loc, const Vec<2,T>& v) const {
		return attribute(loc, v.x, v.y);
	}
	template<typename T>
	const ShaderProgram& attribute(int loc, const Vec<3,T>& v) const {
		return attribute(loc, v.x, v.y, v.z);
	}
	template<typename T>
	const ShaderProgram& attribute(int loc, const Vec<4,T>& v) const {
		return attribute(loc, v.x, v.y, v.z, v.w);
	}
	template<typename T>
	const ShaderProgram& attribute(int loc, const Quat<T>& q) const {
		// note wxyz => xyzw for GLSL vec4:
		return attribute(loc, q.x, q.y, q.z, q.w);
	}

	/// Set whether to print warnings
	static void warnings(bool v){ warnings() = v; }
	static bool& warnings(){ static bool v = false; return v; }

	static void use(unsigned programID);

protected:
	int mInPrim, mOutPrim;	// IO primitives for geometry shaders
	unsigned int mOutVertices = 3;
	std::string mVertSource, mFragSource, mGeomSource;
	mutable std::unordered_map<std::string, int> mUniformLocs, mAttribLocs;
	std::vector<std::string> mTFVaryings;
	std::string mName;
	std::string mVersion;
	std::string mPreamble;
	bool mActive = true;
	bool mOnce = true;

	std::string idString() const;
	
	virtual void get(int pname, void * params) const;
	virtual void getLog(char * buf) const;

	virtual void onCreate();
	virtual void onDestroy();
};

} // ::al

#else // AL_GRAPHICS_SUPPORTS_SHADER not def'd

// Dummy class
class ShaderProgram{};

#endif

#endif
