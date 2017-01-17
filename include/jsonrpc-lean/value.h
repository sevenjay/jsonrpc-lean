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
            DATE_TIME,
            NUMBER,
            INTEGER_32,
            INTEGER_64,
            NIL,
            STRING,
            STRUCT
        };

        Value() : myType(Type::NIL) {}

        Value(Array value) : myType(Type::ARRAY) {
            as.myArray = new Array(std::move(value));
        }

        Value(bool value) : myType(Type::BOOLEAN) { as.myBoolean = value; }

        Value(const DateTime& value) : myType(Type::DATE_TIME) {
            as.myDateTime = new DateTime(value);
            as.myDateTime->tm_isdst = -1;
        }

        Value(double value) : myType(Type::NUMBER) { as.myNumber = value; }

        Value(int32_t value) : myType(Type::INTEGER_32) {
            as.myInteger32 = value;
            as.myInteger64 = value;
            as.myNumber = value;
        }

        Value(int64_t value) : myType(Type::INTEGER_64) {
            as.myInteger32 = static_cast<int32_t>(value);
            as.myInteger64 = value;
            as.myNumber = static_cast<double>(value);
        }

        Value(const char* value) : Value(String(value)) {}

        Value(String value, bool binary = false) : myType(binary ? Type::BINARY : Type::STRING) {
            as.myString = new String(std::move(value));
        }

        Value(Struct value) : myType(Type::STRUCT) {
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
            case Type::INTEGER_32:
            case Type::INTEGER_64:
            case Type::NIL:
                break;

            case Type::ARRAY:
                as.myArray = new Array(other.AsArray());
                break;
            case Type::DATE_TIME:
                as.myDateTime = new DateTime(other.AsDateTime());
                break;
            case Type::BINARY:
            case Type::STRING:
                as.myString = new String(other.AsString());
                break;
            case Type::STRUCT:
                as.myStruct = new Struct(other.AsStruct());
                break;
            }
        }

        Value& operator=(const Value&) = delete;

        Value(Value&& other) noexcept : myType(other.myType), as(other.as) {
            other.myType = Type::NIL;
        }

        Value& operator=(Value&& other) noexcept {
            if (this != &other) {
                Reset();

                myType = other.myType;
                as = other.as;

                other.myType = Type::NIL;
            }
            return *this;
        }

        bool IsArray() const { return myType == Type::ARRAY; }
        bool IsBinary() const { return myType == Type::BINARY; }
        bool IsBoolean() const { return myType == Type::BOOLEAN; }
        bool IsDateTime() const { return myType == Type::DATE_TIME; }
        bool IsDouble() const { return myType == Type::NUMBER; }
        bool IsInteger32() const { return myType == Type::INTEGER_32; }
        bool IsInteger64() const { return myType == Type::INTEGER_64; }
        bool IsNil() const { return myType == Type::NIL; }
        bool IsString() const { return myType == Type::STRING; }
        bool IsStruct() const { return myType == Type::STRUCT; }

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

        const DateTime& AsDateTime() const {
            if (IsDateTime()) {
                return *as.myDateTime;
            }
            throw InvalidParametersFault();
        }

        const double& AsNumber() const {
            if (IsDouble() || IsInteger32() || IsInteger64()) {
                return as.myNumber;
            }
            throw InvalidParametersFault();
        }

        const int32_t& AsInteger32() const {
            if (IsInteger32()) {
                return as.myInteger32;
            } else if (IsInteger64()
                && static_cast<int64_t>(as.myInteger32) == as.myInteger64) {
                return as.myInteger32;
            }
            throw InvalidParametersFault();
        }

        const int64_t& AsInteger64() const {
            if (IsInteger32() || IsInteger64()) {
                return as.myInteger64;
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
            case Type::DATE_TIME:
                return Json();
                break;
            case Type::NUMBER:
                return Json(as.myNumber);
                break;
            case Type::INTEGER_32:
                return Json(as.myInteger32);
                break;
            case Type::INTEGER_64:
                return Json((int)as.myInteger64);
                break;
            case Type::NIL:
                return Json();
                break;
            case Type::STRING:
                return Json(*as.myString);
                break;
            case Type::STRUCT:{
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
            case Type::DATE_TIME:
                delete as.myDateTime;
                break;
            case Type::BINARY:
            case Type::STRING:
                delete as.myString;
                break;
            case Type::STRUCT:
                delete as.myStruct;
                break;

            case Type::BOOLEAN:
            case Type::NUMBER:
            case Type::INTEGER_32:
            case Type::INTEGER_64:
            case Type::NIL:
                break;
            }

            myType = Type::NIL;
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

    template<> inline const Value::DateTime& Value::AsType<typename Value::DateTime>() const {
        return AsDateTime();
    }

    template<> inline const double& Value::AsType<double>() const {
        return AsNumber();
    }

    template<> inline const int32_t& Value::AsType<int32_t>() const {
        return AsInteger32();
    }

    template<> inline const int64_t& Value::AsType<int64_t>() const {
        return AsInteger64();
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
        case Value::Type::DATE_TIME:
            ret += util::FormatIso8601DateTime(value.AsDateTime());
            break;
        case Value::Type::NUMBER:
            ret += std::to_string(value.AsNumber());
            break;
        case Value::Type::INTEGER_32:
            ret += std::to_string(static_cast<int32_t>(value.AsNumber()));
            break;
        case Value::Type::INTEGER_64:
            ret += std::to_string(static_cast<int64_t>(value.AsNumber()));
            break;
        case Value::Type::NIL:
            ret += "<nil>";
            break;
        case Value::Type::STRING:
            ret += '"' + value.AsString() + '"';
            break;
        case Value::Type::STRUCT: {
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
