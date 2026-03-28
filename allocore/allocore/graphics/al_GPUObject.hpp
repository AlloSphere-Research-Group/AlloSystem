#ifndef INC_AL_GPUOBJECT_H
#define INC_AL_GPUOBJECT_H

/*	Allocore -- Multimedia / virtual environment application class library

	Description:
	Ensures that GPU resources are valid even when a rendering context is
	rebuilt.

	Author(s):
	Graham Wakefield, 2010, grrrwaaa@gmail.com
	Lance Putnam, 2010, putnam.lance@gmail.com
*/

/**
	Ensures that GPU resources are valid even when a rendering context is
	rebuilt.

	GPUContext represents an object with a corollary rendering context on a GPU.
	GPUObject is a shared base class for all objects that have a corollary
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
	GPUObjects will register with the first created context by default.
*/

namespace al{

/// Context for signaling resource changes to GPU objects
///
/// \ingroup allocore
class GPUContext {
public:
	GPUContext();
	virtual ~GPUContext();

	/// Triggers create handler for each GPUObject registered in a given context
	void contextCreate();

	/// Triggers destroy handler for each GPUObject registered in a given context
	void contextDestroy();

	/// Get context ID
	int contextID() const { return mContextID; }

	/// Get implementation defined handle to context

	/// The value stored in the handle will depend on the underlying windowing
	/// implementation that creates the GPU context. For example, for SDL, the
	/// handle is an SDL_GLContext.
	void * contextHandle() const { return mContextHandle; }

	void makeDefaultContext();
	static int defaultContextID();

protected:
	int mContextID;
	void * mContextHandle = nullptr;
};


/// Base class for allocated resources on the GPU
///
/// @ingroup allocore
class GPUObject{
public:

	/// \param[in] ctx	a GPU context ID to attach to
	GPUObject(int ctx = GPUContext::defaultContextID());

	/// \param[in] ctx	a GPU context to attach to
	GPUObject(GPUContext& ctx);

	virtual ~GPUObject();


	/// Returns whether object has been created
	bool created() const;

	/// Creates object on GPU
	void create();

	/// Destroys object on GPU
	void destroy();

	/// Returns the assigned object ID
	unsigned long id() const { return mID; }

	void id(unsigned long v){ mID = v; }


	/// Register with a context

	/// This will unregister any existing context registration.
	///
	void contextRegister(int ctx);

	/// Ensure that the GPUObject is ready to use

	/// This is typically placed before any rendering implementation.
	/// If the object has been invalidated, the object will be destroyed and
	/// then created again. Otherwise, the object will simply be created if not
	/// already created.
	void validate();

	/// Triggers re-creation of object safely
	void invalidate();

protected:
	unsigned long mID = 0;
	bool mResubmit = false;

	/// Called when currently assigned context is created
	virtual void onCreate() = 0;

	/// Called when currently assigned context is destroyed
	virtual void onDestroy() = 0;

	// remove from the context:
	void contextUnregister();
};

} // ::al
#endif
