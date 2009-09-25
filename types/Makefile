#=========================================================================
# Allo Types makefile
#=========================================================================

include ../Makefile.config
include  ./Makefile.config

SRC =	test/unitTest.cpp

OBJS = $(SRC:.cpp=.o)

.cpp.o:
	@echo CC $<
	@$(CC) -c $(CFLAGS) -o $*.o $<

testTypes: $(OBJS)
	@echo $@
	@$(CC) $(CFLAGS) -o $@ $(OBJS) $(LFLAGS)

clean:	
	@rm -f *.o
	@rm -f src/*.o
	@rm -f test/*.o


