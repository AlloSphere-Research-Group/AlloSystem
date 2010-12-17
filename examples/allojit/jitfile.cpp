#include "ajit.hpp"
#include "example_using_jitfile.hpp"


using namespace al;

class TestZone : public JitZone {
public:
	TestZone(JIT * jit, World * W) : JitZone(jit, NULL), W(W) {
		printf("created TestZone\n");
	}
	
	virtual ~TestZone() {
		printf("~TestZone()\n");
	}
	
	World * W;
};

/*
	Entry point from JIT engine:
*/
extern "C" JitZone * onload(JIT * jit, World * W) {
	
	return new TestZone(jit, W);
}