#include "testNode.h"
#include "InheritedNode.hpp"
NodeTest::NodeTest(void) {}

NodeTest::~NodeTest(void) {}

void NodeTest::SetUp(void) {}

void NodeTest::TearDown(void) {}

TEST_F(NodeTest, removeThisTest)
{
    EXPECT_EQ(1, 1);
}
