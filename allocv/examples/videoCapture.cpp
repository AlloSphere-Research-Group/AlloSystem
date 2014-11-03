/*
AlloCV Example: Video Capture

Description:
This is an example of how to play a single video stream, either a from a camera
or file.

Author:
Lance Putnam, Nov. 2014
*/

#include "allocv/al_VideoCapture.hpp"
#include "allocore/io/al_App.hpp"
using namespace al;


class MyApp : public App, public VideoCaptureHandler{
public:

	al::VideoCapture vid;
	Texture texture;

	MyApp(){
		
		// Open first enumerated video device
		//vid.open(0);
		
		// Open a file for playback
		vid.open(RUN_MAIN_SOURCE_PATH "beetle.mp4");

		// Set frame rate (files only!)
		//vid.fps(10);

		// Set playback rate factor (files only!)
		//vid.rate(0.2);

		// Set position in milliseconds (files only!)
		//vid.posMsec(10000);

		// Set video properties through OpenCV
		//vid.set(CV_CAP_PROP_AUTO_EXPOSURE, 0.); // no effect
		//vid.set(CV_CAP_PROP_MONOCROME, 1); // no effect
		//vid.set(CV_CAP_PROP_CONVERT_RGB, 1); // no effect
		//vid.set(CV_CAP_PROP_BRIGHTNESS, 0.1); // no effect

		// Print info about video capture
		vid.print();

		// Attach video to VideoCaptureHandler so onVideo gets called
		attach(vid);
		
		// Start video playback
		startVideo();
		
		nav().pos(0,0,4);
		initWindow();
	}

	// This gets called whenever a new video frame is ready
	virtual void onVideo(VideoCapture& vidcap, int streamIdx){

		// Copy video frame into texture; functions returns true on success
		if(vidcap.retrieveFlip(texture.array())){
			
			// Tell texture to use BGR format (typical for OpenCV video)
			texture.format(Graphics::BGR);

			// Loop video (files only!)
			vidcap.loop();
		
			// Loop video between min/max frames (files only!)
			//vidcap.loop(50, 60);
		}
	}

	virtual void onDraw(Graphics& g, const Viewpoint& v){
		//texture.quadViewport(g);
		texture.quad(g, 2*vid.aspect(), 2, -vid.aspect(), -1);
	}
};


int main(){
	MyApp().start();
}
