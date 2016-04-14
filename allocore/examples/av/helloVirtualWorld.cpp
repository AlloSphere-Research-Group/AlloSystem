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
#include "allocore/sound/al_Ambisonics.hpp"

using namespace al;

struct Agent : public SoundSource, public Nav, public Drawable{

	Agent()
	: oscPhase(0), oscEnv(1)
	{
		Nav::pos(-4, 0, -4);
	}

	virtual ~Agent(){}

	virtual void onProcess(AudioIOData& io){
		while(io()){
            
			//float s = io.in(0);
			//float s = rnd::uniform(); // make noise, just to hear something
			float s = sin(oscPhase * M_2PI);
			//float s = al::rnd::uniformS();
			//s *= (oscEnv*=0.999);

			if(oscEnv < 0.00001){ oscEnv=1; oscPhase=0; }

			//float s = phase * 2 - 1;
			oscPhase += 440./io.framesPerSecond();
			if(oscPhase >= 1) oscPhase -= 1;
			writeSample(s*0.2);
		}
	}

	virtual void onUpdateNav(){
		smooth(0.9);

        spin(M_2PI/360, M_2PI/397, 0);
        moveF(0.04);

		step();

		SoundSource::pose(*this);
	}

	virtual void onDraw(Graphics& g){
		g.pushMatrix();
			g.scale(10);
			g.mesh().primitive(g.LINES);
			addWireBox(g.mesh());
			g.draw();
		g.popMatrix();

		g.pushMatrix();
			g.multMatrix(directionMatrix());
			g.begin(g.TRIANGLES);
				float ds = 0.5;

				g.color(1,1,1);
				g.vertex(    0, 0, ds*2);
				g.color(1,1,0);
				g.vertex( ds/2, 0,-ds);
				g.color(1,0,1);
				g.vertex(-ds/2, 0,-ds);

				g.color(1,1,1);
				g.vertex(    0, 0, ds*2);
				g.color(0,1,1);
				g.vertex( 0, ds/2,-ds);
				g.color(1,0,1);
				g.vertex(-ds/2, 0,-ds);

				g.color(1,1,1);
				g.vertex(    0, 0, ds*2);
				g.color(1,1,0);
				g.vertex( ds/2, 0,-ds);
				g.color(0,1,1);
				g.vertex( 0, ds/2, -ds);
			g.end();
		g.popMatrix();
	}

	double oscPhase, oscEnv;
};

#define AUDIO_BLOCK_SIZE 256

AudioScene scene(AUDIO_BLOCK_SIZE);
Listener * listener;
Nav navMaster(Vec3d(0,0,-4), 0.95);
std::vector<Agent> agents(1);
int currentAgent = 0;
Stereographic stereo;


void audioCB(AudioIOData& io){

	for(unsigned i=0; i<agents.size(); ++i){
		io.frame(0);
		agents[i].onUpdateNav();
		agents[i].onProcess(io);
	}

	navMaster.step(0.5);

	listener->pose(navMaster);
    io.frame(0);
    scene.render(io);
}


struct MyWindow : public Window, public Drawable{

	MyWindow() {}

	bool onFrame(){

		Pose pose(navMaster * transform);

		Viewport vp(dimensions().w, dimensions().h);
		stereo.draw(gl, cam, pose, vp, *this);

		return true;
	}

	bool onKeyDown(const Keyboard& k) {

		if (k.key() == Keyboard::TAB) {
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
	Lens cam;
};


int main (int argc, char * argv[])
{
    // Set speaker layout
    const int numSpeakers = 2;
    Speaker speakers[] = {
        Speaker(0,  45,0),
        Speaker(1, -45,0),
    };
    SpeakerLayout speakerLayout;
    speakerLayout.addSpeaker(speakers[0]);
    speakerLayout.addSpeaker(speakers[1]);

    // Create spatializer
    AmbisonicsSpatializer *spat = new AmbisonicsSpatializer(speakerLayout, 2, 1);
    scene.usePerSampleProcessing(true); //per sample processing is less efficient than per buffer (default), but provides higher quality Doppler
    
	// Create listener to render audio
	listener = scene.createListener(spat);
	
	// Now do some visuals
	for(unsigned i=0; i<agents.size(); ++i) scene.addSource(agents[i]);
    
	MyWindow windows[6];

	windows[0].create(Window::Dim(  0, 300, 200,200), "Left");
	windows[1].create(Window::Dim(200, 300, 200,200), "Center");
	windows[2].create(Window::Dim(400, 300, 200,200), "Right");
	windows[3].create(Window::Dim(600, 300, 200,200), "Back");
	windows[4].create(Window::Dim(200, 100, 200,200), "Top");
	windows[5].create(Window::Dim(200, 500, 200,200), "Bottom");

	// side windows
	for(int i=0; i<4; ++i){
		windows[i].append(*new StandardWindowKeyControls);
		windows[i].append(*new NavInputControl(navMaster));
		windows[i].transform.quat().fromAxisAngle(M_PI/2 - i*M_PI/2, Vec3d(0, 1, 0));
		windows[i].cam.fovy(90);
	}

	// top/bottom windows
	for(int i=4; i<6; ++i){
		windows[i].append(*new StandardWindowKeyControls);
		windows[i].append(*new NavInputControl(navMaster));
		windows[i].transform.quat().fromAxisAngle(M_PI/2 - i*M_PI, Vec3d(1, 0, 0));
		windows[i].cam.fovy(90);
	}

	AudioIO audioIO(AUDIO_BLOCK_SIZE, 44100, audioCB, NULL, numSpeakers, 2);
	audioIO.start();

	MainLoop::start();
	return 0;
}
