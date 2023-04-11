#include <iostream>
#include <sstream>
#include <string>
using std::string;

namespace netstring
{
    template<class T, class U>
    concept Derived = std::is_base_of<U, T>::value;

    bool is_digit(char c)
    {
        return '0' <= c && c <= '9';
    }

    template <typename I> requires Derived<I, std::basic_istream<char>>
    class reader
    {
    private:
        I inner;

    public:
        reader(I inner) : inner(inner) {}

        // read the payload of one netstring from the underlying stream
        // and append it to the buffer’s end
        int read(string &buf)
        {
            int size_char_count = 0;
            int payload_size = 0;
            while
                true
                {
                    char c = inner.get();
                    if is_digit (c)
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

            int prev_size = buf.size();
            inner.read(buf, payload_size);
            if (inner.get() != ',')
            {
                // invalid netstring: expected ','
                // resetting caller’s buffer
                buf.resize(prev_size);
                std::abort();
            }

            return size_char_count + 1 + payload_size + 1;
        }

        // int read_list
    };

    template <typename O> requires Derived<O, std::basic_ostream<char>>
    class writer
    {
    private:
        O inner;

    public:
        writer(O inner) : inner(inner) {}
        // write the payload wrapped in a netstring
        // to the underlying stream
        int write(const string &payload)
        {
            string size = std::to_string(payload.size());

            inner.write(size, size.size());
            inner.put(':');
            inner.write(payload, payload.size());
            inner.put(',');

            return size.size() + 1 + payload.size() + 1;
        }

        // int write_list

        // flush / take_inner ?
    };
}

int main()
{
    // string in1 = "hello";
    // string in = "hello";

    // string dummy_socket;
    // auto ns_writer((std::ostringstream(dummy_socket)));
    // auto ns_reader((std::istringstream(dummy_socket)));

    // ns_writer.write(in);

    // string out;
    // string in2 = from_netstring(out);

    // std::cout << in1 << '\n';
    // std::cout << out << '\n';
    // std::cout << in2 << '\n';

    // if (out != "5:hello,")
    // {
    //     std::abort();
    // }
    // std::cout << "to_netstring ok\n";

    // if (in1 != in2)
    // {
    //     std::abort();
    // }
    // std::cout << "from_netstring ok\n";

    return 0;
}