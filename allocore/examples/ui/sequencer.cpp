
#include <fstream>

#include "allocore/io/al_App.hpp"
#include "allocore/graphics/al_Shapes.hpp"

#include "allocore/ui/al_PresetSequencer.hpp"
#include "allocore/ui/al_Parameter.hpp"
#include "allocore/ui/al_Preset.hpp"

using namespace al;

Parameter X("x", "", 0.0, "", -2, 2);
Parameter Y("y", "", 0.0, "", -2, 2);

PresetHandler presetHandler("sequencerDir", true);
PresetSequencer sequencer;
// The sequencer server triggers sequences when it receives a valid sequence
// name on OSC path /sequence
// If you send a message using a command like:
// oscsend 127.0.0.1 9012 /sequence s "seq"
// You will trigger the sequence
SequenceServer sequencerServer; // Send OSC to 127.0.0.1:9011

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
		if (sequencer.running()) {
			g.translate(X.get(), Y.get(), 0);
			g.color(0.0,1.0,0.0);
		} else {
			g.color(0.0,0.0,1.0);
		}
		g.draw(g.mesh());
	}

	virtual void onMouseDown(const ViewpointWindow &w, const Mouse &m) override {
		if (sequencer.running()) {
			sequencer.stopSequence();
			std::cout << "Sequencer stopped" << std::endl;
		} else {
			sequencer.playSequence("seq");
			std::cout << "Sequencer started" << std::endl;
		}
	}
};


void writeExamplePresets()
{
	std::string sequence = R"(preset1:0.0:0.5
preset2:3.0:1.0
preset3:1.0:0.0
preset2:1.5:2.0
::
)";
	std::ofstream fseq("sequencerDir/seq.sequence");
	fseq << sequence;
	fseq.close();

	std::string preset1 = R"(::preset1
/x f 0.4
/y f 0.2
::
)";
	std::ofstream f1("sequencerDir/preset1.preset");
	f1 << preset1;
	f1.close();

	std::string preset2 = R"(::preset2
/x f 0.6
/y f -0.9
::
)";
	std::ofstream f2("sequencerDir/preset2.preset");
	f2 << preset2;
	f2.close();

	std::string preset3 = R"(::preset3
/x f -0.1
/y f 1.0
::
)";
	std::ofstream f3("sequencerDir/preset3.preset");
	f3 << preset3;
	f3.close();

}

int main(int argc, char *argv[])
{
	writeExamplePresets();
	presetHandler << X << Y; // Register parameters with preset handler
	sequencer << presetHandler; // Register preset handler with sequencer
	sequencerServer << sequencer; // Register sequencer with sequence server
	sequencerServer.print();
	MyApp().start();
	return 0;
}

