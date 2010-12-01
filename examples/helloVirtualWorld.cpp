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
#include "al_NavControl.hpp"
using namespace al;

struct Agent : public SoundSource, public Drawable{

	Agent()
	: oscPhase(0), oscEnv(1)
	{}

	virtual ~Agent(){}

	virtual void onProcess(AudioIOData& io){
		while(io()){
			//float s = io.in(0);
			//float s = rnd::uniform(); // make noise, just to hear something
			float s = sin(oscPhase * M_2PI);
			
			s *= (oscEnv*=0.9995);
			
			if(oscEnv < 0.001){ oscEnv=1; oscPhase=0; }
			
			//float s = phase * 2 - 1;
			oscPhase += 440./io.framesPerSecond();
			if(oscPhase >= 1) oscPhase -= 1;
			writeSample(s*1);
		}
	}
	
	virtual void onUpdateNav(){
		smooth(0.9);
		//if((phase+=0.01) >= M_2PI) phase -= M_2PI;
		//pos(cos(phase), sin(phase), 0);
		//pos(0,0,0);
		
		spin(0.3, 0.011, 0.417);
		//moveF(0.01);
		step();
	}
	
	virtual void onDraw(Graphics& g){

		g.pushMatrix();
		g.multMatrix(matrix());
	
		g.begin(g.TRIANGLES);
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
	
	double oscPhase, oscEnv;
};


#define AUDIO_BLOCK_SIZE 256
AudioScene scene(3, 1, AUDIO_BLOCK_SIZE);
Listener * listener;
Nav navMaster(Vec3d(0,0,-4), 0.95);
std::vector<Agent> agents(1);
Stereographic stereo;


void audioCB(AudioIOData& io){
	int numFrames = io.framesPerBuffer();

	for(unsigned i=0; i<agents.size(); ++i){
		io.frame(0);
		agents[i].onUpdateNav();
		agents[i].onProcess(io);
	}

	navMaster.step(0.5);
	listener->pos(navMaster.pos());

	scene.encode(numFrames, io.framesPerSecond());
	scene.render(&io.out(0,0), numFrames);
	
	//printf("%g\n", io.out(0,0));
}


struct MyWindow : public Window, public Drawable{
	
	MyWindow(): gl(new GraphicsBackendOpenGL){}

	bool onFrame(){

		Pose pose(navMaster * transform);
		
		//listener->pos(pose.pos());

		Viewport vp(dimensions().w, dimensions().h);
		stereo.draw(gl, cam, pose, vp, *this);
		
		return true;
	}
	
	bool onKeyDown(const Keyboard& k) {
		
		if (k.key() == Key::Tab) {
			stereo.stereo(!stereo.stereo());
		}
		
		return true;
	}
	
	virtual void onDraw(Graphics& g){

		for(unsigned i=0; i<agents.size(); ++i){
			agents[i].onDraw(g);
		}
	}

	Graphics gl;
	Pose transform;
	Camera cam;
};


int main (int argc, char * argv[]){

	listener = &scene.createListener(2);
	
	listener->speakerPos(0,0,-60);
	listener->speakerPos(1,1, 60);
//	listener->speakerPos(0,0,-45);
//	listener->speakerPos(1,1, 45);
	
	for(unsigned i=0; i<agents.size(); ++i) scene.addSource(agents[i]);

	MyWindow windows[2];

	for(int i=0; i<2; ++i){
		windows[i].add(new StandardWindowKeyControls);
		windows[i].add(new NavInputControl(&navMaster));
		windows[i].create(Window::Dim(i*650,0, 600,480), "Hello Virtual World!");
		windows[i].transform.quat().fromAxisAngle(i*180, 0, 1, 0);
	}

	AudioIO audioIO(AUDIO_BLOCK_SIZE, 44100, audioCB, 0, 2, 1);
	audioIO.start();

	MainLoop::start();
	return 0;
}
