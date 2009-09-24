#=========================================================================
# Allo Network makefile
#=========================================================================

include ../Makefile.config
include  ./Makefile.config

SRC =	$(OSC_SRC) \
	test/unitTest.cpp

OBJS = $(SRC:.cpp=.o)

.cpp.o:
	@echo CC $<
	@$(CC) -c $(CFLAGS) -o $*.o $<

testNetwork: $(OBJS)
	@echo $@
	@$(CC) $(CFLAGS) -o $@ $(OBJS) $(LFLAGS)

clean:	
	@rm -f *.o
	@rm -f $(OSC_DIR_SRC)/*.o
	@rm -f $(OSC_DIR_SRC)/$(OSC_DIR_PF)/*.o
	@rm -f src/*.o
	@rm -f test/*.o
	@cd ../$(SYS_DIR) && make clean


