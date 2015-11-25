#include "utAllocore.h"

bool almostEqual(float v1, float v2) {
	return abs(v1 -v2) < 0.00001;
}

void testStereo() {
	int bufferSize = 8;
	SpeakerLayout speakerLayout = HeadsetSpeakerLayout();
	StereoPanner *panner = new StereoPanner(speakerLayout);
	AudioScene scene(bufferSize);
	SoundSource src;
	scene.createListener(panner);
	AudioIO audioIO(bufferSize, 44100, NULL, NULL, speakerLayout.numSpeakers(), 0, AudioIOData::DUMMY);
//	scene.usePerSampleProcessing(false);
    src.dopplerType(DOPPLER_NONE);
	src.useAttenuation(false);
	scene.addSource(src);
	
	for (int i = 0; i < 8; i++) {
		src.writeSample(0.5);
	}
	
	// Pan full right
	src.pos(1, 0, 0);
	scene.render(audioIO);
	
	for (int i = 0; i < 8; i++) {
		float left = audioIO.out(0, i);	
		float right = audioIO.out(1, i);
		assert(almostEqual(left, 0.0));
		assert(almostEqual(right, 0.5));
	}
	
	for (int i = 0; i < 8; i++) {
		src.writeSample(1.0);
	}
	
	// Pan center
	src.pos(0, 0, 0);
	scene.render(audioIO);
	
	for (int i = 0; i < 8; i++) {
		float left = audioIO.out(0, i);	
		float right = audioIO.out(1, i);
		assert(almostEqual(left, right));
		assert(almostEqual(right, sin(M_PI/4)));
	}
	
	
	for (int i = 0; i < 8; i++) {
		src.writeSample(0.75);
	}
	
	// Pan left
	src.pos(-1, 0, 0);
	scene.render(audioIO);
	
	for (int i = 0; i < 8; i++) {
		float left = audioIO.out(0, i);	
		float right = audioIO.out(1, i);
		assert(almostEqual(left, 0.75));
		assert(almostEqual(right, 0.0));
	}
	
	delete panner;
}



int utAudioScene() {
	testStereo();

	return 0;
}
