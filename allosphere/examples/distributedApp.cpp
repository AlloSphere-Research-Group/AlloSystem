#include <string>  // memset, memcpy
#include <vector>    // vector
#include <iostream>  // cout

#include "allosphere/al_AlloApp.hpp"
#include "allocore/math/al_Vec.hpp"
#include "allocore/graphics/al_Mesh.hpp"
#include "allocore/spatial/al_Pose.hpp"
#include "allocore/io/al_File.hpp"
#include "allocore/math/al_Random.hpp"

#include <Gamma/Noise.h>
#include <Gamma/Oscillator.h>

// FIXME lens() is ambiguous it's in Simulator and OmniGraphicsRendered


using namespace al;
using namespace std;

#define PPCAT_NX(A, B) A##B
#define PPCAT(A, B) PPCAT_NX(A, B)
#define STRINGIZE_NX(A) #A
#define STRINGIZE(A) STRINGIZE_NX(A)

#define NUM_VERTICES 162
//#define NUM_VERTICES 642
//#define NUM_VERTICES 2562
//#define NUM_VERTICES 10242
//#define NUM_VERTICES 40962
//#define NUM_VERTICES 163842
//#define NUM_VERTICES 655362

#define ICOSPHERE_FILE STRINGIZE(PPCAT(NUM_VERTICES, .ico))

struct State {

  // these are the basics. most AlloSphere Apps will want these.
  //
  double t;    // simulation time
  unsigned n;  // "frame" number
  Pose pose;   // for navigation

  // this is how you might control renderering settings.
  //
  double eyeSeparation, nearClip, farClip, focalLength;
  double audioGain;
  Color backgroundColor;
  bool wireFrame;  // alternative is shaded

  Vec3f lightPosition;

  unsigned pokedVertex;
  Vec3f pokedVertexRest;

  // all of the above data could be distributed using unicast OSC to each
  // renderering host (it's only about 60 bytes), but this scheme would suffer
  // from tearing due to out-of-sync sending and receiving of packets. the win
  // above is that all this data appears at each renderering host
  // simultaneously because we're using UDP broadcast, which does not use a
  // foreach to send NUM_VERTICES identical messages to NUM_VERTICES renderering hosts.
  //
  // below is another, different win. here we have an array of vertices that
  // could get as big as the network will bear (~10MB). this data is calculated
  // on the server/simulator, so it does not need to be calculated on the
  // renderer, only interpreted.
  //

  Vec3f p[NUM_VERTICES];
};

bool load(string fileName, Mesh& mesh, vector<vector<int> >& nn) {
  ifstream file(fileName);
  if (!file.is_open()) return false;

  string line;
  int state = 0;
  while (getline(file, line)) {
    if (line == "|") {
      state++;
      continue;
    }
    switch (state) {
      case 0: {
        vector<float> v;
        stringstream ss(line);
        float f;
        while (ss >> f) {
          v.push_back(f);
          if (ss.peek() == ',') ss.ignore();
        }
        mesh.vertex(v[0], v[1], v[2]);
        // cout << v[0] << "|" << v[1] << "|" << v[2] << endl;
      } break;

      case 1: {
        stringstream ss(line);
        int i;
        if (ss >> i)
          mesh.index(i);
        else
          return false;
        // cout << i << endl;
      } break;

      case 2: {
        vector<int> v;
        stringstream ss(line);
        int i;
        while (ss >> i) {
          v.push_back(i);
          if (ss.peek() == ',') ss.ignore();
        }
        if ((v.size() != 5) && (v.size() != 6)) return false;
        nn.push_back(v);
        // cout << nn[nn.size() - 1].size() << endl;
      } break;
    }
  }
  file.close();

  return true;
}


// simulation parameters
//
#define P (200)     // how often to "poke" the blob
#define SK (0.06f)  // spring constant for anchor points
#define NK (0.1f)   // spring constant between neighbors
#define D (0.08f)   // damping factor




