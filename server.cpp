  // https://en.wikipedia.org/wiki/Berkeley_sockets

  #include "cmd.cpp"
  #include "store.cpp"
  #include "tcp.cpp"

  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #include <unistd.h>
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

    // FIXME: use create()
    tcp::Socket server = tcp::Socket::create_unsafe();

    // FIXME: handle expected conveniently
    {auto expected = server.bind("0.0.0.0", port);
    if (!expected) {
      std::cerr << expected.err() << std::endl;
      exit(1);
    }}

    // FIXME: handle expected conveniently
    {auto expected = server.listen();
    if (!expected) {
      std::cerr << expected.err() << std::endl;
      exit(1);
    }}


    // struct sockaddr_in sa{};
    // int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    // if (SocketFD == -1) {
    //   perror("cannot create socket");
    //   exit(EXIT_FAILURE);
    // }

    // sa.sin_family = AF_INET;
    // sa.sin_port = htons(port);
    // sa.sin_addr.s_addr = htonl(INADDR_ANY);
  
    // if (bind(SocketFD, reinterpret_cast<sockaddr *>(&sa), sizeof sa) == -1) {
    //   perror("bind failed");
    //   close(SocketFD);
    //   exit(EXIT_FAILURE);
    // }
  
    // if (listen(SocketFD, 10) == -1) {
    //   perror("listen failed");
    //   close(SocketFD);
    //   exit(EXIT_FAILURE);
    // }

    store::Store store {};
    char buf[4096] = {};
    for (;;) {
      tcp::Socket ConnectFD = server.accept_unsafe();
      // int ConnectFD = accept(SocketFD, nullptr, nullptr);
      // if (ConnectFD == -1) {
      //   perror("accept failed");
      //   close(SocketFD);
      //   exit(EXIT_FAILURE);
      // }
      
      // FIXME: read until the client is dead (this only handles one client)
      // TODO: accept connection, push it to vec, read in non blocking mode (write response if necessary), when the connection is dead pop it
      // TODO: look into epoll, asyncio etc
      for (;;) {
        //FIXME: read one netstring
        auto bytes_read_exp = ConnectFD.recv_(buf, 4096);
        if (!bytes_read_exp) {
          std::cerr << bytes_read_exp.err() << std::endl;
          break;
        }
        size_t bytes_read = bytes_read_exp.val();
        // if (bytes_read == 0) {
        //   break;
        // } else if (bytes_read < 0) {
        //   perror("read failed");
        //   break;
        // }


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
        auto send_all_exp = ConnectFD.send_all(out);
        if (!send_all_exp) {
          std::cerr << send_all_exp.err() << std::endl;
          break;
        }
        // if (write(ConnectFD, out.data(), out.size()) < 0) { // FIXME: wrap in netstring
        //   perror("write failed");
        //   break;
        // }

        // echo
        // if (write(ConnectFD, buf, static_cast<size_t>(bytes_read)) < 0) {
        //   perror("write failed");
        //   break;
        // }
      }

      // if (shutdown(ConnectFD, SHUT_RDWR) == -1) {
      //   perror("shutdown failed");
      // }
      // close(ConnectFD);
    }

    // unreachable
    // close(SocketFD);
    // return EXIT_SUCCESS;
}
