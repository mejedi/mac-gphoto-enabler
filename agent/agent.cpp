#include <deque>
#include <stdint.h>

#include <CoreFoundation/CoreFoundation.h>

#include <IOKit/IOKitLib.h>
#include <IOKit/IOMessage.h>
#include <IOKit/IOCFPlugIn.h>
#include <IOKit/usb/IOUSBLib.h>

#include <sys/queue.h>

class client;
class device_info;
LIST_HEAD(device_list_head, device_info);


/*
 *
 */
class client {

public:
  client(int listen_fd);
  ~client();

private:
  int                        socket;
  std::deque<uint8_t>        buf;
 
  // io_iterator_t              notification;
  device_list_head           device_list;  
};


/*
 *
 */
class device_info {

public:
  device_info(IONotificationPortRef notification_port, io_service_t service, device_list_head *p_head);
  ~device_info();
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
    IONotificationPortRef notification_port,
    io_service_t service,
    device_list_head *p_head
) {
  kern_return_t kr;

  kr = IOServiceAddInterestNotification(
         notification_port, service, kIOGeneralInterest, notify_cb, this, &notification);
  /* FIXME check kr */
  LIST_INSERT_HEAD(p_head, this, device_list_entry); 
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
}


client::~client() {
  device_info *p;

  while ((p = LIST_FIRST(&device_list))) {
    p->destroy();
  }

}
