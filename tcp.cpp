#pragma once


#include "expected.cpp"

#include <iostream>
#include <sstream>
#include <string>
using std::string_view;
#include <variant>
using std::monostate;

// BSD sockets
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

namespace tcp {
    #define value_or_return(expected) {(auto res = (expected), res ? res.value() : (return res.error();))}

    // TODO: provide error enum instead of string?
    using Error = std::string;
    template<class T>
    using expected = expected::expected<T, tcp::Error>;

    static expected<in_addr> _inet_pton(int family, string_view host) {
        in_addr address{};
        auto res = inet_pton(family, host.data(), &address);

        if (res == 0) {
            // TODO: convert family its string repr
            // TODO: use std::format
            std::stringstream ss;
            ss << "`" << host << "` is not a valid network address in the address family `" << family << "`";
            return ss.str();
            // return format("`{}` is not a valid network address in the address family `{}`", host, family);
        } else if (res < 0) {
            return std::string(strerror(errno));
        }

        return address;
    }

    static expected<sockaddr_in> make_sockaddr(string_view host, uint16_t port) {
        auto sin_addr = _inet_pton(AF_INET, host);
        if (!sin_addr) {
            // FIXME: handle expected with macro
            return sin_addr.err();
        }
        return sockaddr_in {
            .sin_family = AF_INET,
            .sin_port = htons(port),
            .sin_addr = sin_addr.val(),
        };
    }

    // tcp::socket
    //
    // .connect(host, port)
    //
    // .bind(host, port)
    // .listen()
    // .accept()
    //
    // use templates and type state to e.g. prevent from calling listen on a non bound socket
    class Socket {
        int fd;
        Socket(int fd_): fd(fd_) {}

        public:

        static expected<Socket> create() {
            int fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (fd == -1) {
                return std::string(strerror(errno));
            }
            return expected<Socket>(Socket(fd));
        }

        // FIXME: remove when create works properly
        static Socket create_unsafe() {
            int fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (fd == -1) {
                ::std::cerr << strerror(errno) << std::endl;
                std::terminate();
            }
            return Socket(fd);
        }

        // no copy because Socket contains a ressource
        Socket(Socket&) = delete;
        Socket& operator=(Socket&) = delete;
        Socket(Socket&&) = default;
        Socket& operator=(Socket&&) = default;
        ~Socket() {
            // ::std::cerr << "socket destructor" << std::endl;
            shutdown(fd, SHUT_RDWR);
            close(fd);
        }

        // TODO: builder pattern? (return expected<Socket> or expected<Socket&>)
        expected<monostate> connect(std::string_view host, uint16_t port) {
            auto sa_e = make_sockaddr(host, port);
            if (!sa_e) {
                // FIXME: handle expected with macro
                return sa_e.err();
            }
            auto sock_addr = sa_e.val();

            if (::connect(fd, reinterpret_cast<sockaddr *>(&sock_addr), sizeof sock_addr) == -1) {  // error: bad file descriptor    
                close(fd); // TODO: is it needed?
                return std::string(strerror(errno));
            }

            return monostate{};
        }

        expected<monostate> bind(std::string_view host, uint16_t port) {
            auto sa_e = make_sockaddr(host, port);
            if (!sa_e) {
                // FIXME: handle expected with macro
                return sa_e.err();
            }
            auto sock_addr = sa_e.val();
            
            if (::bind(fd, reinterpret_cast<sockaddr *>(&sock_addr), sizeof sock_addr) == -1) {
                close(fd); // TODO: needed?
                return std::string(strerror(errno));
            }

            return monostate{};
        }

        // backlog default value: same value as Rust std lib's TcpStream:
        // https://doc.rust-lang.org/src/std/sys_common/net.rs.html
        expected<monostate> listen(uint backlog = 128) {
            if (::listen(fd, static_cast<int>(backlog)) == -1) {
                close(fd); // TODO: needed?
                return std::string(strerror(errno));
            }
            return monostate{};
        }

