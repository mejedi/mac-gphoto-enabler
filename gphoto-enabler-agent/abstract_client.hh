#ifndef GPHOTO_ENABLER_ABSTRACT_CLIENT_HH_
#define GPHOTO_ENABLER_ABSTRACT_CLIENT_HH_

#include "utility.hh"
#include "hub.hh"
#include "client_socket.hh"
#include "dispatch_source.hh"

#include <stdint.h>

#include <string>
#include <deque>

GPHOTO_ENABLER_NS_BEGIN


/*
 * Manages a client socket; data submited via write() method is buffered
 * internally and is sent in a non-blocking manner.
 */
class abstract_client: non_copyable {
  friend class hub;

public:
  virtual ~abstract_client();
  virtual void destroy() = 0;
  void write(const std::string &message);

protected:
  abstract_client(hub *owner, int listen_fd);

private:
  LIST_ENTRY(abstract_client)
                             client_list_entry;
  client_socket              sock;
  dispatch_source            sock_readable;
  dispatch_source            sock_writable;
  std::deque<uint8_t>        buf;
};


GPHOTO_ENABLER_NS_END

#endif

