#=========================================================================
# AlloGLV main makefile
#=========================================================================

LIB_NAME	= alloGLV
INC_DIR		= $(LIB_NAME)/
SRC_DIR		= src/
VPATH		= $(SRC_DIR)
BUILD_DIR	= build/
RUN_DIRS	+= examples/

SRCS		:= $(wildcard $(SRC_DIR)/*.c*)
OBJS		:= $(addsuffix .o, $(basename $(notdir $(SRCS))))

CPPFLAGS	+= -I./ -I$(BUILD_DIR)include

# include dependency headers directly
CPPFLAGS	+= -I../allocore/
CPPFLAGS	+= -I../../GLV/

# include dependencies from parent build folder
CPPFLAGS	+= -I../build/include/
LDFLAGS		+= -L../build/lib/

# include all the linker flags
-include ../allocore/Makefile.link
-include ../../GLV/Makefile.link


include ../Makefile.common
include ../Makefile.rules

# Install library into path specified by DESTDIR
# Include files are copied into DESTDIR/include/ and
# library files are copied to DESTDIR/lib/
install: $(LIB_PATH)
	@$(INSTALL) -d $(DESTDIR)/lib/
	@$(INSTALL) -d $(DESTDIR)/include/$(LIB_NAME)/
	@$(INSTALL) -m 644 $(INC_DIR)/*.h* $(DESTDIR)/include/$(LIB_NAME)/

##	Only copy library if it doesn't exist or is newer than existing
	@if [ $(LIB_PATH) -nt $(DESTDIR)/lib/$(LIB_FILE) ] || [ ! -e $(DESTDIR)/lib/$(LIB_FILE) ]; then \
		echo INSTALL $(LIB_PATH) to $(DESTDIR)/lib/;\
		$(INSTALL) -m 644 $(LIB_PATH) $(DESTDIR)/lib/;\
	fi;

# Remove build files
.PHONY: clean
clean:
	$(call RemoveBuildDir)

include ../Makefile.buildandrun
