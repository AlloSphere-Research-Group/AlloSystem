####### ALLOVSR MAKEFILE ###########

title:
	@echo
	@echo
	@echo /////////////////////////////////////////////////////////////////////////////

LIB_NAME = allovsr
STRICT_WARNINGS = 0
include ../Makefile.common

#LINK TO GRAPHICS LIBRARIES FLAG (GLV, OpenGL, GLUT)
# run make GFX=0 vsr to make versor without built-in graphics capabilities (just data and operations)
GFX = 1

#Locations of Libariesmake
LIB_DIR		= ../build/lib/
#Locations of Source and Header Files in AlloSystem
INC_DIR		= ../build/include/
#Precompiled Header
PCH_DIR		= ../build/include/vsr/pch/

#Location of Example Apps Will be put here
BIN_DIR		= build/bin/

#Warnings
CFLAGS += -Wno-deprecated

#Dependencies
DEPFLAGS = -MMD -MF $(basename $@).dep

-include ../allocore/Makefile.link

#Link
LDFLAGS := -lvsr -lGLV -lalloGLV $(LDFLAGS)
#Link Twice
LDFLAGS += $(LDFLAGS)

IPATH = $(PCH_DIR) \
		$(INC_DIR)vsr/Elements/\
		$(INC_DIR)vsr\
		$(INC_DIR)\
		/usr/local/include/

HPATH = $(addprefix -I, $(IPATH))

CPPFLAGS	+= $(LINK_CPPFLAGS)
#LDFLAGS		+= $(LINK_LDFLAGS)

CFLAGS		:= $(CPPFLAGS) $(CFLAGS)
CXXFLAGS	:= $(CFLAGS) $(CXXFLAGS)


EXEC_TARGETS = tests/%.cpp examples/%.cpp

# Dummy target to force rebuilds
FORCE:

.PRECIOUS: $(EXEC_TARGETS)

dir:
	@mkdir -p $(BIN_DIR)

$(EXEC_TARGETS): FORCE dir
	@echo Building $@ with $(LDFLAGS)
	$(CXX) $(CXXFLAGS) $(HPATH) $@ -o $(BIN_DIR)$(*F) -L$(LIB_DIR) $(LDFLAGS)
	@cd $(BIN_DIR) && ./$(*F)

install: FORCE
	@echo installing allovsr to $(DESTDIR)/include/$(LIB_NAME)
	@$(INSTALL) -d $(DESTDIR)/include/$(LIB_NAME)
	@$(INSTALL) -m 644 $(LIB_NAME)/* $(DESTDIR)/include/$(LIB_NAME)

# Create file with settings for linking to external libraries
linkfile:
	@printf "%b\n" "CPPFLAGS +=$(LINK_CPPFLAGS)\r\nLDFLAGS := -l$(LIB_NAME) $(LINK_LDFLAGS) $(LDFLAGS) "> Makefile.link

.PHONY: clean
clean:
	$(call RemoveBuildDir)
