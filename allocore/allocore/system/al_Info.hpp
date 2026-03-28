#ifndef INC_AL_SYSTEM_INFO_HPP
#define INC_AL_SYSTEM_INFO_HPP

/*	Allocore -- Multimedia / virtual environment application class library

	Description:
	Various functions for retrieving information about the system

	Author(s):
	Lance Putnam, 2010, putnam.lance@gmail.com
	Graham Wakefield, 2011, grrrwaaa@gmail.com
*/

#include <string>

namespace al{

/// Get name of computer
std::string computerName();

/// Get name of current user
std::string userName();

/// Returns the number of processors available
///
/// @ingroup allocore
int numProcessors();

/// Returns true if the processor is the Sandy Bridge architecture
///
/// @ingroup allocore
bool is_sandy_bridge();

/// Valid only for OSX, when built as a .framework
/// Returns path to framework/Resources
///
/// @ingroup allocore
std::string frameworkResourcePath();

} // al::
#endif
