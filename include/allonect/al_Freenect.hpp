
#ifndef AL_KINECTVIEWER_H
#define AL_KINECTVIEWER_H

#include "allocore/al_Allocore.hpp"
#include "alloutil/al_FPS.hpp"

#include <map>

namespace al {


// singleton class:
class Freenect : public ThreadFunction, public FPS {
public:
	
	/// implement this interface to receive depth data updates:
	class DepthCallback {
	public:
		DepthCallback(int device) : mDevice(device) { 
			Freenect::get()[mDevice].add(*this); 
		}
		
		// unregisters with device
		virtual ~DepthCallback() { Freenect::get()[mDevice].remove(*this); };

		// Warning: will be called in a different thread!
		// Array is uint16_t, 640x480, tightly packed.
		virtual void onDepthData(Array& depth) = 0;
		
	protected:
		int mDevice;
	};
	
	typedef std::list<DepthCallback *> Handlers;
	
	/// represents a particular device
	class Device {
	public:
		Device();
		Device(const Freenect::Device& cpy);
		
		/// Add an AudioCallback handler
		Device& add(DepthCallback& v);

		/// Remove all input event handlers matching argument
		Device& remove(DepthCallback& v);
	
		/// (private, do not call directly)
		void process(char * ptr);

	protected:
		Array mDepthData;
		Handlers mHandlers;
	};
	
	static Freenect& get();
	
	/// get a particular device:
	Device& operator[](int i) { return mDevices[i]; }
	
	/// start receiving Freenect data
	bool start();
	
	/// get number of available devices:
	int devices() { return mNumDevices; }

	/// Routine called on thread execution start
	/// (private, do not call directly)
	virtual void operator()();

protected:
	Freenect();
	virtual ~Freenect();
	
	unsigned mNumDevices;
	Thread mThread;
	std::map<int, Freenect::Device> mDevices;
	bool mActive;
};


class FreenectDepthViewer : public Freenect::DepthCallback, public FPS {
public:

	FreenectDepthViewer(int device)
	:	Freenect::DepthCallback(device),
		tex(640, 480, Graphics::LUMINANCE)
	{
		tex.allocate();
	}
	
	virtual ~FreenectDepthViewer() {}

	Array& array() { return tex.array(); }

	virtual void onDepthData(Array& depth) {
        FPS::onFrame();
		uint16_t * in = (uint16_t *)depth.data.ptr;	// depth 0..2048
		uint8_t * out = (uint8_t *)tex.array().data.ptr;
		for (int i=0; i<640*480; i++) {
			out[i] = in[i] / 8;
			//if (i == 100000) printf("%d\n", out[i]);
		}
		tex.dirty();
	}

	Texture tex;
};

}

#endif
