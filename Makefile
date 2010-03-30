#=========================================================================
# AlloCore main makefile
#=========================================================================

include ./Makefile.config

# Include configuration files of modules
# TODO: Permit selective inclusive of modules for building a library
# and doing unit tests.
include ./$(IO_DIR)/Makefile.config
include ./$(PRO_DIR)/Makefile.config
include ./$(SYS_DIR)/Makefile.config
include ./$(TYPES_DIR)/Makefile.config

# prefix full path to source files
IO_SRC		:= $(addprefix $(IO_DIR)/src/, $(IO_SRC))
PRO_SRC		:= $(addprefix $(PRO_DIR)/src/, $(PRO_SRC))
TYPES_SRC	:= $(addprefix $(TYPES_DIR)/src/, $(TYPES_SRC))
SYS_SRC		:= $(addprefix $(SYS_DIR)/src/, $(SYS_SRC))

SRCS = $(IO_SRC) $(PRO_SRC) $(SYS_SRC) $(TYPES_SRC)
OBJS = $(addsuffix .o, $(basename $(notdir $(SRCS))))


#--------------------------------------------------------------------------
# Targets
#--------------------------------------------------------------------------
$(OBJ_DIR)/%.o: %.cpp
	@echo CC $< $@
	@$(CC) -c $(CFLAGS) $< -o $@

$(OBJ_DIR)/%.o: %.c
	@echo CC $< $@
	@$(CC) -c $(CFLAGS) $< -o $@

all: $(BIN_DIR)/$(SLIB_FILE)

$(BIN_DIR)/$(SLIB_FILE): $(addprefix $(OBJ_DIR)/, $(OBJS))
	@echo AR $@
	@$(AR) $@ $^
	@$(RANLIB) $@

#$(DLIB_FILE): $(addprefix $(OBJ_DIR)/, $(OBJS))


.PHONY: unitTests
unitTests: $(BIN_DIR)/$(SLIB_FILE)
	@make --directory $(TEST_DIR)

.PHONY: clean
clean:
#	@echo $(VPATH)
	@find $(BIN_DIR) -type f ! -path '*.svn*' | xargs rm
#	@cd $(IO_DIR) && make clean
#	@cd $(PRO_DIR) && make clean
#	@cd $(SYS_DIR) && make clean

.PHONY: cleanall
cleanall:
	@find $(BUILD_DIR) -type f ! -path '*.svn*' | xargs rm

# Build (and run) all the unit tests
#tests:
#	@cd $(IO_DIR) && make tests
#	@cd $(PRO_DIR) && make tests
#	@cd $(SYS_DIR) && make tests