/*
AlloCV Example: Video Capture (2)

Description:
This is an example of how to play multiple video streams.

Author:
Lance Putnam, Nov. 2014
*/

#include "allocv/al_VideoCapture.hpp"
#include "allocore/io/al_App.hpp"
using namespace al;


class MyApp : public App, public VideoCaptureHandler{
public:

	al::VideoCapture vid[2];
	Texture texture[2];

	MyApp(){
		
		// Open first enumerated video device
		vid[0].open(0);
		
		// Open a file for playback
		vid[1].open(RUN_MAIN_SOURCE_PATH "beetle.mp4");

		vid[0].print();
		vid[1].print();

		// Set number of video streams
		numVideoStreams(2);
		
		// Attach both video captures
		attach(vid[0], 0);
		attach(vid[1], 1);
		
		// Start video threads
		startVideo();
		
		nav().pos(0,0,4);
		initWindow();
	}

	// This gets called whenever a new video frame is ready
	void onVideo(VideoCapture& vidcap, int streamIdx){

		if(vidcap.retrieveFlip(texture[streamIdx].array())){
		
			// Tell texture to use BGR format (typical for OpenCV video)
			texture[streamIdx].format(Graphics::BGR);
			
			// Loop video
			vidcap.loop();
		}
	}

	virtual void onDraw(Graphics& g, const Viewpoint& v){
		texture[0].quad(g, 4./3, 1, -2./3, -0.5 + 0.5);
		texture[1].quad(g, 4./3, 1, -2./3, -0.5 - 0.5);
	}
};


int main(){
	MyApp().start();
}
