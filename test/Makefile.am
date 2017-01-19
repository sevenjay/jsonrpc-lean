if HAVE_UNITTEST
 
# Common C/C++ compiler flags
CCXXFLAGS   =  -fno-strict-aliasing
CCXXFLAGS   += -Wall -Wextra -Werror
AM_CFLAGS   =  $(CCXXFLAGS)
AM_CXXFLAGS =  $(CCXXFLAGS)
 
# Specific C or C++ compiler flags
AM_CFLAGS   += -std=c11
AM_CXXFLAGS += -std=c++11
 
check_PROGRAMS = jsonrpc-test
jsonrpc_test_CPPFLAGS = @GTEST_CFLAGS@ -I$(srcdir)/../src
jsonrpc_test_LDADD = ../src/libjsonrpc-lean.a @GTEST_LIBS@ -lgmock_main -lreadline
jsonrpc_test_SOURCES =
jsonrpc_test_SOURCES += test_server.cpp
 
 
TESTS = jsonrpc-test
 
check-local::
	gcovr -r $(top_srcdir)
 
endif