/*
What is our "Hello world!" app?

An agent orbits around the origin emitting the audio line input. The camera
view can be switched between a freely navigable keyboard/mouse controlled mode
and a sphere follow mode.

Requirements:
2 channels of spatial sound
2 windows, one front view, one back view
stereographic rendering
*/

#include "allocore/al_Allocore.hpp"
#include "alloni/al_Ni.hpp"

using namespace al;

Nav navMaster(Vec3d(0,0,-4), 0.95);
Stereographic stereo;
Kinect kinect(0);

struct MyWindow : public Window, public Drawable {

	GraphicsGL gl;
	Pose transform;
	Camera cam;
	Texture depth;
	al_sec mTime;

	MyWindow()
	:	depth(gl, 640, 480, Texture::LUMINANCE, Texture::FLOAT32),
		mTime(al_time())
	{}

	bool onFrame(){

		al_sec when = al_time();
		al_sec fps = 1.0/(when - mTime);
		mTime = when;

		printf("fps %f (kinect %f)\n", fps, kinect.fps());

		depth.fromArray(&kinect.depthArray());

		Pose pose(navMaster * transform);
		Viewport vp(dimensions().w, dimensions().h);
		stereo.draw(gl, cam, pose, vp, *this);

		gl.pushMatrix();
			// pixel coordinates:
			gl.matrixMode(gl.PROJECTION);
			gl.loadMatrix(Matrix4d::ortho(0, 1, 1, 0, -1, 1));

			gl.color(1, 1, 1);
			depth.bind();
			gl.begin(gl.QUADS);
				gl.texCoord(0, 0);
				gl.vertex  (0, 0, 0);
				gl.texCoord(1, 0);
				gl.vertex  (1, 0, 0);
				gl.texCoord(1, 1);
				gl.vertex  (1, 1, 0);
				gl.texCoord(0, 1);
				gl.vertex  (0, 1, 0);
			gl.end();
			depth.unbind();

		gl.popMatrix();

		return true;
	}

	bool onKeyDown(const Keyboard& k) {
		if (k.key() == Key::Tab) {
			stereo.stereo(!stereo.stereo());
		}
		return true;
	}

	virtual void onDraw(Graphics& gl){


	}
};


int main (int argc, char * argv[]){

	MyWindow window;
	window.create(Window::Dim( 0, 0, 640, 480), "NI");

	window.add(new StandardWindowKeyControls);

	kinect.start();

	MainLoop::start();

	kinect.stop();
	return 0;
}
