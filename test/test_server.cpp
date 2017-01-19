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
using testing::Return;

class Methods {
public:
    double Add(double a, double b) {
        return a + b;
    }

    double AddArray(const Json::array& a) {
        return std::accumulate(a.begin(), a.end(), double(0),
            [](const double& a, const Json& b) { return a + b.number_value(); });
    };

    std::string Concat(const std::string& a, const std::string& b) {
        return a + b;
    }

    std::string FromBinary(const Json& b) {
        return{ b.string_value().begin(), b.string_value().end() };
    }

    Json::object ToObject(const Json::array& a) {
        Json::object s;
        for (size_t i = 0; i < a.size(); ++i) {
            s[std::to_string(i)] = Json(a[i]);
        }
        return s;
    }
};

class MethodsMock : public Methods {
 public:
  MOCK_METHOD2(Add, double(double a, double b));
  MOCK_METHOD1(AddArray, double(const Json::array& a));
  MOCK_METHOD2(Concat, std::string(const std::string& a, const std::string& b));
  MOCK_METHOD1(ToObject, Json::object(const Json::array& a));
};


void PrintNotification(const std::string& a) {
    printf("notification %s\n", a.c_str());
}

MethodsMock GlobalMock;

std::string StaticConcat(const std::string& a, const std::string& b){
    return GlobalMock.Concat(a, b);
}
Json::object StaticToObject (const Json::array& a){
    return GlobalMock.ToObject(a);
}

class JsonRpcTest: public ::testing::Test {

public:

protected:
    void CheckExpectedResponse(std::string r){
        EXPECT_EQ(r, expectedResponse);
        printf("response: %s\n\n", r.c_str());
    }

    jsonrpc::Server server;
    Methods methods;

    std::string response;
    std::string expectedResponse;

    std::string addRequest = "{\"jsonrpc\":\"2.0\",\"method\":\"add\",\"id\":0,\"params\":[3,2]}";
    std::string concatRequest = "{\"jsonrpc\":\"2.0\",\"method\":\"concat\",\"id\":1,\"params\":[\"Hello, \",\"World!\"]}";
    std::string addArrayRequest = "{\"jsonrpc\":\"2.0\",\"method\":\"add_array\",\"id\":2,\"params\":[[1111,2222]]}";
    std::string toObjectRequest = "{\"jsonrpc\":\"2.0\",\"method\":\"to_object\",\"id\":4,\"params\":[[12,\"foobar\",[12,\"foobar\"]]]}";
    std::string printNotificationRequest = "{\"jsonrpc\":\"2.0\",\"method\":\"print_notification\",\"params\":[\"This is just a notification, no response expected!\"]}";

};


/// @test
TEST_F(JsonRpcTest, InvokeMethod) {
    auto& dispatcher = server.GetDispatcher();
    // if it is a member method, you must use this 3 parameter version, passing an instance of an object that implements it
    dispatcher.AddMethod("add", &MethodsMock::Add, GlobalMock);
    dispatcher.AddMethod("add_array", &MethodsMock::AddArray, GlobalMock);

    expectedResponse = "{\"id\": 0, \"jsonrpc\": \"2.0\", \"result\": 5}";
    EXPECT_CALL(GlobalMock, Add(3, 2)).WillOnce(Return(5));
    response = server.HandleRequest(addRequest.c_str());
    printf("request %s\n", addRequest.c_str());
    printf("response: %s\n\n", response.c_str());

    EXPECT_CALL(GlobalMock, AddArray(Json::array {1111, 2222})).WillOnce(Return(3333));
    response = server.HandleRequest(addArrayRequest.c_str());
    printf("request %s\n", addArrayRequest.c_str());
    printf("response: %s\n\n", response.c_str());
}

/// @test
TEST_F(JsonRpcTest, InvokeStatic) {
    auto& dispatcher = server.GetDispatcher();
    // if it is just a regular function (non-member or static), you can you the 2 parameter AddMethod
    dispatcher.AddMethod("concat", &StaticConcat);
    dispatcher.AddMethod("to_object", &StaticToObject);

    // Concat
    EXPECT_CALL(GlobalMock, Concat("Hello, ", "World!")).WillOnce(Return("Hello, World!"));
    response = server.HandleRequest(concatRequest.c_str());
    EXPECT_EQ(methods.Concat("Hello, ", "World!"), "Hello, World!");
    expectedResponse = "{\"id\": 1, \"jsonrpc\": \"2.0\", \"result\": \"Hello, World!\"}";
    EXPECT_EQ(response, expectedResponse);
    printf("request %s\n", concatRequest.c_str());
    printf("response: %s\n\n", response.c_str());


    // To Object
    std::string err;
    std::string a = "[12,\"foobar\",[12,\"foobar\"]]";
    auto array = Json::parse(a, err).array_items();
    EXPECT_TRUE(err.empty());

    std::string r = "{\"0\": 12, \"1\": \"foobar\", \"2\": [12, \"foobar\"]}";
    auto object =  Json::parse(r, err).object_items();
    EXPECT_TRUE(err.empty());

    EXPECT_CALL(GlobalMock, ToObject(array)).WillOnce(Return(object));
    response = server.HandleRequest(toObjectRequest.c_str());
    EXPECT_EQ(methods.ToObject(array), object);
    expectedResponse = "{\"id\": 4, \"jsonrpc\": \"2.0\", \"result\": " + r + "}";
    EXPECT_EQ(response, expectedResponse);
    printf("request %s\n", toObjectRequest.c_str());
    printf("response: %s\n\n", response.c_str());
}

