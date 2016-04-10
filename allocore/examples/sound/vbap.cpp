


#include <atomic>

#include "allocore/al_Allocore.hpp"
#include "allocore/sound/al_Vbap.hpp"
#include "allocore/sound/al_Speaker.hpp"
#include "alloutil/al_AlloSphereSpeakerLayout.hpp"
#include "allocore/io/al_App.hpp"
#include "allocore/graphics/al_Font.hpp"

using namespace al;

#define BLOCK_SIZE (2048)
#define NUM_SOURCES 1
// Number of speakers in ring layout
#define NUM_SPEAKS_RING 8

enum{
	RING_LAYOUT,
	THREE_D_LAYOUT,
	ALLO_LAYOUT,
	TEST_LAYOUT
};

// per sample processing refers to how the positions are updated for sources
// in a scene. i.e. you can update the position of the source and hence
// the gains of the output per sample or per buffer.
bool perSampProcessing = false;
float speedMult = 0.05;

//int layout = ALLO_LAYOUT;
//int layout = TEST_LAYOUT;
int layout = THREE_D_LAYOUT;
//int layout = RING_LAYOUT;

SpeakerLayout speakerLayout;
Vbap* panner;
AudioScene scene(BLOCK_SIZE);
Listener * listener;
SoundSource srcs[NUM_SOURCES];

float srcElev = 1.f;

Vec3d srcpos(0.0,0.0,0.0);
std::atomic<float> *mPeaks;


//Currently bypasses al_AudioScene
inline void perSample(AudioIOData& io){

	static unsigned int t = 0;

	int numFrames = io.framesPerBuffer();

	for(int j=0; j<numFrames; j++){
		double sec = (t / io.fps());
		float fac = sec*2.0*M_PI;

		for(int i = 0; i < NUM_SOURCES; i++){

			//Calculate source position
			float tta = sec*speedMult*2.0*M_PI + (2.0*M_PI/NUM_SOURCES)*(i+1.0);
			float x = 12.0*cos(tta);
			float y = 12.0*sin(tta);
			float z = 4.0*sin(2.8 *tta);

			//Calculate source sample
			float smp = sin(fac*(880*((i/3.0)+1)))*(1.0/(i+1));

			Vec3d pos(x,y,srcElev + z);
			srcpos = pos;
			srcs[i].writeSample(smp);
			srcs[i].pos(x, y, srcElev + z - 0.5);
//			smp = srcs[i].attenuation(pos.mag())*smp;
			panner->perform(io,srcs[i],pos,BLOCK_SIZE,j,smp);
		}
		++t;
	}
}

inline void perBuffer(AudioIOData& io){

	static unsigned int t = 0;
	int numFrames = io.framesPerBuffer();
	double sec;
	for(int j=0; j<numFrames; j++){

		sec = (t / io.fps());
		// The audio for each source must still be computed ber sample
		for(int i = 0; i < NUM_SOURCES; i++){
			float smp = sin(sec*(220*(i+1))*2*M_PI)*(1.0/(i+1));
			srcs[i].writeSample(smp);
		}
		++t;
	}
	// But the positions can be computed once per buffer
	for(int i = 0; i < NUM_SOURCES; i++){
		float tta = sec*speedMult*2.0*M_PI + (2.0*M_PI/NUM_SOURCES)*(i+1.0);
		float x = 12.0*cos(tta);
		float y = 12.0*sin(tta);
		float z = 4.0*sin(2.8 * tta);
		Vec3d pos(x,y,srcElev + z);
		srcpos = pos;
		srcs[i].pos(x,y, z - 1.0);
	}
	scene.render(io);
}

void audioCB(AudioIOData& io){
	if(perSampProcessing){
		perSample(io);
	} else{
		perBuffer(io);
	}

	Speakers &speakers = speakerLayout.speakers();
	for (int i = 0; i < speakers.size(); i++) {
		mPeaks[i].store(0);
	}
	for (int speaker; speaker < speakers.size(); speaker++) {
		float rms = 0;
		for (int i = 0; i < io.framesPerBuffer(); i++) {
			int deviceChannel = speakers[speaker].deviceChannel;
			if(deviceChannel < io.channelsOut()) {
				float sample = io.out(speakers[speaker].deviceChannel, i);
				rms += sample * sample;
			}
		}
		rms = sqrt(rms/io.framesPerBuffer());
		mPeaks[speaker].store(rms);
	}
}


//Converts spatial coordinate system to Allocore's OpenGL coordinate system
Vec3d convertCoords(Vec3d v){
	Vec3d returnVector;
	returnVector.x = v.x;
	returnVector.y = v.z;
	returnVector.z = -v.y;
	return returnVector;
}

class MyApp : public App
{
	Font mFont;
	Mesh mText;

	Mesh mSpeakerMesh;
	std::vector<Mesh> mVec;

	std::vector<Vec3d> sCoords;
	std::vector<int>  sChannels;

