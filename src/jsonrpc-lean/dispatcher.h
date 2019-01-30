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

#ifndef JSONRPC_LEAN_DISPATCHER_H
#define JSONRPC_LEAN_DISPATCHER_H

#include "fault.h"
#include "request.h"
#include "response.h"

//#if __cplusplus <= 201103L
#include "integer_seq.h"
//namespace std {
//using redi::index_sequence;
//using redi::index_sequence_for;
//} // namespace std
//#endif

#include <functional>
#include <utility>
#include <vector>

namespace jsonrpc {

    class MethodWrapper {
    public:
        typedef std::function<Json(const Request::Parameters&)> Method;

        explicit MethodWrapper(Method method) : myMethod(method) {}

        MethodWrapper(const MethodWrapper&) = delete;
        MethodWrapper& operator=(const MethodWrapper&) = delete;

        bool IsHidden() const { return myIsHidden; }
        void SetHidden(bool hidden = true) { myIsHidden = hidden; }

        MethodWrapper& SetHelpText(std::string help) {
            myHelpText = std::move(help);
            return *this;
        }

        const std::string& GetHelpText() const { return myHelpText; }

        template<typename... ParameterTypes>
        MethodWrapper& AddSignature(Json::Type returnType, ParameterTypes... parameterTypes) {
            mySignatures.emplace_back(std::initializer_list < Json::Type > {returnType, parameterTypes...});
            return *this;
        }

        const std::vector<std::vector<Json::Type>>&
            GetSignatures() const { return mySignatures; }

        Json operator()(const Request::Parameters& params) const {
            return myMethod(params);
        }

    private:
        Method myMethod;
        bool myIsHidden = false;
        std::string myHelpText;
        std::vector<std::vector<Json::Type>> mySignatures;
    };

    struct AliasWrapper{
      std::string name;
      Request::Parameters parameters;
      // Todo(jsiloto): Add generalize for more parameters
    };

    template<typename> struct ToStdFunction;

    template<typename ReturnType, typename... ParameterTypes>
    struct ToStdFunction < ReturnType(*)(ParameterTypes...) > {
        typedef std::function<ReturnType(ParameterTypes...)> Type;
    };

    template<typename ReturnType, typename T, typename... ParameterTypes>
    struct ToStdFunction < ReturnType(T::*)(ParameterTypes...) > {
        typedef std::function<ReturnType(ParameterTypes...)> Type;
    };

    template<typename ReturnType, typename T, typename... ParameterTypes>
    struct ToStdFunction < ReturnType(T::*)(ParameterTypes...) const > {
        typedef std::function<ReturnType(ParameterTypes...)> Type;
    };

    template<typename MethodType, bool isClass>
    struct StdFunction {};

    template<typename MethodType>
    struct StdFunction < MethodType, false > {
        typedef typename ToStdFunction<MethodType>::Type Type;
    };

    template<typename MethodType>
    struct StdFunction < MethodType, true > {
        typedef typename ToStdFunction <
            decltype(&MethodType::operator()) > ::Type Type;
    };

    class Dispatcher {
    public:
        std::vector<std::string> GetMethodNames(bool includeHidden = false) const {
            std::vector<std::string> names;
            names.reserve(myMethods.size());

            for (auto& method : myMethods) {
                if (includeHidden || !method.second.IsHidden()) {
                    names.emplace_back(method.first);
                }
            }

            return names;
        }

        MethodWrapper& GetMethod(const std::string& name) {
            return myMethods.at(name);
        }

        MethodWrapper& AddMethod(std::string name, MethodWrapper::Method method) {
            auto result = myMethods.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(std::move(name)),
                std::forward_as_tuple(std::move(method)));
            if (!result.second) {
                throw std::invalid_argument(name + ": method already added");
            }
            return result.first->second;
        }

        template<typename MethodType>
        MethodWrapper&
        //typename std::enable_if<!std::is_convertible<MethodType, std::function<Json(const Request::Parameters&)>>::value && !std::is_member_pointer<MethodType>::value, MethodWrapper>::type&
        AddMethod(std::string name, MethodType method) {
            //static_assert(!std::is_bind_expression<MethodType>::value,
            //    "Use AddMethod with 3 arguments to add member method");
            typename StdFunction<MethodType, std::is_class<MethodType>::value>::Type function(std::move(method));
            return AddMethodInternal(std::move(name), std::move(function));
        }

        template<typename T>
        MethodWrapper& AddMethod(std::string name, Json(T::*method)(const Request::Parameters&), T& instance) {
            return AddMethod(std::move(name), std::bind(method, &instance, std::placeholders::_1));
        }

        template<typename T>
        MethodWrapper& AddMethod(std::string name, Json(T::*method)(const Request::Parameters&) const, T& instance) {
            return AddMethod(std::move(name), std::bind(method, &instance, std::placeholders::_1));
        }

