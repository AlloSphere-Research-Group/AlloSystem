#ifndef SOUNDENGINE_HPP
#define SOUNDENGINE_HPP

#include "Lithe/LitheCore.h"
#include <vector>
#include <string>

typedef int (*ModuleFactoryFunction)(void);

class SoundEngine
{
public:
	SoundEngine() {}
	
	static int instantiateModule(int moduleID)
	{
		if( moduleID >= ModuleNames.size() ) 
			throw std::range_error("Module with id: " + std::to_string(moduleID) + " not registered");
		else
			return ModuleConstructors[moduleID]();
	}

	friend int REGISTER_MODULE(std::string module_name, ModuleFactoryFunction module_factory_function);

private:
    static std::vector<std::string> ModuleNames;
  	static std::vector<ModuleFactoryFunction> ModuleConstructors;
};

std::vector<std::string> SoundEngine::ModuleNames;
std::vector<ModuleFactoryFunction> SoundEngine::ModuleConstructors;

#include <iostream>

int REGISTER_MODULE(std::string module_name, ModuleFactoryFunction module_factory_function) 
{
	SoundEngine::ModuleNames.push_back(module_name);
	SoundEngine::ModuleConstructors.push_back(module_factory_function);

	int module_id = SoundEngine::ModuleConstructors.size() - 1;
	return module_id;
}


#endif // SOUNDENGINE_HPP