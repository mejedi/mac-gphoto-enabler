#include "abstract_client.hh"

#include <errno.h>

#include <array>

GPHOTO_ENABLER_NS_BEGIN


abstract_client::abstract_client(hub *owner_, int listen_fd)
: sock(listen_fd),
  sock_readable(DISPATCH_SOURCE_TYPE_READ, sock.get_fd()),
  sock_writable(DISPATCH_SOURCE_TYPE_WRITE, sock.get_fd()) {

  sock.make_nonblocking();

  sock_writable.set_event_handler(^{
    if (buf.empty()) {
      sock_writable.suspend();
    }

    std::array<uint8_t, 2048> buf_;
    size_t len = std::min(buf_.size(), buf.size());
    std::copy(buf.begin(), buf.begin()+len, buf_.begin());

    ssize_t s = ::write(sock.get_fd(), &buf_[0], len);
    if (s>=0) {
      buf.erase(buf.begin(), buf.begin()+s);
    } else {
      switch (errno) {
        case EPIPE:
          this->destroy();
          break;
      }
    }
  });

  sock_readable.set_event_handler(^{
    uint8_t buf[512];
    ssize_t s;

    s = read(sock.get_fd(), buf, sizeof buf);
    if (s == 0 || (s == -1 && errno != EINTR)) {
      this->destroy();
    }
  });
  sock_readable.resume();

  LIST_INSERT_HEAD(&owner_->client_list, this, client_list_entry);
}


abstract_client::~abstract_client() {
  LIST_REMOVE(this, client_list_entry);
}


void abstract_client::write(const std::string &message)
{
  bool resume = buf.empty();

  buf.insert(buf.end(), message.begin(), message.end());
  if (resume) {
    sock_writable.resume();
  }
}


GPHOTO_ENABLER_NS_END

