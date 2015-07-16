/*

*/

#include <iostream>

#include "allocore/io/al_App.hpp"
#include "allocore/types/al_SingleRWRingBuffer.hpp"
#include "alloaudio/al_OutputMaster.hpp"

using namespace std;
using namespace al;

static void addWaveformDisplay(float *audioBuffer, int numElements, Mesh& m) {
  int zoomOut = 1;  // # audio samples per OpenGL Mesh vertex
  m.primitive(Graphics::LINE_STRIP);

  int n = 0;  // current sample #
  for (int i = 0; i < numElements; ++i) {
    float max = -1, min = 1;
    for (int j = 0; j < zoomOut; ++j) {
	  if (audioBuffer[n] > max)
		max = audioBuffer[n];
	  if (audioBuffer[n] < min)
		min = audioBuffer[n];
      n++;
    }
	float verticalScale = 1;
	m.vertex((float)i / (numElements / zoomOut), max*verticalScale, 0);
	m.vertex((float)i / (numElements / zoomOut), min*verticalScale, 0);
  }
}

class MyApp : public App{
public:
	double phase;

	MyApp(int num_chnls, double sampleRate) :
		mNumChannels(num_chnls)
	{
		mAudioBuffers = (SingleRWRingBuffer **) calloc(num_chnls, sizeof(SingleRWRingBuffer *));
		for (int i = 0; i < mNumChannels; i++) {
			mAudioBuffers[i] = new SingleRWRingBuffer(8192*sizeof(float));
		}
		nav().pos(0,0,2);

        initWindow();
		initAudio(sampleRate, 512, mNumChannels, mNumChannels);
    }

    // Audio callback
    void onSound(AudioIOData& io){
		for (int i = 0; i < mNumChannels; i++) {
			const char *inbuf = (char *) io.inBuffer(i);
			int written = mAudioBuffers[i]->write(inbuf, io.framesPerBuffer() * sizeof(float) );
			if (written < io.framesPerBuffer() * sizeof(float)) {
				std::cout << "Buffer overrun!" << std::endl;
			}
		}
    }

    void onAnimate(double dt){

    }

    void onDraw(Graphics& g, const Viewpoint& v){
		for (int i = 0; i < mNumChannels; i++) {
			int numFrames = mAudioBuffers[i]->read((char *) mFloatBuffer, 8192 * sizeof(float) );
			Mesh m;
			m.reset();
			addWaveformDisplay(mFloatBuffer, numFrames/sizeof(float), m);
			g.pushMatrix();
			g.translate(-0.5, 0.3 * i, 0);
			g.draw(m);
			g.popMatrix();
		}
    }
	SingleRWRingBuffer **mAudioBuffers;

	float mFloatBuffer[8192*sizeof(float)];
	int mNumChannels;
};


int main(){
	int num_chnls = 2;
	double sampleRate = 44100;
	MyApp(num_chnls, sampleRate).start();
}