//struct Blob : App, AlloSphereAudioSpatializer, InterfaceServerClient {

//  cuttlebone::Maker<State, 9000> maker;
//  State* state;

//  vector<vector<int>> nn;
//  vector<Vec3f> velocity;
//  vector<Vec3f> original;

//  // a boolean value that is read and reset (false) by the simulation step and
//  // written (true) by audio, keyboard and mouse callbacks.
//  //
//  bool shouldPoke;

//  // a mesh we use to do graphics rendering in this app
//  //
//  Mesh mesh;

//  // noise generator
//  gam::NoisePink<> pinkNoise;
//  gam::Sine<> sine;

//  // sound source to represent a sound in space
//  SoundSource tap;

//  Blob()
//    : maker(Simulator::defaultBroadcastIP()),
//      InterfaceServerClient(Simulator::defaultInterfaceServerIP()) {
//    // create our state struct
//    state = new State;
//    memset(state, 0, sizeof(State));

//    shouldPoke = true;

//    SearchPaths searchPaths;
//    searchPaths.addSearchPath(".", false);
//    searchPaths.addSearchPath("/alloshare/blob", false);
//    searchPaths.addAppPaths();

//    // load a mesh from file
//    mesh.primitive(Graphics::TRIANGLES);
//    if (!load(searchPaths.find(ICOSPHERE_FILE).filepath(), mesh, nn)) {
//      cout << "cannot find " << ICOSPHERE_FILE << endl;
//      Main::get().stop();
//    }

//    // initialize simulator local state
//    velocity.resize(mesh.vertices().size(), Vec3f(0, 0, 0));
//    original.resize(mesh.vertices().size());
//    for (int i = 0; i < mesh.vertices().size(); i++)
//      original[i] = mesh.vertices()[i];

//    sine.freq(440);

//    // initialize lens parameters
//    lens().near(0.1);
//    lens().far(100);
//    lens().focalLength(6);
//    lens().eyeSepAuto();

//    // initialize state to be broadcasted
//    state->n = 0;
//	for (int i = 0; i < NUM_VERTICES; i++) state->p[i] = original[i];
//    state->audioGain = 0.97;
//    state->backgroundColor = Color(0.1, 1);
//    state->wireFrame = true;

//    initWindow(Window::Dim(0, 0, 600, 400), "Blob Control Center", 60);

//    // init audio and ambisonic spatialization
//    AlloSphereAudioSpatializer::initAudio();
//    AlloSphereAudioSpatializer::initSpatialization();
//    gam::Sync::master().spu(AlloSphereAudioSpatializer::audioIO().fps());

//    // add our sound source to the audio scene
//    scene()->addSource(tap);

//    // use this for smoother spatialization and dopler effect
//    // good for fast moving sources or listener
//    // computationally expensive!!
//    scene()->usePerSampleProcessing(true);
    
//    // set interface server nav/lens to App's nav/lens
//    InterfaceServerClient::setNav(nav());
//    InterfaceServerClient::setLens(lens());
//   }

//  virtual void poke() {
//      shouldPoke = false;
//	  int n = rnd::uniform(NUM_VERTICES);
//      state->pokedVertex = n;
//      state->pokedVertexRest = original[n];
//      Vec3f v = Vec3f(rnd::uniformS(), rnd::uniformS(), rnd::uniformS());
//      for (unsigned k = 0; k < nn[n].size(); k++) state->p[nn[n][k]] += v * 0.5;
//      state->p[n] += v;

//      sine.freq(n*20. + 100.);

//      // Illuminate the poke with this weird trick...
//      state->lightPosition = (state->pokedVertexRest) * 2.3 + v * 4.567890;

//      LOG("poke!");
//  }


//  virtual void onAnimate(double dt) {
//    static cuttlebone::Stats fps("Blob::step");
//    fps(dt);

//    // handle messages from interface server
//    while (InterfaceServerClient::oscRecv().recv()) {}

//    // simulate
//    if (shouldPoke) poke();

