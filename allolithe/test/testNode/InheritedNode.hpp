#ifndef INHERITED_NODE_HPP
#define INHERITED_NODE_HPP

#include "allolithe/allolithe.hpp"

#define inherited_numInlets 2
#define inherited_numOutlets 2
#define inherited_numParams 4

class InheritedNode : public al::Node
{ 
public:
	InheritedNode(void) : al::Node(inherited_numInlets, inherited_numOutlets, inherited_numParams)
	{

	}

	virtual void DSP(void)
	{
	}
};


#endif