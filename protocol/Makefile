#=========================================================================
# AlloCore Protocol makefile
#=========================================================================

include ../Makefile.config
include  ./Makefile.config

.cpp.o:
	@echo CC $<
	@$(CC) -c $(CFLAGS) $(OSC_CFLAGS) -o $*.o $<

test/testOSC.cpp: $(OSC_OBJ) FORCE
	@$(CC) $(CFLAGS) ../$(SYS_DIR)/src/al_Time.cpp $@ -o $(TEST_EXEC) $(OSC_OBJ) $(LFLAGS)
	@$(RUN_TEST)

test/testSerialize.cpp: FORCE
	@$(CC) $(CFLAGS) src/al_Serialize.c src/al_Serialize.cpp $@ -o $(TEST_EXEC) $(LFLAGS)
	@$(RUN_TEST)

tests: test/testOSC.cpp test/testSerialize.cpp

.PHONY: clean
clean:	
	@rm -f src/*.o
	@rm -f test/*.o
	@rm -f $(OSC_OBJ)
	@cd ../$(SYS_DIR) && make clean


