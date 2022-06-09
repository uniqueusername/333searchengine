/*
 * Copyright ©2022 Hal Perkins.  All rights reserved.  Permission is
 * hereby granted to students registered for University of Washington
 * CSE 333 for use solely during Spring Quarter 2022 for purposes of
 * the course.  No other use, copying, distribution, or modification
 * is permitted without prior written consent. Copyrights for
 * third-party components of this work must be honored.  Instructors
 * interested in reusing these course materials should contact the
 * author.
 */

// copyright 2022 ary jha
// aryanjha@uw.edu <1902079>

#include <stdio.h>       // for snprintf()
#include <unistd.h>      // for close(), fcntl()
#include <sys/types.h>   // for socket(), getaddrinfo(), etc.
#include <sys/socket.h>  // for socket(), getaddrinfo(), etc.
#include <arpa/inet.h>   // for inet_ntop()
#include <netdb.h>       // for getaddrinfo()
#include <errno.h>       // for errno, used by strerror()
#include <string.h>      // for memset, strerror()
#include <iostream>      // for std::cerr, etc.

#include "./ServerSocket.h"

using namespace std;

extern "C" {
  #include "libhw1/CSE333.h"
}

namespace hw4 {

void AcquireAddrAndDNS(struct sockaddr *addr, size_t addr_len,
                       string *post_addr, string *hname, uint16_t *port);

ServerSocket::ServerSocket(uint16_t port) {
  port_ = port;
  listen_sock_fd_ = -1;
}

ServerSocket::~ServerSocket() {
  // Close the listening socket if it's not zero.  The rest of this
  // class will make sure to zero out the socket if it is closed
  // elsewhere.
  if (listen_sock_fd_ != -1)
    close(listen_sock_fd_);
  listen_sock_fd_ = -1;
}

bool ServerSocket::BindAndListen(int ai_family, int* const listen_fd) {
  // Use "getaddrinfo," "socket," "bind," and "listen" to
  // create a listening socket on port port_.  Return the
  // listening socket through the output parameter "listen_fd"
  // and set the ServerSocket data member "listen_sock_fd_"

  // STEP 1:

  // populate hints
  struct addrinfo hints;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET6;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  hints.ai_flags |= AI_V4MAPPED;
  hints.ai_protocol = IPPROTO_TCP;
  hints.ai_canonname = nullptr;
  hints.ai_addr = nullptr;
  hints.ai_next = nullptr;

  struct addrinfo *result;
  int res = getaddrinfo(nullptr, to_string(port_).c_str(), &hints, &result);

  // did addrinfo() fail?
  if (res != 0) {
    cerr << "getaddrinfo() failed: " << gai_strerror(res) << std::endl;
    return false;
  }

  // loop through address structures until we can bind to a socket
  *listen_fd = -1;
  for (struct addrinfo *rp = result; rp != nullptr; rp = rp->ai_next) {
    *listen_fd = socket(rp->ai_family,
                        rp->ai_socktype,
                        rp->ai_protocol);
    if (*listen_fd == -1) {
      // socket creation failed, try again
      cerr << "socket() failed: " << strerror(errno) << endl;
      *listen_fd = 0;
      continue;
    }

    // configure socket
    int optval = 1;
    setsockopt(*listen_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    if (bind(*listen_fd, rp->ai_addr, rp->ai_addrlen) == 0) break;

    close(*listen_fd);
    *listen_fd = -1;
  }

  freeaddrinfo(result);

  if (*listen_fd == -1) {
    cerr << "couldn't bind to any addresses" << endl;
    return false;
  }

  if (listen(*listen_fd, SOMAXCONN) != 0) {
    cerr << "failed to mark socket as listening: " << strerror(errno) << endl;
    close(*listen_fd);
    return false;
  }

  listen_sock_fd_ = *listen_fd;
  return true;
}

bool ServerSocket::Accept(int* const accepted_fd,
                          std::string* const client_addr,
                          uint16_t* const client_port,
                          std::string* const client_dns_name,
                          std::string* const server_addr,
                          std::string* const server_dns_name) const {
  // Accept a new connection on the listening socket listen_sock_fd_.
  // (Block until a new connection arrives.)  Return the newly accepted
  // socket, as well as information about both ends of the new connection,
  // through the various output parameters.

  // STEP 2:
  // loop forever and accept client connections
  while (1) {
    struct sockaddr_storage caddr;
    socklen_t caddr_len = sizeof(caddr);
    int client_fd = accept(listen_sock_fd_,
                           reinterpret_cast<struct sockaddr *>(&caddr),
                           &caddr_len);
    if (client_fd < 0) {
      if ((errno == EINTR) || (errno == EAGAIN)) continue;
      cerr << "failure on accept: " << strerror(errno) << endl;
      return false;
    }

    *accepted_fd = client_fd;

    // client stuff
    AcquireAddrAndDNS(reinterpret_cast<struct sockaddr *>(&caddr), caddr_len,
                      client_addr, client_dns_name, client_port);

    // server stuff
    struct sockaddr_storage saddr;
    socklen_t saddr_len = sizeof(saddr);
    Verify333(getsockname(client_fd,
              reinterpret_cast<struct sockaddr *>(&saddr),
              &saddr_len) == 0);
    AcquireAddrAndDNS(reinterpret_cast<struct sockaddr *>(&saddr), saddr_len,
                      server_addr, server_dns_name, client_port);

    return true;
  }
}

void AcquireAddrAndDNS(struct sockaddr *addr, size_t addr_len,
                       string *post_addr, string *hname, uint16_t *port) {
  if (addr->sa_family == AF_INET) {
    // ipv4
    char astring[INET_ADDRSTRLEN];
    struct sockaddr_in *in4 = reinterpret_cast<struct sockaddr_in *>(addr);
    inet_ntop(AF_INET, &(in4->sin_addr), astring, INET_ADDRSTRLEN);
    *post_addr = astring;
    *port = htons(in4->sin_port);
  } else if (addr->sa_family == AF_INET6) {
    // ipv6
    char astring[INET6_ADDRSTRLEN];
    struct sockaddr_in6 *in6 = reinterpret_cast<struct sockaddr_in6 *>(addr);
    inet_ntop(AF_INET6, &(in6->sin6_addr), astring, INET6_ADDRSTRLEN);
    *post_addr = astring;
    *port = htons(in6->sin6_port);
  } else {
    cout << " ???? address and port ????" << endl;
  }

  char hostname[1024];
  Verify333(getnameinfo(addr, addr_len, hostname, 1024, nullptr, 0, 0) == 0);
  *hname = hostname;
}

}  // namespace hw4
