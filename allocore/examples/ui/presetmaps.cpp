#include "allocore/ui/al_Preset.hpp"
#include "allocore/ui/al_PresetMapper.hpp"

using namespace al;

int main(int argc, char *argv[])
{
    PresetHandler handler("presets-example");
    PresetMapper mapper;
    mapper.registerPresetHandler(handler);
    return 0;
}
