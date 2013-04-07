#ifndef GPHOTO_ENABLER_USB_DEVICE_HH_
#define GPHOTO_ENABLER_USB_DEVICE_HH_

#include "agent_app.hh"
#include "abstract_device.hh"

GPHOTO_ENABLER_NS_BEGIN


enum usb_event_type {
  USB_DEVICE_ADD,
  USB_DEVICE_REMOVE
};


class usb_device: public abstract_device {
public:
  static void add(
      agent_app *app,
      io_iterator_t service,
      uint8_t bus_num,
      uint16_t address,
      uint16_t vendor_id,
      uint16_t product_id,
      const std::string &vendor,
      const std::string &product);

  virtual void destroy();

  virtual void write_snapshot(abstract_client *cl);

private:
  usb_device(
    hub *owner,
    IONotificationPortRef notification_port,
    io_service_t service,
    uint8_t bus_num,
    uint16_t address,
    uint16_t vendor_id,
    uint16_t product_id,
    const std::string &vendor,
    const std::string &product);

  virtual void notify(io_service_t service, natural_t msg_type, void *msg_arg);

  std::string format_notification(usb_event_type t);

  uint8_t                    bus_num;
  uint16_t                   address;
  uint16_t                   vendor_id;
  uint16_t                   product_id;
  std::string                vendor;
  std::string                product;
};


GPHOTO_ENABLER_NS_END

#endif

