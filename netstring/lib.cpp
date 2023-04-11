#include <iostream>
#include <sstream>
#include <string>
using std::string;

namespace netstring
{
    template<class T, class U>
    concept Derived = std::is_base_of<U, T>::value;

    template <typename I> requires Derived<I, std::basic_istream<char>>
    class reader
    {
    private:
        I inner;

    public:
        reader(I inner) : inner(std::move(inner)) {}

        // read the payload of one netstring from the underlying stream
        // and append it to the buffer’s end
        int read(string &buf)
        {
            int size_char_count = 0;
            int payload_size = 0;
            while
                (true)
                {
                    char c = inner.get();
                    if (std::isdigit (c))
                    {
                        size_char_count++;
                        payload_size *= 10;
                        payload_size += c - '0';
                    }
                    else if (c == ':')
                    {
                        break;
                    }
                    else
                    {
                        // invalid netstring: unexpected character / std::char_traits<char>::eof
                        // expected digit or ':'
                        std::abort();
                    }
                }

            if (size_char_count == 0)
            {
                // invalid netstring: no size
                std::abort();
            }

            auto prev_size = buf.size();
            buf.resize(prev_size + payload_size); // FIXME: resize initializes the characters, which is not needed
            inner.read(&buf[prev_size], payload_size);
            if (inner.get() != ',')
            {
                // invalid netstring: expected ','
                // resetting caller’s buffer
                buf.resize(prev_size);
                std::abort();
            }

            return size_char_count + 1 + payload_size + 1;
        }

        // consumes the reader
        I into_inner() {
            return std::move(inner);
        }

        // int read_list
    };

    template <typename O> requires Derived<O, std::basic_ostream<char>>
    class writer
    {
    private:
        O inner;

    public:
        writer(O inner) : inner(std::move(inner)) {}
        // write the payload wrapped in a netstring
        // to the underlying stream
        int write(const string &payload)
        {
            string size = std::to_string(payload.size());

            inner.write(size.c_str(), size.size());
            inner.put(':');
            inner.write(payload.c_str(), payload.size());
            inner.put(',');

            return size.size() + 1 + payload.size() + 1;
        }

        // consumes the writer
        O into_inner() {
            return std::move(inner);
        }

        // flush?
    };
}

void assert_eq(string got, string expected) {
    if (got != expected) {
    std::cout << got.length() << " " << expected.length() << "\n";
        std::cout << "expected: `" << expected << "`, got: `" << got << "`, aborting ...\n";
        std::abort();
    }
}

void tests() {
    std::cout << "running tests . . .";

    std::basic_stringstream<char> dummy_socket("");

    netstring::writer<std::basic_stringstream<char>> ns_writer(std::move(dummy_socket));
    
    string in1 = "hello";
    string in2 = "world";
    
    ns_writer.write(in1);
    ns_writer.write(in2);

    dummy_socket = ns_writer.into_inner(); // ns_writer is now invalid
    assert_eq(dummy_socket.str(), "5:hello,5:world,");

    string out = "";
    netstring::reader ns_reader(std::move(dummy_socket));
    ns_reader.read(out);
    assert_eq(out, in1);
    ns_reader.read(out);
    assert_eq(out, in1 + in2);

    std::cout << " ok\n";
}

int main()
{
    tests();
    return 0;
}