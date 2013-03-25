#include "client_socket.hh"

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>

GPHOTO_ENABLER_NS_BEGIN


client_socket::client_socket(int listen_fd) {
  sockaddr_storage addr;
  socklen_t len = sizeof addr;

  fd = accept(listen_fd, reinterpret_cast<sockaddr *>(&addr), &len);
  if (fd == -1) {
    /* FIXME */
    throw out_of_clients();
  }
}


client_socket::~client_socket() {
  close(fd);
  /* FIXME */
}


void client_socket::make_nonblocking() {
  fcntl(fd, F_SETFL, O_NONBLOCK);
  /* FIXME */
}


GPHOTO_ENABLER_NS_END

