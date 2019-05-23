// This file is derived from xsonrpc Copyright (C) 2015 Erik Johansson <erik@ejohansson.se>
// This file is part of jsonrpc-lean, a c++11 JSON-RPC client/server library.
//
// Modifications and additions for jsonrpc-lean Copyright (C) 2015 Adriano Maia <tony@stark.im>
//

#ifndef JSONRPC_LEAN_REQUEST_H
#define JSONRPC_LEAN_REQUEST_H

#include "json.h"

#include <deque>
#include <string>

namespace jsonrpc {

    class Writer;

    class Request {
    public:
        typedef std::deque<Json> Parameters;

        Request(std::string methodName, Parameters parameters, Json id)
            : myMethodName(std::move(methodName)),
            myParameters(std::move(parameters)),
            myId(std::move(id)) {
            // Empty
        }

        const std::string& GetMethodName() const { return myMethodName; }
        const Parameters& GetParameters() const { return myParameters; }
        const Json& GetId() const { return myId; }

        std::string Write() const {
            return Write(myMethodName, myParameters, myId);
        }

        static std::string Write(const std::string& methodName, const Parameters& params, const Json& id) {
        Json::object RequestJson;
        RequestJson[json::JSONRPC_NAME] = json::JSONRPC_VERSION_2_0;
        RequestJson[json::METHOD_NAME] = methodName;
        RequestJson[json::ID_NAME] = id;

        Json::array array;
        for (auto& param : params) {
            array.push_back(param);
        }
        RequestJson[json::PARAMS_NAME] = Json(array);

        return Json(RequestJson).dump();
        }

    private:
        std::string myMethodName;
        Parameters myParameters;
        Json myId;
    };

} // namespace jsonrpc

#endif //JSONRPC_LEAN_REQUEST_H
