#ifndef GPHOTO_ENABLER_CLIENT_SOCKET_HH_
#define GPHOTO_ENABLER_CLIENT_SOCKET_HH_

#include "utility.hh"

GPHOTO_ENABLER_NS_BEGIN


/*
 * RAII wrapper for a socket.  Does accept() inside the ctor.  If
 * accept() fails with EAGAIN raises out_of_client exception
 */
class client_socket: non_copyable {

public:
  client_socket(int listen_fd);
  ~client_socket();
  void make_nonblocking();

  int get_fd() { return fd; }

private:
  int fd;
};


/*
 * Not inheriting from std::exception since this needs to be
 * super lightweight.
 */
class out_of_clients {};


GPHOTO_ENABLER_NS_END

#endif

