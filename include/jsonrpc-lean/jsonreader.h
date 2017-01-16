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

#ifndef JSONRPC_LEAN_JSONREADER_H
#define JSONRPC_LEAN_JSONREADER_H

#include "reader.h"
#include "fault.h"
#include "json.h"
#include "request.h"
#include "response.h"
#include "util.h"
#include "value.h"
#include <utility>
#include <string>

#define RAPIDJSON_NO_SIZETYPEDEFINE
namespace rapidjson { typedef ::std::size_t SizeType; }

#include "../json11.hpp"
using Json=json11::Json;

namespace jsonrpc {

    class JsonReader final : public Reader {
    public:
        JsonReader(const std::string& data) {

            std::string err;
            printf("\ndata: %s", data.c_str());
            myDocument = Json::parse(data, err);

            printf("\njson: %s", myDocument.dump().c_str());

            // TODO(jsiloto): Add exception
            if (!err.empty()) {
                throw ParseErrorFault(
                    "Parse error: " + err);
            }
        }

        // Reader
        Request GetRequest() override {
            if (!myDocument.is_object()) {
                throw InvalidRequestFault();
            }

            ValidateJsonrpcVersion();

            auto method = myDocument[json::METHOD_NAME];
            if (method == Json() || !method.is_string()) {
                throw InvalidRequestFault();
            }

            Request::Parameters parameters;
            auto params = myDocument[json::PARAMS_NAME];
            if (params != Json()) {
                if (!params.is_array()) {
                    throw InvalidRequestFault();
                }

                for (auto& param: params.object_items()) {
                	std::string str;
                	param.second.dump(str);
                	printf("str: %s", str.c_str());
                    parameters.push_back(GetValue(param.second));
                }
            }

            auto id = myDocument[json::ID_NAME];
            if (id == Json()) {
                // Notification
                return Request(method.string_value(), std::move(parameters), false);
            }

            return Request(method.string_value(), std::move(parameters),
                GetId(id));
        }

        Response GetResponse() override {


            if (!myDocument.is_object()) {
                throw InvalidRequestFault();
            }

            ValidateJsonrpcVersion();



            auto id = myDocument[json::ID_NAME];
            if (id == Json()) {
                throw InvalidRequestFault();
            }


            auto result = myDocument[json::RESULT_NAME];
            auto error = myDocument[json::ERROR_NAME];



            if (result != Json()) {
                if (error != Json()) {
                    throw InvalidRequestFault();
                }
                return Response(GetValue(result), GetId(id));
            } else if (error != Json()) {
                if (result != Json()) {
                    throw InvalidRequestFault();
                }
                if (!error.is_object()) {
                    throw InvalidRequestFault();
                }
                auto code = error[json::ERROR_CODE_NAME];
                if (code == Json() || !code.is_number()) {
                    throw InvalidRequestFault();
                }
                auto message = error[json::ERROR_MESSAGE_NAME];
                if (message == Json() || !message.is_string()) {
                    throw InvalidRequestFault();
                }

                return Response(code.number_value(), message.string_value(),
                    GetId(id));
            } else {
                throw InvalidRequestFault();
            }
        }

        Value GetValue() override {
            return GetValue(myDocument);
        }

    private:
        void ValidateJsonrpcVersion() const {
            auto jsonrpc = myDocument[json::JSONRPC_NAME];
            if (jsonrpc == Json()
                || !jsonrpc.is_string()
                || jsonrpc.string_value() != json::JSONRPC_VERSION_2_0) {
                throw InvalidRequestFault();
            }
        }

        Value GetValue(const Json& value) const {
            switch (value.type()) {
            case Json::NUL:
                return Value();
            case Json::BOOL:
                return Value(value.bool_value());
            case Json::OBJECT: {
                Value::Struct data;
                for (auto& it: value.object_items()) {
                    data.emplace(it.first, GetValue(it.second));
                }
                return Value(std::move(data));
            }
            case Json::ARRAY: {
                Value::Array array;
                array.reserve(value.array_items().size());
                for (auto& it: value.array_items()) {
                    array.emplace_back(GetValue(it));
                }
                return Value(std::move(array));
            }
            case Json::STRING: {
                std::string str = value.string_value();
                const bool binary = str.find('\0') != std::string::npos;
                return Value(std::move(str), binary);
            }
            case Json::NUMBER:
                return Value(value.number_value());
                break;
            }

            throw InternalErrorFault();
        }

        Value GetId(const Json& id) const {
            if (id.is_string()) {
                return id.string_value();
            } else if (id.is_number()) {
                return id.number_value();
            } else if (id.is_null()) {
                return{};
            }

            throw InvalidRequestFault();
        }

        std::string myData;
        //rapidjson::Document myDocument;
        json11::Json myDocument;
    };

} // namespace jsonrpc

#endif // JSONRPC_LEAN_JSONREADER_H
