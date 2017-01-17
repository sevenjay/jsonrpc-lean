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

#include "../include/jsonrpc-lean/jsonformathandler.h"
#include "../include/jsonrpc-lean/formathandler.h"
#include "../include/jsonrpc-lean/server.h"

#include <numeric>
#include <string>
#include <memory>
#include <stdint.h>

class Math {
public:
	double Add(double a, double b) {
		return a + b;
	}

	double AddArray(const jsonrpc::Value::Array& a) {
		return std::accumulate(a.begin(), a.end(), double(0),
			[](const double& a, const jsonrpc::Value& b) { return a + b.AsNumber(); });
	};
};

std::string Concat(const std::string& a, const std::string& b) {
	return a + b;
}

jsonrpc::Value ToBinary(const std::string& s) {
	return jsonrpc::Value(s, true);
}

std::string FromBinary(const jsonrpc::Value& b) {
	return{ b.AsBinary().begin(), b.AsBinary().end() };
}

jsonrpc::Value::Struct ToStruct(const jsonrpc::Value::Array& a) {
	jsonrpc::Value::Struct s;
	for (size_t i = 0; i < a.size(); ++i) {
		s[std::to_string(i)] = jsonrpc::Value(a[i]);
	}
	return s;
}

void PrintNotification(const std::string& a) {
    printf("notification %s\n", a.c_str());
}

void RunServer() {
	Math math;
	jsonrpc::Server server;
	
	jsonrpc::JsonFormatHandler jsonFormatHandler;
	server.RegisterFormatHandler(jsonFormatHandler);

	auto& dispatcher = server.GetDispatcher();
	// if it is a member method, you must use this 3 parameter version, passing an instance of an object that implements it
	dispatcher.AddMethod("add", &Math::Add, math);
	dispatcher.AddMethod("add_array", &Math::AddArray, math); 
	
	// if it is just a regular function (non-member or static), you can you the 2 parameter AddMethod
	dispatcher.AddMethod("concat", &Concat);
	dispatcher.AddMethod("to_binary", &ToBinary);
	dispatcher.AddMethod("from_binary", &FromBinary);
	dispatcher.AddMethod("to_struct", &ToStruct);
	dispatcher.AddMethod("print_notification", &PrintNotification);

	dispatcher.GetMethod("add")
		.SetHelpText("Add two integers")
		.AddSignature(jsonrpc::Value::Type::INTEGER_32, jsonrpc::Value::Type::INTEGER_32, jsonrpc::Value::Type::INTEGER_32);

	//bool run = true;
	//dispatcher.AddMethod("exit", [&]() { run = false; }).SetHidden();
	
	const char addRequest[] = "{\"jsonrpc\":\"2.0\",\"method\":\"add\",\"id\":0,\"params\":[3,2]}";
	const char concatRequest[] = "{\"jsonrpc\":\"2.0\",\"method\":\"concat\",\"id\":1,\"params\":[\"Hello, \",\"World!\"]}";
	const char addArrayRequest[] = "{\"jsonrpc\":\"2.0\",\"method\":\"add_array\",\"id\":2,\"params\":[[1000,2147483647]]}";
	const char toBinaryRequest[] = "{\"jsonrpc\":\"2.0\",\"method\":\"to_binary\",\"id\":3,\"params\":[\"Hello World!\"]}";
	const char toStructRequest[] = "{\"jsonrpc\":\"2.0\",\"method\":\"to_struct\",\"id\":4,\"params\":[[12,\"foobar\",[12,\"foobar\"]]]}";
	const char printNotificationRequest[] = "{\"jsonrpc\":\"2.0\",\"method\":\"print_notification\",\"params\":[\"This is just a notification, no response expected!\"]}";

	std::string outputFormatedData;
    printf("request %s\n", addRequest);
    outputFormatedData = server.HandleRequest(addRequest);
    printf("response: %s\n\n", outputFormatedData.c_str());

    printf("request %s\n", concatRequest);
    outputFormatedData = server.HandleRequest(concatRequest);
    printf("response: %s\n\n", outputFormatedData.c_str());

    printf("request %s\n", addArrayRequest);
    outputFormatedData = server.HandleRequest(addArrayRequest);
    printf("response: %s\n\n", outputFormatedData.c_str());

    printf("request %s\n", toBinaryRequest);
    outputFormatedData = server.HandleRequest(toBinaryRequest);
    printf("response: %s\n\n", outputFormatedData.c_str());

    printf("request %s\n", toStructRequest);
    outputFormatedData = server.HandleRequest(toStructRequest);
    printf("response: %s\n\n", outputFormatedData.c_str());

    printf("request %s\n", printNotificationRequest);
    outputFormatedData = server.HandleRequest(printNotificationRequest);
    printf("response size: %d\n\n", (int)outputFormatedData.size());
}

int main() {
	try {
		RunServer();
	} catch (const std::exception& ex) {
	    printf("Error %s\n", ex.what());
		return 1;
	}

	return 0;
}
