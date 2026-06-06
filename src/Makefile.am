## Process this file with automake to produce Makefile.in

INCLUDES = \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	@PACKAGE_CFLAGS@

bin_PROGRAMS = interfaces

interfaces_SOURCES = \
	main.c \
	support.c support.h \
	interface.c interface.h \
	callbacks.c callbacks.h \
	member.c member.h \
	course.c course.h \
	coach.c coach.h \
	event.c event.h \
	equipment.c equipment.h

interfaces_LDADD = @PACKAGE_LIBS@ $(INTLLIBS) -lcurl

