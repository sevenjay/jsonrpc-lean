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

#ifndef JSONRPC_LEAN_RESPONSE_H
#define JSONRPC_LEAN_RESPONSE_H

#include "json.h"

namespace jsonrpc {

    class Response {
    public:
        Response(Json value, Json id) : myResult(std::move(value)),
            myIsFault(false),
            myFaultCode(0),
            myId(std::move(id)) {
        }

        Response(int32_t faultCode, std::string faultString, Json id) : myIsFault(true),
            myFaultCode(faultCode),
            myFaultString(std::move(faultString)),
            myId(std::move(id)) {
        }
        Response(int32_t faultCode, std::string faultString, std::string faultData, Json id) : myIsFault(true),
            myFaultCode(faultCode),
            myFaultString(std::move(faultString)),
            myFaultData(std::move(faultData)),
            myId(std::move(id)) {
        }

        Json::object ResponseObject() const {
            Json::object ResponseJson;
            ResponseJson[json::JSONRPC_NAME] = json::JSONRPC_VERSION_2_0;
            ResponseJson[json::ID_NAME] = myId;
            return std::move(ResponseJson);
        }

        Json Write() const {
            Json::object ResponseJson = ResponseObject();
            if (myIsFault) {
                Json::object ErrorJson;
                ErrorJson[json::ERROR_CODE_NAME] = myFaultCode;
                ErrorJson[json::ERROR_MESSAGE_NAME] = myFaultString;
                if(!myFaultData.empty()) ErrorJson[json::ERROR_DATA_NAME] = myFaultData;
                ResponseJson[json::ERROR_NAME] = Json(ErrorJson);
            } else {
                ResponseJson[json::ID_NAME] = myId;
                ResponseJson[json::RESULT_NAME] = myResult;
            }
            return Json(ResponseJson);
        }

        Json& GetResult() { return myResult; }
        bool IsFault() const { return myIsFault; }

        void ThrowIfFault() const {
            if (!IsFault()) {
                return;
            }

            switch (static_cast<Fault::ReservedCodes>(myFaultCode)) {
            case Fault::RESERVED_CODE_MIN:
            case Fault::RESERVED_CODE_MAX:
            case Fault::SERVER_ERROR_CODE_MIN:
            case Fault::SERVER_ERROR_CODE_DEFAULT:
                break;
            case Fault::PARSE_ERROR:
                throw ParseErrorFault(myFaultString);
            case Fault::INVALID_REQUEST:
                throw InvalidRequestFault(myFaultString);
            case Fault::METHOD_NOT_FOUND:
                throw MethodNotFoundFault(myFaultString);
            case Fault::INVALID_PARAMETERS:
                throw InvalidParametersFault(myFaultString);
            case Fault::INTERNAL_ERROR:
                throw InternalErrorFault(myFaultString);
            }

            if (myFaultCode >= Fault::SERVER_ERROR_CODE_MIN
                && myFaultCode <= Fault::SERVER_ERROR_CODE_MAX) {
                throw ServerErrorFault(myFaultCode, myFaultString);
            }

            if (myFaultCode >= Fault::RESERVED_CODE_MIN
                && myFaultCode <= Fault::RESERVED_CODE_MAX) {
                throw PreDefinedFault(myFaultCode, myFaultString);
            }

            throw Fault(myFaultString, myFaultCode);
        }

        const Json& GetId() const { return myId; }

    private:
        Json myResult;
        bool myIsFault;
        int myFaultCode;
        std::string myFaultString;
        std::string myFaultData;
        Json myId;
    };

} // namespace jsonrpc

#endif // JSONRPC_LEAN_RESPONSE_H
