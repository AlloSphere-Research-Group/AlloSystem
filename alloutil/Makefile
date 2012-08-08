#=========================================================================
# AlloUtil main makefile
#=========================================================================

LIB_NAME	= alloutil

include ../Makefile.common

INC_DIR		= $(LIB_NAME)/
SRC_DIR		= src/
VPATH		+= $(SRC_DIR)

SRCS		= $(wildcard $(SRC_DIR)*.c*)
OBJS		= $(addsuffix .o, $(basename $(notdir $(SRCS))))

CPPFLAGS	+= -I/usr/local/include/ -I/usr/include/
CPPFLAGS	+= -I./
LDFLAGS		+= -L$(BUILD_DIR)lib

-include ../allocore/Makefile.link

CPPFLAGS	+= -I../$(BUILD_DIR)include
LDFLAGS		+= -L../$(BUILD_DIR)lib

# Define flags for external dependencies
LINK_CPPFLAGS 	+= -I/usr/include/lua5.1/

LINK_LDFLAGS	+= -lallocore

ifeq ($(PLATFORM), linux)
	LINK_LDFLAGS += -llua5.1
else ifeq ($(PLATFORM), macosx)
	LINK_LDFLAGS += -llua
else ifeq ($(PLATFORM), windows)
	LINK_LDFLAGS += -llua
endif

include ../Makefile.rules
include ../Makefile.buildandrun

# Install library into path specified by DESTDIR
# Include files are copied into DESTDIR/include/ and
# library files are copied to DESTDIR/lib/
install: $(LIB_PATH)
	@$(INSTALL) -d $(DESTDIR)/lib/
	@$(INSTALL) -d $(DESTDIR)/include/$(LIB_NAME)/
	@$(INSTALL) -C -m 644 $(INC_DIR)/*.h* $(DESTDIR)/include/$(LIB_NAME)/

##	Only copy library if it doesn't exist or is newer than existing
	@if [ $(LIB_PATH) -nt $(DESTDIR)/lib/$(LIB_FILE) ] || [ ! -e $(DESTDIR)/lib/$(LIB_FILE) ]; then \
		echo INSTALL $(LIB_PATH) to $(DESTDIR)/lib/;\
		$(INSTALL) -C -m 644 $(LIB_PATH) $(DESTDIR)/lib/;\
	fi;

# Remove build files
.PHONY: clean
clean:
	@$(RM) $(LIB_PATH)
	$(call RemoveDir, $(OBJ_DIR))

