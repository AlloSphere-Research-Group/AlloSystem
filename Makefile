#=========================================================================
# AlloCore main makefile
#=========================================================================

include Makefile.config

# Include configuration files of modules
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
SRCS		= $(GFX_SRC) $(IO_SRC) $(PRO_SRC) $(MATH_SRC) $(SND_SRC) $(SPA_SRC) $(SYS_SRC) $(TYP_SRC)
OBJS		= $(addsuffix .o, $(basename $(notdir $(SRCS))))

CPPFLAGS	+= $(EXT_CPPFLAGS)
LDFLAGS		+= $(EXT_LDFLAGS)

CPPFLAGS	+= $(addprefix -I, $(INC_DIRS) $(RINC_DIRS))
LDFLAGS		:= $(addprefix -L, $(LIB_DIRS)) $(LDFLAGS)

CFLAGS		:= $(CPPFLAGS) $(CFLAGS)
CXXFLAGS	:= $(CFLAGS) $(CXXFLAGS)

#--------------------------------------------------------------------------
# Rules
#--------------------------------------------------------------------------
.PHONY: clean cleanall test

# Build object file from C++ source
$(OBJ_DIR)%.o: %.cpp
	@echo CXX $< $@
	@$(CXX) -c $(CXXFLAGS) $< -o $@

# Build object file from C source
$(OBJ_DIR)%.o: %.c
	@echo CC $< $@
	@$(CC) -c $(CFLAGS) $< -o $@

all: $(SLIB_PATH)

# Build static library
$(SLIB_PATH): createFolders $(addprefix $(OBJ_DIR), $(OBJS))
	@echo AR $@
	@$(RM) $@
	@$(AR) $@ $(filter %.o, $^)
	@$(RANLIB) $@
#	@libtool -static $(LDFLAGS) $(filter %.o, $^) -o $@
#	@libtool -static $@ $(patsubst %, $(DEV_DIR)lib/lib%.a, $(STATIC_LIBS)) -o $@

# Dummy target to force rebuilds
FORCE:

# Compile and run source files in examples/ folder
examples/%.cpp: $(SLIB_PATH) FORCE
	@$(CXX) $(CXXFLAGS) -o $(BIN_DIR)$(*F) $@ $(LDFLAGS) -whole-archive $(SLIB_PATH)
#	@$(CXX) $(CXXFLAGS) -o $(BIN_DIR)$(*F) $@ $(LDFLAGS) $(SLIB_PATH)
#	@$(CXX) $(CXXFLAGS) -o $(BIN_DIR)$(*F) $@ $(SLIB_PATH)
ifneq ($(AUTORUN), 0)
	@$(BIN_DIR)$(*F)
endif

# Build unit tests
test: $(SLIB_PATH)
	@$(MAKE) -C $(TEST_DIR)

# Install library into path specified by DESTDIR
# Include files are copied into DESTDIR/include/LIB_NAME and
# library files are copied to DESTDIR/lib
install: $(SLIB_PATH)
	@$(INSTALL) -d $(DESTDIR)/
	@$(INSTALL) -d $(DESTDIR)/lib
	@$(INSTALL) -d $(DESTDIR)/include/$(LIB_NAME)

	@for v in $(MODULE_DIRS); do \
		$(INSTALL) -d  $(DESTDIR)/include/$(LIB_NAME)/$$v; \
		$(INSTALL) -c -m 644 $(INC_DIR)/$$v/*.h* $(DESTDIR)/include/$(LIB_NAME)/$$v;\
	done

#	@$(INSTALL) -d $(addprefix $(DESTDIR)/include/$(LIB_NAME)/, $(MODULE_DIRS))
	@$(INSTALL) -c -m 644 $(SLIB_PATH) $(DESTDIR)/lib
	@$(INSTALL) -c -m 644 $(DEV_DIR)lib/*.a $(DESTDIR)/lib
#	@$(INSTALL) -c -m 644 $(EXT_LIB_DIR)* $(DESTDIR)/lib
	@$(RANLIB) $(DESTDIR)/lib/$(SLIB_FILE)

# Remove active build configuration binary files
clean: createFolders
	@$(RM) -r $(BIN_DIR)*

# Remove all build configuration binary files
cleanall:
	@$(RM) -r $(BUILD_DIR)*

createFolders:
	@mkdir -p $(OBJ_DIR)

# Create file with settings for linking to external libraries
external:
	@echo '\
CPPFLAGS += $(EXT_CPPFLAGS) \r\n\
LDFLAGS  += $(EXT_LDFLAGS) \
'> Makefile.external
