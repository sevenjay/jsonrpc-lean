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

#include "../include/jsonrpc-lean/client.h"
#include "../include/jsonrpc-lean/fault.h"

#include <cstring>
#include <limits>
#include <memory>

void LogArguments() {}

template<typename Head>
void LogArguments(Head&& head) {
    printf("%s", jsonrpc::toString(head).c_str());
}

template<typename Head, typename... Tail>
void LogArguments(Head&& head, Tail&&... tail) {
    std::string str = jsonrpc::toString(head) + ", ";
    printf("%s", str.c_str());
    LogArguments(std::forward<Tail>(tail)...);
}

void LogArguments(jsonrpc::Request::Parameters& params) {
    for (auto it = params.begin(); it != params.end(); ++it) {
        if (it != params.begin()) {
            printf(", ");
        }
        printf("%s", it->toJson().dump().c_str());
    }
}

size_t CallErrors = 0;

template<typename... T>
void LogCall(jsonrpc::Client& client, std::string method, T&&... args) {
    printf("%s(", method.c_str());
    LogArguments(std::forward<T>(args)...);
    printf("):\n>>> ");
    try {
        std::string str = client.BuildRequestData(std::move(method), std::forward<T>(args)...);
        printf("%s", str.c_str());
    } catch (const jsonrpc::Fault& fault) {
        ++CallErrors;
        printf("Error: %s", fault.what());
    }
    printf("\n");
}

template<typename... T>
void LogNotificationCall(jsonrpc::Client& client, std::string method, T&&... args) {
    printf("%s(", method.c_str());
    LogArguments(std::forward<T>(args)...);
    printf("):\n>>> ");
    try {
        std::string str = client.BuildNotificationData(std::move(method), std::forward<T>(args)...);
        printf("%s", str.c_str());
    }
    catch (const jsonrpc::Fault& fault) {
        ++CallErrors;
        printf("Error: %s", fault.what());
    }
    printf("\n\n");
}

int main(int argc, char** argv) {

	const char addResponse[] = "{\"jsonrpc\":\"2.0\",\"id\":0,\"result\":5}";
    const char concatResponse[] = "{\"jsonrpc\":\"2.0\",\"id\":1,\"result\":\"Hello, World!\"}";
    const char addArrayResponse[] = "{\"jsonrpc\":\"2.0\",\"id\":2,\"result\":2147484647}";
    const char toStructResponse[] = "{\"jsonrpc\":\"2.0\",\"id\":4,\"result\":{\"0\":12,\"1\":\"foobar\",\"2\":[12,\"foobar\"]}}";	
	

    try {
        jsonrpc::Client client;

        LogCall(client, "add", 3, 2);
		jsonrpc::Response parsedResponse = client.ParseResponse(addResponse);
		printf("Parsed response: %f\n\n", parsedResponse.GetResult().AsNumber());
		
        LogCall(client, "concat", "Hello, ", "World!");
		parsedResponse = client.ParseResponse(concatResponse);
        printf("Parsed response: %s\n\n", toString(parsedResponse.GetResult()).c_str());

        jsonrpc::Request::Parameters params;
        {
            jsonrpc::Value::Array a;
            a.emplace_back(1000);
            a.emplace_back(std::numeric_limits<int32_t>::max());
            params.push_back(std::move(a));
        }
        LogCall(client, "add_array", params);
		parsedResponse = client.ParseResponse(addArrayResponse);
		printf("Parsed response: %s\n\n", toString(parsedResponse.GetResult()).c_str());

        LogCall(client, "to_binary", "Hello World!"); // once the result here is parsed, the underlying AsString can be just an array of bytes, not necessarily printable characters
        LogCall(client, "from_binary", jsonrpc::Value("Hi!")); // "Hi!" can be an array of bytes, not necessarily printable characters
		LogNotificationCall(client, "print_notification", "This is just a notification, no response expected!");

        params.clear();
        {
            jsonrpc::Value::Array a;
            a.emplace_back(12);
            a.emplace_back("foobar");
            a.emplace_back(a);
            params.push_back(std::move(a));
        }
        LogCall(client, "to_struct", params);
		parsedResponse = client.ParseResponse(toStructResponse);
		printf("Parsed response: %s\n\n", toString(parsedResponse.GetResult()).c_str());

        params.clear();
        {
            jsonrpc::Value::Array calls;
            {
                jsonrpc::Value::Struct call;
                call["method"] = jsonrpc::Value("add");
                {
                    jsonrpc::Value::Array params;
                    params.emplace_back(23);
                    params.emplace_back(19);
                    call["params"] = std::move(params);
                }
                calls.emplace_back(std::move(call));
            }
            {
                jsonrpc::Value::Struct call;
                call["method"] = jsonrpc::Value("does.NotExist");
                calls.emplace_back(std::move(call));
            }
            {
                jsonrpc::Value::Struct call;
                call["method"] = jsonrpc::Value("concat");
                {
                    jsonrpc::Value::Array params;
                    params.emplace_back("Hello ");
                    params.emplace_back("multicall!");
                    call["params"] = std::move(params);
                }
                calls.emplace_back(std::move(call));
            }
            params.emplace_back(std::move(calls));
        }

    } catch (const std::exception& ex) {
        printf("Error: %s\n", ex.what());
        return 1;
    }

    if (CallErrors > 0) {
        printf("Error: %d call(s) failed\n", CallErrors);
        return 1;
    }

    return 0;
}
