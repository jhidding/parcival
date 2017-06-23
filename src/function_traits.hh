/* Copyright 2017 Johan Hidding
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */
#pragma once

#include <tuple>
#include <functional>

namespace Parcival
{
    template <typename F>
    struct function_traits;

    template <typename return_type, typename... arg_types>
    struct function_traits<return_type (*)(arg_types...)>
        : public function_traits<return_type (arg_types...)>
    {};

    template <typename R, typename... Args>
    struct function_traits<R(Args...)>
    {
        static constexpr size_t arity = sizeof...(Args);
        using return_type = R;

        using function_type = std::function<R (Args...)>;

        template <size_t N>
        struct arg
        {
            static_assert(N < arity, "error: invalid argument index.");
            using type = typename std::tuple_element
                <N, std::tuple<Args...>>::type;
        };
    };

    template <typename class_type,
              typename return_type,
              typename... arg_types>
    struct function_traits<return_type (class_type::*)(arg_types...)>
        : public function_traits<return_type (class_type&, arg_types...)>
    {};

    template <typename class_type,
              typename return_type,
              typename... arg_types>
    struct function_traits<return_type (class_type::*)(arg_types...) const>
        : public function_traits<return_type (class_type&, arg_types...)>
    {};

    template <typename class_type,
              typename return_type>
    struct function_traits<return_type (class_type::*)>
        : public function_traits<return_type (class_type&)>
    {};

    template <typename F>
    struct function_traits
    {
        using call_type = function_traits<decltype(&F::operator())>;
        using return_type = typename call_type::return_type;

        static constexpr size_t arity = call_type::arity - 1;
        using function_type = typename call_type::function_type;

        template <size_t N>
        struct arg
        {
            static_assert(N < arity, "error: invalid argument index.");
            using type = typename call_type::template arg<N+1>::type;
        };
    };
}
