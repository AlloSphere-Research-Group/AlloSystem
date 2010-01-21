#=========================================================================
# AlloCore IO makefile
#=========================================================================

include ../Makefile.config
include  ./Makefile.config

test/testAudioIO.cpp: src/al_AudioIO.o
	@$(CC) $(CFLAGS) $@ -o $(*F) $(filter %.o, $^) $(LFLAGS) $(LFLAGS_AUDIO)
	@./$(*F) &

test/testWindowGL.cpp: $(OBJ_WINDOW)
	@$(CC) $(CFLAGS) $@ -o $(*F) $(filter %.o, $^) $(LFLAGS) $(LFLAGS_WINDOW)
	@./$(*F) &

tests: test/testAudioIO.cpp test/testWindowGL.cpp

# the old way builds .o and then links
#.cpp.o:
#	@echo CC $<
#	@$(CC) -c $(CFLAGS) -o $*.o $<

#testAudioIO: src/al_AudioIO.o test/testAudioIO.o
#	@echo $@
#	@$(CC) $(CFLAGS) -o $@ $(filter %.o, $^) $(LFLAGS) $(LFLAGS_AUDIO)

#testWindowGL: $(OBJ_WINDOW) test/testWindowGL.o
#	@echo $@
#	@$(CC) $(CFLAGS) -o $@ $(filter %.o, $^) $(LFLAGS) $(LFLAGS_WINDOW)

#tests: testAudioIO testWindowGL

.PHONY: clean
clean:	
	@rm -f src/*.o
	@rm -f test/*.o


