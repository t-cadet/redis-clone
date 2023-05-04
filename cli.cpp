#include "tcp.cpp"

#include <iostream>
using std::cout;
using std::cin;
#include <string>
using std::string_view;
using std::string;


static void print_help() {
    // FIXME: update help
    cout << "Welcome to rediss-cli stub . . .\n";
    cout << "\n";
    cout << "COMMANDS";
    cout << "\n";
    cout << "exit               - exit the CLI\n";
    cout << "echo <string>      - returns <string>\n";
    cout << "netstring <string> - returns the netstring version of <string>\n";
    cout << '\n';
}

// SIGPIPE      P1990      Term    Broken pipe: write to pipe with no readers; see pipe(7)
// If  all  file  descriptors  referring  to the read end of a pipe have been closed,
// then a write(2) will cause a SIGPIPE signal to be generated for the calling process.
// https://blog.erratasec.com/2018/10/tcpip-sockets-and-sigpipe.html
// When a process writes to a socket that has received an RST, the SIGPIPE signal is sent to the process.
// The default action of this signal is to terminate the process,
// so the process must catch the signal to avoid being involuntarily terminated.
void sigpipe_handler(int signal) {
    std::cerr << "received SIGPIPE (" << signal << ')' << std::endl;
}

int main(int argc, char * argv[])
{
    // TODO: remove?
    signal(SIGPIPE, sigpipe_handler);

    string host = "127.0.0.1";
    uint16_t port = 8888;

    for (int i = 1; i < argc; i+=2) {
        if (string_view(argv[i]) == "--host") {
            host = string(argv[i+1]);
        }
        if (string_view(argv[i]) == "--port") {
            port = static_cast<uint16_t>(stoul(string(argv[i+1])));
        }
    }

    #define value_or_eprint_exit(expected) {(auto res = (expected), res ? res.value() : (std::cerr << res.error() << std::endl; exit(1)))}

    print_help();
    for(;;) {
        string cmd {};
        char response[4096];

        std::cerr << "connecting to server . . .";
        // TODO: use macro to handle failure
        // tcp::Socket client = value_or_eprint_exit(tcp::Socket::create());
        // value_or_eprint_exit(client.connect(host, port));
        tcp::Socket client = tcp::Socket::create_unsafe(); // FIXME: use create()

        // FIXME: use macros
        auto client_exp = client.connect(host, port);
        if(!bool(client_exp)) {
            std::cerr << client_exp.err() << std::endl;
            exit(1);
        }
        std::cerr << " done" << std::endl;

        for(;;) {
            // DISPLAY PROMPT
            cout << "rediss-cli> ";

            // GET USER INPUT
            cmd.clear();
            getline(std::cin, cmd);


            // SEND INPUT TO SERVER
            {auto exp = client.send_all(cmd);
            if (!exp) {
                std::cerr << "write_all failed: " << exp.err() << '\n';
                break;
            }}

            // GET SERVER RESPONSE
            // FIXME: read one nestring: expected = client >> Netstring("")
            auto exp = client.recv_(response, 4096);
            if (!(exp)) {
                std::cerr << "recv failed: " << exp.err() << '\n';
                break;
            }

            // DISPLAY SERVER RESPONSE
            // std::cout << "rediss-server> ";
            for(auto i = 0; i < exp.val(); i++) {
                std::cout << response[i]; // TODO: parse result and print it formatted
            }
            std::cout << std::endl;
        }
    }
}
