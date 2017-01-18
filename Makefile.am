AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS}

pkgconfigdir = $(datadir)/pkgconfig
pkgconfig_DATA = @PACKAGE_NAME@.pc

SUBDIRS = src test
dist_noinst_SCRIPTS = autogen.sh

if HAVE_DOXYGEN
doxyfile.stamp: doxygen
	echo Timestamp > doxyfile.stamp
CLEANFILES = doxyfile.stamp
#all-local: doxyfile.stamp
endif

