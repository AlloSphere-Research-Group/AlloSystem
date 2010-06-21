#=========================================================================
# AlloCore main makefile
#=========================================================================

include ./Makefile.config

# Include configuration files of modules
# TODO: Permit selective inclusive of modules for building a library
# and doing unit tests.
include ./$(SRC_DIR)/$(IO_DIR)/Makefile.config
include ./$(SRC_DIR)/$(MATH_DIR)/Makefile.config
include ./$(SRC_DIR)/$(PRO_DIR)/Makefile.config
include ./$(SRC_DIR)/$(SPA_DIR)/Makefile.config
include ./$(SRC_DIR)/$(SYS_DIR)/Makefile.config
include ./$(SRC_DIR)/$(TYPES_DIR)/Makefile.config

# prefix full path to source files
IO_SRC		:= $(addprefix $(SRC_DIR)/$(IO_DIR)/, $(IO_SRC))
MATH_SRC	:= $(addprefix $(SRC_DIR)/$(MATH_DIR)/, $(MATH_SRC))
PRO_SRC		:= $(addprefix $(SRC_DIR)/$(PRO_DIR)/, $(PRO_SRC))
TYPES_SRC	:= $(addprefix $(SRC_DIR)/$(TYPES_DIR)/, $(TYPES_SRC))
SPA_SRC		:= $(addprefix $(SRC_DIR)/$(SPA_DIR)/, $(SPA_SRC))
SYS_SRC		:= $(addprefix $(SRC_DIR)/$(SYS_DIR)/, $(SYS_SRC))

SRCS		= $(IO_SRC) $(PRO_SRC) $(MATH_SRC) $(SPA_SRC) $(SYS_SRC) $(TYPES_SRC)
OBJS		= $(addsuffix .o, $(basename $(notdir $(SRCS))))

CFLAGS		+= $(addprefix -I./, $(INC_DIRS))
DLIB_FILE 	:= $(addprefix $(BIN_DIR)/, $(DLIB_FILE))
SLIB_FILE 	:= $(addprefix $(BIN_DIR)/, $(SLIB_FILE))

#--------------------------------------------------------------------------
# Targets
#--------------------------------------------------------------------------
# Build object file from C++ source
$(OBJ_DIR)/%.o: %.cpp
	@echo CC $< $@
	@$(CC) -c $(CFLAGS) $< -o $@

# Build object file from C source
$(OBJ_DIR)/%.o: %.c
	@echo CC $< $@
	@$(CC) -c $(CFLAGS) $< -o $@

all: $(SLIB_FILE)

# Build static library
$(SLIB_FILE): createFolders $(addprefix $(OBJ_DIR)/, $(OBJS))
	@echo AR $@
	@rm -f $@
	@$(AR) $@ $(filter %.o, $^)
	@$(RANLIB) $@

# Build dynamic (shared) library
# TODO: implement
#$(DLIB_FILE): createFolders $(addprefix $(OBJ_DIR)/, $(OBJS))

# Dummy target to force rebuilds
FORCE:

# Compile and run source files in examples/ folder
examples/%.cpp: $(SLIB_FILE) FORCE
	@$(CC) $(CFLAGS) -o $(BIN_DIR)/$(*F) $@ $(LFLAGS) $(SLIB_FILE) $(LIB_EXT)
	@./$(BIN_DIR)/$(*F) &

# Build unit tests
.PHONY: test
test: $(SLIB_FILE)
	@make --directory $(TEST_DIR)

# Remove active build configuration binary files
.PHONY: clean
clean: createFolders
#	@echo $(VPATH)
	@find $(BIN_DIR) -type f ! -path '*.svn*' | xargs rm -f
#	@cd $(IO_DIR) && make clean
#	@cd $(PRO_DIR) && make clean
#	@cd $(SYS_DIR) && make clean

# Remove all build configuration binary files
.PHONY: cleanall
cleanall:
	@find $(BUILD_DIR) -type f ! -path '*.svn*' | xargs rm -f

createFolders:
	@mkdir -p $(OBJ_DIR)

