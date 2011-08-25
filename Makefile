#=========================================================================
# AlloCore main makefile
#=========================================================================

include Makefile.config

# Include configuration files of AlloCore modules
# TODO: Permit selective inclusive of modules for building a library
# and doing unit tests.

MODULE_DIRS := $(GFX_DIR) $(IO_DIR) $(MATH_DIR) $(PRO_DIR) $(SND_DIR) $(SPA_DIR) $(SYS_DIR) $(TYP_DIR)

include $(SRC_DIR)$(GFX_DIR)Makefile.config
include $(SRC_DIR)$(IO_DIR)Makefile.config
include $(SRC_DIR)$(MATH_DIR)Makefile.config
include $(SRC_DIR)$(PRO_DIR)Makefile.config
include $(SRC_DIR)$(SND_DIR)Makefile.config
include $(SRC_DIR)$(SPA_DIR)Makefile.config
include $(SRC_DIR)$(SYS_DIR)Makefile.config
include $(SRC_DIR)$(TYP_DIR)Makefile.config

# Prefix full path to source files
GFX_SRC		:= $(addprefix $(SRC_DIR)$(GFX_DIR), $(GFX_SRC))
IO_SRC		:= $(addprefix $(SRC_DIR)$(IO_DIR), $(IO_SRC))
MATH_SRC	:= $(addprefix $(SRC_DIR)$(MATH_DIR), $(MATH_SRC))
PRO_SRC		:= $(addprefix $(SRC_DIR)$(PRO_DIR), $(PRO_SRC))
TYP_SRC		:= $(addprefix $(SRC_DIR)$(TYP_DIR), $(TYP_SRC))
SND_SRC		:= $(addprefix $(SRC_DIR)$(SND_DIR), $(SND_SRC))
SPA_SRC		:= $(addprefix $(SRC_DIR)$(SPA_DIR), $(SPA_SRC))
SYS_SRC		:= $(addprefix $(SRC_DIR)$(SYS_DIR), $(SYS_SRC))

# These are all the source files
SRCS		= \
		$(GFX_SRC) $(IO_SRC) $(PRO_SRC) $(MATH_SRC) \
		$(SND_SRC) $(SPA_SRC) $(SYS_SRC) $(TYP_SRC)

OBJS		= $(addsuffix .o, $(basename $(notdir $(SRCS))))

