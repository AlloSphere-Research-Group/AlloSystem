
#ifndef AL_FREENECT_H
#define AL_FREENECT_H

#include "allocore/al_Allocore.hpp"
#include "alloutil/al_FPS.hpp"
#include "libfreenect.h"

#include <map>

namespace al {

// run libfreenect on a backtround thread to reduce CPU load:
class Freenect1 : public Thread, public ThreadFunction {
public:		

	static void depth_cb(freenect_device *dev, void *depth, uint32_t timestamp) {
		Callback * cb = (Callback * )freenect_get_user(dev);
		if (cb) {
			cb->depth.array().data.ptr = (char *)depth;
			cb->depth.updatePixels();
			cb->onDepth(cb->depth, timestamp); 
		}
	}
	
	static void video_cb(freenect_device *dev, void *video, uint32_t timestamp) {
		Callback * cb = (Callback * )freenect_get_user(dev);
		if (cb) {
			cb->video.array().data.ptr = (char *)video;
			cb->video.updatePixels();
			cb->onVideo(cb->video, timestamp);
		}
	}
	
	static bool check(const char * what, int code) {
		if (code < 0) {
			printf("error (%s): %d\n", what, code);
			return false;
		}
		return true;
	}

	class Callback {
	public:
		Callback(int idx=0) {
			Freenect1& self = get();
			if (check("open", freenect_open_device(self.ctx, &dev, idx))) {
				freenect_set_user(dev, this); 
				reconfigure();
			}
		}
		
		static double rawDepthToMeters(uint16_t raw) {
			static const double k1 = 1.1863;
			static const double k2 = 1./2842.5;
			static const double k3 = 0.1236;
			return k3 * tan(raw*k2 + k1);
		}
		
		void reconfigure() {
			AlloArrayHeader header;
			
			const freenect_frame_mode dmode = freenect_get_current_depth_mode(dev);
			header.type = AlloUInt16Ty;
			header.components = 1;
			header.dimcount = 2;
			header.dim[0] = dmode.width;
			header.dim[1] = dmode.height;
			header.stride[0] = (dmode.data_bits_per_pixel+dmode.padding_bits_per_pixel)/8;
			header.stride[1] = header.stride[0] * dmode.width;
			depth.configure(header);
			depth.array().dataCalloc();
			
			const freenect_frame_mode vmode = freenect_get_current_video_mode(dev);
			header.type = AlloUInt8Ty;
			header.components = 3;
			header.dimcount = 2;
			header.dim[0] = vmode.width;
			header.dim[1] = vmode.height;
			header.stride[0] = (vmode.data_bits_per_pixel+vmode.padding_bits_per_pixel)/8;
			header.stride[1] = header.stride[0] * vmode.width;
			video.configure(header);
			video.array().dataCalloc();
		}
		
		virtual ~Callback() { 
			freenect_set_user(dev, 0); 
		}
		
		// Warning: these will be called from a background thread:
		virtual void onVideo(Texture& raw, uint32_t timestamp) {}
		virtual void onDepth(Texture& raw, uint32_t timestamp) {}
		
		bool startVideo() { 
			freenect_set_video_callback(dev, video_cb);
			return check("start_video", freenect_start_video(dev)); 
		}
		bool stopVideo() { return check("stop_video", freenect_stop_video(dev)); }
		bool startDepth() { 
			freenect_set_depth_callback(dev, depth_cb);
			return check("start_depth", freenect_start_depth(dev)); 
		}
		bool stopDepth() { return check("stop_depth", freenect_stop_depth(dev)); }
		
		// returns tilt in radians
		double tilt() {
			double t = 0;
			freenect_update_tilt_state(dev);
			freenect_raw_tilt_state * state = freenect_get_tilt_state(dev);
			if (state) {
				t = freenect_get_tilt_degs(state);
			} else {
				printf("error: no state\n");
			}
			printf("tilt %f\n", t);
			return t * M_DEG2RAD;
		}
		
