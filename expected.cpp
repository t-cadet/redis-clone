#pragma once

// FIXME: remove test dependencies
#include "utils.cpp"
#include <iostream>

#include <exception>
using std::terminate;

namespace expected {
    // TODO: flesh out & optimize or use std
    template<class T, class E>
    class expected {
        bool is_value;
        union {
            T value;
            E error;
        };


        public:
            expected(E&& error_): error(std::move(error_)) {
                is_value = false;
            }

            expected(T&& value_): value(std::move(value_)) {
                is_value = true;
            }

            // TODO: allow copy if T and E are trivially copy
            expected(expected const& other) = delete;
            expected& operator=(expected const& other) = delete;
            expected(expected&& other) {
                is_value = other.is_value;
                if (is_value) {
                    new (&value) T(std::move(other.value));
                } else {
                    // error = std::move(other.error);
                    // TODO: check that placement new is the right thing to use here
                    // (the above line produced a segfault with E = string)
                    new (&error) E(std::move(other.error));
                }
            }
            expected& operator=(expected&& other) {
                if (this != &other) {
                    this->~expected();
                    new (this) expected(std::move(other));
                }
                return *this;
            }

            ~expected() {
                if (is_value) {
                    value.~T();
                } else {
                    error.~E();
                }
            }

            static expected unexpected(E error) {
                return expected(error);
            }

            constexpr explicit operator bool() const noexcept {
                return is_value;
            }

            T val() {
                if (is_value) {
                    return std::move(value);
                }
                terminate();
            }

            E err() {
                if (is_value) {
                    // FIXME: use exception
                    terminate();
                }
                return std::move(error);
            }

            T unchecked_value() noexcept {
                return std::move(value);
            }

            E unchecked_error() noexcept {
                return std::move(error);
            }
    };

    //////////////////////////////
    //  Rust's try! like macro  //
    //////////////////////////////
    #define ACTION_BREAK(x) break
    #define ACTION_CONTINUE(x) continue
    #define ACTION_RETURN(x) return x.unchecked_error()
    #define ACTION_THROW(x) throw x.unchecked_error()
    
    #define ACTION_PBREAK(x) ::std::cerr << x.unchecked_error() << '\n'; break
    #define ACTION_PCONTINUE(x) ::std::cerr << x.unchecked_error() << '\n'; continue

    // Note: this macro requires the Statement-Exprs
    // extension. It seems to be available on both
    // GCC and CLANG but not MSVC. Not using this
    // extension would make the macro much less
    // ergonomic as it would expand to a statement
    // and would not be usable as function arg,
    // struct field init, etc.
    #define VALUE_OR(flow, expected)({          \
        auto evaluated = std::move(expected);   \
        if(!evaluated) {                        \
            ACTION_##flow(evaluated);             \
        }                                       \
        evaluated.unchecked_value();            \
    })

    // Note: statement version of the macro that
    // requires no extension but is less ergonomic
    //
    // #define VALUE_OR(flow, varname, expected)   \
    // auto evaluated = std::move(expected);       \
    // if(!evaluated) {                            \
    //     FLOW_##flow(evaluated);                 \
    // }                                           \
    // auto varname = evaluated.unchecked_value()

    expected<int, string> test() {
        std::cerr << "running expected tests . . .";

        expected<int, std::string> ok(1);
        expected<int, std::string> ko("ko");

        utils::assert_eq(bool(ok), true);
        utils::assert_eq(bool(ko), false);

        utils::assert_eq(ok.val(), 1);
        utils::assert_eq<std::string>(ko.err(), "ko");

        utils::assert_eq(VALUE_OR(RETURN, ok), 1);
        for(;;) {
            VALUE_OR(BREAK, ko);
            utils::assert_eq("err", "expected to break out of the loop");
        }

        expected<int, std::string> ko2("ko");
        for(;;) {
            try {
                VALUE_OR(THROW, ko2);
            } catch (std::string e) {
                utils::assert_eq(e, string("ko"));
                break;
            }
            utils::assert_eq("err", "expected to break out of the loop after catching the exception");
        }

        // Note: tests for statement version of the macro
        //
        // VALUE_OR(RETURN, val, ok);
        // utils::assert_eq(val, 1);
        // for(;;) {
        //     VALUE_OR(BREAK, err, ko);
        //     utils::assert_eq("???", "to break out of the loop");
        // }

        std::cerr << " done\n" << std::endl;
        return expected<int, std::string>(1);
    }
}

// TODO: move tests in their own files
// int main() {
//     expected::test();
// }
