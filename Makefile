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

all:	$(BINDIR)/proj2.1 $(BINDIR)/proj2.2 $(BINDIR)/proj2.3a $(BINDIR)/proj2.3b

$(OBJDIR)/proj2.1.o: $(SRCDIR)/proj2.1.c | $(OBJDIR)
	$(MPICC) -Wall -g -c -O -I$(INCDIR) -o $@ $^

$(OBJDIR)/proj2.2.o: $(SRCDIR)/proj2.2.c | $(OBJDIR)
	$(MPICC) -Wall -g -c -O -I$(INCDIR) -o $@ $^

$(OBJDIR)/proj2.3a.o: $(SRCDIR)/proj2.3a.c | $(OBJDIR)
	$(MPICC) -Wall -g -c -O -I$(INCDIR) -o $@ $^

$(OBJDIR)/proj2.3b.o: $(SRCDIR)/proj2.3b.c | $(OBJDIR)
	$(MPICC) -Wall -g -c -O -I$(INCDIR) -o $@ $^

$(BINDIR)/proj2.1: $(OBJDIR)/proj2.1.o | $(BINDIR)
	$(MPICC) -o $@ $^

$(BINDIR)/proj2.2: $(OBJDIR)/proj2.2.o | $(BINDIR)
	$(MPICC) -o $@ $^

$(BINDIR)/proj2.3a: $(OBJDIR)/proj2.3a.o | $(BINDIR)
	$(MPICC) -lm -o $@ $^

$(BINDIR)/proj2.3b: $(OBJDIR)/proj2.3b.o | $(BINDIR)
	$(MPICC) -lm -o $@ $^

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
