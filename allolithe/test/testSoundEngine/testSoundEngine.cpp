#include "testSoundEngine.h"
#include "DummyModule.hpp" /// Registers itself as a module with the SoundEngine

SoundEngineTest::SoundEngineTest(void) {}

SoundEngineTest::~SoundEngineTest(void) {}

void SoundEngineTest::SetUp(void) {}

void SoundEngineTest::TearDown(void) {}

TEST_F(SoundEngineTest, register_module_test)
{
    EXPECT_EQ(DummyModule::module_id, 0);
   	ASSERT_STREQ( al::SoundEngine::ModuleNames[0].c_str(), "dummy" );
   	EXPECT_TRUE( al::SoundEngine::ModuleConstructors[0] == DummyModuleFactory);
}

TEST_F(SoundEngineTest, instantiate_module_test)
{
	int nodeID = al::SoundEngine::instantiateModule( DummyModule::module_id );
	EXPECT_EQ( nodeID, 0);
	EXPECT_EQ(al::SoundEngine::instantiatedNodeIDs[0], nodeID);

	// Instnatiate another one
	int another_nodeID = al::SoundEngine::instantiateModule( DummyModule::module_id );
	EXPECT_EQ( another_nodeID, 1);
	EXPECT_EQ(al::SoundEngine::instantiatedNodeIDs[1], another_nodeID);

	al::Node n2(1, 1, 2); // Instnatiate some other node that isn't tracked by SoundEngine
	int third_nodeID = al::SoundEngine::instantiateModule( DummyModule::module_id );
	EXPECT_EQ( third_nodeID, 3);  // because n2 was instantiated before this
	EXPECT_EQ(al::SoundEngine::instantiatedNodeIDs[2], third_nodeID);
}

TEST_F(SoundEngineTest, delete_module_test)
{
	int nodeID = al::SoundEngine::instantiateModule( DummyModule::module_id );
	int another_nodeID = al::SoundEngine::instantiateModule( DummyModule::module_id );
	al::Node n2(1, 1, 2);
	int third_nodeID = al::SoundEngine::instantiateModule( DummyModule::module_id );

	int old_size = al::SoundEngine::instantiatedNodeIDs.size();
	EXPECT_NO_THROW( al::Node::getNodeRef(nodeID));
	al::SoundEngine::deleteModuleInstance(nodeID);
	EXPECT_THROW( al::Node::getNodeRef(nodeID), std::runtime_error);
	EXPECT_EQ(al::SoundEngine::instantiatedNodeIDs.size(), old_size -1);

	EXPECT_NO_THROW( al::Node::getNodeRef(another_nodeID));
	EXPECT_NO_THROW( al::Node::getNodeRef(third_nodeID));
}