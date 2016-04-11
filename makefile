
TOOLS := bunch-generator \
converter \
revert-lines \
scan-columns \
scramble-lines \
seleziona-particelle \
sum-columns


all:
	@for i in $(TOOLS) ; do \
	$(MAKE) -C $$i ;\
	done

clean:
	@for i in $(TOOLS) ; do \
	$(MAKE) -C $$i clean ;\
	done

cleanall: clean
	