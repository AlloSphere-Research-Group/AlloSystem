#=========================================================================
# AlloCore unit tests makefile
#=========================================================================

LIB_NAME	= allounittests
INC_DIR		= ./
SRC_DIR		= ./
VPATH		= $(SRC_DIR)

SRCS		= $(wildcard $(SRC_DIR)/ut*.cpp)
OBJS		= $(addsuffix .o, $(basename $(notdir $(SRCS))))

-include ../Makefile.external
include ../Makefile.common
include ../Makefile.rules

CXXFLAGS	+= -I../include
LDFLAGS		+= -L../$(BUILD_DIR)lib -L$(BUILD_DIR)lib -l$(LIB_NAME)

LINK_LIBS_PATH	= $(wildcard ../$(BUILD_DIR)lib/*.a)
LINK_LIBS_FLAGS	=
ifeq ($(PLATFORM), linux)
	LINK_LIBS_FLAGS += $(addprefix -l :, $(notdir $(LINK_LIBS_PATH)))
	LDFLAGS += -L/usr/local/lib
endif


#-------------------------------------------------------------------------
# Rules
#-------------------------------------------------------------------------

test: unitTests.cpp $(SLIB_PATH)
#	@echo $(LINK_LIBS_FLAGS)
#	@echo LDFLAGS: $(LDFLAGS)
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) $(LINK_LIBS_FLAGS) $(LINK_LIBS_PATH) -o $(BIN_DIR)unitTests
ifneq ($(AUTORUN), 0)
	@cd $(BIN_DIR) && ./unitTests
endif