//	for (int i = 0; i < NUM_VERTICES; i++) {
//      Vec3f& v = state->p[i];
//      Vec3f force = (v - original[i]) * -SK;

//      for (int k = 0; k < nn[i].size(); k++) {
//        Vec3f& n = state->p[nn[i][k]];
//        force += (v - n) * -NK;
//      }

//      force -= velocity[i] * D;
//      velocity[i] += force;
//    }

//    // update each point from simulation
//	for (int i = 0; i < NUM_VERTICES; i++) state->p[i] += velocity[i];

//    // update basic state parameters
//    //
//    state->t += dt;
//    state->n++;
//    state->pose = nav();
//    state->nearClip = lens().near();
//    state->farClip = lens().far();
//    state->focalLength = lens().focalLength();
//    state->eyeSeparation = lens().eyeSep();

//    // set the new state (it'll get broadcast to each renderer)
//    //
//    maker.set(*state);

//    // copy the vertex positions into the mesh
//    //
//	memcpy(&mesh.vertices()[0], &state->p[0], sizeof(Vec3f) * NUM_VERTICES);
//  }

//  virtual void onDraw(Graphics& g, const Viewpoint& v) {
//    g.clearColor(state->backgroundColor);
//    if (state->wireFrame)
//      g.polygonMode(Graphics::LINE);
//    else
//      g.polygonMode(Graphics::FILL);
//    g.draw(mesh);
//  }

//  virtual void onSound(AudioIOData& io) {
//    static cuttlebone::Stats fps("onSound()");
//    static float currentNoiseAmplitude = 0;

//    fps(io.secondsPerBuffer());

//    float maxInputAmplitude = 0.0f;
    
//    // set the pose of our audio source
//    tap.pose(Pose(state->p[state->pokedVertex], Quatf()));

//    // "f" is the desired noise amplitude based on the state
//    float f =
//      (state->p[state->pokedVertex] - state->pokedVertexRest).mag() - 0.45;
    
//    if (f > 0.99) f = 0.99;
//    if (f < 0) f = 0;



//    while (io()) {
//      // find largest input amplitude of block
//      //
//      float in = fabs(io.in(0));
//      if (in > maxInputAmplitude) maxInputAmplitude = in;

//      // Make this 0.0001 so it will sound good
//      float ProportionOfErrorToEliminateOnThisAudioSample = 0.0001;
//      currentNoiseAmplitude += (f-currentNoiseAmplitude) *
//        ProportionOfErrorToEliminateOnThisAudioSample;

//      // output sample directly or write sample to output through our audio source
//      // io.out(0) = io.out(1) = pinkNoise() * f * state->audioGain;
//      // tap.writeSample( pinkNoise() * currentNoiseAmplitude * state->audioGain );
//      tap.writeSample( sine() * currentNoiseAmplitude * state->audioGain );
//    }

//    // poke the blob if the largest amplitude is above some threshold
//    //
//    if (maxInputAmplitude > 0.707f) shouldPoke = true;

//    // set listener pose and render audio sources
//    listener()->pose(state->pose);
//    scene()->render(io);
//  }

//  virtual void onKeyDown(const ViewpointWindow& w, const Keyboard& k) {
//    shouldPoke = true;

//    if (k.key() == ' ') {
//      state->wireFrame = !state->wireFrame;
//      if (state->wireFrame) {
//        LOG("wireframe on");
//      } else {
//        LOG("wireframe off");
//      }
//    }
//  }

//  virtual void onMouseDown(const ViewpointWindow& w, const Mouse& m) {
//    shouldPoke = true;
//  }
//};


// simulation parameters
//
#define SK (0.06f)  // spring constant for anchor points
#define NK (0.1f)   // spring constant between neighbors
#define D (0.08f)   // damping factor



//struct Blob : AudioRenderer {
//  cuttlebone::Taker<State, 9000> taker;
//  State* state;

//  SoundSource tap;

