#ifndef INCLUDE_AL_COMPILER_HPP
#define INCLUDE_AL_COMPILER_HPP

/*
 *  AlloSphere Research Group / Media Arts & Technology, UCSB, 2009
 */

/*
	Copyright (C) 2006-2008. The Regents of the University of California (REGENTS). 
	All Rights Reserved.

	Permission to use, copy, modify, distribute, and distribute modified versions
	of this software and its documentation without fee and without a signed
	licensing agreement, is hereby granted, provided that the above copyright
	notice, the list of contributors, this paragraph and the following two paragraphs 
	appear in all copies, modifications, and distributions.

	IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
	SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING
	OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF REGENTS HAS
	BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
	THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
	PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED
	HEREUNDER IS PROVIDED "AS IS". REGENTS HAS  NO OBLIGATION TO PROVIDE
	MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
*/

#include <vector>
#include <string>

/*
	A wrapper around the LLVM/Clang APIs
	
	
	Usage:
	
	std::string code = ...	// source to compile
	std::string path = ...  // additional include paths
	std::string funcname = ... // function to JIT from code
	
	JIT * jit;
	void (*fptr)();		
	
	// set up compiler:
	Compiler cc;
	cc.options.CPlusPlus = 1;
	cc.options.user_includes.push_back(path);
	
	// compile code
	if (cc.compile(code)) {
		// optimize it
		cc.optimize();
		// print out LLVM IR
		cc.dump();
		// load into execution engine (returns a JIT object)
		// the compiler is reset, and can be re-used
		// the JIT can be used to instantiate functions
		// these functions will be deleted when the JIT is deleted
		jit = cc.jit();
		if (jit) {
			// grab a function from the JIT object
			// this is where just-in-time machine code generation actually occurs
			fptr = (void (*)())(jit->getfunctionptr(funcname));
			if (fptr) {
				// call the generated function
				fptr();
			}
			// done with jitted code, delete it:
			delete jit;
			jit = NULL;
		}
	}
*/	

namespace al {

class JIT;
class ModuleImpl;

/*
	Compiler is responsible for compiling, reading, and linking different sources
*/	
class Compiler {
public: 
	struct Options {
		bool CPlusPlus; 
		std::vector<std::string> system_includes;
		std::vector<std::string> user_includes;
		
		Options() 
			: CPlusPlus(true) {}
	};
	Compiler::Options options;
	
	void cpp(bool v) { options.CPlusPlus = v; }
	void include(std::string path) { options.user_includes.push_back(path); }
	void system_include(std::string path) { options.system_includes.push_back(path); }
	
	Compiler();
	~Compiler();

	/*
		Note: successive compiles/bitcode reads are linked together.
	*/
	bool compile(std::string code);	
	bool readbitcode(std::string path);	
	bool writebitcode(std::string path);	
	
	/// valid olevels: O1, O2, O3
	void optimize(std::string olevel = "O2");
	
	/*
		discards any code compiled so far.
	*/
	void clear();
	
	/*
		transfers code compiled so far into the JIT engine 
		runs any static constructors for this code at this point
		calls clear() to reset this Compiler
	*/
	JIT * jit();	
	
	/*
		print module to stdout
	*/
	void dump();
				
private:
	friend class ModuleImpl;
	class ModuleImpl * mImpl;
};

/*
	JIT is responsible for retrieving pointers and functions 
	from the code registered with the run-time execution engine
*/	
class JIT {
private:
	friend class Compiler;
	JIT();
public:
	~JIT();	// when a JIT is freed, the functions/pointers associated with it are freed to
	
	// TODO: maybe also offer a per-function optimize here?
	void * getfunctionptr(std::string funcname);
	void * getglobalptr(std::string globalname);
	
	/*
		print module to stdout
	*/
	void dump();
	
	/* 
		JIT supports a reference-count mechanism
	*/
	void retain() { mRefs++; }
	void release() { if (--mRefs == 0) { unload(); } }
	
	/*
		Returns false if the JIT has no module (cannot emit functions or globals)
	*/
	bool valid() { return mImpl != NULL; }
	
private:
	void unload();

	friend class ModuleImpl;
	class ModuleImpl * mImpl;
	int mRefs;
};




} // al::


#endif /* include guard */
