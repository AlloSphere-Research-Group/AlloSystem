#ifndef INC_AL_DISPLAY_LIST_HPP
#define INC_AL_DISPLAY_LIST_HPP

/*	Allocore -- Multimedia / virtual environment application class library

	Description:
	GPU display list for drawing static geometry

	Author(s):
	Lance Putnam, 2010, putnam.lance@gmail.com
*/

#include "allocore/graphics/al_OpenGL.hpp"
#include "allocore/graphics/al_GPUObject.hpp"

namespace al{

#ifdef AL_GRAPHICS_USE_OPENGL

/// Display list for drawing static geometry
///
/// @ingroup allocore
class DisplayList : public GPUObject {
public:

	/// Begin the display list's rendering commands
	void begin(){ validate(); glNewList(id(), GL_COMPILE); }

	/// End the display list's rendering commands
	void end(){ glEndList(); }

	/// Draw the display list
	void draw(){ glCallList(id()); }

protected:
	void onCreate() override {
		mID = glGenLists(1);
	}

	void onDestroy() override {
		glDeleteLists(mID, 1);
	}
};

#endif

} // al::
#endif
