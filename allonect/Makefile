#=========================================================================
# Allonect main makefile
#=========================================================================

LIB_NAME	= allonect
INC_DIR		= $(LIB_NAME)/
SRC_DIR		= src/
VPATH		= $(SRC_DIR)
BUILD_DIR	= build/

SRCS		:= $(wildcard $(SRC_DIR)/*.c*)
OBJS		:= $(addsuffix .o, $(basename $(notdir $(SRCS))))

#CPPFLAGS	+= -I$(BUILD_DIR)include -I../$(INC_DIR)
#CPPFLAGS	+= -I../$(LIB_NAME)
CPPFLAGS	+= -I./
CPPFLAGS	+= -I../allocore/ -I../alloutil -I/usr/local/include/libfreenect

LINK_LDFLAGS	+= -lfreenect -lusb-1.0

#ifeq ($(PLATFORM), linux)
#	#nogood because the one in the repos is out of date
#	CPPFLAGS	+= -I/usr/local/include/libfreenect
#	CPPFLAGS	+= -I../../$(PLATFORM_DIR)/include -I../../$(PLATFORM_DIR)/include
#else
#	#CPPFLAGS	+= -I../../$(PLATFORM_DIR)/include -I../../$(PLATFORM_DIR)/include
#	LINK_LDFLAGS += -framework Carbon -framework IOKit
#endif

include ../Makefile.common
include ../Makefile.rules

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

