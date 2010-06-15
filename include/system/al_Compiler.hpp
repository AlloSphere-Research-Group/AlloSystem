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

namespace al {

/*
	A wrapper around the LLVM/Clang APIs
*/	
class Compiler {
public: 
	struct Options {
		unsigned CPlusPlus; 
		std::vector<std::string> system_includes;
		std::vector<std::string> user_includes;
		
		Options() 
			: CPlusPlus(1) {}
	};
	
	Compiler::Options options;
	
	Compiler(std::string name = "untitled");
	~Compiler();

	/*
		successive compiles are linked together.
	*/
	bool compile(std::string code);	
	bool readbitcode(std::string path);	
	
	void * getfunctionptr(std::string funcname);
	void * getglobalptr(std::string globalname);
	
	bool writebitcode(std::string path);				

private:
	friend class CompilerImpl;
	class CompilerImpl * mImpl;
};


} // al::


#endif /* include guard */
