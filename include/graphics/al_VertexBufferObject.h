#ifndef INCLUDE_AL_GRAPHICS_VBO_HPP
#define INCLUDE_AL_GRAPHICS_VBO_HPP

#include "graphics/al_Common.hpp"
#include "graphics/al_GPUObject.hpp"

namespace al {
namespace gfx{

/*

Vertex Buffer Objects (VBOs) create buffer memory for vertex attributes in high-performance memory (in contrast, Vertex Arrays store buffer memory in the client CPU, incurring the overhead of data transfer). If the buffer object is used to store pixel data, it is called Pixel Buffer Object (PBO).

VBOs provide an interface to access these buffers in a similar fashion to vertex arrays. Hints of 'target' and 'mode' help the implementation determine whether to use system, AGP or video memory. 

Unlike display lists, the data in vertex buffer object can be read and updated by mapping the buffer into client's memory space.

Another important advantage of VBO is sharing the buffer objects with many clients, like display lists and textures. Since VBO is on the server's side, multiple clients will be able to access the same buffer with the corresponding identifier.
*/

class VBO : public GPUObject{
public:

	VBO() : mBufferID(0), mTarget(BufferType::ArrayBuffer), mUsage(BufferUsage::DynamicDraw) {}

	void upload(const void * data, size_t size) {
		glBufferDataARB(mTarget, (GLsizei)size, data, mUsage);
	}
	
	virtual void onCreate() {
		const GLsizei num_buffers = 1;
		glGenBuffersARB(num_buffers, &mBufferID);
		glBindBufferARB(mTarget, mBufferID);
	}
	
	virtual void onDestroy() {
		
	}
	
protected:
	GLuint mBufferID;
	// GL_ARRAY_BUFFER_ARB for vertex data, GL_ELEMENT_ARRAY_BUFFER_ARB for index data
	BufferType::t mTarget;
	// GL_{STATIC|DYNAMIC|STREAM}_{DRAW|READ|COPY}_ARB
	// STATIC: specified once and used many times
	// DYNAMIC: specified and used repeatedly
	// STREAM: specified once and used once
	// DRAW: application -> GL
	// READ: GL -> application	(for PBO and FBO)
	// COPY: GL -> GL			(for PBO and FBO)
	BufferUsage::t mUsage;
	


};


} // ::al::gfx
} // ::al

#endif
