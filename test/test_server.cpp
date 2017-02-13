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
#include <tuple>
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

MethodsMock GlobalMock;

std::string StaticConcat(const std::string& a, const std::string& b){
    return GlobalMock.Concat(a, b);
}
Json::object StaticToObject (const Json::array& a){
    return GlobalMock.ToObject(a);
}

void Error(){
    throw jsonrpc::ServerErrorFault(jsonrpc::Fault::SERVER_ERROR_CODE_MIN, "Server Error");
}

void PrintNotification(const std::string& a) {
    printf("notification %s\n", a.c_str());
}


jsonrpc::Server server;
jsonrpc::Dispatcher& dispatcher = server.GetDispatcher();
void initialize(){
    // Workaround for bug in TestWithParam diamond inheritance
    static bool init = false;
    if(!init){
        // if it is a member method, you must use this 3 parameter version, passing an instance of an object that implements it
        dispatcher.AddMethod("add", &MethodsMock::Add, GlobalMock);
        dispatcher.AddAlias("add", "add_alias", 3);

        dispatcher.AddMethod("add_array", &MethodsMock::AddArray, GlobalMock);

        // if it is just a regular function (non-member or static), you can you the 2 parameter AddMethod
        dispatcher.AddMethod("concat", &StaticConcat);
        dispatcher.AddAlias("concat", "concat_alias", "Hello ");


        dispatcher.AddMethod("to_object", &StaticToObject);
        dispatcher.AddMethod("error", &Error);
        dispatcher.AddMethod("print_notification", &PrintNotification);
    }
    init = true;
}



class JsonRpcTest: public ::testing::Test {
public:
    JsonRpcTest(){
        initialize();
    }

protected:

    Methods methods;

    std::string response;
    std::string expectedResponse;
    std::string addRequest = "{\"jsonrpc\":\"2.0\",\"method\":\"add\",\"id\":0,\"params\":[3,2]}";
    std::string concatRequest = "{\"jsonrpc\":\"2.0\",\"method\":\"concat\",\"id\":1,\"params\":[\"Hello, \",\"World!\"]}";
    std::string addArrayRequest = "{\"jsonrpc\":\"2.0\",\"method\":\"add_array\",\"id\":\"2\",\"params\":[[1111,2222]]}";
    std::string toObjectRequest = "{\"jsonrpc\":\"2.0\",\"method\":\"to_object\",\"id\":4,\"params\":[[12,\"foobar\",[12,\"foobar\"]]]}";
    std::string printNotificationRequest = "{\"jsonrpc\":\"2.0\",\"method\":\"print_notification\",\"params\":[\"This is just a notification, no response expected!\"]}";


    std::string addAliasRequest = "{\"jsonrpc\":\"2.0\",\"method\":\"add_alias\",\"id\":0,\"params\":[2]}";
    std::string concatAliasRequest = "{\"jsonrpc\":\"2.0\",\"method\":\"concat_alias\",\"id\":1,\"params\":[\"World!\"]}";

};


/// @test
TEST_F(JsonRpcTest, AddMethodTwice) {
    jsonrpc::Server server2;
    server2.GetDispatcher().AddMethod("concat", &StaticConcat);
    EXPECT_THROW(server2.GetDispatcher().AddMethod("concat", &StaticConcat), std::invalid_argument);
}


/// @test
TEST_F(JsonRpcTest, InvokeMethod) {
    expectedResponse = "{\"id\": 0, \"jsonrpc\": \"2.0\", \"result\": 5}";
    EXPECT_CALL(GlobalMock, Add(3, 2)).WillOnce(Return(5));
    response = server.HandleRequest(addRequest);
    printf("request %s\n", addRequest.c_str());
    printf("response: %s\n\n", response.c_str());

    EXPECT_CALL(GlobalMock, AddArray(Json::array {1111, 2222})).WillOnce(Return(3333));
    response = server.HandleRequest(addArrayRequest);
    printf("request %s\n", addArrayRequest.c_str());
    printf("response: %s\n\n", response.c_str());


    response = server.HandleRequest(printNotificationRequest);
    printf("request %s\n", printNotificationRequest.c_str());
    printf("response: %s\n\n", response.c_str());
}

