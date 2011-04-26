CC           := gcc
WFLAGS       := -Wstrict-prototypes -Wmissing-prototypes \
                -Wmissing-declarations -Wimplicit -Wreturn-type -Wunused \
                -Wcomment -Wformat
#                -Wuninitialized -Werror
CPPFLAGS     := -I.. -Iloader
CFLAGS       := -g $(WFLAGS)

srcdir       := .

OBJECTS := sis3150usb_vme_calls.o sis3150usb_vme_linux.o usrpadaptor.o \
  loader/fusb.o loader/md5.o loader/usrp_basic.o loader/usrp_config.o \
  loader/usrp_dbid.o loader/usrp_local_sighandler.o loader/usrp_prims.o \
  loader/usrp_standard.o

lib_sis3150.a:  $(OBJECTS)
	ar cr $@ $^

clean:
	rm -f *.o loader/*.o

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






