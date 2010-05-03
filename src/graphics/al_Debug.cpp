#include "graphics/al_Config.h"
#include "graphics/al_Debug.hpp"

namespace al {
namespace gfx{

int error(const char *pre, FILE * out){
	GLenum err = glGetError();
	#define CS(v, str) case v: printf("%s%s\n", pre, str); break;
	#define POST "The offending command is ignored and has no other side effect than to set the error flag."
	switch(err){
		CS(GL_INVALID_ENUM,	"An unacceptable value is specified for an enumerated argument. "POST)
		CS(GL_INVALID_VALUE, "A numeric argument is out of range. "POST)
		CS(GL_INVALID_OPERATION, "The specified operation is not allowed in the current state. "POST)
		CS(GL_STACK_OVERFLOW, "This command would cause a stack overflow. "POST)
		CS(GL_STACK_UNDERFLOW, "This command would cause a stack underflow. "POST)
		CS(GL_OUT_OF_MEMORY, "There is not enough memory left to execute the command.  The state of the GL is undefined, except for the state of the error flags, after this error is recorded.")
		CS(GL_TABLE_TOO_LARGE, "The specified table exceeds the implementation's maximum supported table size. "POST)
		default:;
	}
	#undef CS
	#undef POST
	return (int)err;
}

} // ::al::gfx
} // ::al