//  Blob() {
//	state = new State;
//	memset(state, 0, sizeof(State));
//	// initAudio();
//	initSpatialization();
//	scene()->addSource(tap);
//  }

//  virtual ~Blob() {}

//  virtual void onSound(AudioIOData& io) {
//	static cuttlebone::Stats fps("onSound()");
//	fps(io.secondsPerBuffer());

//	tap.pose(Pose(state->p[state->pokedVertex], Quatd(1, 0, 0, 0)));

//	int popCount = taker.get(*state);
//	float f = (state->p[state->pokedVertex] - state->pokedVertexRest).mag() - 0.304134 /* mean */;
//	if (f > 0.8) f = 0.8;
//	if (f < 0) f = 0;
//	while (io()) {
//	  // XXX rnd::uniformS() should be gamma noise
//	  // XXX use interpolation on gain
//	  tap.writeSample(rnd::uniformS() * f);
//	  // io.out(0) = rnd::uniformS() * f;
//	}
//	listener()->pose(state->pose);
//	scene()->render(io);
//  }

//  virtual void start() {
//	taker.start();           // non-blocking
//	AudioRenderer::start();  // blocks
//  }
//};



//class SphereApp : App, AlloSphereAudioSpatializer, InterfaceServerClient {

//  cuttlebone::Maker<State, 9000> maker;
//  State* state;

//  vector<vector<int>> nn;
//  vector<Vec3f> velocity;
//  vector<Vec3f> original;

//  // a boolean value that is read and reset (false) by the simulation step and
//  // written (true) by audio, keyboard and mouse callbacks.
//  //
//  bool shouldPoke;

//  // a mesh we use to do graphics rendering in this app
//  //
//  Mesh mesh;

//  // noise generator
//  gam::NoisePink<> pinkNoise;

//  // sound source to represent a sound in space
//  SoundSource tap;

//  SphereApp()
//    : maker(Simulator::defaultBroadcastIP()),
//      InterfaceServerClient(Simulator::defaultInterfaceServerIP()) {
//    // create our state struct
//    state = new State;
//    memset(state, 0, sizeof(State));

//    shouldPoke = true;

//    SearchPaths searchPaths;
//    searchPaths.addSearchPath(".", false);
//    searchPaths.addSearchPath("/alloshare/blob", false);
//    searchPaths.addAppPaths();

//    // load a mesh from file
//    mesh.primitive(Graphics::TRIANGLES);


//    // initialize simulator local state


//    // initialize lens parameters
//    lens().near(0.1);
//    lens().far(100);
//    lens().focalLength(6);
//    lens().eyeSepAuto();

//    // initialize state to be broadcasted
//    state->n = 0;
//    for (int i = 0; i < NUM_VERTICES; i++) state->p[i] = original[i];
//    state->audioGain = 0.97;
//    state->backgroundColor = Color(0.1, 1);
//    state->wireFrame = true;

//    initWindow(Window::Dim(0, 0, 600, 400), "Blob Control Center", 60);

//    // init audio and ambisonic spatialization
//    AlloSphereAudioSpatializer::initAudio();
//    AlloSphereAudioSpatializer::initSpatialization();
//    // gam::Sync::master().spu(AlloSphereAudioSpatializer::audioIO().fps());

//    // add our sound source to the audio scene
//    scene()->addSource(tap);


//    // OSC
//    App::oscRecv().open(9999, "", 0.1, Socket::UDP|Socket::DGRAM);
//    App::oscRecv().handler(*this);
//    App::oscRecv().start();

//    // set interface server nav/lens to App's nav/lens
//    InterfaceServerClient::setNav(nav());
//    InterfaceServerClient::setLens(lens());
//  }





//};

using namespace al;

