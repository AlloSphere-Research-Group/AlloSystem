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

FORCE:

test/testOSC.cpp: $(OSC_OBJ) FORCE
	@$(CC) $(CFLAGS) ../$(SYS_DIR)/src/al_Time.cpp $@ -o $(*F) $(OSC_OBJ) $(LFLAGS)
	@./$(*F)

test/testSerialize.cpp: FORCE
	@$(CC) $(CFLAGS) src/al_Serialize.c src/al_Serialize.cpp $@ -o $(*F) $(LFLAGS)
	@./$(*F)

.PHONY: clean
clean:	
	@rm -f src/*.o
	@rm -f test/*.o
	@rm -f $(OSC_OBJ)
	@cd ../$(SYS_DIR) && make clean


