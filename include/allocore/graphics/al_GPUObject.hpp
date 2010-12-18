#ifndef INCLUDE_AL_GRAPHICS_GPUOBJECT_H
#define INCLUDE_AL_GRAPHICS_GPUOBJECT_H

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

/**
	Ensures that GPU resources are valid even when a rendering context is
	rebuilt.
	
	GPUContext represents an object with a corollary rendering context on a GPU.
	GPUObject is a shared base class for all objects that have a corrollary 
	object on the GPU, and which must be re-submitted when the rendering context
	is rebuilt.
	An object that inherits GPUContext must call contextDestroy() when the
	context has been invalidated. For example, the al::Window object does this
	automatically when the window is closed or enters/leaves fullscreen. Doing
	so will mark all associated GPUObjects as invalidated (calling their
	onDestroy() handlers).
	Each GPUObject should call validate() before attempting to render; this will
	re-submit the resources (by calling its onCreate() handler).
	
	GPUContexts are identified by an integer ID, which increments for each
	context created. 
	The first created context has ID=0. GPUObjects will register with this
	context by default.
*/


namespace al{

class GPUContext {
public:	
	GPUContext();
	
	// triggers destroy handler for each GPUObject registered in a given context
	void contextDestroy();
	
	int contextID() { return mContextID; }
	
protected:
	int mContextID;
};

/// Base class for allocated resources on the GPU
class GPUObject{
public:
	
	GPUObject(int ctx=0): mID(0) { contextRegister(ctx); }
	GPUObject(GPUContext& ctx): mID(0) { contextRegister(ctx.contextID()); }
	virtual ~GPUObject(){ contextUnregister(); }
	
	
	/// register with a context
	/// will unregister any existing context registration
	void contextRegister(int ctx=0);
	
	/// ensure that the GPUObject is ready to use
	/// typically placed before any rendering implementation
	void validate() {
		if (!created()) create();
	}
	
	bool created(){ return id()!=0; }

	/// Creates object on GPU
	void create(){
		if(created()){ destroy(); }
		onCreate();
	}
	
	/// Destroys object on GPU
	void destroy(){ 
		if(created()) {
			onDestroy(); mID=0; 
		}
	}
	
	/// Returns the assigned object id
	const long id() const { return mID; }
	void id(long v) {mID = v;}

protected:

	// remove from the context:
	void contextUnregister();
	
	long mID;
	virtual void onCreate() = 0;
	virtual void onDestroy() = 0;
};

} // ::al

#endif
