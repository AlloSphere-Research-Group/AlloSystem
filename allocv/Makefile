#=========================================================================
# AlloCV (AlloSystem / OpenCV bindings) main Makefile
#=========================================================================

LIB_NAME	= allocv
include ../Makefile.common

INC_DIR		= $(LIB_NAME)/
SRC_DIR		= src/
VPATH		+= $(SRC_DIR)

SRCS		= $(wildcard $(SRC_DIR)*.c*)
OBJS		= $(addsuffix .o, $(basename $(notdir $(SRCS))))

CPPFLAGS	+= -I./
CPPFLAGS	+= -I../allocore/
LDFLAGS		+= -L$(BUILD_DIR)lib

-include ../allocore/Makefile.link

CPPFLAGS	+= -I../$(BUILD_DIR)include
LDFLAGS		+= -L../$(BUILD_DIR)lib

# Define flags for external dependencies

LINK_CXXFLAGS	+= -Wno-non-virtual-dtor -Wno-overloaded-virtual

ifeq ($(PLATFORM), windows)
LINK_CPPFLAGS 	+=
LIBS_OPENCV		= calib3d contrib core features2d flann gpu highgui imgproc legacy ml nonfree objdetect photo stitching superres video videostab
LINK_LDFLAGS	+= $(addsuffix 246, $(addprefix -lopencv_, $(LIBS_OPENCV)))
else
LINK_CPPFLAGS 	+= `pkg-config --cflags opencv`
LINK_LDFLAGS	+= `pkg-config --libs opencv`
endif

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
	@$(RM) $(LIB_PATH)
	$(call RemoveDir, $(OBJ_DIR))

include ../Makefile.buildandrun
