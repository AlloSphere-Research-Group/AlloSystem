/*
Allocore Example: Render To Disk

Description:
This demonstrates how to use the RenderToDisk class to render an audiovisual 
scene to disk as an image sequence and sound file. The rendering is toggled on
and off with control-r.

Author:
Lance Putnam, April 2015
*/

#include "allocore/io/al_App.hpp"
#include "allocore/io/al_RenderToDisk.hpp"
using namespace al;


class MyApp : public App{
public:

	float phase=0, pan=0, angle=0;
	Mesh shape;
	RenderToDisk render;

	MyApp(){
		// Set path to render files to:
		// If not specified, then a directory is automatically generated.
		//render.path("./renderNRTSave/");

		// Select the render mode:
		// REAL_TIME attempts to render to disk without interrupting real-time
		// i/o. NON_REAL_TIME renders as fast as possible and interrupts 
		// real-time i/o.
		//render.mode(RenderToDisk::REAL_TIME);		
		render.mode(RenderToDisk::NON_REAL_TIME);

		// Set the image type and amount of compression:
		// The default is "png" with medium compression.
		//render.imageFormat("jpg", 50);

		// Set the sound file format:
		// The default is FLOAT32.
		//render.soundFormat(render.PCM16);

		addDodecahedron(shape);
		shape.color(HSV(0.1));
		shape.decompress();
		shape.generateNormals();
		nav().pos(0,0,4);
		initWindow(Window::Dim(800,600));
		initAudio(44100, 128, 2,0);
		printf("Press ctrl-r to toggle rendering\n");
	}

	void onSound(AudioIOData& io){
	
		float panFreq = 0.2/io.fps();	

		while(io()){
			phase += panFreq;
			if(phase >= 1) phase -= 1;
			
			float s = sin(2200 * phase*2*M_PI) * 0.2;

			pan = cos(phase * 2*M_PI);
			float mixR = pan*0.5+0.5;
			float mixL = 1 - mixR;

			io.out(0) = s * mixL;
			io.out(1) = s * mixR;
		}
	}


	void onAnimate(double dt){
		// If using NON_REAL_TIME mode, then you must call this to override
		// the real-time delta time.
		render.adaptFrameDur(dt);
		
		angle += dt * 30;
	}
	
	void onDraw(Graphics& g){
		g.light().pos(100,1000,100);
		g.pushMatrix();
		g.translate(pan, 0, 0);
		g.rotate(angle);
		g.draw(shape);
		g.popMatrix();
	}

	void onKeyDown(const Keyboard& k){
		switch(k.key()){
		case 'r': if(k.ctrl()){

			// Here, we toggle the rendering on/off
			render.toggle(
				audioIO(),	// The audio i/o to render from
				window(),	// The window to render from
				29.97		// Graphics frame rate. If not specified, then this 
							//   defaults to the current window frame rate.
			);

			// It is also possible to render only the audio...
			//render.toggle(audioIO());

			// ... or only the graphics.
			//render.toggle(window());

			printf("Rendering %s\n", render.active() ? "started" : "stopped");
			} break;
		}
	}
};


int main(){
	MyApp().start();
}