class MyApp: public AlloApp<State>
{
public:
	MyApp() : AlloApp() {

		shouldPoke = true;
		mesh.primitive(Graphics::TRIANGLES);

		SearchPaths searchPaths;
		searchPaths.addSearchPath(".", false);
		searchPaths.addSearchPath("/alloshare/blob", false);
		searchPaths.addSearchPath("allosphere/examples", false);
		searchPaths.addSearchPath("/home/andres/Documents/src/Allostuff/AlloSystem-github/allosphere/examples", false);
		searchPaths.addAppPaths();

		if (!load(searchPaths.find(ICOSPHERE_FILE).filepath(), mesh, nn)) {
			cout << "cannot find " << ICOSPHERE_FILE << endl;
			Main::get().stop();
		}

		velocity.resize(mesh.vertices().size(), Vec3f(0, 0, 0));
		original.resize(mesh.vertices().size());
		for (int i = 0; i < mesh.vertices().size(); i++) {
			original[i] = mesh.vertices()[i];
		}
		OmniStereoGraphicsRenderer::lens().near(0.1);
		OmniStereoGraphicsRenderer::lens().far(100);
		OmniStereoGraphicsRenderer::lens().focalLength(6);
		OmniStereoGraphicsRenderer::lens().eyeSepAuto();

		scene()->addSource(tap);
		init();
		initState();
	}


	virtual void init() {
		cout << "init" << endl;
	}

	virtual bool initState() {
		cout << "initState()" << endl;
		state()->n = 0;
		for (int i = 0; i < NUM_VERTICES; i++) state()->p[i] = original[i];
		state()->audioGain = 0.1;
		state()->backgroundColor = Color(0.1, 1);
		state()->wireFrame = true;
		return true;
	}

//	virtual void onAnimate(double dt) {
//		// static cuttlebone::Stats fps("onAnimate()");
//		// fps(dt);
//		int popCount = taker().get(*state());
//		if (popCount > 0) {
//			memcpy(&mesh.vertices()[0], &state()->p[0], sizeof(Vec3f) * NUM_VERTICES);
//			if (!state()->wireFrame) mesh.generateNormals();
//			pose = state()->pose;
//		}
//	}


	virtual void onAnimate(double dt) {
		static cuttlebone::Stats fps("Blob::step");
		fps(dt);

		// handle messages from interface server
		while (InterfaceServerClient::oscRecv().recv()) {}

		// simulate
		if (shouldPoke) {
			shouldPoke = false;
			int n = rnd::uniform(NUM_VERTICES);
			state()->pokedVertex = n;
			state()->pokedVertexRest = original[n];
			Vec3f v = Vec3f(rnd::uniformS(), rnd::uniformS(), rnd::uniformS());
			for (unsigned k = 0; k < nn[n].size(); k++) state()->p[nn[n][k]] += v * 0.5;
			state()->p[n] += v;
			LOG("poke!");
		}

		for (int i = 0; i < NUM_VERTICES; i++) {
			Vec3f& v = state()->p[i];
			Vec3f force = (v - original[i]) * -SK;

			for (int k = 0; k < nn[i].size(); k++) {
				Vec3f& n = state()->p[nn[i][k]];
				force += (v - n) * -NK;
			}

			force -= velocity[i] * D;
			velocity[i] += force;
		}

		// update each point from simulation
		for (int i = 0; i < NUM_VERTICES; i++) state()->p[i] += velocity[i];

		// update basic state parameters
		//
		state()->t += dt;
		state()->n++;
		state()->pose = OmniStereoGraphicsRenderer::nav();
		state()->nearClip = OmniStereoGraphicsRenderer::lens().near();
		state()->farClip = OmniStereoGraphicsRenderer::lens().far();
		state()->focalLength = OmniStereoGraphicsRenderer::lens().focalLength();
		state()->eyeSeparation = OmniStereoGraphicsRenderer::lens().eyeSep();

		// set the new state (it'll get broadcast to each renderer)
		//
		maker().set(*state());

		// copy the vertex positions into the mesh
		//
		memcpy(&mesh.vertices()[0], &state()->p[0], sizeof(Vec3f) * NUM_VERTICES);
	}



