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

struct Agent : public SoundSource{

	Agent()
	: phase(0)
	{}

	virtual ~Agent(){}

	virtual void onProcess(AudioIOData& io){
		while(io()){
			//float s = io.in(0);
			float s = rnd::uniform()*0.2; // make noise, just to hear something
			writeSample(s);
		}
	}
	
	virtual void onUpdateNav(){
		smooth(0.9);
		if((phase+=0.01) >= M_2PI) phase -= M_2PI;
		pos(cos(phase), sin(phase), 0);
		step();
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
		agents[i].onUpdateNav();
		agents[i].onProcess(io);
	}
	
	navMaster.step();
	
//	for(int i=0; i<numFrames; i++){
//		src.writeSample(rnd::uniform(1.,-1.)*0.2);
//	}
	
	scene.encode(numFrames, io.framesPerSecond());
	scene.render(&io.out(0,0), numFrames);
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
	
	void onDraw(gfx::Graphics& g){
		g.antialiasing(gfx::NICEST);
		g.pointSize(10);
		g.begin(gfx::POINTS);
		for(unsigned i=0; i<agents.size(); ++i){
			g.vertex(agents[i].pos());
			g.color(1,1,0);
		}
		g.end();
		
//		g.depthTesting(1);
//		g.begin(gfx::TRIANGLES);
//		{
//		int N=3000;
//		double r = 4;
//		double f1 = 101;
//		double f2 = 166;
//		for(int i=0; i<N; i++){
//			double p = float(i)/N*M_2PI;
//			double x = r*cos(f1*p) * sin(f2*p);
//			double y = r*sin(f1*p) * sin(f2*p);
//			double z = r*cos(f2*p);
//			double c = cos(p)*0.25 + 0.25;
//
//			g.color(HSV(0.2+c, 0.5, i%3 ? 0.5 : 0));
//			g.vertex(x, y, z);
//		}
//		}
//		g.end();
	}

	gfx::Graphics gl;
	Nav nav;
	Camera cam;
	gfx::Stereographic stereo;
};


int main (int argc, char * argv[]){

	listener = &scene.createListener(2);
	
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
