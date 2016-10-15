#include "Lithe/LitheCore.h"
#include "allocore/ui/al_Parameter.hpp"
#include <string>

namespace al{

template<int NParams>
class Node : public lithe::Node
{
public:
	const int numParameters = NParams;
	al::Parameter* parameters[NParams];

	Node(int numInlets, int numOutlets) : lithe::Node(numInlets, numOutlets) {	}
	
	~Node(void)
	{
		for(int i=0; i<numParameters; ++i)
		{
			delete parameters[i];
		}
	}
	
	virtual void init_parameters() = 0;
};


}; // namespace al