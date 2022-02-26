#include "utAllocore.h"
#include "allocore/io/al_AudioIO.hpp"
#include "allocore/sound/al_Ambisonics.hpp"
#include "allocore/sound/al_AudioScene.hpp"
#include "allocore/sound/al_StereoPanner.hpp"
#include "allocore/sound/al_Vbap.hpp"
using namespace al;

// To support old ordering
#define AIO_OUT(chan, frame) io.bufferOut().at(frame, chan)

void testBasicStereo() {
	SpeakerLayout speakerLayout = HeadsetSpeakerLayout();
	StereoPanner panner(speakerLayout);
	//AudioIO io(8, 44100, NULL, NULL, speakerLayout.numSpeakers(), 0);
	AudioIO io;
	io.configure(8, 44100, speakerLayout.numSpeakers(), 0);

	io.bufferOut().zero(); // out buffers contain garbage by default, must zero manually as not using the callback facilities.

	Pose listeningPose(Vec3d(0, 0, 4)); // Default Pose facing forward
	panner.renderSample(io, listeningPose, 0.5, 0);

	assert(almostEqual(AIO_OUT(0, 0), 0.5 * sin(M_PI/4.0)));
	assert(almostEqual(AIO_OUT(0, 1), 0.5 * sin(M_PI/4.0)));

	listeningPose = Pose(Vec3d(1, 0, 0)); // Full pan right
	panner.renderSample(io, listeningPose, 0.5, 1);

	assert(almostEqual(AIO_OUT(1, 0), 0.0));
	assert(almostEqual(AIO_OUT(1, 1), 0.5));

	listeningPose = Pose(Vec3d(-1, 0, 0)); // Full pan left
	panner.renderSample(io, listeningPose, 0.5, 2);

	assert(almostEqual(AIO_OUT(2, 0), 0.5));
	assert(almostEqual(AIO_OUT(2, 1), 0.0));

	// Test buffer rendering
	io.bufferOut().zero();
	listeningPose = Pose(Vec3d(0, 0, 4)); // Default Pose facing forward
	float input[8] = {0.5, 0.5, 0.4, 0.3, 0.2, 0.1, -0.3, -0.4};
	panner.renderBuffer(io, listeningPose, input, 8);

	for (int i = 0; i < 8; i++) {
		assert(almostEqual(AIO_OUT(i, 0), input[i] * sin(M_PI/4.0)));
		assert(almostEqual(AIO_OUT(i, 1), input[i] * sin(M_PI/4.0)));
	}

	io.bufferOut().zero();
	listeningPose = Pose(Vec3d(1, 0, 0));
	panner.renderBuffer(io, listeningPose, input, 8);

	for (int i = 0; i < 8; i++) {
		assert(almostEqual(AIO_OUT(i, 0), 0.0));
		assert(almostEqual(AIO_OUT(i, 1), input[i]));
	}

}

void testStereoAudioScene(int bufferSize) {
	SpeakerLayout speakerLayout = HeadsetSpeakerLayout();
	StereoPanner *panner = new StereoPanner(speakerLayout);
	AudioScene scene(bufferSize);
	SoundSource src;
	Listener *listener = scene.createListener(panner);
	AudioIO io;
	io.configure(bufferSize, 44100, speakerLayout.numSpeakers(), 0);
	src.dopplerType(DOPPLER_NONE);
	src.useAttenuation(false);
	scene.addSource(src);

	for (int i = 0; i < bufferSize; i++) {
		src.writeSample(0.5);
	}

	src.pos(1, 0, 0); // Pan full right
	scene.render(io);

	for (int i = 0; i < bufferSize; i++) {
		float left = AIO_OUT(0, i);
		float right = AIO_OUT(1, i);
		assert(almostEqual(left, 0.0));
		assert(almostEqual(right, 0.5));
	}

	 // Render another buffer without changing position
	for (int i = 0; i < bufferSize; i++) {
		src.writeSample(0.6);
	}

	scene.render(io);

	for (int i = 0; i < bufferSize; i++) {
		float left = AIO_OUT(0, i);
		float right = AIO_OUT(1, i);
		assert(almostEqual(left, 0.0));
		assert(almostEqual(right, 0.6));
	}

	//

	for (int i = 0; i < bufferSize; i++) {
		src.writeSample(1.0);
	}

	// Pan center
	src.pos(0, 0, -4);
	scene.render(io);

	for (int i = 0; i < bufferSize; i++) {
		float left = AIO_OUT(0, i);
		float right = AIO_OUT(1, i);
		assert(almostEqual(left, sin(M_PI/4)));
		assert(almostEqual(right, sin(M_PI/4)));
	}

	for (int i = 0; i < bufferSize; i++) {
		src.writeSample(0.75);
	}

	// Pan left
	src.pos(-1, 0, 0);
	scene.render(io);

	for (int i = 0; i < bufferSize; i++) {
		float left = AIO_OUT(0, i);
		float right = AIO_OUT(1, i);
		assert(almostEqual(left, 0.75));
		assert(almostEqual(right, 0.0));
	}

	delete panner;
}

