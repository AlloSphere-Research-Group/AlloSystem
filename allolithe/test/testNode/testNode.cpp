#include "testNode.h"
#include "InheritedNode.hpp"
#include <iostream>

NodeTest::NodeTest(void) {}

NodeTest::~NodeTest(void) {}

void NodeTest::SetUp(void) {}

void NodeTest::TearDown(void) {}

TEST_F(NodeTest, constructorTest)
{
	InheritedNode n1;
	EXPECT_EQ( n1.numInlets(), inherited_numInlets);
	EXPECT_EQ( n1.numOutlets(), inherited_numOutlets);

	EXPECT_EQ( n1.numParameters, inherited_numParams);
	EXPECT_EQ( n1.parameters.size(), inherited_numParams);

	al::Node n2(1, 1, 2);
	EXPECT_FLOAT_EQ(n2.parameters[0]->get(), 0.0);;
	EXPECT_FLOAT_EQ(n2.parameters[1]->get(), 0.0);;
	EXPECT_EQ(n2.numInlets(), 1);
	EXPECT_EQ(n2.numOutlets(), 1);
}
