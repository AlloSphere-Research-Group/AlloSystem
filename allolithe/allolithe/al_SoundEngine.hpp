#ifndef SOUNDENGINE_HPP
#define SOUNDENGINE_HPP

#include "Lithe/LitheCore.h"
#include <vector>
#include <string>
#include <algorithm>

namespace al {

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
		{
			instantiatedNodeIDs.push_back(ModuleConstructors[moduleID]());
			return instantiatedNodeIDs.back();
		}
	}

	static void deleteModuleInstance(int nodeID)
	{
		for(int i=0; i<instantiatedNodeIDs.size(); ++i)
		{
			if( instantiatedNodeIDs[i] == nodeID )
			{
				instantiatedNodeIDs.erase(instantiatedNodeIDs.begin()+i);
				delete al::Node::getNodeRef(nodeID);
				return;
			}
		}
		throw std::runtime_error("Unable to find node with ID:"+std::to_string(nodeID));
	}

	static std::vector<int> instantiatedNodeIDs;
    static std::vector<std::string> ModuleNames;
  	static std::vector<ModuleFactoryFunction> ModuleConstructors;
};

std::vector<int> SoundEngine::instantiatedNodeIDs;
std::vector<std::string> SoundEngine::ModuleNames;
std::vector<ModuleFactoryFunction> SoundEngine::ModuleConstructors;

int REGISTER_MODULE(std::string module_name, ModuleFactoryFunction module_factory_function) 
{
	SoundEngine::ModuleNames.push_back(module_name);
	SoundEngine::ModuleConstructors.push_back(module_factory_function);

	int module_id = SoundEngine::ModuleConstructors.size() - 1;
	return module_id;
}

} // namespace al


#endif // SOUNDENGINE_HPP