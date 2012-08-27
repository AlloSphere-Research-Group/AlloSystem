#=========================================================================
# AlloCore unit tests makefile
#=========================================================================

LIB_NAME	= allocoreunittest
include ../../Makefile.common

INC_DIR		= ./
SRC_DIR		= ./
VPATH		= $(SRC_DIR)
SRCS		= $(wildcard $(SRC_DIR)/ut*.cpp)
OBJS		= $(addsuffix .o, $(basename $(notdir $(SRCS))))

-include ../Makefile.link

CPPFLAGS	+= -I../
LDFLAGS		:= -L../$(BUILD_DIR)lib -L$(BUILD_DIR)lib $(LDFLAGS) -l$(LIB_NAME)

LINK_LIBS_PATH	= $(wildcard ../$(BUILD_DIR)lib/*.a)
LINK_LIBS_FLAGS	=
ifeq ($(PLATFORM), linux)
	LINK_LIBS_FLAGS += $(addprefix -l :, $(notdir $(LINK_LIBS_PATH)))
	LDFLAGS := -L/usr/lib/ -L/usr/local/lib/ $(LDFLAGS)
endif


#-------------------------------------------------------------------------
# Rules
#-------------------------------------------------------------------------
include ../../Makefile.rules

test: unitTests.cpp $(LIB_PATH)
#	@echo $(LINK_LIBS_FLAGS)
#	@echo LDFLAGS: $(LDFLAGS)
	$(CXX) $(CXXFLAGS) $^ $(LINK_LIBS_FLAGS) $(LINK_LIBS_PATH) $(LDFLAGS) -o $(BIN_DIR)unitTests
ifneq ($(AUTORUN), 0)
	@cd $(BIN_DIR) && ./unitTests
endif

# Remove build files
.PHONY: clean
clean:
	$(call RemoveBuildDir)

