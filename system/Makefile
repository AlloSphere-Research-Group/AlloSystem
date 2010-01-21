#=========================================================================
# Allo System makefile
#=========================================================================

include ../Makefile.config
include  ./Makefile.config

SRC =	test/unitTest.cpp

OBJS = $(SRC:.cpp=.o)

.cpp.o:
	@echo CC $<
	@$(CC) -c $(CFLAGS) -o $*.o $<

testSystem: $(OBJS)
	@echo $@
	@$(CC) $(CFLAGS) src/al_Time.cpp -o $@ $(OBJS) $(LFLAGS)

clean:	
	@rm -f *.o
	@rm -f src/*.o
	@rm -f test/*.o


