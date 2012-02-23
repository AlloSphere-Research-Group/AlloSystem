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



Freenect1& Freenect1::get() { 
	static Freenect1 singleton;
	return singleton; 
}








//freenect_context * ctx = 0;
//std::map<freenect_device *, int> deviceMap;
//
//
//Freenect& Freenect::get() {
//	static Freenect singleton;
//	return singleton;
//}
//
//Freenect :: Freenect() : mNumDevices(0), mActive(0) {
//	if (freenect_init(&ctx, NULL) < 0) {
//		printf("freenect_init() failed\n");
//	}
//	
//	//freenect_set_log_level(ctx, FREENECT_LOG_DEBUG);
//	freenect_select_subdevices(ctx, (freenect_device_flags)(FREENECT_DEVICE_MOTOR | FREENECT_DEVICE_CAMERA));
//		
//	mNumDevices = freenect_num_devices (ctx);
//	printf ("Number of Freenect devices found: %d\n", mNumDevices);
//	
//}
//
//Freenect :: ~Freenect() {
//	if (mActive) {
//		mActive = false;
//		mThread.join();
//	}
//}
//
//Freenect::Device::Device() {
//	// format internal array:
//	mDepthData.data.ptr = 0;
//	mDepthData.header.type = AlloUInt16Ty;
//	mDepthData.header.components = 1;
//	mDepthData.header.dim[0] = 640;
//	mDepthData.header.dim[1] = 480;
//	mDepthData.header.dimcount = 2;
//	allo_array_setstride(&mDepthData.header, 1);
//	
//}
//
//Freenect::Device::Device(const Freenect::Device& cpy) {
//	// format internal array:
//	mDepthData.data.ptr = 0;
//	mDepthData.header.type = AlloUInt16Ty;
//	mDepthData.header.components = 1;
//	mDepthData.header.dim[0] = 640;
//	mDepthData.header.dim[1] = 480;
//	mDepthData.header.dimcount = 2;
//	allo_array_setstride(&mDepthData.header, 1);
//	
//}
//
//void Freenect::Device::process(char * ptr) {
//	mDepthData.data.ptr = ptr;
//	for (int i=0; i<mHandlers.size(); i++) {
//		Handlers::iterator it = mHandlers.begin(); 
//		while(it != mHandlers.end()){
//			(*it)->onDepthData(mDepthData); 
//			++it; 
//		}
//	}
//}
//
//
//
//Freenect::Device& Freenect::Device::add(Freenect::DepthCallback& v){ 
//	remove(v);
//	mHandlers.push_back(&v);
//	return *this;
//}
//
//Freenect::Device& Freenect::Device::remove(Freenect::DepthCallback& v){
//	Freenect::Handlers& H = mHandlers;
//	Freenect::Handlers::iterator it = std::find(H.begin(), H.end(), &v);
//	if(it != H.end()){
//		H.erase(it);
//	}
//	return *this;
//}
//
//static void onDepth(freenect_device *dev, void *v_depth, uint32_t timestamp) {	
//	Freenect& freenect = Freenect::get();
//	
//	// which device?
//	int deviceID = deviceMap[dev];	
//	if (deviceID >= 0) {
//		freenect[deviceID].process((char *)v_depth);
//	}
//}
//
//bool Freenect :: start() {
//	return mThread.start((ThreadFunction&)*this);
//}
//
///// Routine called on thread execution start
//void Freenect :: operator()() {
//	for (int i=0; i<mNumDevices; i++) {
//	
//		freenect_device * dev;
//		if (freenect_open_device(ctx, &dev, i) < 0) {
//			printf("Could not open device %d\n", i);
//			return;
//		}
//		
//		deviceMap[dev] = i;
//	
//		freenect_set_depth_callback(dev, onDepth);
//		freenect_set_depth_mode(dev, freenect_find_depth_mode(FREENECT_RESOLUTION_MEDIUM, FREENECT_DEPTH_11BIT));
//		freenect_start_depth(dev);
//	}
//	
//	mActive = true;
//	while (mActive && freenect_process_events(ctx) >= 0) {
//		//printf("fps %f\n", fps());
//		al_sleep(0.001);
//	}
//}


