# Makefile to build or clean all labs.
SUBDIRS += 0-pi-setup 
SUBDIRS += 1-mailboxes
SUBDIRS += 2-dynamic-code-gen

.PHONY: all check clean
all check clean: $(SUBDIRS)

all: TARGET=all
check: TARGET=check
clean: TARGET=clean

# No, you can't do TARGET=$@, or at least I don't know how to.

# recursive call to make
$(SUBDIRS): force
	$(MAKE) -s -C $@ $(TARGET) RUN=0

clean:
	rm -f *~ *.bak

.PHONY: force
	force :;
