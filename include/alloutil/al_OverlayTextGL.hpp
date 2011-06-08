#ifndef INC_AL_OVERLAY_TEXT_OPENGL_HPP
#define INC_AL_OVERLAY_TEXT_OPENGL_HPP

#include "allocore/graphics/al_GraphicsOpenGL.hpp"
#include "allocore/graphics/al_Font.hpp"

/*!
	A printf for OpenGL windows:
 */

namespace al {

class OverlayText {
public:
	
	OverlayText(Graphics& gl, std::string fontfile)
	: font(gl, fontfile) {}
	
	void printf(std::string line) {
		lines.push_back(line);
	}
	void printf(const char * fmt, ...) {
		static char line[1024];
		va_list args;
		va_start(args, fmt);
		vsnprintf(line, 1024, fmt, args);
		va_end(args);
		lines.push_back(line);
	}
	
	void draw(Graphics& gl) {
		gl.pushMatrix();
		// because OpenGL/font is upside-down:
		gl.scale(1, -1, 1);
		for (std::vector<std::string>::iterator it = lines.begin(); it != lines.end(); it++) {
			gl.translate(0, -font.size(), 0);
			font.render(gl, *it);
		}
		gl.popMatrix();
		lines.clear();
	}
	
	void clear() {
		lines.clear();
	}
	
	std::vector<std::string> lines;
	
	Font font;
};

} // al::

#endif