        template<typename ReturnType, typename T, typename... ParameterTypes>
        MethodWrapper& AddMethod(std::string name, ReturnType(T::*method)(ParameterTypes...), T& instance) {
            std::function<ReturnType(ParameterTypes...)> function = [&instance, method](ParameterTypes&&... params) -> ReturnType {
                return (instance.*method)(std::forward<ParameterTypes>(params)...);
            };
            return AddMethodInternal(std::move(name), std::move(function));
        }

        template<typename ReturnType, typename T, typename... ParameterTypes>
        MethodWrapper& AddMethod(std::string name, ReturnType(T::*method)(ParameterTypes...) const, T& instance) {
            std::function<ReturnType(ParameterTypes...)> function = [&instance, method](ParameterTypes&&... params) -> ReturnType {
                return (instance.*method)(std::forward<ParameterTypes>(params)...);
            };
            return AddMethodInternal(std::move(name), std::move(function));
        }

        void RemoveMethod(const std::string& name) {
            myMethods.erase(name);
        }

        template<typename... ParameterTypes>
        void AddAlias(const std::string& method, const std::string alias, ParameterTypes... parameters){
          AliasWrapper a = AddAliasInternal(method, parameters...);
          myAliases.emplace(alias, a);
        }



        virtual Response Invoke(std::string name, Request::Parameters parameters, const Json& id) const {
            try {
                // find alias if existing
                auto alias = myAliases.find(name);
                if(alias != myAliases.end()){
                    // resolve alias
                    name = alias->second.name;
                    //parameters.push_front(alias->second.parameters.front());
                    // concatenate parameters
                    auto pars = alias->second.parameters;
                    while(!pars.empty()){
                        parameters.push_front(pars.back());
                        pars.pop_back();
                    }
                }

                auto method = myMethods.find(name);
                if (method == myMethods.end()) {
                    throw MethodNotFoundFault("Method not found: " + name);
                }
                return{ method->second(parameters), Json(id) };
            }
            catch (const Fault& fault) {
                return Response(fault.GetCode(), fault.GetString(), Json(id));
            }
            catch (const std::out_of_range&) {
                InvalidParametersFault fault;
                return Response(fault.GetCode(), fault.GetString(), Json(id));
            }
            catch (const std::invalid_argument& ex){
                InvalidParametersFault fault;
                return Response(fault.GetCode(), fault.GetString(), Json(id));
            }
            catch (const std::exception& ex) {
                ServerErrorFault fault(Fault::SERVER_ERROR_CODE_DEFAULT, ex.what());
                return Response(fault.GetCode(), fault.GetString(), Json(id));
            }
            catch (...) {
                return Response(0, "unknown error", Json(id));
            }
        }

    private:
        template<typename ReturnType, typename... ParameterTypes>
        MethodWrapper& AddMethodInternal(std::string name, std::function<ReturnType(ParameterTypes...)> method) {
            return AddMethodInternal(std::move(name), std::move(method), redi::index_sequence_for < ParameterTypes... > {});
        }

        template<typename... ParameterTypes>
        MethodWrapper& AddMethodInternal(std::string name, std::function<void(ParameterTypes...)> method) {
            std::function<Json(ParameterTypes...)> returnMethod = [method](ParameterTypes&&... params) -> Json {
                method(std::forward<ParameterTypes>(params)...);
                return Json();
            };
            return AddMethodInternal(std::move(name), std::move(returnMethod), redi::index_sequence_for < ParameterTypes... > {});
        }

        template<typename ReturnType, typename... ParameterTypes, std::size_t... index>
        MethodWrapper& AddMethodInternal(std::string name, std::function<ReturnType(ParameterTypes...)> method, redi::index_sequence<index...>) {
            MethodWrapper::Method realMethod = [method](const Request::Parameters& params) -> Json {
                if (params.size() != sizeof...(ParameterTypes)) {
                    throw InvalidParametersFault();
                }
                return method(params[index].AsType<typename std::decay<ParameterTypes>::type>()...);
            };
            return AddMethod(std::move(name), std::move(realMethod));
        }


        AliasWrapper AddAliasInternal(std::string method){
          Request::Parameters params {};
          AliasWrapper alias {method, params};
          return std::move(alias);
        }


        template<typename FirstType, typename... ParameterTypes>
        AliasWrapper AddAliasInternal(std::string method, FirstType f, ParameterTypes... p){
          AliasWrapper alias = AddAliasInternal(method, p...);
          alias.parameters.push_front(f);
          return std::move(alias);
        }


    protected:
        std::map<std::string, MethodWrapper> myMethods;
        std::map<std::string, AliasWrapper> myAliases;
    };

} // namespace jsonrpc

#endif // JSONRPC_LEAN_DISPATCHER_H