	Light mLight;


public:
	MyApp(std::vector<SpeakerTriple> sts): mFont("allocore/share/fonts/VeraMoIt.ttf", 20) {

		mLight.pos(0,6,0);

		//Setup the triangles
		for(int j = 0; j < sts.size();++j){

			SpeakerTriple s = sts[j];
			Mesh* mesh = new Mesh();
			mesh->primitive(Graphics::TRIANGLES);

			for(int i = 0; i < 3; ++i){
				Vec3d coords = convertCoords(s.vec[i]);
				mesh->vertex(coords.x,coords.y,coords.z);
			}

			Color c(rnd::uniform(1.f,0.1f), rnd::uniform(), rnd::uniform(),1.0);
			for(int i = 0;i < 3;++i){
				mesh->color(c);
			}

			mVec.push_back(*mesh);

		}

		addSurface(mText,0,4);

		Speakers sp = speakerLayout.speakers();
		addSphere(mSpeakerMesh);
		for(int i = 0; i <speakerLayout.speakers().size();i++ ){
			Speaker s = sp[i];
			sChannels.push_back(s.deviceChannel);
			sCoords.push_back(convertCoords(s.vec()));
		}

		for(int i = 0; i < mVec.size(); ++i){
			mVec[i].decompress();
			mVec[i].generateNormals();
		}

		nav().pos(0, 2, 45);
		initWindow();
	}


	void onDraw(Graphics& g){
		g.blendAdd();

		//Draw the triangles
		for(int i = 0; i < mVec.size(); ++i){
			g.draw(mVec[i]);
		}

		//Draw one of the sources
		g.pushMatrix();
		g.translate(convertCoords(srcpos));
		g.scale(0.1);
		g.draw(mSpeakerMesh);
		g.popMatrix();

		//Draw the speakers
		for(int i = 0; i < sCoords.size(); ++i){

			g.pushMatrix();

			g.translate(sCoords[i]);
			float peak = mPeaks[i].load();
			g.scale(0.02 + 0.04 * peak * 300);

			int chan = sChannels[i];
			mFont.write(mText,std::to_string(chan));
			mFont.texture().bind();
			g.draw(mText);
			mFont.texture().unbind();

			g.draw(mSpeakerMesh);
			g.popMatrix();
		}
	}
};



int main (int argc, char * argv[]){

	if(layout == RING_LAYOUT){
		speakerLayout = SpeakerRingLayout<NUM_SPEAKS_RING>(0.f,0.f,10.f);
	}
	if(layout == THREE_D_LAYOUT){
		float rad = 10.0f;
		speakerLayout.addSpeaker(Speaker(0, 30, 0, rad));
		speakerLayout.addSpeaker(Speaker(1, -30, 0, rad));
		speakerLayout.addSpeaker(Speaker(2, 110, 0, rad));
		speakerLayout.addSpeaker(Speaker(3, -110, 0, rad));

		speakerLayout.addSpeaker(Speaker(4, 0, 80, rad));
		speakerLayout.addSpeaker(Speaker(5, 0, -80, rad));
	}


	//Creates three rings for testing different numbers o
	if(layout == TEST_LAYOUT){

		//Speakers per ring
		int r1 = 30;
		int r2 = 31;
		int r3 = 32;

		float el1 = 30.0f;
		float el2 = 0.0f;
		float el3 = -40.f;

		float rad = 20.0f;

		int spkInc = 0;

		for(int i = 0; i < r1;i++ ){
			speakerLayout.addSpeaker(Speaker(spkInc,(360.f/r1)*i,el1,rad));
			spkInc++;
		}

		for(int i = 0; i < r2;i++ ){
			speakerLayout.addSpeaker(Speaker(spkInc,(360.f/r2)*i,el2,rad));
			spkInc++;
		}
		for(int i = 0; i < r3;i++ ){
			speakerLayout.addSpeaker(Speaker(spkInc,(360.f/r3)*i,el3,rad));
			spkInc++;
		}
	}

	if(layout == ALLO_LAYOUT){
		//AlloSphereSpeakerLayout radius defaults to 1
		//This changes the radius
		SpeakerLayout tempLayout;
		tempLayout= AlloSphereSpeakerLayout();
		Speakers spks = tempLayout.speakers();

		for(Speaker s : spks){
			s.radius = 10.f;
			speakerLayout.addSpeaker(s);

		}
	}


	panner = new Vbap(speakerLayout);
	panner->setIs3D(layout==RING_LAYOUT ? false:true);
	listener = scene.createListener(panner);

	for(int i = 0; i < NUM_SOURCES; i++){
		srcs[i].usePerSampleProcessing(perSampProcessing);
		srcs[i].dopplerType(DOPPLER_NONE);
		scene.addSource(srcs[i]);
	}


	scene.usePerSampleProcessing(perSampProcessing);
	panner->setEnabled(true);
	panner->print();

	//Determine number of output channels
	Speakers allSpeakers = speakerLayout.speakers();
	int highestChannel = 0;
	for(Speaker s:allSpeakers){
		if(s.deviceChannel > highestChannel){
			highestChannel = s.deviceChannel;
		}
	}

	mPeaks = new std::atomic<float>[speakerLayout.speakers().size()];

	int outputChannels = highestChannel + 1;
	AudioIO audioIO(BLOCK_SIZE, 44100, audioCB, NULL, outputChannels, 0);
	audioIO.start();

	std::vector<SpeakerTriple> speakerTriangles = panner->triplets() ;

	MyApp(speakerTriangles).start();
	getchar();

	audioIO.stop();
	delete mPeaks;

	return 0;
}



