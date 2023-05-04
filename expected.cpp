#pragma once

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

            expected(expected& other) = delete;
            expected operator=(expected& other) = delete;
            expected(expected&& other) {
                is_value = other.is_value;
                if (is_value) {
                    value = std::move(other.value);
                } else {
                    error = std::move(other.error);
                }
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
                    terminate();
                }
                return error;
            }
    };
}
