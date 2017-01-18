/*
 * test_server.cpp
 *
 *  Created on: Jan 18, 2017
 *      Author: jsiloto
 */




#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <array>
#include <cstring>
#include <functional>
#include <numeric>
#include "jsonrpc-lean/server.h"

using testing::_;
using testing::Args;
using testing::ElementsAreArray;
using testing::Invoke;


class Math {
public:
    double Add(double a, double b) {
        return a + b;
    }

    double AddArray(const Json::array& a) {
        return std::accumulate(a.begin(), a.end(), double(0),
            [](const double& a, const Json& b) { return a + b.number_value(); });
    };
};

std::string Concat(const std::string& a, const std::string& b) {
    return a + b;
}

std::string FromBinary(const Json& b) {
    return{ b.string_value().begin(), b.string_value().end() };
}

Json::object ToStruct(const Json::array& a) {
    Json::object s;
    for (size_t i = 0; i < a.size(); ++i) {
        s[std::to_string(i)] = Json(a[i]);
    }
    return s;
}

void PrintNotification(const std::string& a) {
    printf("notification %s\n", a.c_str());
}

class JsonRpcTest : public ::testing::Test {
 protected:
    jsonrpc::Server server;

    std::string addRequest = "{\"jsonrpc\":\"2.0\",\"method\":\"add\",\"id\":0,\"params\":[3,2]}";
    std::string concatRequest = "{\"jsonrpc\":\"2.0\",\"method\":\"concat\",\"id\":1,\"params\":[\"Hello, \",\"World!\"]}";
    std::string addArrayRequest = "{\"jsonrpc\":\"2.0\",\"method\":\"add_array\",\"id\":2,\"params\":[[1000,2147483647]]}";
    std::string toStructRequest = "{\"jsonrpc\":\"2.0\",\"method\":\"to_struct\",\"id\":4,\"params\":[[12,\"foobar\",[12,\"foobar\"]]]}";
    std::string printNotificationRequest = "{\"jsonrpc\":\"2.0\",\"method\":\"print_notification\",\"params\":[\"This is just a notification, no response expected!\"]}";

};

/// @test Test AD7490::RunRuntimeError.
TEST_F(JsonRpcTest, Invoke) {

    auto& dispatcher = server.GetDispatcher();
    Math math;

    // if it is a member method, you must use this 3 parameter version, passing an instance of an object that implements it
    dispatcher.AddMethod("add", &Math::Add, math);
    dispatcher.AddMethod("add_array", &Math::AddArray, math);

    // if it is just a regular function (non-member or static), you can you the 2 parameter AddMethod
    dispatcher.AddMethod("concat", &Concat);
    dispatcher.AddMethod("from_binary", &FromBinary);
    dispatcher.AddMethod("to_struct", &ToStruct);
    dispatcher.AddMethod("print_notification", &PrintNotification);

    dispatcher.GetMethod("add").SetHelpText("Add two integers").AddSignature(
            Json::Type::NUMBER, Json::Type::NUMBER, Json::Type::NUMBER);

    std::string outputFormatedData;
    printf("request %s\n", addRequest.c_str());
    outputFormatedData = server.HandleRequest(addRequest.c_str());
    printf("response: %s\n\n", outputFormatedData.c_str());

    printf("request %s\n", concatRequest.c_str());
    outputFormatedData = server.HandleRequest(concatRequest.c_str());
    printf("response: %s\n\n", outputFormatedData.c_str());

    printf("request %s\n", addArrayRequest.c_str());
    outputFormatedData = server.HandleRequest(addArrayRequest.c_str());
    printf("response: %s\n\n", outputFormatedData.c_str());

    printf("request %s\n", toStructRequest.c_str());
    outputFormatedData = server.HandleRequest(toStructRequest.c_str());
    printf("response: %s\n\n", outputFormatedData.c_str());

    printf("request %s\n", printNotificationRequest.c_str());
    outputFormatedData = server.HandleRequest(printNotificationRequest.c_str());
    printf("response size: %d\n\n", (int)outputFormatedData.size());
}

