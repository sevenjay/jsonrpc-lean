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

namespace jsonrpc {

    class Value : public Json{
    public:
        typedef std::vector<Value> Array;
        typedef tm DateTime;
        typedef std::string String;
        typedef std::map<std::string, Value> Struct;

        static Json fromArray(Array value){
            std::vector<Json> array;
            for (auto& element : value) {
                array.push_back(element);
            }
            return Json(array);
        }

        static Json fromStruct(Struct value){
            Json::object object;
            for (auto& element : value) {
                object[element.first] = element.second;
            }
            return Json(object);
        }

        Value() : myType(Type::NUL) {}

        Value(Array value) :Json(fromArray(value)), myType(Type::ARRAY) {
            //as.myArray = new Array(std::move(value));
        }

        Value(bool value) :Json(value), myType(Type::BOOL) {
            //as.myBoolean = value;
        }

        Value(double value) :Json(value), myType(Type::NUMBER) {
            //as.myNumber = value;
        }

        Value(int32_t value) :Json(value), myType(Type::NUMBER) {
            //as.myNumber = value;
        }

        Value(int64_t value) :Json(static_cast<double>(value)), myType(Type::NUMBER) {
            //as.myNumber = static_cast<double>(value);
        }

        Value(const char* value) : Value(String(value)) {}

        Value(String value) :Json(value), myType(Type::STRING) {
            //as.myString = new String(std::move(value));
        }

        Value(Struct value) :Json(fromStruct(value)), myType(Type::OBJECT) {
            //as.myStruct = new Struct(std::move(value));
        }

        Value(Json object): Json(object), myType(object.type()){

        }


        ~Value() {
            Reset();
        }

        template<typename T>
        Value(std::vector<T> value) : Value(Array{}) {
            Array a {};
            for (auto& v : value) {
                a.emplace_back(std::move(v));
            }
            *this = Value(Json(a));
        }

        template<typename T>
        Value(const std::map<std::string, T>& value) : Value(Struct{}) {
            Struct a {};
            for (auto& v : value) {
                a.emplace(v.first, v.second);
            }
            *this = Value(Json(a));
        }

        template<typename T>
        Value(const std::unordered_map<std::string, T>& value) : Value(Struct{}) {
            Struct a {};
            for (auto& v : value) {
                a.emplace(v.first, v.second);
            }
            *this = Value(Json(a));
        }

        explicit Value(const Value& other) : Json(other), myType(other.myType) {
            switch (myType) {
            case Type::BOOL:
            case Type::NUMBER:
            case Type::NUL:
                break;

            case Type::ARRAY:
                //as.myArray = new Array(other.AsArray());
                break;
            case Type::STRING:
                //as.myString = new String(other.AsString());
                break;
            case Type::OBJECT:
                //as.myStruct = new Struct(other.AsStruct());
                break;
            }
        }

        Value& operator=(const Value&) = delete;

        Value(Value&& other) noexcept : Json(other), myType(other.myType) {
            other.myType = Type::NUL;
        }

        Value& operator=(Value&& other) noexcept {
            if (this != &other) {
                Reset();
                Json::operator=(other);
                myType = other.myType;
                //as = other.as;

                other.myType = Type::NUL;
            }
            return *this;
        }

        /*
        const Array& AsArray() const {
            if (is_array()) {
                return *as.myArray;
            }
            throw InvalidParametersFault();
        }

        const String& AsBinary() const { return AsString(); }

        const bool& AsBoolean() const {
            if (is_bool()) {
                return as.myBoolean;
            }
            throw InvalidParametersFault();
        }

        const double& AsNumber() const {
            if (is_number()) {
                return as.myNumber;
            }
            throw InvalidParametersFault();
        }

        const String& AsString() const {
            if (is_string()) {
                return *as.myString;
            }
            throw InvalidParametersFault();
        }

        const Struct& AsStruct() const {
            if (is_object()) {
                return *as.myStruct;
            }
            throw InvalidParametersFault();
        }
        */

        template<typename T>
        T AsType() const;

        //inline const Value& operator[](Array::size_type i) const;
        //inline const Value& operator[](const Struct::key_type& key) const;

    private:
        void Reset() {
            switch (myType) {
            case Type::ARRAY:
                //delete as.myArray;
                break;
            case Type::STRING:
                //delete as.myString;
                break;
            case Type::OBJECT:
                //delete as.myStruct;
                break;
            case Type::BOOL:
            case Type::NUMBER:
            case Type::NUL:
                break;
            }

            myType = Type::NUL;

        }

        Type myType;
        /*
        union {
            Array* myArray;
            bool myBoolean;
            DateTime* myDateTime;
            String* myString;
            Struct* myStruct;
            struct {
                double myNumber;
                int32_t myInteger32;
                int64_t myInteger64;
            };
        } as;
        */
    };

    template<> inline  Json::array Value::AsType<typename Json::array>() const {
        return array_items();
    }

    template<> inline  Json::object Value::AsType<typename Json::object>() const {
        return object_items();
    }

    /*
    template<> inline  Value::Array Value::AsType<typename Value::Array>() const {
        return AsArray();
    }
    */

    template<> inline  bool Value::AsType<bool>() const {
        return bool_value();
    }

    template<> inline  double Value::AsType<double>() const {
        return number_value();
    }

    template<> inline  std::string Value::AsType<typename std::string>() const {
        return string_value();
    }

    /*
    template<> inline Value::Struct Value::AsType<typename Value::Struct>() const {
        return AsStruct();
    }
    */

    template<> inline Json Value::AsType<Json>() const {
        return Json(*this);
    }

    /*
    template<> inline Value Value::AsType<Value>() const {
        return Value(*this);
    }
    */

    /*
    inline const Value& Value::operator[](Array::size_type i) const {
        return Json::operator[](i);
        //return AsArray().at(i);
    };

    inline const Value& Value::operator[](const Json::object::key_type& key) const {
        return Json::operator[](key);
        //return AsStruct().at(key);
    }
    */


} // namespace jsonrpc

#endif // JSONRPC_LEAN_VALUE_H
