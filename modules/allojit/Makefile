#=========================================================================
# AlloCore main makefile
#=========================================================================

LIB_NAME	= allojit
INC_DIR		= $(LIB_NAME)/
SRC_DIR		= src/
VPATH		= $(SRC_DIR)

SRCS		:= $(wildcard $(SRC_DIR)/*.c*)
OBJS		:= $(addsuffix .o, $(basename $(notdir $(SRCS))))

CPPFLAGS	+= -I./ -I../../include/

include ../../Makefile.common
include ../../Makefile.rules


# Install library into path specified by DESTDIR
# Include files are copied into DESTDIR/include/ and
# library files are copied to DESTDIR/lib/
install: $(SLIB_PATH)
	@$(INSTALL) -d $(DESTDIR)/lib
	@$(INSTALL) -d $(DESTDIR)/include/$(LIB_NAME)

	@$(INSTALL) -c -m 644 $(INC_DIR)$/*.h* $(DESTDIR)/include/$(LIB_NAME)

	@$(INSTALL) -c -m 644 $(SLIB_PATH) $(DESTDIR)/lib
#	@$(RANLIB) $(DESTDIR)/lib/$(SLIB_FILE)
