// This file is derived from xsonrpc Copyright (C) 2015 Erik Johansson <erik@ejohansson.se>
// This file is part of jsonrpc-lean, a c++11 JSON-RPC client/server library.
//
// Modifications and additions Copyright (C) 2015 Adriano Maia <tony@stark.im>
//
// This library is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation; either version 2.1 of the License, or (at your
// option) any later version.
//
// This library is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
// for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this library; if not, write to the Free Software Foundation,
// Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

#ifndef JSONRPC_LEAN_VALUE_H
#define JSONRPC_LEAN_VALUE_H

#include <cstdint>
#include <iosfwd>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include "util.h"
#include "fault.h"

struct tm;
#include "../json11.hpp"
using Json=json11::Json;

namespace json11{
    template<> inline  Json::array Json::AsType<typename Json::array>() const {
        return array_items();
    }

    template<> inline  Json::object Json::AsType<typename Json::object>() const {
        return object_items();
    }

    template<> inline  bool Json::AsType<bool>() const {
        return bool_value();
    }

    template<> inline  double Json::AsType<double>() const {
        return number_value();
    }

    template<> inline  std::string Json::AsType<typename std::string>() const {
        return string_value();
    }

    template<> inline Json Json::AsType<Json>() const {
        return Json(*this);
    }
}


#endif // JSONRPC_LEAN_VALUE_H
