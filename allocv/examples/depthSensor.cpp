/*
AlloCV Example: Depth Sensor

Description:
This is an example of how to get frames from an OpenNI-supported depth sensor 
such as a Kinect or XtionPRO.

More information:
http://docs.opencv.org/doc/user_guide/ug_highgui.html

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

		// Open the depth sensor
		vid.open(CV_CAP_OPENNI);

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

		// Copy depth map into texture; functions returns true on success
		if(vidcap.retrieveFlip(texture.array(), CV_CAP_OPENNI_DEPTH_MAP)){

			// Do something with depth map here...

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
