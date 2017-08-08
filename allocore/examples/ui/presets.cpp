

#include "allocore/io/al_App.hpp"
#include "allocore/ui/al_Parameter.hpp"
#include "allocore/ui/al_Preset.hpp"

using namespace al;

Parameter X("X", "Position", 0.0, "", -1.0, 1.0);
Parameter Y("Y", "Position", 0.0, "", -1.0, 1.0);
Parameter Size("Scale", "Size", 0.0, "", 0.1, 3.0);

PresetHandler presets("presets-example"); // set the preset root directory here

class MyApp : public App
{
public:
	MyApp() {
		initWindow(Window::Dim(800,600), "Presets", 15);
		nav().pos(Vec3d(0,0,8));
	}

	virtual void onDraw(Graphics &g) override
	{
		g.lighting(true);
		light();
		addCone(g.mesh());
		g.pushMatrix();
		X = X.get() + rng.uniform(-0.1, 0.1);
		Y = Y.get() + rng.uniform(-0.1, 0.1);
		Size = Size.get() + rng.uniform(-0.03, 0.03);

		g.translate(X.get(), Y.get(), 0);
		g.scale(Size.get());
		g.draw(g.mesh());
		g.popMatrix();
	}

	virtual void onKeyDown(const Keyboard &k) override
	{
		if (k.alt()) {
			switch(k.key()) {
			case '1':
				presets.storePreset("preset1");
				std::cout << "Preset 1 stored." << std::endl;
				break;
			case '2':
				presets.storePreset("preset2");
				std::cout << "Preset 2 stored." << std::endl;
				break;
			case '3':
				presets.storePreset("preset3");
				std::cout << "Preset 3 stored." << std::endl;
				break;
			}
		} else {
			switch(k.key()) {
			case '1':
				presets.recallPreset("preset1");
				std::cout << "Preset 1 loaded." << std::endl;
				break;
			case '2':
				presets.recallPreset("preset2");
				std::cout << "Preset 2 loaded." << std::endl;
				break;
			case '3':
				presets.recallPreset("preset3");
				std::cout << "Preset 3 loaded." << std::endl;
				break;
			}
		}
	}
private:
	rnd::Random<> rng; // Random number generator
	Light light;
};


int main(int argc, char *argv[])
{
	std::cout << "Press 1, 2 or 3 to recall preset, add alt key to store." << std::endl;
	presets << X << Y << Size; // Add parameters to preset handling
	presets.setSubDirectory("bank1");

	PresetServer presetServer("127.0.0.1", 9012);
	presetServer << presets; // Expose preset management through OSC
	presetServer.addListener("127.0.0.1", 13560); // This address will be notified whenever the preset changes
	MyApp().start();
	return 0;
}
