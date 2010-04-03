#=========================================================================
# AlloCore main makefile
#=========================================================================

include ./Makefile.config

# Include configuration files of modules
# TODO: Permit selective inclusive of modules for building a library
# and doing unit tests.
include ./$(SRC_DIR)/$(IO_DIR)/Makefile.config
include ./$(SRC_DIR)/$(PRO_DIR)/Makefile.config
include ./$(SRC_DIR)/$(SYS_DIR)/Makefile.config
include ./$(SRC_DIR)/$(TYPES_DIR)/Makefile.config

# prefix full path to source files
IO_SRC		:= $(addprefix $(SRC_DIR)/$(IO_DIR)/, $(IO_SRC))
PRO_SRC		:= $(addprefix $(SRC_DIR)/$(PRO_DIR)/, $(PRO_SRC))
TYPES_SRC	:= $(addprefix $(SRC_DIR)/$(TYPES_DIR)/, $(TYPES_SRC))
SYS_SRC		:= $(addprefix $(SRC_DIR)/$(SYS_DIR)/, $(SYS_SRC))

SRCS = $(IO_SRC) $(PRO_SRC) $(SYS_SRC) $(TYPES_SRC)
OBJS = $(addsuffix .o, $(basename $(notdir $(SRCS))))


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

all: $(BIN_DIR)/$(SLIB_FILE)

# Build static library
$(BIN_DIR)/$(SLIB_FILE): $(addprefix $(OBJ_DIR)/, $(OBJS))
	@echo AR $@
	@$(AR) $@ $^
	@$(RANLIB) $@

# Build dynamic (shared) library
# TODO: implement
#$(DLIB_FILE): $(addprefix $(OBJ_DIR)/, $(OBJS))


# Build unit tests
.PHONY: test
test: $(BIN_DIR)/$(SLIB_FILE)
	@make --directory $(TEST_DIR)

# Remove active build configuration binary files
.PHONY: clean
clean:
#	@echo $(VPATH)
	@find $(BIN_DIR) -type f ! -path '*.svn*' | xargs rm -f
#	@cd $(IO_DIR) && make clean
#	@cd $(PRO_DIR) && make clean
#	@cd $(SYS_DIR) && make clean

# Remove all build configuration binary files
.PHONY: cleanall
cleanall:
	@find $(BUILD_DIR) -type f ! -path '*.svn*' | xargs rm -f

