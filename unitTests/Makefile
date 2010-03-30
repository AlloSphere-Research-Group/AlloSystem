#=========================================================================
# AlloCore unit tests makefile
#=========================================================================

include ../Makefile.config
include ../$(IO_DIR)/Makefile.config

#VPATH += 

SRCS =	utIOAudioIO.cpp \
	utIOWindowGL.cpp \
	utProtocolOSC.cpp \
	utProtocolSerialize.cpp \
	utSystem.cpp \
	utTypes.cpp \
	utAllocore.cpp

OBJS = $(addsuffix .o, $(basename $(notdir $(SRCS))))

LFLAGS += -L../$(BIN_DIR) -L../$(DEV_DIR)/lib -lallocore

../$(OBJ_DIR)/%.o: %.cpp
	@echo CC $< $@
	@$(CC) -c $(CFLAGS) $< -o $@

../$(BIN_DIR)/utAllocore: $(addprefix ../$(OBJ_DIR)/, $(OBJS))
#	@echo $(LFLAGS)
	@echo $(@F)
	@$(CC) $(CFLAGS) -o $@ $^ $(LFLAGS)


#$(BIN_DIR)/%: $(OBJ_DIR)/%.o
#	@echo $@
#	@$(CC) $(CFLAGS) $@ -o $@ $^ $(LFLAGS)
#	@$(CC) $(CFLAGS) $@ -o $(TEST_EXEC) $(filter %.o, $^) $(LFLAGS)
#	@$(RUN_TEST)