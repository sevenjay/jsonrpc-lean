# Common C/C++ compiler flags
CCXXFLAGS   =  -fno-strict-aliasing
CCXXFLAGS   += -Wall -Wextra -Werror
CCXXFLAGS   += -ffunction-sections -fdata-sections  # Maybe remove those for the application?
AM_CFLAGS   =  $(CCXXFLAGS)
AM_CXXFLAGS =  $(CCXXFLAGS)
 
# Specific C or C++ compiler flags
AM_CFLAGS   += -std=c11
AM_CXXFLAGS += -std=c++11


noinst_LIBRARIES = lib@PACKAGE_NAME@.a

@PACKAGE_NAME@_includedir = $(includedir)/@PACKAGE_NAME@


lib@PACKAGE_NAME@_a_SOURCES =
lib@PACKAGE_NAME@_a_SOURCES += json11.cpp
 
nobase_@PACKAGE_NAME@_include_HEADERS =
nobase_@PACKAGE_NAME@_include_HEADERS += json11.hpp
nobase_@PACKAGE_NAME@_include_HEADERS += jsonrpc-lean/client.h
nobase_@PACKAGE_NAME@_include_HEADERS += jsonrpc-lean/dispatcher.h
nobase_@PACKAGE_NAME@_include_HEADERS += jsonrpc-lean/fault.h
nobase_@PACKAGE_NAME@_include_HEADERS += jsonrpc-lean/integer_seq.h
nobase_@PACKAGE_NAME@_include_HEADERS += jsonrpc-lean/json.h
nobase_@PACKAGE_NAME@_include_HEADERS += jsonrpc-lean/jsonreader.h
nobase_@PACKAGE_NAME@_include_HEADERS += jsonrpc-lean/request.h
nobase_@PACKAGE_NAME@_include_HEADERS += jsonrpc-lean/response.h
nobase_@PACKAGE_NAME@_include_HEADERS += jsonrpc-lean/server.h
nobase_@PACKAGE_NAME@_include_HEADERS += jsonrpc-lean/util.h
nobase_@PACKAGE_NAME@_include_HEADERS += jsonrpc-lean/value.h

AUTOMAKE_OPTIONS = subdir-objects
 
 
include $(CPQD_BT)/mk/cpqd_drc_rules.mk


