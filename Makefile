BINDIR=bin
OBJDIR=build
SRCDIR=src
INCDIR=include
LIBDIR=lib
DOXDIR=dox

CWD=`pwd`
MAKE=make

CC=$(CROSS_COMPILE)gcc
MPICC=mpicc

all:	$(BINDIR)/proj1.1

$(OBJDIR)/proj1.1.o: $(SRCDIR)/proj1.1.c | $(OBJDIR)
	$(MPICC) -Wall -g -c -O -I$(INCDIR) -o $@ $^

$(BINDIR)/proj1.1: $(OBJDIR)/proj1.1.o | $(BINDIR)
	$(MPICC) -o $@ $^

$(BINDIR):
	mkdir $@

$(LIBDIR):
	mkdir $@

$(OBJDIR):
	mkdir $@

$(DOXDIR):
	mkdir $@

doxygen: doxyfile | $(DOXDIR)
	doxygen $^

clean:
	rm -rf $(OBJDIR)

distclean:	| clean
	rm -rf $(BINDIR) $(LIBDIR) $(DOXDIR)