/// @test
TEST_F(JsonRpcTest, InvokeStatic) {

    // Concat
    EXPECT_CALL(GlobalMock, Concat("Hello, ", "World!")).WillOnce(Return("Hello, World!"));
    response = server.HandleRequest(concatRequest);
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
    response = server.HandleRequest(toObjectRequest);
    EXPECT_EQ(methods.ToObject(array), object);
    expectedResponse = "{\"id\": 4, \"jsonrpc\": \"2.0\", \"result\": " + r + "}";
    EXPECT_EQ(response, expectedResponse);
    printf("request %s\n", toObjectRequest.c_str());
    printf("response: %s\n\n", response.c_str());
}

/// @test
TEST_F(JsonRpcTest, InvokeAlias) {
    expectedResponse = "{\"id\": 0, \"jsonrpc\": \"2.0\", \"result\": 5}";
    EXPECT_CALL(GlobalMock, Add(3, 2)).WillOnce(Return(5));
    response = server.HandleRequest(addAliasRequest);

    // Concat
    expectedResponse = "{\"id\": 1, \"jsonrpc\": \"2.0\", \"result\": \"Hello, World!\"}";
    EXPECT_CALL(GlobalMock, Concat("Hello, ", "World!")).WillOnce(Return("Hello, World!"));
    response = server.HandleRequest(concatRequest);
    EXPECT_EQ(methods.Concat("Hello, ", "World!"), "Hello, World!");
    EXPECT_EQ(response, expectedResponse);
}



class JsonRpcErrorTest: public ::testing::TestWithParam<
        std::tr1::tuple<std::string, std::string, jsonrpc::Fault::ReservedCodes>> {

};

INSTANTIATE_TEST_CASE_P(Errors, JsonRpcErrorTest, ::testing::Values(
        std::make_tuple("[3,2]", "Invalid request", jsonrpc::Fault::INVALID_REQUEST),
        std::make_tuple("{\"parse error",
                "Parse error: unexpected end of input in string",
                jsonrpc::Fault::PARSE_ERROR),
        std::make_tuple("{\"jsonrpc\":\"2.0\",\"params\":[3] }",
                "Invalid request",
                jsonrpc::Fault::INVALID_REQUEST),
        std::make_tuple("{\"jsonrpc\":\"2.0\",\"method\":\"test\", \"params\": 3  }",
                "Invalid request",
                jsonrpc::Fault::INVALID_REQUEST),
        std::make_tuple("{\"jsonrpc\":\"2.0\",\"method\":\"test\", \"params\": [3], \"id\": 3  }",
                "Method not found: test",
                jsonrpc::Fault::METHOD_NOT_FOUND),
        std::make_tuple("{\"jsonrpc\":\"2.0\",\"method\":\"error\", \"params\": [], \"id\": 3  }",
                "Server Error",
                jsonrpc::Fault::SERVER_ERROR_CODE_MIN),
        std::make_tuple("{\"jsonrpc\":\"1.0\",\"method\":\"error\", \"params\": [], \"id\": 3  }",
                "Invalid request",
                jsonrpc::Fault::INVALID_REQUEST),
        std::make_tuple("{\"jsonrpc\":\"2.0\",\"method\":\"error\", \"params\": [], \"id\": [3, 3]  }",
                "Invalid request",
                jsonrpc::Fault::INVALID_REQUEST),
        std::make_tuple("{\"jsonrpc\":\"2.0\",\"method\":\"concat\",\"id\":1,\"params\":[\"Hello, \"]}",
                "Invalid parameters",
                jsonrpc::Fault::INVALID_PARAMETERS)
));

TEST_P(JsonRpcErrorTest, BadInvoke) {

    std::string err;
    auto request = std::get<0>(GetParam());
    auto response = std::get<1>(GetParam());
    auto code =  std::get<2>(GetParam());

    auto response_str = server.HandleRequest(request);
    printf("response: %s\n", response_str.c_str());
    Json error = Json::parse(response_str, err)["error"];
    EXPECT_TRUE(err.empty());
    EXPECT_EQ(error["code"].number_value(), code);
    EXPECT_EQ(error["message"].string_value(), response);
}

