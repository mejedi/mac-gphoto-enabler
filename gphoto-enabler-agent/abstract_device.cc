#include "abstract_device.hh"

GPHOTO_ENABLER_NS_BEGIN


abstract_device::abstract_device(
  hub *owner_,
  IONotificationPortRef notification_port,
  io_service_t service
) {

  kern_return_t kr;

  kr = IOServiceAddInterestNotification(
         notification_port, service, kIOGeneralInterest, notify_cb, this, &notification);
  /* FIXME check kr */

  TAILQ_INSERT_TAIL(&owner_->device_list, this, device_list_entry);
  owner = owner_;
}


abstract_device::~abstract_device() {
  kern_return_t kr;

  TAILQ_REMOVE(&owner->device_list, this, device_list_entry);

  kr = IOObjectRelease(notification);
  /* FIXME check kr */
}


void abstract_device::notify_cb(void *p, io_service_t service, natural_t msg_type, void *msg_arg) {
  /* FIXME try catch */
  static_cast<abstract_device *>(p)->notify(service, msg_type, msg_arg);
}


GPHOTO_ENABLER_NS_END