		Texture depth, video;
		
	protected:
		freenect_device * dev;
	};
	
	// ThreadFunction:
	virtual void operator()() {
		// listen for messages on main thread:
		while (active) {
			//printf(".");
			// blocking call:
			freenect_process_events(ctx);
		}
	}
	
	static Freenect1& get();
	
	static void stop() {
		Freenect1& self = get();
		self.active = 0;
		self.Thread::join();
	}

	virtual ~Freenect1() {
		active = false;
		Thread::join();
		freenect_shutdown(ctx);
	}
	
private:
	Freenect1() : Thread() {
		// TODO: handle multiple contexts?
		freenect_usb_context * usb_ctx = NULL;
		int res = freenect_init(&ctx, usb_ctx);
		if (res < 0) {
			printf("error: failed to initialize libfreenect\n");
			exit(0);
		}
		
		int numdevs = freenect_num_devices(ctx);
		printf("%d devices\n", numdevs);
		
		active = true;
		Thread::start(*this);
	}
	
	Freenect1 * singleton;
	freenect_context * ctx;
	bool active;
};


////// OLDER VERSION (DEPRECATED) //////
//
//// singleton class:
//class Freenect : public ThreadFunction, public FPS {
//public:
//	
//	/// implement this interface to receive depth data updates:
//	class DepthCallback {
//	public:
//		DepthCallback(int device) : mDevice(device) { 
//			Freenect::get()[mDevice].add(*this); 
//		}
//		
//		// unregisters with device
//		virtual ~DepthCallback() { Freenect::get()[mDevice].remove(*this); };
//
//		// Warning: will be called in a different thread!
//		// Array is uint16_t, 640x480, tightly packed.
//		virtual void onDepthData(Array& depth) = 0;
//		
//	protected:
//		int mDevice;
//	};
//	
//	typedef std::list<DepthCallback *> Handlers;
//	
//	/// represents a particular device
//	class Device {
//	public:
//		Device();
//		Device(const Freenect::Device& cpy);
//		
//		/// Add an AudioCallback handler
//		Device& add(DepthCallback& v);
//
//		/// Remove all input event handlers matching argument
//		Device& remove(DepthCallback& v);
//	
//		/// (private, do not call directly)
//		void process(char * ptr);
//
//	protected:
//		Array mDepthData;
//		Handlers mHandlers;
//	};
//	
//	static Freenect& get();
//	
//	/// get a particular device:
//	Device& operator[](int i) { return mDevices[i]; }
//	
//	/// start receiving Freenect data
//	bool start();
//	
//	/// get number of available devices:
//	int devices() { return mNumDevices; }
//
//	/// Routine called on thread execution start
//	/// (private, do not call directly)
//	virtual void operator()();
//
//protected:
//	Freenect();
//	virtual ~Freenect();
//	
//	unsigned mNumDevices;
//	Thread mThread;
//	std::map<int, Freenect::Device> mDevices;
//	bool mActive;
//};
//
//
//class FreenectDepthViewer : public Freenect::DepthCallback, public FPS {
//public:
//
//	FreenectDepthViewer(int device)
//	:	Freenect::DepthCallback(device),
//		tex(640, 480, Graphics::LUMINANCE)
//	{
//		tex.allocate();
//	}
//	
//	virtual ~FreenectDepthViewer() {}
//
//	Array& array() { return tex.array(); }
//
//	virtual void onDepthData(Array& depth) {
//        FPS::onFrame();
//		uint16_t * in = (uint16_t *)depth.data.ptr;	// depth 0..2048
//		uint8_t * out = (uint8_t *)tex.array().data.ptr;
//		for (int i=0; i<640*480; i++) {
//			out[i] = in[i] / 8;
//			//if (i == 100000) printf("%d\n", out[i]);
//		}
//		tex.dirty();
//	}
//
//	Texture tex;
//};

}

#endif
