#ifndef DUMMY_MODULE_HPP
#define DUMMY_MODULE_HPP

#include "allolithe/allolithe.hpp"

class DummyModule : public al::Node
{
public:
	enum ModuleParams : int
	{
		Param_1, 
		Param_2, 
		NUM_PARAMS
	};
	enum ModuleInlets : int
	{
		Inlet_1, 
		Inlet_2, 
		NUM_INLETS
	};

	enum ModuleOutlets : int
	{
		Outlet_1, 
		Outlet_2, 
		NUM_OUTLETS
	};

	DummyModule(void) : 
		al::Node(ModuleInlets::NUM_INLETS, ModuleOutlets::NUM_OUTLETS, ModuleParams::NUM_PARAMS)
		{

		}

	static int module_id;
};

int DummyModuleFactory(void)
{
	DummyModule* new_instance = new DummyModule;
	return new_instance->getID();
}

int DummyModule::module_id = al::REGISTER_MODULE( "dummy", DummyModuleFactory );


#endif //DUMMY_MODULE_HPP