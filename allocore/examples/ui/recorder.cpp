
#include <fstream>

#include "allocore/io/al_App.hpp"
#include "allocore/graphics/al_Shapes.hpp"

#include "allocore/ui/al_Parameter.hpp"
#include "allocore/ui/al_Preset.hpp"
#include "allocore/ui/al_SequenceRecorder.hpp"

using namespace al;

Parameter X("x", "", 0.0, "", -2, 2);
Parameter Y("y", "", 0.0, "", -2, 2);

PresetHandler presetHandler("sequencerDir", true);
SequenceRecorder recorder;

class MyApp :public App
{
public:
	MyApp()
	{
		addSphere(graphics().mesh(), 0.2);
		initWindow();
		nav().pullBack(4);
	}

	virtual void onDraw(Graphics &g) override {
		g.translate(X.get(), Y.get(), 0);
		g.draw(g.mesh());
	}

	virtual void onKeyDown(const ViewpointWindow &win, const Keyboard &k) override {

		switch(k.key()) {
		case 'r':
			recorder.startRecord();
			break;
		case ' ':
			recorder.stopRecord();
			break;
		case '1':
			presetHandler.recallPreset("preset1");
			break;
		case '2':
			presetHandler.recallPreset("preset2");
			break;
		case '3':
			presetHandler.recallPreset("preset3");
			break;
		}
	}
};

int main(int argc, char *argv[])
{
	presetHandler << X << Y; // Register parameters with preset handler
	recorder << presetHandler; // Register preset handler with sequencer

	MyApp().start();
	return 0;
}

