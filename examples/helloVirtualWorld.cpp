/*
What is our "Hello world!" app?

A sphere orbits around the origin emiting the audio line input. The camera
view can be switched between a freely navigable keyboard/mouse controlled mode 
and a sphere follow mode.

Requirements:
2 channels of spatial sound
2 windows, one front view, one back view
stereographic rendering
*/

#include "al_Allocore.hpp"
#include "al_NavControl.hpp"

struct Agent : public SoundSource, public gfx::Drawable{

	Agent()
	: phase(0)
	{}

	virtual ~Agent(){}

	virtual void onProcess(AudioIOData& io){
		while(io()){
			//float s = io.in(0);
			float s = rnd::uniform()*0.9; // make noise, just to hear something
			writeSample(s);
		}
	}
	
	virtual void onUpdateNav(){
		smooth(0.9);
		if((phase+=0.01) >= M_2PI) phase -= M_2PI;
		//pos(cos(phase), sin(phase), 0);
		//pos(0,0,0);
		
		spin(0.3, 0.011, 0.417);
		//moveF(0.01);
		step();
	}
	
	virtual void onDraw(gfx::Graphics& g){

		g.pushMatrix();
		g.multMatrix(matrix());
	
		g.begin(gfx::TRIANGLES);
			float ds = 0.5;
			g.vertex(    0, 0, ds*2);
			g.vertex( ds/2, 0,-ds);
			g.vertex(-ds/2, 0,-ds);
			
			g.color(1,1,1);
			g.color(1,1,0);
			g.color(1,0,1);
		g.end();
		
		g.popMatrix();	
	}
	
	double phase;
};


#define AUDIO_BLOCK_SIZE 256
AudioScene scene(3, 1, AUDIO_BLOCK_SIZE);
Listener * listener;
Nav navMaster(Vec3d(0,0,-4), 0.95);
std::vector<Agent> agents(1);


void audioCB(AudioIOData& io){
	int numFrames = io.framesPerBuffer();

	for(unsigned i=0; i<agents.size(); ++i){
		io.frame(0);
		agents[i].onUpdateNav();
		agents[i].onProcess(io);
	}

	navMaster.step();

	scene.encode(numFrames, io.framesPerSecond());
	scene.render(&io.out(0,0), numFrames);
	
	//printf("%g\n", io.out(0,0));
}


struct MyWindow : public WindowGL, public gfx::Drawable{
	
	MyWindow(): gl(new gfx::GraphicsBackendOpenGL){}

	void onFrame(){

		nav.step();
		cam.set(navMaster);
		cam *= nav;
		cam.updateUnitVectors();

		stereo.draw(gl, cam, *this, dimensions().w, dimensions().h);
	}
	
	virtual void onDraw(gfx::Graphics& g){
		g.antialiasing(gfx::NICEST);

		for(unsigned i=0; i<agents.size(); ++i){
			agents[i].onDraw(g);
		}
	}

	gfx::Graphics gl;
	Nav nav;
	Camera cam;
	gfx::Stereographic stereo;
};


int main (int argc, char * argv[]){

	listener = &scene.createListener(2);
	
	listener->speakerPos(0,0,-90);
	listener->speakerPos(1,1, 90);
	
	for(unsigned i=0; i<agents.size(); ++i) scene.addSource(agents[i]);

	MyWindow windows[2];

	for(int i=0; i<2; ++i){
		windows[i].add(new StandardWindowKeyControls);
		windows[i].add(new NavInputControl(&navMaster));
		windows[i].create(WindowGL::Dim(600,480,i*650), "Hello Virtual World!");
		windows[i].nav.turnU(i*180);
	}

	AudioIO audioIO(AUDIO_BLOCK_SIZE, 44100, audioCB, 0, 2, 1);
	audioIO.start();

	MainLoop::start();
	return 0;
}
