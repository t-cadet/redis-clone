#pragma once

#include <exception>
#include <iostream>
using std::cerr;
using std::terminate;
using std::istream;
#include <string>
using std::string;
#include <sstream>

namespace utils {
    void assert_eq(string got, string expected) {
        if (got != expected) {
            std::ostringstream error {};
            error << "assertion failed - expected: `" << expected << "`, got: `" << got << "`\n";
            throw std::invalid_argument(error.str());
        }
    }

    istream& read_until(istream& is, string& buf, char until) {
        char c = is.get();
        while (!is.eof() && c != until) {
            buf.push_back(c);
            c = is.get();
        }
        return is;
    }
}
