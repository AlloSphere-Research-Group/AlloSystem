#=========================================================================
# Allo System makefile
#=========================================================================

include ../Makefile.config
include  ./Makefile.config

test/testSystem.cpp: $(OBJS) FORCE
	@$(CC) $(CFLAGS) src/al_Time.cpp $@ -o $(*F) $(LFLAGS)
	@$(RUN_TEST)

tests: test/testSystem.cpp

clean:	
	@rm -f *.o
	@rm -f src/*.o
	@rm -f test/*.o


