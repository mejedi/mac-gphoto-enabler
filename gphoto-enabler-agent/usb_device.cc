#include "abstract_client.hh"
#include "agent_app.hh"
#include "usb_device.hh"

#include <inttypes.h>

#include <IOKit/IOMessage.h>

GPHOTO_ENABLER_NS_BEGIN


void usb_device::add(
    agent_app *app,
    io_iterator_t service,
    uint8_t bus_num,
    uint16_t address,
    uint16_t vendor_id,
    uint16_t product_id,
    const std::string &vendor,
    const std::string &product) {

  auto dev = new usb_device(
    app->get_hub(),
    app->get_notification_port(),
    service,
    bus_num, address, vendor_id, product_id, vendor, product);

  fprintf(stderr, "add    usb:%03d,%03d %s %s\n", bus_num, address, vendor.c_str(), product.c_str());

  app->get_hub()->broadcast(dev->format_notification(USB_DEVICE_ADD));
}


void usb_device::destroy() {
  delete this;
}


void usb_device::write_snapshot(abstract_client *cl) {
  cl->write(this->format_notification(USB_DEVICE_ADD));
}


usb_device::usb_device(
  hub *owner,
  IONotificationPortRef notification_port,
  io_service_t service,
  uint8_t bus_num_,
  uint16_t address_,
  uint16_t vendor_id_,
  uint16_t product_id_,
  const std::string &vendor_,
  const std::string &product_)

: abstract_device(owner, notification_port, service),
  bus_num(bus_num_), address(address_),
  vendor_id(vendor_id_), product_id(product_id_),
  vendor(vendor_), product(product_) {

  ;
}


void usb_device::notify(io_service_t service, natural_t msg_type, void *msg_arg) {
  if (msg_type == kIOMessageServiceIsTerminated) {

    fprintf(stderr, "remove usb:%03d,%03d %s %s\n", bus_num, address, vendor.c_str(), product.c_str());
    get_owner()->broadcast(this->format_notification(USB_DEVICE_REMOVE));
    this->destroy();
  }
}


std::string usb_device::format_notification(usb_event_type t) {
  std::string buf;

  char header[128];

  snprintf(
    header, sizeof header,
    "%-6s vid:%04" PRIx16 " pid:%04" PRIx16 " usb:%03d,%03d",
    (t == USB_DEVICE_ADD ? "add": "remove"),
    vendor_id, product_id, bus_num, address);

  buf.append(header);

  if (!vendor.empty())
    buf.append(" ").append(vendor);

  if (!product.empty())
    buf.append(" ").append(product);

  buf.append("\n");

  return buf;
}


GPHOTO_ENABLER_NS_END

