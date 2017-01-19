[![Build Status](https://travis-ci.org/jsiloto/jsonrpc-lean.svg?branch=master)](https://travis-ci.org/jsiloto/jsonrpc-lean)
[![Coverage Status](https://coveralls.io/repos/github/jsiloto/jsonrpc-lean/badge.svg)](https://coveralls.io/github/jsiloto/jsonrpc-lean)

# jsonrpc-lean

This is a fork from uskr/jsonrpc-lean intended for use in embedded systems.
Original rapidjson (https://github.com/miloyip/rapidjson) dependency
makes use of C++ stream libraries that imply in a large amount of linked
code to binary products.
This may be undesired in code memory limited systems.
This fork aims to overcome this replacing rapidjson with a lightweight
json implementation json11(https://github.com/dropbox/json11).


