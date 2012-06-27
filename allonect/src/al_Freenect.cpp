#include "allonect/al_Freenect.hpp"

extern "C" {
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <assert.h>
	#include "libfreenect.h"

	#include <pthread.h>
}

#include <map>

#define MAX_KINECTS 2

using namespace al;

Freenect& Freenect::get() { 
	static Freenect singleton;
	return singleton; 
}

void Freenect::depth_cb(freenect_device *dev, void *depth, uint32_t timestamp) {
	Callback * cb = (Callback * )freenect_get_user(dev);
	if (cb) {
		cb->depth.array().data.ptr = (char *)depth;
		cb->depth.dirty();
		cb->onDepth(cb->depth, timestamp); 
	}
}

void Freenect::video_cb(freenect_device *dev, void *video, uint32_t timestamp) {
	Callback * cb = (Callback * )freenect_get_user(dev);
	if (cb) {
		cb->video.array().data.ptr = (char *)video;
		cb->video.dirty();
		cb->onVideo(cb->video, timestamp);
	}
}