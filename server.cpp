#include "cmd.cpp"
#include "expected.cpp"
#include "store.cpp"
#include "tcp.cpp"

#include <iostream>
#include <sstream>

int main(int argc, char * argv[])
{
  // TODO: print help
  uint16_t port = 8888;
  for (int i = 1; i < argc; i+=2) {
    if (std::string_view(argv[i]) == "--port") {
        port = static_cast<uint16_t>(std::stoul(std::string(argv[i+1])));
    }
  }

  tcp::Socket server = VALUE_OR(THROW, tcp::Socket::create());
  VALUE_OR(THROW, server.bind("0.0.0.0", port));
  VALUE_OR(THROW, server.listen());

  store::Store store {};
  char buf[4096] = {};
  for (;;) {
    tcp::Socket client_sock = VALUE_OR(THROW, server.accept());
    
    // TODO: accept connection, push it to vec, read in non blocking mode (write response if necessary), when the connection is dead pop it
    // TODO: look into epoll, asyncio etc
    for (;;) {
      //FIXME: read one netstring
      size_t bytes_read = VALUE_OR(PBREAK, client_sock.recv_(buf, 4096));


      // debug
      std::cerr << "debug:";
      for(int j = 0; j < bytes_read; j++) {
          std::cerr << buf[j];
      }
      std::cerr << std::endl;

      // process CMD

      // FIXME: optimize
      // or use e.g. Cmd::parse(socket).run(store).write(socket)
      stringstream cmd_in(string(buf, static_cast<size_t>(bytes_read)));
      stringstream cmd_out("");

      cmd::Cmd* cmd;
      cmd_in >> cmd;
      cmd_out << cmd->run(store);
      string out = cmd_out.str();
      if (out.empty()) {
        out = "(empty)"; // FIXME: hack to send empty response until we send netstring back
      }

      // write output
      // FIXME: wrap in netstring
      VALUE_OR(PBREAK, client_sock.send_all(out));
    }
  }
}
