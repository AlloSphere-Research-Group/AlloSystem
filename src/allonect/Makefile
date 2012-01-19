
LIB_NAME	= allonect
INC_DIR		= ../../include/$(LIB_NAME)/
SRC_DIR		= ./
VPATH		= $(SRC_DIR)

SRCS		:= $(wildcard $(SRC_DIR)/*.c*)
OBJS		:= $(addsuffix .o, $(basename $(notdir $(SRCS))))

CPPFLAGS	+= -I../../include

EXT_LDFLAGS += -lfreenect

ifeq ($(PLATFORM), linux)
	#nogood because the one in the repos is out of date
	#CPPFLAGS	+= -I/usr/include
	CPPFLAGS	+= -I../../$(PLATFORM_DIR)/include -I../../$(PLATFORM_DIR)/include
else
	CPPFLAGS	+= -I../../$(PLATFORM_DIR)/include -I../../$(PLATFORM_DIR)/include
endif

include ../../Makefile.common
include ../../Makefile.rules

#INC_DIRS	+= DEV_DIR/include/ni

# Install library into path specified by DESTDIR
# Include files are copied into DESTDIR/include/ and
# library files are copied to DESTDIR/lib/
install: $(LIB_PATH)
	@$(INSTALL) -d $(DESTDIR)/lib
	@$(INSTALL) -d $(DESTDIR)/include/$(LIB_NAME)
	@$(INSTALL) -C -m 644 $(INC_DIR)$/*.h* $(DESTDIR)/include/$(LIB_NAME)
#	@echo $(notdir $(SLIB_PATH))
#	For some reason this can result in an error if the file exists.
#	Just ignore it and continue...
	@-$(INSTALL) -C -m 644 $(LIB_PATH) $(DESTDIR)/lib
#	@$(RANLIB) $(DESTDIR)/lib/$(SLIB_FILE)
