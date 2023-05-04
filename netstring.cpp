#pragma once

#include "utils.cpp"

#include <iostream>
#include <sstream> //FIXME: only used for tests
#include <string>
using std::string;


namespace netstring {
    template<class T, class U>
    concept Derived = std::is_base_of<U, T>::value;
    // template<class T, typename Fn>
    // concept HasMemberFn = std::is_member_function_pointer<decltype(&Type::foo)>::value;

    class Netstring {
        public:
            std::string inner;
            Netstring(string inner_): inner(std::move(inner_)) {}
    };

    class NetstringView {
        public:
            std::string_view inner;
            constexpr NetstringView(std::string_view inner_): inner(inner_) {}
    };

    template <typename I> requires netstring::Derived<I, std::istream>
    I& operator>>(I& is, Netstring& ns) {
        int size_char_count = 0;
        size_t payload_size = 0;
        int c = is.get();
        while (std::isdigit (c)) {
                size_char_count++;
                payload_size *= 10;
                payload_size += static_cast<size_t>(c - '0');
                c = is.get();
        }

        if (size_char_count == 0)
        {
            // invalid netstring: no size
            std::abort(); // FIXME: use optional or expected
        }
        if (c != ':') {
            // invalid netstring: unexpected character / std::char_traits<char>::eof
            std::abort(); // FIXME: use optional or expected
        }

        size_t prev_size = ns.inner.size();
        ns.inner.resize(prev_size + payload_size); // FIXME: use reserve
        is.read(&ns.inner[prev_size], static_cast<std::streamsize>(payload_size));
        if (is.get() != ',')
        {
            // invalid netstring: expected ','
            // resetting caller’s buffer
            ns.inner.resize(prev_size);
            std::abort(); // FIXME: handle error
        }

        return is;
    }

    // FIXME: do not derive from STL, require function write instead
    template <typename O> requires netstring::Derived<O, std::ostream>
    O& operator<<(O& os, const NetstringView& nv) {
            string payload = string(nv.inner);
            string size = std::to_string(payload.size());

            os.write(size.c_str(), static_cast<std::streamsize>(size.size()));
            os.write(":", 1);
            os.write(payload.c_str(), static_cast<std::streamsize>(payload.size()));
            os.write(",", 1);

            return os;
    }

    constexpr NetstringView operator ""_nv(const char* str, std::size_t len) {
        return NetstringView(std::string_view(str, len));
    }

    static void tests() {
        std::cerr << "running netstring tests . . .";

        string in1 = "hello";
        string in2 = "world";

        std::stringstream dummy_socket("");
        Netstring out("");

        dummy_socket << "hello"_nv;
        utils::assert_eq(dummy_socket.str(), "5:hello,");

        dummy_socket << NetstringView(in2);
        utils::assert_eq(dummy_socket.str(), "5:hello,5:world,");

        dummy_socket >> out;
        utils::assert_eq(out.inner, in1);

        dummy_socket >> out;
        utils::assert_eq(out.inner, in1 + in2);

        std::cerr << " ok\n";
    }
}

int main()
{
    netstring::tests();

    return 0;
}
