#=========================================================================
# AlloCore Protocol makefile
#=========================================================================

include ../Makefile.config
include  ./Makefile.config

.cpp.o:
	@echo CC $<
	@$(CC) -c $(CFLAGS) -o $*.o $<

#testOSC: $(OSC_OBJ)
#	@echo $@
#	@$(CC) $(CFLAGS) ../$(SYS_DIR)/src/al_Time.cpp test/testOSC.cpp -o $@ $(OSC_OBJ) $(LFLAGS)

test/testOSC.cpp: $(OSC_OBJ)
	@echo $@
	@$(CC) $(CFLAGS) ../$(SYS_DIR)/src/al_Time.cpp $@ -o $(*F) $(OSC_OBJ) $(LFLAGS)

test/testSerialize.cpp: FORCE
	@echo $@
	@$(CC) $(CFLAGS) src/al_Serialize.c src/al_Serialize.cpp $@ -o $(*F) $(LFLAGS)

FORCE:

.PHONY: clean
clean:	
	@rm -f src/*.o
	@rm -f test/*.o
	@rm -f $(OSC_OBJ)
	@cd ../$(SYS_DIR) && make clean


