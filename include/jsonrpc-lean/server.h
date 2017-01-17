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

#ifndef JSONRPC_LEAN_SERVER_H
#define JSONRPC_LEAN_SERVER_H

#include "request.h"
#include "value.h"
#include "fault.h"
#include "response.h"
#include "dispatcher.h"
#include "jsonreader.h"


#include <string>

namespace jsonrpc {

    class Server {
    public:
        Server() {}
        ~Server() {}

        Server(const Server&) = delete;
        Server& operator=(const Server&) = delete;
        Server(Server&&) = delete;
        Server& operator=(Server&&) = delete;

        Dispatcher& GetDispatcher() { return myDispatcher; }

        // If aRequestData is a Notification (the client doesn't expect a response), the returned FormattedData will have an empty ->GetData() buffer and ->GetSize() will be 0
        std::string HandleRequest(const std::string& aRequestData) {
            Json responseJson;

            try {
                auto reader = JsonReader(aRequestData);
                Request request = reader.GetRequest();

                auto response = myDispatcher.Invoke(request.GetMethodName(), request.GetParameters(), request.GetId());
                if (!response.GetId().IsBoolean() || response.GetId().AsBoolean() != false) {
                    // if Id is false, this is a notification and we don't have to write a response
                    responseJson = response.Write();
                }
            } catch (const Fault& ex) {
                responseJson = Response(ex.GetCode(), ex.GetString(), Value()).Write();
            }

            return std::move(responseJson.dump());
        }
    private:
        Dispatcher myDispatcher;
    };

} // namespace jsonrpc

#endif // JSONRPC_LEAN_SERVER_H