CPPFLAGS	+= $(addprefix -I, $(INC_DIRS) $(RINC_DIRS) $(BUILD_DIR)/include)
LDFLAGS		:= $(addprefix -L, $(LIB_DIRS) $(BUILD_DIR)/lib) $(LDFLAGS)
LINK_LIBS_PATH	= $(wildcard $(BUILD_DIR)lib/*.a)
LINK_LIBS_FLAGS	=

#--------------------------------------------------------------------------
# Rules
#--------------------------------------------------------------------------

help:
	@echo No rule specified.
	@echo The possible rules are:
	@echo     allocore .... build allocore
	@echo     allojit ..... build allocore JIT extension
	@echo     alloutil .... build allocore utilities extension
	@echo     gamma ....... build Gamma external
	@echo     glv ......... build GLV external

include Makefile.rules


# Compile and run source files in examples/ folder
EXEC_TARGETS = examples/%.cpp
ifeq ($(PLATFORM), linux)
	LINK_LIBS_FLAGS += $(addprefix -l :, $(notdir $(LINK_LIBS_PATH)))
endif
.PRECIOUS: $(EXEC_TARGETS)
$(EXEC_TARGETS): allocore alloutil FORCE
#	@echo $(LINK_LIBS_FLAGS)
	$(CXX) $(CXXFLAGS) -o $(BIN_DIR)$(*F) $@ $(LDFLAGS) $(LINK_LIBS_FLAGS) $(LINK_LIBS_PATH)
ifneq ($(AUTORUN), 0)
	@cd $(BIN_DIR) && ./$(*F)
endif


# Build (and run) from one or more source files in a directory
EXEC_DIR_TARGETS = examples/%
.PRECIOUS: $(EXEC_DIR_TARGETS)
$(EXEC_DIR_TARGETS): LSRC = $(wildcard $@/*.cpp) $(wildcard $@/*.c)
$(EXEC_DIR_TARGETS): EXEC_NAME = $(subst /,_,$(*D))
$(EXEC_DIR_TARGETS): allocore alloutil FORCE
	$(CXX) $(CXXFLAGS) -o $(BIN_DIR)$(EXEC_NAME) $(LSRC) $(LDFLAGS) $(LINK_LIBS_FLAGS) $(LINK_LIBS_PATH) -I$@
ifneq ($(AUTORUN), 0)
	@cd $(BIN_DIR) && ./$(EXEC_NAME)
endif


extended: all alloni

all: extensions externals

# AlloCore extensions
extensions: alloutil allocore

allocore: $(SLIB_PATH)
#	Copy main header files into build directory
	@for v in `cd $(INC_DIR)/$@ && find * -type d ! -path '*.*'` .; do\
		$(INSTALL) -d $(BUILD_DIR)/include/$@/$$v;\
		$(INSTALL) -C -m 644 $(INC_DIR)/$@/$$v/*.h* $(BUILD_DIR)/include/$@/$$v;\
	done

#	Copy library dependencies
#	Copying only occurs if the destination file doesn't exist or the source file is newer
#	@echo DEV_LIB_DIR = $(DEV_LIB_DIR)
#	@echo SLIB_EXT = $(SLIB_EXT)
#	@echo DLIB_EXT = $(DLIB_EXT)
#	@for v in `cd $(DEV_LIB_DIR) && find . \(-name \*.$(SLIB_EXT) -o -name \*.$(DLIB_EXT) \)`; do\
#	@SLIBEXT = $(SLIB_EXT); DLIBEXT=$(DLIB_EXT);\
#	for v in `cd $(DEV_LIB_DIR) && find . \(-name \*.a -o -name \*.so \)`; do\
#	@for v in `cd $(DEV_LIB_DIR) && find . -type f | egrep "\.($(SLIB_EXT)|$(DLIB_EXT)$"`; do\
	@for v in `cd $(DEV_LIB_DIR) && find *.$(SLIB_EXT) *.$(DLIB_EXT)`; do\
		if [ $(DEV_LIB_DIR)/$$v -nt $(BUILD_DIR)/lib/$$v ] || [ ! -e $(BUILD_DIR)/lib/$$v ]; then\
			echo Copying $(DEV_LIB_DIR)/$$v to $(BUILD_DIR)/lib;\
			$(INSTALL) -C -m 644 $(DEV_LIB_DIR)/$$v $(BUILD_DIR)/lib;\
		fi;\
	done
	
#	@$(MAKE) install DESTDIR=$(BUILD_DIR)

allojit alloutil alloni:
	@$(MAKE) -C src/$@ install BUILD_DIR=../../$(BUILD_DIR) DESTDIR=../../$(BUILD_DIR)


# AlloCore externals
externals: gamma glv

gamma glv:
	@$(MAKE) -C externals/$@ install DESTDIR=../../$(BUILD_DIR)


#$(EXTRA_MODULES):
#	@$(MAKE) -C modules/$@ external
#	@$(MAKE) -C modules/$@ install DESTDIR=`pwd`/$(BUILD_DIR)


# Install library into path specified by DESTDIR
# Include files are copied into DESTDIR/include/LIB_NAME
# Library files are copied into DESTDIR/lib
install: allocore

#	copy all header files from local build directory to destination
	@for v in `cd $(BUILD_DIR)/include && find * -type d ! -path '*.*'`; do \
		$(INSTALL) -d $(DESTDIR)/include/$$v; \
		$(INSTALL) -C -m 644 $(BUILD_DIR)/include/$$v/*.h* $(DESTDIR)/include/$$v;\
	done

# 	copy all library files from local build directory to destination
	@for v in `cd $(BUILD_DIR)/lib && find * -type d ! -path '*.*'` .; do \
		$(INSTALL) -d $(DESTDIR)/lib/$$v; \
		$(INSTALL) -C -m 644 $(BUILD_DIR)/lib/$$v/*.a $(DESTDIR)/lib/$$v; \
	done


# Archive repository
archive:
	$(eval $@_TMP := $(shell mktemp -d tmp.XXXXXXXXXX))
	@echo Creating archive, this may take some time...
	@echo Creating temporary export...
	@svn export --force . $($@_TMP)
	@echo Compressing...
	@cd $($@_TMP) && tar -czf ../allocore.tar.gz .
	@echo Compression complete.
	@rm -R $($@_TMP)


# Remove build files
.PHONY: clean
clean: createFolders
# Clean only removes object files for now; avoids unintentional removal of user files
#	@$(RM) -rf $(BUILD_DIR)*
#	@$(RM) -rf $(TEST_DIR)/$(BUILD_DIR)*
	@$(RM) -f $(OBJ_DIR)*
	@$(RM) -f $(TEST_DIR)/$(OBJ_DIR)*
	@$(MAKE) -C externals/gamma clean
	@$(MAKE) -C externals/glv clean

# Build unit tests
test: allocore external
	@$(MAKE) -C $(TEST_DIR) test
