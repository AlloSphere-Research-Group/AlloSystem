#ifndef INHERITED_NODE_HPP
#define INHERITED_NODE_HPP

#include "allolithe/allolithe.hpp"

#define numInlets 2
#define numOutlets 2
#define numParams 4

class InheritedNode : public al::Node
{
	InheritedNode(void) : al::Node(numInlets, numOutlets, numParams)
	{

	}

	virtual void DSP(void)
	{
		// lithe::Sample 

	}
};


#endif