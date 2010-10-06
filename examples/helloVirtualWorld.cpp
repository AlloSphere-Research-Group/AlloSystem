/*
What is our "Hello world!" app?

A sphere orbits around the origin emiting the audio line input. The camera
view can be switched between a freely navigable mode and a follow mode on
the sphere.

2 channel spatial sound
2 windows, one front view, one back view
stereographic

*/

#include "al_Allocore.hpp"
#include "al_NavControl.hpp"

struct Agent : public SoundSource{

	Agent()
	: phase(0)
	{}

	virtual ~Agent(){}

	virtual void onProcess(AudioIOData& io){
		while(io()){
			float s = io.in(0);
			
			writeSample(s);

//			float sl, sr;
//			reverb(s, sl, sr);
//
//			io.sum(0) = sl*0.2;
//			io.sum(1) = sr*0.2;
		}
	}
	
	virtual void onUpdateNav(){
		smooth(0.9);
		if((phase+=0.01) >= M_2PI) phase -= M_2PI;
		pos().set(cos(phase), sin(phase), 0);
		step();
	}
	
	double phase;
};


#define AUDIO_BLOCK_SIZE 256
AudioScene scene(3, 1, AUDIO_BLOCK_SIZE);
Reverb<float> reverb;
Listener * listener;
SoundSource src;
Nav navMaster(Vec3d(0,0,-4), 0.9);

std::vector<Agent> agents(1);


void audioCB(AudioIOData& io){
	int numFrames = io.framesPerBuffer();

//	for(int i=0; i<numFrames; ++i){
//		float s = io.in(0,i);
//
//		float sl, sr;
//		reverb(s, sl, sr);
//
//		io.out(0,i) = sl*0.2;
//		io.out(1,i) = sr*0.2;
//	}

	for(unsigned i=0; i<agents.size(); ++i){
		io.frame(0);
		agents[i].onProcess(io);
		agents[i].onUpdateNav();
	}
	
	navMaster.step(0.2);
	
//	for(int i=0; i<numFrames; i++){
//		src.writeSample(rnd::uniform(1.,-1.)*0.2);
//	}
	
	scene.encode(numFrames, io.framesPerSecond());
	scene.render(&io.out(0,0), numFrames);
}


struct MyWindow : public WindowGL, public gfx::Drawable{
	
	MyWindow(): gl(new gfx::GraphicsBackendOpenGL)
	{}

	void onFrame(){
		cam.set(navMaster);
		stereo.draw(gl, cam, *this, dimensions().w, dimensions().h);
	}
	
	void draw(gfx::Graphics& g){
		
		g.pointSize(10);
		g.begin(gfx::POINTS);
		for(unsigned i=0; i<agents.size(); ++i){
			g.vertex(agents[i].pos());
			g.color(1,1,0);
		}
		g.end();
	}

	gfx::Graphics gl;
	Camera cam;
	gfx::Stereographic stereo;
};


int main (int argc, char * argv[]){

	listener = &scene.createListener(2);
	//scene.addSource(src);
	
	for(unsigned i=0; i<agents.size(); ++i) scene.addSource(agents[i]);

	MyWindow windows[2];

	for(int i=0; i<2; ++i){
		windows[i].add(new StandardWindowKeyControls);
		windows[i].add(new NavInputControl(&navMaster));
		//windows[i].add(new NavInputControl(&windows[i].cam));
		windows[i].create(WindowGL::Dim(600,480,i*650), "Hello Virtual World!");
	}

	AudioIO audioIO(AUDIO_BLOCK_SIZE, 44100, audioCB, 0, 2, 1);
	audioIO.start();

	MainLoop::start();
	return 0;
}