        // TODO: returns the peer's address too ? (address abstraction containing the host & port?)
        // TODO: look into SOCK_CLOEXEC
        // expected<Socket> accept() {
        Socket accept_unsafe() {
            int connected_fd = ::accept(fd, nullptr, nullptr);
            if (connected_fd == -1) {
                close(fd); // TODO: needed?
                ::std::cerr << strerror(errno) << std::endl;
                std::terminate();
            }
            return Socket(connected_fd);
        }

        // TODO: should this take the same options as the send syscall?
        expected<monostate> send_all(string_view buf) {
            ssize_t written = 0;
            while (buf.size() > 0) {
                written = send(fd, buf.data(), std::min<std::size_t>(buf.size(), 4096), 0);
                if (written < 0) {
                    return std::string(strerror(errno));
                } else {
                    buf = buf.substr(static_cast<size_t>(written));
                }
            }
            return monostate{};
        }

        // TODO: make buf a string?
        expected<monostate> recv_n(char* buf, size_t n) {
            size_t total_read = 0;
            while (total_read < n) {
                ssize_t read = recv(fd, buf + total_read, n - total_read, 0);
                if (read == 0) {
                    return std::string("reached EOF: peer has performed an orderly shutdown");
                } else if (read < 0) {
                    return std::string(strerror(errno));
                }
                total_read += static_cast<size_t>(read);
            }
            return monostate{};
        }

        // FIXME: delete
        expected<size_t> recv_(char* buf, size_t n) {
            ssize_t read = recv(fd, buf, n, 0);
            if (read == 0) {
                return std::string("reached EOF: peer has performed an orderly shutdown");
            } else if (read < 0) {
                return std::string(strerror(errno));
            }
            return static_cast<size_t>(read);
        }


        // DEBUG APIS
            // int optval;
            // socklen_t optlen = sizeof(optval);
            // if (getsockopt(listener.fd, SOL_SOCKET, SO_ERROR, &optval, &optlen) == -1) {
            //     perror("getsockopt failed");
            // }
            // cout << "SO_ERROR=" << strerror(optval) << std::endl;
            /*
            pub fn take_error(&self) -> io::Result<Option<io::Error>> {
                let raw: c_int = getsockopt(self, libc::SOL_SOCKET, libc::SO_ERROR)?;
                if raw == 0 { Ok(None) } else { Ok(Some(io::Error::from_raw_os_error(raw as i32))) }
            }

            pub fn set_nonblocking(&self, nonblocking: bool) -> io::Result<()> {
                let mut nonblocking = nonblocking as libc::c_int;
                cvt(unsafe { libc::ioctl(self.as_raw_fd(), libc::FIONBIO, &mut nonblocking) }).map(drop)
            }
            */

            // tcp_info info;
            // socklen_t info_len = sizeof(optval);
            // if (getsockopt(listener.fd, IPPROTO_TCP, TCP_INFO, &info, &info_len) == -1) {
            //     perror("getsockopt failed");
            // }
            // cout << "TCP_INFO=" << info.tcpi_state << std::endl;

            // int send_buffer_len = 0;
            // if (ioctl(listener.fd, TIOCOUTQ, &send_buffer_len) < 0) {
            //     perror("ioctl failed");
            // }
            // cout << "send_buffer_len=" << send_buffer_len << '\n';

            /*
            pub fn setsockopt<T>(
    sock: &Socket,
    level: c_int,
    option_name: c_int,
    option_value: T,
) -> io::Result<()> {
    unsafe {
        cvt(c::setsockopt(
            sock.as_raw(),
            level,
            option_name,
            &option_value as *const T as *const _,
            mem::size_of::<T>() as c::socklen_t,
        ))?;
        Ok(())
    }
}

pub fn getsockopt<T: Copy>(sock: &Socket, level: c_int, option_name: c_int) -> io::Result<T> {
    unsafe {
        let mut option_value: T = mem::zeroed();
        let mut option_len = mem::size_of::<T>() as c::socklen_t;
        cvt(c::getsockopt(
            sock.as_raw(),
            level,
            option_name,
            &mut option_value as *mut T as *mut _,
            &mut option_len,
        ))?;
        Ok(option_value)
    }
}

            */
    };
}