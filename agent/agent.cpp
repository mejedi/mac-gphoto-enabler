#include <deque>
#include <tr1/array>
#include <stdexcept>

#include <CoreFoundation/CoreFoundation.h>

#include <IOKit/IOKitLib.h>
#include <IOKit/IOMessage.h>
#include <IOKit/IOCFPlugIn.h>
#include <IOKit/usb/IOUSBLib.h>

#include <dispatch/dispatch.h>

#include <sys/queue.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>


class client;
class device_info;
LIST_HEAD(device_list_head, device_info);


/*
 * Manages a client socket; data submited via write() method is buffered
 * internally and is sent in a non-blocking manner.
 *
 * Also stores the head of the device list; see below.
 */
class client {
  friend class device_info;

public:
  client(int listen_fd);
  virtual ~client();
  virtual void shutdown() = 0;
  void write(const uint8_t *buf, size_t size);

private:
  int                        sock;
  dispatch_source_t          sock_dispatch;
  std::deque<uint8_t>        buf;

  // io_iterator_t              notification;
  device_list_head           device_list;
};


/*
 * This exception is raised when accept() on the listening socket passed
 * to client ctor fails with EAGAIN (no pending connection).
 *
 * Not inheriting from std::exception since this needs to be
 * super lightweight.
 */
class out_of_clients {

};


/*
 * Wraps an IOKit device handle we did subscribe to.
 */
class device_info {

public:
  device_info(client *owner, IONotificationPortRef notification_port, io_service_t service);
  virtual ~device_info();
  virtual void destroy() = 0;

  client *get_owner() const { return owner; }

protected:
  virtual void notify(io_service_t service, natural_t msg_type, void *msg_arg) = 0;

private:
  static void notify_cb(void *p, io_service_t service, natural_t msg_type, void *msg_arg);

  LIST_ENTRY(device_info)    device_list_entry;
  io_object_t                notification;
  client                    *owner;
};


device_info::device_info(
    client *owner_,
    IONotificationPortRef notification_port,
    io_service_t service
) {
  kern_return_t kr;

  kr = IOServiceAddInterestNotification(
         notification_port, service, kIOGeneralInterest, notify_cb, this, &notification);
  /* FIXME check kr */
  LIST_INSERT_HEAD(&owner_->device_list, this, device_list_entry);

  owner = owner_;
}


device_info::~device_info() {
  kern_return_t kr;

  LIST_REMOVE(this, device_list_entry);
  kr = IOObjectRelease(notification);
  /* FIXME check kr */
}


void device_info::notify_cb(void *p, io_service_t service, natural_t msg_type, void *msg_arg) {
  /* FIXME try catch */
  static_cast<device_info *>(p)->notify(service, msg_type, msg_arg);
}


client::client(int listen_fd) {

  sockaddr_storage addr;
  socklen_t len = sizeof addr;

  LIST_INIT(&device_list);

  sock = accept(listen_fd, reinterpret_cast<sockaddr *>(&addr), &len);
  if (sock==-1) {
    throw out_of_clients();
  }

  fcntl(sock, F_SETFL, O_NONBLOCK); /* FIXME */

  sock_dispatch = dispatch_source_create(DISPATCH_SOURCE_TYPE_WRITE,
                                         sock, 0, dispatch_get_main_queue());
  if (!sock_dispatch) {
    close(sock);
    throw -1;
    /* FIXME */
  }

  dispatch_source_set_event_handler(sock_dispatch, ^{
    if (buf.empty()) {
      dispatch_suspend(sock_dispatch);
    }

    std::tr1::array<uint8_t, 2048> buf_;
    size_t len = std::min(buf_.size(), buf.size());
    std::copy(buf.begin(), buf.begin()+len, buf_.begin());

    ssize_t s = ::write(sock, &buf_[0], len);
    if (s>=0) {
      buf.erase(buf.begin(), buf.begin()+s);
    } else {
      switch (errno) {
        case EPIPE:
          shutdown();
          break;
      }
    }
  });

  dispatch_resume(sock_dispatch);
}


client::~client() {
  device_info *p;

  while ((p = LIST_FIRST(&device_list))) {
    p->destroy();
  }

  dispatch_source_cancel(sock_dispatch);
  dispatch_release(sock_dispatch);
  close(sock); /* FIXME */
}


void client::write(const uint8_t *buf_, size_t size)
{
  bool resume = buf.empty();

  buf.insert(buf.end(), buf_, buf_ + size);
  if (resume) {
    dispatch_resume(sock_dispatch);
  }
}

