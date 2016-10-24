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

	const int numParameters;
	
private:
	void instantiate_parameters(void);
};


}; // namespace al