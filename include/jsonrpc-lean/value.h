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

    class Value {
    public:
        typedef std::vector<Value> Array;
        typedef tm DateTime;
        typedef std::string String;
        typedef std::map<std::string, Value> Struct;

        enum class Type {
            ARRAY,
            BINARY,
            BOOLEAN,
            NUMBER,
            NUL,
            STRING,
            OBJECT
        };

        Value() : myType(Type::NUL) {}

        Value(Array value) : myType(Type::ARRAY) {
            as.myArray = new Array(std::move(value));
        }

        Value(bool value) : myType(Type::BOOLEAN) { as.myBoolean = value; }

        Value(double value) : myType(Type::NUMBER) { as.myNumber = value; }

        Value(int32_t value) : myType(Type::NUMBER) {
            as.myNumber = value;
        }

        Value(int64_t value) : myType(Type::NUMBER) {
            as.myNumber = static_cast<double>(value);
        }

        Value(const char* value) : Value(String(value)) {}

        Value(String value, bool binary = false) : myType(binary ? Type::BINARY : Type::STRING) {
            as.myString = new String(std::move(value));
        }

        Value(Struct value) : myType(Type::OBJECT) {
            as.myStruct = new Struct(std::move(value));
        }

        ~Value() {
            Reset();
        }

        template<typename T>
        Value(std::vector<T> value) : Value(Array{}) {
            as.myArray->reserve(value.size());
            for (auto& v : value) {
                as.myArray->emplace_back(std::move(v));
            }
        }

        template<typename T>
        Value(const std::map<std::string, T>& value) : Value(Struct{}) {
            for (auto& v : value) {
                as.myStruct->emplace(v.first, v.second);
            }
        }

        template<typename T>
        Value(const std::unordered_map<std::string, T>& value) : Value(Struct{}) {
            for (auto& v : value) {
                as.myStruct->emplace(v.first, v.second);
            }
        }

        explicit Value(const Value& other) : myType(other.myType), as(other.as) {
            switch (myType) {
            case Type::BOOLEAN:
            case Type::NUMBER:
            case Type::NUL:
                break;

            case Type::ARRAY:
                as.myArray = new Array(other.AsArray());
                break;
            case Type::BINARY:
            case Type::STRING:
                as.myString = new String(other.AsString());
                break;
            case Type::OBJECT:
                as.myStruct = new Struct(other.AsStruct());
                break;
            }
        }

        Value& operator=(const Value&) = delete;

        Value(Value&& other) noexcept : myType(other.myType), as(other.as) {
            other.myType = Type::NUL;
        }

        Value& operator=(Value&& other) noexcept {
            if (this != &other) {
                Reset();

                myType = other.myType;
                as = other.as;

                other.myType = Type::NUL;
            }
            return *this;
        }

        bool IsArray() const { return myType == Type::ARRAY; }
        bool IsBinary() const { return myType == Type::BINARY; }
        bool IsBoolean() const { return myType == Type::BOOLEAN; }
        bool IsNumber() const { return myType == Type::NUMBER; }
        bool IsNil() const { return myType == Type::NUL; }
        bool IsString() const { return myType == Type::STRING; }
        bool IsStruct() const { return myType == Type::OBJECT; }

        const Array& AsArray() const {
            if (IsArray()) {
                return *as.myArray;
            }
            throw InvalidParametersFault();
        }

        const String& AsBinary() const { return AsString(); }

        const bool& AsBoolean() const {
            if (IsBoolean()) {
                return as.myBoolean;
            }
            throw InvalidParametersFault();
        }

        const double& AsNumber() const {
            if (IsNumber()) {
                return as.myNumber;
            }
            throw InvalidParametersFault();
        }

        const String& AsString() const {
            if (IsString() || IsBinary()) {
                return *as.myString;
            }
            throw InvalidParametersFault();
        }

        const Struct& AsStruct() const {
            if (IsStruct()) {
                return *as.myStruct;
            }
            throw InvalidParametersFault();
        }

        template<typename T>
        inline const T& AsType() const;

        Type GetType() const { return myType; }

        Json toJson() const {
            switch (myType) {
            case Type::ARRAY:{
                std::vector<Json> array;
                for (auto& element : *as.myArray) {
                    array.push_back(element.toJson());
                }
                return Json(array);
                break;
            }
            case Type::BINARY:
                return Json(*as.myString);
                break;
            case Type::BOOLEAN:
                return Json(as.myBoolean);
                break;
            case Type::NUMBER:
                return Json(as.myNumber);
                break;
            case Type::NUL:
                return Json();
                break;
            case Type::STRING:
                return Json(*as.myString);
                break;
            case Type::OBJECT:{
                Json::object object;
                for (auto& element : *as.myStruct) {
                    object[element.first] = element.second.toJson();
                }
                return Json(object);
                break;
            }
            default:
                return Json();
            }
        }

        inline const Value& operator[](Array::size_type i) const;
        inline const Value& operator[](const Struct::key_type& key) const;

    private:
        void Reset() {
            switch (myType) {
            case Type::ARRAY:
                delete as.myArray;
                break;
            case Type::BINARY:
            case Type::STRING:
                delete as.myString;
                break;
            case Type::OBJECT:
                delete as.myStruct;
                break;
            case Type::BOOLEAN:
            case Type::NUMBER:
            case Type::NUL:
                break;
            }

            myType = Type::NUL;
        }

        Type myType;
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
    };

    template<> inline const Value::Array& Value::AsType<typename Value::Array>() const {
        return AsArray();
    }

    template<> inline const bool& Value::AsType<bool>() const {
        return AsBoolean();
    }

    template<> inline const double& Value::AsType<double>() const {
        return AsNumber();
    }

    template<> inline const Value::String& Value::AsType<typename Value::String>() const {
        return AsString();
    }

    template<> inline const Value::Struct& Value::AsType<typename Value::Struct>() const {
        return AsStruct();
    }

    template<> inline const Value& Value::AsType<Value>() const {
        return *this;
    }

    inline const Value& Value::operator[](Array::size_type i) const {
        return AsArray().at(i);
    };

    inline const Value& Value::operator[](const Struct::key_type& key) const {
        return AsStruct().at(key);
    }

    std::string toString(const Value& value) {
        std::string ret;
        switch (value.GetType()) {
        case Value::Type::ARRAY: {
            ret += "[";
            auto& a = value.AsArray();
            for (auto it = a.begin(); it != a.end(); ++it) {
                if (it != a.begin()) {
                    ret += ", ";
                }
                ret += toString(*it);
            }
            ret += ']';
            break;
        }
        case Value::Type::BINARY:
            ret += util::Base64Encode(value.AsBinary());
            break;
        case Value::Type::BOOLEAN:
            ret += value.AsBoolean();
            break;
        case Value::Type::NUMBER:
            ret += std::to_string(value.AsNumber());
            break;
        case Value::Type::NUL:
            ret += "<nil>";
            break;
        case Value::Type::STRING:
            ret += '"' + value.AsString() + '"';
            break;
        case Value::Type::OBJECT: {
            ret += '{';
            auto& s = value.AsStruct();
            for (auto it = s.begin(); it != s.end(); ++it) {
                if (it != s.begin()) {
                    ret += ", ";
                }
                ret += toString(it->first) + ": " + toString(it->second);
            }
            ret += '}';
            break;
        }
        }
        return ret;
    }

} // namespace jsonrpc

#endif // JSONRPC_LEAN_VALUE_H
