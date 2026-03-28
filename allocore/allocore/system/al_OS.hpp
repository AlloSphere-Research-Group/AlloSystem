#ifndef INC_AL_OS_HPP
#define INC_AL_OS_HPP

/*	Allocore -- Multimedia / virtual environment application class library

	Description:
	Various functions for communicating with and sending requests to the OS

	Author(s):
	Lance Putnam, 2024
*/

namespace al{

/// Control screensaver or other display idling behavior

/// \param[in] whether		Whether app requires display. If true, resets
///							display idle timer. If false, allows display to idle.
/// \param[in] continuous	If true, display idle reset is continuous. If false,
///							display idle reset is called once.
void requiresDisplay(bool whether, bool continuous = true);

} // al::
#endif
