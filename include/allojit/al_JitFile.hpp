#ifndef INCLUDE_AL_COMPILER_JITFILE_HPP
#define INCLUDE_AL_COMPILER_JITFILE_HPP

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

#include "allocore/system/al_Time.hpp"
#include "allojit/al_Compiler.hpp"	

#include <vector>
#include <list>
#include <string>

/*!
	Handling the loading of files via JIT.
	
	The external file loaded will be monitored in a background process, 
		and if modified, will be re-loaded.
	A JitZone is associated with each file; 
		this JitZone is destroyed when the file is reloaded.
		
	JitFile is templated on a type S, representing some global object passed to, 
		used by and stored in the JitFile
	
	Usage:
	
	// where template parameter S is mapped to object type World:
	JitFile<World> jitfile("myjitablefile.cpp", myWorld);
	
	// configure the JitFile's compiler:
	Compiler& cc = jitfile.compiler();
	cc.system_include(pathToClangHeaders);
	cc.include(pathToLibraryHeaders);
	cc.include(pathToAppHeaders);
	
	
	// myjitablefile.cpp should contain a function defined as follows:
	#include "al_Compiler.hpp"
	extern "C" JitZone * onload(JIT * jit, World * W) {
		
		// create a JitZone to store persistent jitted objects from this file
		JitZone * localZone = new JitZone(jit);

		// initialize all state local to this file, and store in localZone
		// ...
		
		// return zone to the JitFile caller, for later garbage collection
		return localZone;
	}
	
*/	

namespace al {

/*
	A Jitted file + file watching
*/
template <typename GlobalState>
class JitFile {
public:
	
	///! the entry point signature used in JITable files:
	typedef JitZone * (*jitEntryFptr)(JIT *, GlobalState *);
	
	///! constructor requires a full path and optional GlobalState object
	JitFile(std::string path, GlobalState * state=NULL);
	
	///! utility to retrieve the Compiler object (e.g. to set search paths)
	Compiler& compiler() { return mCC; }

protected:

	// file-watching background task:
	void tick(al_sec t);

//	std::vector<std::string> includes;
//	std::vector<std::string> system_includes;
	
	std::string mFilePath;	// path to file watched
	al_sec mMtime;			// last modification date of file
	Compiler mCC;			// compiler used to compile the file's contents
	JitZone * mJitZone;		// manage garbage collection of JITted code
	GlobalState * mState;	// global state passed to each JITted instance
};



/*
	INLINE IMPLEMENTATION
*/
#pragma mark -


template <typename GlobalState>
JitFile<GlobalState>::JitFile(std::string path, GlobalState * state) 
: mFilePath(path.data()), mMtime(0), mJitZone(NULL), mState(state)
{
	// launch the background process to monitor the file
	MainLoop::queue().send(MainLoop::now()+0.1, this, &JitFile::tick);
}

// background process monitoring and JITting the file
template <typename GlobalState>
void JitFile<GlobalState>::tick(al_sec t) {
	// if file exists
	if (File::exists(mFilePath.c_str())) {
		al_sec mtime = File::modified(mFilePath.c_str());
		// and current modification date differs form the stored modification date
		if (mMtime != mtime) {
			printf("JitFile(%s): opening\n", mFilePath.c_str());
			// store new modification date
			mMtime = mtime;
			
			// try to open and read the contents of the file
			File file(mFilePath.c_str(), "r");
			if (file.open()) {
				std::string code = file.readAll();
				file.close();
				
				// try to compile and JIT the file
				if (mCC.compile(code)) {
					mCC.optimize();
					//mCC.dump();
					JIT * jit = mCC.jit();
					if (jit) {
						// get the entry point into the file:
						jitEntryFptr f = (jitEntryFptr)(jit->getfunctionptr("onload"));
						if (f) {
						
							// dispose of the previous JITted instance of the file:
							if (mJitZone) {
								//printf("JitFile(%s): freeing JitZone %p\n", mFilePath.c_str(), mJitZone);
								delete mJitZone;
								mJitZone = NULL;
							}
							
							// initialized and acquire the new JITted instance of the file:
							mJitZone = f(jit, mState);
							//printf("JitFile(%s): captured JitZone %p\n", mFilePath.c_str(), mJitZone);
						} else {
							printf("JitFile(%s): onload handler not found\n", mFilePath.c_str());
						}
					}
				} else {
					printf("JitFile(%s): couldn't compile\n", mFilePath.c_str());
				}
			} else {
				printf("JitFile(%s): couldn't open file\n", mFilePath.c_str());
			}
		}
	} else {
		printf("JitFile(%s): file not found\n", mFilePath.c_str());
	}
	// wash and repeat
	MainLoop::queue().send(t+0.1, this, &JitFile::tick);
}


} // al::


#endif /* include guard */
