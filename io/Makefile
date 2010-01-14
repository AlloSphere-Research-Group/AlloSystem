#=========================================================================
# AlloCore IO makefile
#=========================================================================

include ../Makefile.config
include  ./Makefile.config

.cpp.o:
	@echo CC $<
	@$(CC) -c $(CFLAGS) -o $*.o $<

testAudioIO: src/al_AudioIO.o test/testAudioIO.o
	@echo $@
	@$(CC) $(CFLAGS) -o $@ $(filter %.o, $^) $(LFLAGS) $(LFLAGS_AUDIO)

testWindowGL: $(OBJ_WINDOW) test/testWindowGL.o
	@echo $@
	@$(CC) $(CFLAGS) -o $@ $(filter %.o, $^) $(LFLAGS) $(LFLAGS_WINDOW)

.PHONY: clean
clean:	
	@rm -f src/*.o
	@rm -f test/*.o


