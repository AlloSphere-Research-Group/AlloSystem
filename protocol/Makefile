#=========================================================================
# AlloCore Protocol makefile
#=========================================================================

include ../Makefile.config
include  ./Makefile.config

.cpp.o:
	@echo CC $<
	@$(CC) -c $(CFLAGS) -o $*.o $<

testOSC: $(OSC_OBJ)
	@echo $@
	@$(CC) $(CFLAGS) ../$(SYS_DIR)/src/al_time.cpp test/testOSC.cpp -o $@ $(OSC_OBJ) $(LFLAGS)

.PHONY: clean
clean:	
	@rm -f src/*.o
	@rm -f test/*.o
	@rm -f $(OSC_OBJ)
	@cd ../$(SYS_DIR) && make clean


