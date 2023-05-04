#pragma once

#include "utils.cpp"

#include <fstream>
using std::ifstream;
using std::ofstream;
#include <string>
using std::string;
using std::string_view;
#include <iostream>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

namespace store {
    class Store {
        std::unordered_map<string, string> inner;

        public:
            static string ping() {
                return "pong";
            }

            std::optional<string> get(const string& key) const {
                // FIXME: optimize
                try {
                    return inner.at(key);
                } catch (const std::out_of_range) {
                    return {};
                }
            }

            void set(string key, string value) {
                inner.insert_or_assign(key, value);
            }

            bool del(const string& key) {
                return static_cast<bool>(inner.erase(key));
            }

            size_t save(string path) const {
                size_t count = 0;
                // FIXME: optimize
                ofstream file(path);
                for (const auto& [key, value] : inner) {
                    // FIXME: change format (netstring)
                    file << key << ' ' << value << '\n';
                    count++;
                }
                return count;
            }

            size_t load(string path) {
                size_t count = 0;
                // FIXME: optimize
                ifstream file(path);
                string key = "";
                string value = "";

                // FIXME: change format (netstring)
                utils::read_until(file, key, ' ');
                utils::read_until(file, value, '\n');
                while (!file.eof()) {
                    this->set(key, value);
                    count++;
                    key.clear();
                    value.clear();
                    utils::read_until(file, key, ' ');
                    utils::read_until(file, value, '\n');
                }
                return count;
            }

            size_t size() const {
                return inner.size();
            }

            size_t clear() {
                size_t sz = inner.size();
                inner.clear();
                return sz;
            }
    };

    void tests() {
        std::cout << "running store tests . . .";

        utils::assert_eq(Store::ping(), "pong");

        Store s {};

        s.set("a", "1");
        utils::assert_eq(s.get("a").value(), "1");

        if (!s.del("a")) {
            std::abort();
        }

        if (s.get("a")) {
            std::cerr << "expected nullopt\n";
            std::abort();
        }

        if (s.del("a")) {
            std::abort();
        }

        std::cout << " ok\n";
    }
}

// int main() {
//     store::tests();

//     return 0;
// }