void testMultipleSourcesStereo(int bufferSize) {
	SpeakerLayout speakerLayout = HeadsetSpeakerLayout();
	StereoPanner *panner = new StereoPanner(speakerLayout);
	AudioScene scene(bufferSize);
	SoundSource src1,src2, src3, src4;
	scene.createListener(panner);
	AudioIO io;
	io.configure(bufferSize, 44100, speakerLayout.numSpeakers(), 0);

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

	for (int i = 0; i < bufferSize; i++) {
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
	scene.render(io);

	for (int i = 0; i < bufferSize; i++) {
		float left = AIO_OUT(0, i);
		float right = AIO_OUT(1, i);
		assert(almostEqual(left, 0.05));
		assert(almostEqual(right, 0.5));
	}
}

void testMultipleSourcesMovingStereo() {
	int bufferSize = 8;
	SpeakerLayout speakerLayout = HeadsetSpeakerLayout();
	StereoPanner *panner = new StereoPanner(speakerLayout);
	AudioScene scene(bufferSize);
	SoundSource src1,src2, src3, src4;
	scene.createListener(panner);
	AudioIO io;
	io.configure(bufferSize, 44100, speakerLayout.numSpeakers(), 0);
	scene.usePerSampleProcessing(true); // to enable interpolation of movement
	src1.dopplerType(DOPPLER_NONE);
	src1.useAttenuation(false);
	src2.dopplerType(DOPPLER_NONE);
	src2.useAttenuation(false);
	src3.dopplerType(DOPPLER_NONE);
	src3.useAttenuation(false);
	scene.addSource(src1);
	scene.addSource(src2);
	scene.addSource(src3);

	for (int i = 0; i < bufferSize; i++) {
		src1.writeSample(0.492);
		src2.writeSample(0.25);
		src3.writeSample(0.1);
	}

	src1.pos(1, 0, 0);
	src2.pos(-1, 0, 0);
	src3.pos(-1, 0, 0);
	scene.render(io);

	for (int i = 0; i < bufferSize; i++) {
		float left = AIO_OUT(0, i);
		float right = AIO_OUT(1, i);
		assert(almostEqual(left, 0.35));
		assert(almostEqual(right, 0.492));
	}

	src1.pos(-1, 0, 0);
	src2.pos(1, 0, 0);
	src3.pos(1, 0, 0);
	scene.render(io);

	for (int i = 0; i < bufferSize; i++) {
		float left = AIO_OUT(0, i);
		float right = AIO_OUT(1, i);
		assert(almostEqual(left, 0.492));
		assert(almostEqual(right, 0.35));
	}
}

void testHeadphoneRendering() {
//	FIXME add tests for headphone speaker leayout
}


void testVbapTriples() {
	// Simple 2D triangle
	SpeakerLayout speakerLayout;
	speakerLayout.addSpeaker(Speaker(0, 0, 0, 1));
	speakerLayout.addSpeaker(Speaker(-120, 0, 0, 1));
	speakerLayout.addSpeaker(Speaker(120, 0, 0, 1));
	Vbap panner(speakerLayout);

	std::vector<SpeakerTriple> triples = panner.triplets();
	assert(triples[0].s1 == 0 && triples[0].s2 == 1);
	assert(triples[1].s1 == 1 && triples[1].s2 == 2);
	assert(triples[2].s1 == 2 && triples[2].s2 == 0);

	//Octahedron
	SpeakerLayout speakerLayout3D;
	speakerLayout3D.addSpeaker(Speaker(0, 0, 0, 1));
	speakerLayout3D.addSpeaker(Speaker(1, -90, 0, 1));
	speakerLayout3D.addSpeaker(Speaker(2, 90, 0, 1));
	speakerLayout3D.addSpeaker(Speaker(3, 180, 0, 1));
	speakerLayout3D.addSpeaker(Speaker(4, 0, 90, 1));
	speakerLayout3D.addSpeaker(Speaker(5, 0, -90, 1));

	Vbap panner3D(speakerLayout3D, true);
	assert(panner3D.triplets().size() == 8);

	// FIXME add testing for triplet generation of provided layouts


}

void testVbapGains() {
	AudioIO io;
	io.configure(8, 44100, 8, 0);
	// Simple 2D triangle
	SpeakerLayout speakerLayout;
	speakerLayout.addSpeaker(Speaker(0, 0, 0, 1));
	speakerLayout.addSpeaker(Speaker(1, -120, 0, 1));
	speakerLayout.addSpeaker(Speaker(2, 120, 0, 1));
	Vbap panner(speakerLayout);
	io.bufferOut().zero(); // out buffers contain garbage by default, must zero manually as not using the callback facilities.

	Pose listeningPose(Vec3d(0, 0, 4)); // Default Pose facing forward
	panner.renderSample(io, listeningPose, 0.5, 0);

	assert(almostEqual(AIO_OUT(0, 0), 0.5));
	assert(almostEqual(AIO_OUT(1, 0), 0.0));
	assert(almostEqual(AIO_OUT(2, 0), 0.0));

	listeningPose = Pose(Vec3d(0, 0, -4)); // Default Pose facing back
	panner.renderSample(io, listeningPose, 0.5, 1);

	assert(almostEqual(AIO_OUT(0, 1), 0));
	assert(almostEqual(AIO_OUT(1, 1), 0.5 * cos(M_PI/4.0)));
	assert(almostEqual(AIO_OUT(2, 1), 0.5 * cos(M_PI/4.0)));

	listeningPose = Pose(Vec3d(0, 0, -4)); // Back
	panner.renderSample(io, listeningPose, 0.25, 2);

	assert(almostEqual(AIO_OUT(0, 2), 0));
	assert(almostEqual(AIO_OUT(1, 2), 0.25 * cos(M_PI/4.0)));
	assert(almostEqual(AIO_OUT(2, 2), 0.25 * cos(M_PI/4.0)));

	listeningPose = Pose(Vec3d(3.4641016151377535, 0, 2)); // Right
	panner.renderSample(io, listeningPose, 0.25, 3);

	assert(almostEqual(AIO_OUT(0, 3), 0.25 * cos(M_PI/4.0)));
	assert(almostEqual(AIO_OUT(1, 3), 0));
	assert(almostEqual(AIO_OUT(2, 3), 0.25 * cos(M_PI/4.0)));

	// 3D Octahedron

	SpeakerLayout speakerLayout3D;
	speakerLayout3D.addSpeaker(Speaker(0, 0, 0, 1));
	speakerLayout3D.addSpeaker(Speaker(1, -90, 0, 1));
	speakerLayout3D.addSpeaker(Speaker(2, 90, 0, 1));
	speakerLayout3D.addSpeaker(Speaker(3, 180, 0, 1));
	speakerLayout3D.addSpeaker(Speaker(4, 0, 90, 1));
	speakerLayout3D.addSpeaker(Speaker(5, 0, -90, 1));

	Vbap panner3D(speakerLayout3D, true);
	io.bufferOut().zero(); // out buffers contain garbage by default, must zero manually as not using the callback facilities.

	listeningPose = Pose(Vec3d(0, 0, 4)); // Default Pose facing forward
	panner3D.renderSample(io, listeningPose, 0.2, 0);
	assert(almostEqual(AIO_OUT(0, 0), 0.2));
	assert(almostEqual(AIO_OUT(1, 0), 0.0));
	assert(almostEqual(AIO_OUT(2, 0), 0.0));
	assert(almostEqual(AIO_OUT(3, 0), 0.0));
	assert(almostEqual(AIO_OUT(4, 0), 0.0));
	assert(almostEqual(AIO_OUT(5, 0), 0.0));

	listeningPose = Pose(Vec3d(1, 1, 1)); // Elevated Right
	panner3D.renderSample(io, listeningPose, 0.25, 4);

	assert(almostEqual(AIO_OUT(0, 4), 0.25 * sqrt(1.0/3.0)));
	assert(almostEqual(AIO_OUT(1, 4), 0));
	assert(almostEqual(AIO_OUT(2, 4), 0.25 * sqrt(1.0/3.0)));
	assert(almostEqual(AIO_OUT(3, 4), 0.0));
	assert(almostEqual(AIO_OUT(4, 4), 0.25 * sqrt(1.0/3.0)));
	assert(almostEqual(AIO_OUT(5, 4), 0.0));

	float input[8] = {0.5f, 0.5f, 0.4f, 0.3f, 0.2f, 0.1f, -0.3f, -0.4f};
	io.bufferOut().zero();
	listeningPose = Pose(Vec3d(1, 1, 1)); // Elevated Right
	panner3D.renderBuffer(io, listeningPose, input, 8);

	for (int i = 0; i < 8; i++) {
		assert(almostEqual(AIO_OUT(0, i), input[i] * sqrt(1.0/3.0)));
		assert(almostEqual(AIO_OUT(1, i), 0));
		assert(almostEqual(AIO_OUT(2, i), input[i]  * sqrt(1.0/3.0)));
		assert(almostEqual(AIO_OUT(3, i), 0.0));
		assert(almostEqual(AIO_OUT(4, i), input[i]  * sqrt(1.0/3.0)));
		assert(almostEqual(AIO_OUT(5, i), 0.0));
	}
}

void testVbapRing() {
	SpeakerLayout speakerLayout = SpeakerRingLayout<8>();
	Vbap panner(speakerLayout);
	AudioIO io;
	io.configure(8, 44100, speakerLayout.numSpeakers(), 0);

	io.bufferOut().zero(); // out buffers contain garbage by default, must zero manually as not using the callback facilities.

	Pose listeningPose(Vec3d(0, 0, 4)); // Default Pose facing forward
	panner.renderSample(io, listeningPose, 0.5, 0);

	assert(almostEqual(AIO_OUT(0, 0), 0.5));
	assert(almostEqual(AIO_OUT(1, 0), 0.0));
	assert(almostEqual(AIO_OUT(2, 0), 0.0));
	assert(almostEqual(AIO_OUT(3, 0), 0.0));
	assert(almostEqual(AIO_OUT(4, 0), 0.0));
	assert(almostEqual(AIO_OUT(5, 0), 0.0));
	assert(almostEqual(AIO_OUT(6, 0), 0.0));
	assert(almostEqual(AIO_OUT(7, 0), 0.0));

	listeningPose = Pose(Vec3d(1, 0, 0)); // Full pan right
	panner.renderSample(io, listeningPose, 0.5, 1);

	assert(almostEqual(AIO_OUT(0, 1), 0.0));
	assert(almostEqual(AIO_OUT(1, 1), 0.0));
	assert(almostEqual(AIO_OUT(2, 1), 0.5));
	assert(almostEqual(AIO_OUT(3, 1), 0.0));
	assert(almostEqual(AIO_OUT(4, 1), 0.0));
	assert(almostEqual(AIO_OUT(5, 1), 0.0));
	assert(almostEqual(AIO_OUT(6, 1), 0.0));
	assert(almostEqual(AIO_OUT(7, 1), 0.0));

	listeningPose = Pose(Vec3d(-2, 0, 0)); // Full pan left
	panner.renderSample(io, listeningPose, 0.5, 2);

	assert(almostEqual(AIO_OUT(1, 2), 0.0));
	assert(almostEqual(AIO_OUT(0, 2), 0.0));
	assert(almostEqual(AIO_OUT(2, 2), 0.0));
	assert(almostEqual(AIO_OUT(3, 2), 0.0));
	assert(almostEqual(AIO_OUT(4, 2), 0.0));
	assert(almostEqual(AIO_OUT(5, 2), 0.0));
	assert(almostEqual(AIO_OUT(6, 2), 0.5));
	assert(almostEqual(AIO_OUT(7, 2), 0.0));

	listeningPose = Pose(Vec3d(0.3826834323650899, 0.0, -0.9238795325112867) ); // SSE
	panner.renderSample(io, listeningPose, 0.5, 3);

	assert(almostEqual(AIO_OUT(0, 3), 0.0));
	assert(almostEqual(AIO_OUT(1, 3), 0.0));
	assert(almostEqual(AIO_OUT(2, 3), 0.0));
	assert(almostEqual(AIO_OUT(3, 3), 0.5 * cos(M_PI/4.0)));
	assert(almostEqual(AIO_OUT(4, 3), 0.5 * cos(M_PI/4.0)));
	assert(almostEqual(AIO_OUT(5, 3), 0.0));
	assert(almostEqual(AIO_OUT(6, 3), 0.0));
	assert(almostEqual(AIO_OUT(7, 3), 0.0));

	// Test buffer rendering
	io.bufferOut().zero();
	listeningPose = Pose(Vec3d(-0.9238795325112867, 0.0, -0.3826834323650899) ); // SWW
	float input[8] = {0.5f, 0.5f, 0.4f, 0.3f, 0.2f, 0.1f, -0.3f, -0.4f};
	panner.renderBuffer(io, listeningPose, input, 8);

	for (int i = 0; i < 8; i++) {
		assert(almostEqual(AIO_OUT(0, i), 0.0));
		assert(almostEqual(AIO_OUT(1, i), 0.0));
		assert(almostEqual(AIO_OUT(2, i), 0.0));
		assert(almostEqual(AIO_OUT(3, i), 0.0));
		assert(almostEqual(AIO_OUT(4, i), 0.0));
		assert(almostEqual(AIO_OUT(5, i), input[i] * sin(M_PI/4.0)));
		assert(almostEqual(AIO_OUT(6, i), input[i] * sin(M_PI/4.0)));
		assert(almostEqual(AIO_OUT(7, i), 0.0));
	}
}

void testAmbisonicsFirstOrder2D(int bufferSize) {
	// TODO Finish ambisonics scene tester
	SpeakerLayout speakerLayout = OctalSpeakerLayout();
	AudioIO io;
	io.configure(bufferSize, 44100, speakerLayout.numSpeakers(), 0);

	AmbisonicsSpatializer *panner = new AmbisonicsSpatializer(speakerLayout, 2, 1);
	AudioScene scene(io.framesPerBuffer());
	SoundSource src;
	scene.createListener(panner);
	src.dopplerType(DOPPLER_NONE);
	src.useAttenuation(false);
	scene.addSource(src);

	for (int i = 0; i < bufferSize; i++) {
		src.writeSample(0.5);
	}

	// Front
	src.pos(0, 0, -1);
	scene.render(io);

	for (int i = 0; i < bufferSize; i++) {
		float front = AIO_OUT(0, i);
		for (int chan = 1; chan < speakerLayout.numSpeakers(); chan++) {
			float spkr = AIO_OUT(chan, i);
			assert(front > spkr);
//			std::cout << spkr << std::endl;
		}
	}

	for (int i = 0; i < bufferSize; i++) {
		src.writeSample(1.0);
	}

	// Pan center
	src.pos(0, 0, 0);
	scene.render(io);

	for (int i = 0; i < bufferSize; i++) {
		float left = AIO_OUT(0, i);
		float right = AIO_OUT(1, i);
		assert(almostEqual(left, right));
		assert(almostEqual(right, sin(M_PI/4)));
	}

	for (int i = 0; i < bufferSize; i++) {
		src.writeSample(0.75);
	}

	// Pan left
	src.pos(-1, 0, 0);
	scene.render(io);

	for (int i = 0; i < bufferSize; i++) {
		float left = AIO_OUT(0, i);
		float right = AIO_OUT(1, i);
		assert(almostEqual(left, 0.75));
		assert(almostEqual(right, 0.0));
	}

	delete panner;
}

int utAudioScene() {
	// Stereo
	testBasicStereo();
	testStereoAudioScene(8);
	testStereoAudioScene(4096);
	testMultipleSourcesStereo(8);
	testMultipleSourcesStereo(4096);
	testMultipleSourcesMovingStereo();

	// Headphones
	testHeadphoneRendering();

	// VBAP
	testVbapTriples();
	testVbapGains();
	testVbapRing();

	// DBAP
	// FIMXE add tests for DBAP

	// Ambisonics
	testAmbisonicsFirstOrder2D(8);

	return 0;
}
