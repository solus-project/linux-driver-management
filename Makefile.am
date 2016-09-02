ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST = ${top_srcdir}/README.md \
	     ${top_srcdir}/LICENSE.LGPL2.1

NULL =
CLEANFILES =

# set library version info
LIBLDM_CURRENT=0
LIBLDM_REVISION=0
LIBLDM_AGE=0

AM_CFLAGS = \
	-fstack-protector -Wall -pedantic \
	-Wstrict-prototypes -Wundef -fno-common \
	-Werror-implicit-function-declaration \
	-Wformat -Wformat-security -Werror=format-security \
	-Wconversion -Wunused-variable -Wunreachable-code \
	-Wall -W -D_FORTIFY_SOURCE=2 -std=c11

AM_CPPFLAGS = \
	-I $(top_srcdir)/src/include

lib_LTLIBRARIES = libldm.la

# Main library
libldm_la_SOURCES = 		\
	src/include/atomics.h	\
	src/include/ldm.h	\
	src/include/util.h	\
	src/detect/main.c 	\
	src/manager/manager.c

libldm_la_CFLAGS = \
	$(AM_CFLAGS) \
	$(CORE_CFLAGS) \
	-fvisibility=hidden

libldm_la_LDFLAGS = \
	$(AM_LDFLAGS) \
	-version-info $(LIBLDM_CURRENT):$(LIBLDM_REVISION):$(LIBLDM_AGE) \
	-export-dynamic

libldm_la_LIBADD = \
	$(CORE_LIBS)
