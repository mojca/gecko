 CC           := gcc
WFLAGS       := -Wstrict-prototypes -Wmissing-prototypes \
                -Wmissing-declarations -Wimplicit -Wreturn-type -Wunused \
                -Wcomment -Wformat
#                -Wuninitialized -Werror
CPPFLAGS     := -I.. -Iheader
CFLAGS       := -g -ansi $(WFLAGS)

srcdir       := .

lib_sis3100.a:  sis3100_vme_calls.o sharc_utils.o
	ar cr $@ $^

sis3100_vme_calls.o: sis3100_vme_calls.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $^




sharc_utils.o: sharc_utils.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c   $^

clean:
	rm -f *.o

depend:
	cp Makefile Makefile.bak
	sed -e '/^# DO NOT DELETE THIS LINE/,$$d' < Makefile.bak > Makefile
	echo '# DO NOT DELETE THIS LINE' >> Makefile
	echo ' ' >> Makefile
	for i in $(cfiles) ; do \
	  echo checking $$i ; \
	  $(CC) -M $(CPPFLAGS) $(srcdir)/$$i >> Makefile ; \
	done
# DO NOT DELETE THIS LINE






