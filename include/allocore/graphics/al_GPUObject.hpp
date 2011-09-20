#ifndef INCLUDE_AL_GRAPHICS_GPUOBJECT_H
#define INCLUDE_AL_GRAPHICS_GPUOBJECT_H

/*	Allocore --
	Multimedia / virtual environment application class library
	
	Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology, UCSB.
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


	File description:
	Ensures that GPU resources are valid even when a rendering context is
	rebuilt.

	File author(s):
	Graham Wakefield, 2010, grrrwaaa@gmail.com
	Lance Putnam, 2010, putnam.lance@gmail.com
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

/// Context for signaling resource changes to GPU objects
class GPUContext {
public:	
	GPUContext();
	virtual ~GPUContext();
	
	/// Triggers destroy handler for each GPUObject registered in a given context
	void contextDestroy();
	
	/// Get context ID
	int contextID() const { return mContextID; }
	
	void makeDefaultContext();
	static int defaultContextID();
	
protected:
	int mContextID;
};


/// Base class for allocated resources on the GPU
class GPUObject{
public:
	
	GPUObject(int ctx = GPUContext::defaultContextID()): mID(0) { contextRegister(ctx); }
	GPUObject(GPUContext& ctx): mID(0), bResubmit(false) { contextRegister(ctx.contextID()); }
	virtual ~GPUObject(){ contextUnregister(); }
	
	
	/// register with a context
	/// will unregister any existing context registration
	void contextRegister(int ctx=0);
	
	/// ensure that the GPUObject is ready to use
	/// typically placed before any rendering implementation
	void validate() {
		if (bResubmit) { destroy(); bResubmit=false; }
		if (!created()) create();
	}
	
	/// Triggers re-creation of object safely
	void invalidate() {
		bResubmit = true;
	}
	
	bool created() const { return id()!=0; }

	/// Creates object on GPU
	void create(){
		if(created()){ destroy(); }
		onCreate();
	}
	
	/// Destroys object on GPU
	void destroy(){ 
		if(created()) onDestroy(); 
		mID=0;
	}
	
	/// Returns the assigned object id
	unsigned long id() const { return mID; }
	void id(unsigned long v) {mID = v;}

protected:

	// remove from the context:
	void contextUnregister();
	
	unsigned long mID;
	bool bResubmit;
	
	/// Called when currently assigned context is created
	virtual void onCreate() = 0;
	
	/// Called when currently assigned context is destroyed
	virtual void onDestroy() = 0;
};

} // ::al

#endif
