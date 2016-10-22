#include "Lithe/LitheCore.h"
#include "allocore/ui/al_Parameter.hpp"
#include <string>

namespace al{

class Node : public lithe::Node
{
public:
	std::vector<al::Parameter*> parameters;

	Node(int numInlets, int numOutlets, int numParams);
	
	~Node(void);

	static al::Node* getNodeRef(int nodeID);
	
	virtual void init_parameters(void);

	const int numParameters;
};


}; // namespace al