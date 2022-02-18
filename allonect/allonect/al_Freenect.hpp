#ifndef AL_FREENECT_HPP
#define AL_FREENECT_HPP

/*	Allocore --
	Multimedia / virtual environment application class library

	Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology, UCSB.
	Copyright (C) 2012. The Regents of the University of California.
	All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

		Redistributions of source code must retain the above copyright notice,
		this list of conditions and the following disclaimer.

		Redistributions in binary form must reproduce the above copyright
		notice, this list of conditions and the following disclaimer in the
		documentation and/or other materials provided with the distribution.

		Neither the name of the University of California nor the names of its
		contributors may be used to endorse or promote products derived from
		this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
	IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
	ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
	LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
	CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
	POSSIBILITY OF SUCH DAMAGE.


	File description:
	Binding to freenect library.

	File author(s):
	Graham Wakefield, 2012, grrrwaaa@gmail.com
*/

#include "allocore/al_Allocore.hpp"
#include "libfreenect.h"

#include <map>

namespace al {

// run libfreenect on a backtround thread to reduce CPU load:
class Freenect : public Thread, public ThreadFunction {
public:

	static void depth_cb(freenect_device *dev, void *depth, uint32_t timestamp);
	static void video_cb(freenect_device *dev, void *video, uint32_t timestamp);
	static bool check(const char * what, int code);

	class Callback {
	public:

		Callback(int idx=0);
		virtual ~Callback();

		// Warning: these will be called from a background thread:
		virtual void onVideo(Texture& raw, uint32_t timestamp) {}
		virtual void onDepth(Texture& raw, uint32_t timestamp) {}


		static Vec3f depthToEye(int x, int y, uint16_t d);
		static double rawDepthToMeters(uint16_t raw);

		void reconfigure();



		bool startVideo();
		bool stopVideo();
		bool startDepth();
		bool stopDepth();

		// returns tilt in radians
		double tilt();
		void tilt(double radians);

		Texture depth, video;

	protected:
		freenect_device * dev;
	};

	// ThreadFunction:
	virtual void operator()();

	static Freenect& get();

	static void stop();

	virtual ~Freenect();

private:
	Freenect();

	Freenect * singleton;
	freenect_context * ctx;
	bool active;
};

inline bool Freenect::check(const char * what, int code) {
	if (code < 0) {
		AL_WARN("Error (%s): %d", what, code);
		return false;
	}
	return true;
}

inline Freenect::Callback::Callback(int idx) {
	dev = 0;
	Freenect& self = get();
	if (check("open", freenect_open_device(self.ctx, &dev, idx))) {
		freenect_set_user(dev, this);
		reconfigure();
	}
}

inline Freenect::Callback::~Callback() {
	if (dev) freenect_set_user(dev, 0);
}

inline double Freenect::Callback::rawDepthToMeters(uint16_t raw) {
	static const double k1 = 1.1863;
	static const double k2 = 1./2842.5;
	static const double k3 = 0.1236;
	return k3 * tan(raw*k2 + k1);
}

// @see http://nicolas.burrus.name/index.php/Research/KinectCalibration
inline Vec3f Freenect::Callback::depthToEye(int x, int y, uint16_t d) {
	// size of a pixel in meters, at zero/near plane:
	static const double metersPerPixelX = 1./594.21434211923247;
	static const double metersPerPixelY = 1./591.04053696870778;
	// location of X,Y corner at zero plane, in pixels:
	static const double edgeX = 339.30780975300314;	// x edge pixel
	static const double edgeY = 242.73913761751615;	// y edge pixel
	const double meters = rawDepthToMeters(d);
	const double disparityX = (x - edgeX);	// in pixels
	const double disparityY = (y - edgeY);	// in pixels
	return Vec3f(meters * disparityX * metersPerPixelX,
				 meters * disparityY * metersPerPixelY,
				 meters
				);
}

inline void Freenect::Callback::reconfigure() {
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

inline bool Freenect::Callback::startVideo() {
	if (dev) {
		freenect_set_video_callback(dev, video_cb);
		return check("start_video", freenect_start_video(dev));
	}
	return false;
}
inline bool Freenect::Callback::stopVideo() {
	if (dev) {
		return check("stop_video", freenect_stop_video(dev));
	}
	return false;
}
inline bool Freenect::Callback::startDepth() {
	if (dev) {
		freenect_set_depth_callback(dev, depth_cb);
		return check("start_depth", freenect_start_depth(dev));
	}
	return false;
}
inline bool Freenect::Callback::stopDepth() {
	if (dev) {
		return check("stop_depth", freenect_stop_depth(dev));
	}
	return false;
}

// returns tilt in radians
inline double Freenect::Callback::tilt() {
	double t = 0;
	if (dev) {
		freenect_update_tilt_state(dev);
		freenect_raw_tilt_state * state = freenect_get_tilt_state(dev);
		if (state) {
			t = freenect_get_tilt_degs(state);
		} else {
			AL_WARN("Error: no state");
		}
	}
	return t * M_DEG2RAD;
}

inline void Freenect::Callback::tilt(double radians) {
	freenect_set_tilt_degs(dev, radians * M_RAD2DEG);
}

// ThreadFunction:
inline void Freenect::operator()() {
	// listen for messages on main thread:
	while (active) {
		//printf(".");
		// blocking call:
		freenect_process_events(ctx);
	}
}

static Freenect& get();

inline void Freenect::stop() {
	Freenect& self = get();
	self.active = 0;
	self.Thread::join();
}

inline Freenect::~Freenect() {
	active = false;
	Thread::join();
	freenect_shutdown(ctx);
}

inline Freenect::Freenect() : Thread() {
	// TODO: handle multiple contexts?
	freenect_usb_context * usb_ctx = NULL;
	int res = freenect_init(&ctx, usb_ctx);
	if (res < 0) {
		AL_WARN("Error: failed to initialize libfreenect");
		exit(0);
	}

	int numdevs = freenect_num_devices(ctx);
	printf("%d devices\n", numdevs);

	active = true;
	Thread::start(*this);
}

}

#endif