	virtual void onDraw(Graphics& g) {
	  shader().uniform("lighting", 0.5);
	  light.pos(state()->lightPosition);
	  light();

	  omni().clearColor() = state()->backgroundColor;
	  omni().sphereRadius(state()->focalLength);
	  OmniStereoGraphicsRenderer::lens().near(state()->nearClip);
	  OmniStereoGraphicsRenderer::lens().far(state()->farClip);
	  OmniStereoGraphicsRenderer::lens().eyeSep(state()->eyeSeparation);
	  if (state()->wireFrame)
		g.polygonMode(Graphics::LINE);
	  else
		g.polygonMode(Graphics::FILL);
	  g.color(1,1,1);
	  g.draw(mesh);
	}

	//  virtual void onDraw(Graphics& g, const Viewpoint& v) {
	//    lens().near(state->nearClip);
	//    lens().far(state->farClip);
	//    g.clearColor(state->backgroundColor);
	//    if (state->wireFrame)
	//      g.polygonMode(Graphics::LINE);
	//    else
	//      g.polygonMode(Graphics::FILL);
	//    g.draw(mesh);
	//  }


	virtual void onSound(AudioIOData& io) {
		static cuttlebone::Stats fps("onSound()");
		static float currentNoiseAmplitude = 0;

		fps(io.secondsPerBuffer());

		float maxInputAmplitude = 0.0f;

		// set the pose of our audio source
		tap.pose(Pose(state()->p[state()->pokedVertex], Quatf()));

		// "f" is the desired noise amplitude based on the state
		float f =
				(state()->p[state()->pokedVertex] - state()->pokedVertexRest).mag() - 0.45;

		if (f > 0.99) f = 0.99;
		if (f < 0) f = 0;



		while (io()) {
			// find largest input amplitude of block
			//
			float in = fabs(io.in(0));
			if (in > maxInputAmplitude) maxInputAmplitude = in;

			// Make this 0.0001 so it will sound good
			float ProportionOfErrorToEliminateOnThisAudioSample = 0.0001;
			currentNoiseAmplitude += (f-currentNoiseAmplitude) *
					ProportionOfErrorToEliminateOnThisAudioSample;

			// output sample directly or write sample to output through our audio source
			// io.out(0) = io.out(1) = pinkNoise() * f * state->audioGain;
			tap.writeSample( pinkNoise() * currentNoiseAmplitude * state()->audioGain );
		}

		// poke the blob if the largest amplitude is above some threshold
		//
		if (maxInputAmplitude > 0.707f) shouldPoke = true;

		// set listener pose and render audio sources
		listener()->pose(state()->pose);
		scene()->render(io);
	}

	virtual void onMessage(osc::Message& m) {
//		cout << "Received OSC message: ";
//		m.print();

		if (m.addressPattern() == "/poke") {
			shouldPoke = true;
		} else if(m.addressPattern() == "/accelerometer"){
			float x,y,z;
			m >> x >> y >> z;     // read three floats from message
			Vec3f v(x,y,z);
			float mag = v.mag();
			cout << "accelerometer magnitude: " << mag << endl;
			if(mag > 0.9) shouldPoke = true;
		}
	}

	virtual bool onKeyDown(const ViewpointWindow& w, const Keyboard& k) {
		shouldPoke = true;
		if (k.key() == ' ') {
			state()->wireFrame = !state()->wireFrame;
			if (state()->wireFrame) {
				LOG("wireframe on");
			} else {
				LOG("wireframe off");
			}
		}
		return true;
	}

	virtual bool onMouseDown(const ViewpointWindow& w, const Mouse& m) {
		shouldPoke = true;
		return true;
	}

private:
	Mesh mesh;
	Light light;
	bool shouldPoke;
	vector<vector<int>> nn;
	vector<Vec3f> velocity;
	vector<Vec3f> original;
	// noise generator
	gam::NoisePink<> pinkNoise;
	gam::Sine<> sine;

	// sound source to represent a sound in space
	SoundSource tap;
};

int main() {
	MyApp app;
	app.start();
}





