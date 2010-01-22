#=========================================================================
# AlloCore IO makefile
#=========================================================================

include ../Makefile.config
include  ./Makefile.config

.cpp.o:
	@echo CC $<
	@$(CC) -c $(CFLAGS) -o $*.o $<

test/testAudioIO.cpp: src/al_AudioIO.o FORCE
	@$(CC) $(CFLAGS) $@ -o $(TEST_EXEC) $(filter %.o, $^) $(LFLAGS) $(LFLAGS_AUDIO)
#	@$(RUN_TEST)

test/testWindowGL.cpp: $(OBJ_WINDOW) FORCE
	@$(CC) $(CFLAGS) $@ -o $(TEST_EXEC) $(filter %.o, $^) $(LFLAGS) $(LFLAGS_WINDOW)
#	@$(RUN_TEST)

tests: test/testAudioIO.cpp test/testWindowGL.cpp

.PHONY: clean
clean:	
	@rm -f src/*.o
	@rm -f test/*.o


