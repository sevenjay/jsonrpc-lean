AC_INIT([jsonrpc-lean], [0.0.0], [jsiloto], [jsonrpc-lean],
        [https://github.com/jsiloto/jsonrpc-lean])
AC_PREREQ([2.59])

AM_INIT_AUTOMAKE([1.10 -Wall no-define foreign subdir-objects])

dnl Following parameters are used to register this component for future re-use
dnl JIRA if no component for this package has been created just provide the ID
dnl otherwise ID/component/XXXXXX for direct linkage.
                  

PACKAGE_KEYWORDS="jsonrpc-lean"
PACKAGE_TYPE="library"
PACKAGE_LANGUAGES="C++"

PACKAGE_DESCRIPTION="Transport Agnostic C++ Json-RPC implementations without stream dependency"
PACKAGE_DESCRIPTION_LONG="Long description for the lib one liner still"

AC_DEFINE_UNQUOTED(PACKAGE_DESCRIPTION, "${PACKAGE_DESCRIPTION}", [package description])
AC_DEFINE_UNQUOTED(PACKAGE_DESCRIPTION_LONG, "${PACKAGE_DESCRIPTION_LONG}", [long package description])

AC_SUBST([PACKAGE_KEYWORDS],[$PACKAGE_KEYWORDS])
AC_SUBST([PACKAGE_TYPE],[$PACKAGE_TYPE])
AC_SUBST([PACKAGE_LANGUAGES],[$PACKAGE_LANGUAGES])
AC_SUBST([PACKAGE_DESCRIPTION], [${PACKAGE_DESCRIPTION}], [package description])
AC_SUBST([PACKAGE_DESCRIPTION_LONG], [${PACKAGE_DESCRIPTION_LONG}], [long package description])

AC_SUBST([PACKAGE_REQUIRES],[$PACKAGE_REQUIRES],[])


# Remove default "-g -O2" flags if user hasn't supplied custom flags
: ${CFLAGS=""}
: ${CXXFLAGS=""}
 
AC_CANONICAL_HOST


AC_ARG_WITH([debug], AS_HELP_STRING([--with-debug], [Build with debugging symbols]))
AS_IF([test "x$with_debug" = "xyes"], [
  CPPFLAGS="-D_DEBUG=1 ${CPPFLAGS}"
  CFLAGS="-g -gdwarf-2 -gstrict-dwarf -O0 ${CFLAGS}"
], [
  CFLAGS="-O2 ${CFLAGS}"
])
 
 
AC_ARG_WITH([unittest], AS_HELP_STRING([--with-unittest], [Build with unittesting (implies coverage profiling/debugging symbols]))
AS_IF([test "x$with_unittest" = "xyes"], [
  CFLAGS="-fprofile-arcs -ftest-coverage -g -O0 ${CFLAGS}"
  LIBS="-lgcov ${LIBS}"
])
AM_CONDITIONAL([HAVE_UNITTEST], [test "x$with_unittest" = "xyes"])


AC_CHECK_PROGS([DOXYGEN], [doxygen])
AS_IF([test -z "$DOXYGEN"], [
  AC_MSG_WARN([doxygen not found - sudo apt-get install doxygen - might resolve this])
])

AM_CONDITIONAL([HAVE_DOXYGEN], [test -n "$DOXYGEN"])
AC_SUBST([DOC_OUTPUT_DIR], ["doc"])
AC_SUBST([DOC_INPUT], ["${srcdir}/README.md ${srcdir}/doc ${srcdir}/src ${srcdir}/test"])
AC_SUBST([DOC_MAINPAGE], ["${srcdir}/README.md"])

CXXFLAGS="${CFLAGS} ${CXXFLAGS}"
 
AC_PROG_CXX
AC_PROG_CC
AC_PROG_RANLIB
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])
 
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile ${PACKAGE_TARNAME}.pc:pc.in])
AC_CONFIG_FILES([src/Makefile])
AC_CONFIG_FILES([test/Makefile])
AC_CONFIG_FILES([Doxyfile])
AC_OUTPUT