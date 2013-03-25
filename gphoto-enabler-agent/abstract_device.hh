#ifndef GPHOTO_ENABLER_ABSTRACT_DEVICE_HH_
#define GPHOTO_ENABLER_ABSTRACT_DEVICE_HH_

#include "utility.hh"
#include "hub.hh"

#include <IOKit/IOKitLib.h>

GPHOTO_ENABLER_NS_BEGIN


/*
 * Wraps an IOKit device handle we did subscribe to.
 */
class abstract_device: non_copyable {
  friend class hub;

public:
  virtual ~abstract_device();
  virtual void destroy() = 0;
  virtual void write_snapshot(abstract_client *) = 0;

  hub *get_owner() const { return owner; }

protected:
  abstract_device(
    hub *owner,
    IONotificationPortRef notification_port,
    io_service_t service);

  virtual void notify(
    io_service_t service,
    natural_t msg_type,
    void *msg_arg) = 0;

private:
  static void notify_cb(
    void *p,
    io_service_t service,
    natural_t msg_type, void *msg_arg);

  TAILQ_ENTRY(abstract_device)
                             device_list_entry;
  hub                       *owner;
  io_object_t                notification;
};


GPHOTO_ENABLER_NS_END

#endif

