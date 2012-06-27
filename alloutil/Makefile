#=========================================================================
# AlloUtil main makefile
#=========================================================================

LIB_NAME	= alloutil
INC_DIR		= $(LIB_NAME)/
SRC_DIR		= src/
VPATH		= $(SRC_DIR)
BUILD_DIR	= ../build/

SRCS		:= $(wildcard $(SRC_DIR)/*.c*)
OBJS		:= $(addsuffix .o, $(basename $(notdir $(SRCS))))

CPPFLAGS	+= -I$(BUILD_DIR)include -I../$(INC_DIR)


include ../Makefile.common
include ../Makefile.rules

# Install library into path specified by DESTDIR
# Include files are copied into DESTDIR/include/ and
# library files are copied to DESTDIR/lib/
install: $(LIB_PATH)
	@$(INSTALL) -d $(DESTDIR)/lib/
	@$(INSTALL) -d $(DESTDIR)/include/$(LIB_NAME)/
	@$(INSTALL) -C -m 644 $(INC_DIR)/*.h* $(DESTDIR)/include/$(LIB_NAME)/
#	@echo $(notdir $(SLIB_PATH))

##	Only copy library if it doesn't exist or is newer than existing
	@if [ $(LIB_PATH) -nt $(DESTDIR)/lib/$(LIB_FILE) ] || [ ! -e $(DESTDIR)/lib/$(LIB_FILE) ]; then \
		echo INSTALL $(LIB_PATH) -> $(DESTDIR)/lib/;\
		$(INSTALL) -C -m 644 $(LIB_PATH) $(DESTDIR)/lib/;\
	fi;
#	@$(RANLIB) $(DESTDIR)/lib/$(SLIB_FILE)

