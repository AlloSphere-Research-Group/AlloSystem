/*
Allocore Example MsgTube

Description:
Demonstration of MsgTube

Not a terribly convincing demonstration, since it could be achieved using
shared memory in this case, but a convincing demonstration would be too complex.

MsgTube might be partly unnecessary with C++0x lambda object thingies... 

Author:
Graham Wakefield, 2011
*/

#include "allocore/al_Allocore.hpp"
#include "allocore/types/al_MsgTube.hpp"


using namespace al;
	
struct MyWindow : public Window, public AudioCallback {
    
    MyWindow()
	{
		x = 10;
		y = 10;
		phase = 0;
		setfreq(0, this, x, y);
	}
	
	virtual ~MyWindow() {}
    
	bool onFrame(){
		
		// go ortho:
        gl.viewport(0, 0, width(), height());
		gl.clear(gl.DEPTH_BUFFER_BIT | gl.COLOR_BUFFER_BIT);
		gl.projection(Matrix4d::ortho(0, width(), height(), 0, -1, 1));
		gl.modelView(Matrix4d::identity());

		// draw line to current point
		gl.begin(gl.LINES);
			gl.vertex(0, 0, 0);
			gl.vertex(x, y, 0);
		gl.end();
		
		return true;
	}
	
	// send messages in response to mouse events:
	virtual bool onMouseDown(const Mouse& m){ 
		// cache for rendering
		x = m.x(); y = m.y();
		// send to audio thread:
		audio_inbox.send(setfreq, this, x / width(), 1. - y/height());
		return true;
	}
	virtual bool onMouseDrag(const Mouse& m){ 
		// cache for rendering
		x = m.x(); y = m.y();
		// send to audio thread:
		audio_inbox.send(setfreq, this, x / width(), 1. - y/height());
		return true;
	}
	virtual bool onMouseUp(const Mouse& m){ 
		// cache for rendering
		x = m.x(); y = m.y();
		// send to audio thread:
		audio_inbox.send(setfreq, this, x / width(), 1. - y/height());
		return true;
	}
	
	// a message to send:
	static void setfreq(al_sec t, MyWindow * self, double x, double y) {
		self->freq = y * 4400;
	}
    
	
	virtual void onAudioCB(al::AudioIOData& io) {
		// get next clock time:
		al_sec until = audio_inbox.now + io.secondsPerBuffer();
		
		// receive (execute) any new messages:
		audio_inbox.executeUntil(until);
		
		// synthesize sound
		double rps = freq * M_2PI / io.framesPerSecond();
		while (io()) {
			phase += rps;
			io.out(0) = sin(phase);
		}
		
		// update clock for new incoming messages:
		audio_inbox.now = until;
	}
	
	// properties for audio thread synthesis 
	double freq, phase;
	// properties for graphical rendering
	double x, y;
	// messaging between graphical & audio threads
	MsgTube audio_inbox;
	// renderer
    Graphics gl;
};

AudioIO audio;
MyWindow win1;

int main(){    
    win1.create(Window::Dim(800, 600), "Allocore Example: MsgTube");
	
	audio.append(win1);
	audio.start();

	MainLoop::start();
	return 0;
}
