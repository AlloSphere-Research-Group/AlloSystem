#=========================================================================
# AlloNi main makefile
#=========================================================================

LIB_NAME	= alloni
include ../Makefile.common

INC_DIR		= $(LIB_NAME)/
SRC_DIR		= src/
VPATH		= $(SRC_DIR)

SRCS		:= $(wildcard $(SRC_DIR)/*.c*)
OBJS		:= $(addsuffix .o, $(basename $(notdir $(SRCS))))

CPPFLAGS	+= -I./
CPPFLAGS	+= -I../allocore/

ifeq ($(PLATFORM), linux)
	CPPFLAGS	+= -I/usr/include/ni/ -I/usr/include/nite/
else
	CPPFLAGS	+= -I$(PLATFORM_DIR)/include/ni/ -I$(PLATFORM_DIR)/include/nite/
endif

include ../Makefile.rules

#INC_DIRS	+= DEV_DIR/include/ni

# Install library into path specified by DESTDIR
# Include files are copied into DESTDIR/include/ and
# library files are copied to DESTDIR/lib/
install: $(SLIB_PATH)
	@$(INSTALL) -d $(DESTDIR)/lib
	@$(INSTALL) -d $(DESTDIR)/include/$(LIB_NAME)
	@$(INSTALL) -C -m 644 $(INC_DIR)$/*.h* $(DESTDIR)/include/$(LIB_NAME)
	@$(INSTALL) -C -m 644 $(SLIB_PATH) $(DESTDIR)/lib
