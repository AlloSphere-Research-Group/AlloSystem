#include "allolithe/al_Node.hpp"

namespace al{

Node::Node(int numInlets, int numOutlets, int numParams) : 
	lithe::Node(numInlets, numOutlets), 
	numParameters(numParams) 
	{

	}

Node::~Node(void)
{
	for(int i=0; i<numParameters; ++i)
	{
		delete parameters[i];
	}
}

al::Node* Node::getNodeRef(int nodeID)
{
	try
	{
		al::Node* node_ref = static_cast<al::Node*>(lithe::Node::getNodeRef(nodeID));
		return node_ref;
	}
	catch(const char * str)
	{
		throw std::runtime_error(str);
	}
}

void Node::init_parameters() 
{
	for(int i=0; i<numParameters; ++i)
	{
		std::string param_name = "parameter"+std::to_string(i);
		parameters.push_back( new Parameter(param_name, "group", 0.0) );
	}
}

} // namespace al