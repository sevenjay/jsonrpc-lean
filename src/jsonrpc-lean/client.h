// This file is derived from xsonrpc Copyright (C) 2015 Erik Johansson <erik@ejohansson.se>
// This file is part of jsonrpc-lean, a c++11 JSON-RPC client/server library.
//
// Modifications and additions for jsonrpc-lean Copyright (C) 2015 Adriano Maia <tony@stark.im>
//

#ifndef JSONRPC_LEAN_CLIENT_H
#define JSONRPC_LEAN_CLIENT_H

#include "request.h"
#include "value.h"
#include "fault.h"
#include "jsonreader.h"
#include "response.h"
#include "dispatcher.h"

#include <functional>
#include <string>
#include <memory>
#include <stdexcept>

namespace jsonrpc {

    class FormatHandler;

    class Client {
    public:
        Client() : myId(0) {

        }

        ~Client() {}

        std::string BuildRequestData(const std::string& methodName, const Request::Parameters& params = {}) {
            return BuildRequestDataInternal(methodName, params);
        }

        template<typename FirstType, typename... RestTypes>
        typename std::enable_if<!std::is_same<typename std::decay<FirstType>::type, Request::Parameters>::value, std::string>::type
        BuildRequestData(const std::string& methodName, FirstType&& first, RestTypes&&... rest) {
            Request::Parameters params;
            params.emplace_back(std::forward<FirstType>(first));

            return BuildRequestDataInternal(methodName, params, std::forward<RestTypes>(rest)...);
        }

        std::string BuildNotificationData(const std::string& methodName, const Request::Parameters& params = {}) {
            return BuildNotificationDataInternal(methodName, params);
        }

        template<typename FirstType, typename... RestTypes>
        typename std::enable_if<!std::is_same<typename std::decay<FirstType>::type, Request::Parameters>::value, std::string>::type
        BuildNotificationData(const std::string& methodName, FirstType&& first, RestTypes&&... rest) {
            Request::Parameters params;
            params.emplace_back(std::forward<FirstType>(first));

            return BuildNotificationDataInternal(methodName, params, std::forward<RestTypes>(rest)...);
        }

        Response ParseResponse(const std::string& aResponseData) {
            return ParseResponseInternal(aResponseData);
        }

        Client(const Client&) = delete;
        Client& operator=(const Client&) = delete;
        Client(Client&&) = delete;
        Client& operator=(Client&&) = delete;

    private:
        template<typename FirstType, typename... RestTypes>
        std::string BuildRequestDataInternal(const std::string& methodName, Request::Parameters& params, FirstType&& first, RestTypes&&... rest) {
            params.emplace_back(std::forward<FirstType>(first));
            return BuildRequestDataInternal(methodName, params, std::forward<RestTypes>(rest)...);
        }

        std::string BuildRequestDataInternal(const std::string& methodName, const Request::Parameters& params) {
            const auto id = myId++;
            return Request::Write(methodName, params, id);
        }

        template<typename FirstType, typename... RestTypes>
        std::string BuildNotificationDataInternal(const std::string& methodName, Request::Parameters& params, FirstType&& first, RestTypes&&... rest) {
            params.emplace_back(std::forward<FirstType>(first));
            return BuildNotificationDataInternal(methodName, params, std::forward<RestTypes>(rest)...);
        }

        std::string BuildNotificationDataInternal(const std::string& methodName, const Request::Parameters& params) {
            return Request::Write(methodName, params, false);
        }

        Response ParseResponseInternal(const std::string& aResponseData) {
            auto reader = JsonReader(aResponseData);
            Response response = reader.GetResponse();
            response.ThrowIfFault();
            return std::move(response);
        }

        int32_t myId;
    };

} // namespace jsonrpc

#endif // JSONRPC_LEAN_CLIENT_H
