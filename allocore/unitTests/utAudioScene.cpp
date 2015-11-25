#include "utAllocore.h"

//#define assert

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

void testMultipleSources() {
	int bufferSize = 8;
	SpeakerLayout speakerLayout = HeadsetSpeakerLayout();
	StereoPanner *panner = new StereoPanner(speakerLayout);
	AudioScene scene(bufferSize);
	SoundSource src1,src2, src3, src4;
	scene.createListener(panner);
	AudioIO audioIO(bufferSize, 44100, NULL, NULL, speakerLayout.numSpeakers(), 0, AudioIOData::DUMMY);
//	scene.usePerSampleProcessing(false);
	src1.dopplerType(DOPPLER_NONE);
	src1.useAttenuation(false);
	src2.dopplerType(DOPPLER_NONE);
	src2.useAttenuation(false);
	src3.dopplerType(DOPPLER_NONE);
	src3.useAttenuation(false);
	src4.dopplerType(DOPPLER_NONE);
	src4.useAttenuation(false);
	scene.addSource(src1);
	scene.addSource(src2);
	scene.addSource(src3);
	scene.addSource(src4);

	for (int i = 0; i < 8; i++) {
		src1.writeSample(0.5);
		src2.writeSample(0.25);
		src3.writeSample(0.1);
		src4.writeSample(-0.3);
	}

	// Pan full right
	src1.pos(1, 0, 0);
	src2.pos(-1, 0, 0);
	src3.pos(-1, 0, 0);
	src4.pos(-1, 0, 0);
	scene.render(audioIO);

	for (int i = 0; i < 8; i++) {
		float left = audioIO.out(0, i);
		float right = audioIO.out(1, i);
		assert(almostEqual(left, 0.05));
		assert(almostEqual(right, 0.5));
	}
}


int utAudioScene() {
	testStereo();
	testMultipleSources();

	return 0;
}
