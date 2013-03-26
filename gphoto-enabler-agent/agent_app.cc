#include "agent_app.hh"
#include "client.hh"
#include "usb_device.hh"

#include <CoreFoundation/CoreFoundation.h>

#include <IOKit/usb/IOUSBLib.h>

GPHOTO_ENABLER_NS_BEGIN


namespace {
  bool std_string_from_cfstring(std::string &dest, CFStringRef src) {
    const auto encoding = kCFStringEncodingUTF8;
    const char *p = CFStringGetCStringPtr(src, encoding);

    if (p) {
      dest = p;
      return true;
    } else {
      CFIndex len = CFStringGetLength(src);
      CFIndex buf_size = CFStringGetMaximumSizeForEncoding(len, encoding);
      std::vector<char> buf(buf_size + 1);

      if (CFStringGetCString(src, &buf[0], buf.size(), encoding)) {
        dest = &buf[0];
        return true;
      }
    }
    return false;
  }
}


agent_app::agent_app() {
  clients_count = 0;
  notification_port = IONotificationPortCreate(kIOMasterPortDefault);
  /* FIXME */
  IONotificationPortSetDispatchQueue(notification_port, dispatch_get_main_queue());
}


agent_app::~agent_app() {
  IONotificationPortDestroy(notification_port);
}


void agent_app::client_did_connect() {
  fprintf(stderr, "client did connect\n");
  if (clients_count++ == 0) {
    fprintf(stderr, "first client connected\n");
  }
}


void agent_app::client_will_disconnect() {
  fprintf(stderr, "client will disconnect\n");
  if (--clients_count == 0) {
    fprintf(stderr, "last client disconnected\n");
  }
}


void agent_app::setup_device_matching() {
  kern_return_t kr;
  io_iterator_t iter;
  CFMutableDictionaryRef matching_dict;

  matching_dict = IOServiceMatching(kIOUSBDeviceClassName);
  /* FIXME error check */

  kr = IOServiceAddMatchingNotification(
         get_notification_port(),
         kIOFirstMatchNotification,
         matching_dict,
         init_devices_cb,
         this,
         &iter);
  /* FIXME error check */

  init_devices(iter);
}


void agent_app::setup_launchd_listeners() {
  launch_data_t checkin_request;
  launch_data_t checkin_response;
  launch_data_t sockets_dict;

  checkin_request = launch_data_new_string(LAUNCH_KEY_CHECKIN);
  if (!checkin_request) {
    return;
  }

  checkin_response = launch_msg(checkin_request);
  launch_data_free(checkin_request);
  checkin_request = NULL;

  if (!checkin_response) {
    fprintf(stderr, "launchd error\n");
    return;
  }

  if (LAUNCH_DATA_ERRNO == launch_data_get_type(checkin_response)) {
    int error = launch_data_get_errno(checkin_response);
    fprintf(stderr, "launchd error: %s\n", strerror(error));
    return;
  }

  sockets_dict = launch_data_dict_lookup(checkin_response, LAUNCH_JOBKEY_SOCKETS);
  if (sockets_dict) {
    launch_data_dict_iterate(sockets_dict, setup_launchd_listeners_helper, this);
  }

  launch_data_free(checkin_response);
}


void agent_app::init_devices(io_iterator_t iter) {
  kern_return_t kr;
  io_iterator_t service;
  CFMutableDictionaryRef props = NULL;

  CFTypeRef v;
  int32_t i32;
  int64_t i64;

  while ((service = IOIteratorNext(iter))) {

    kr = IORegistryEntryCreateCFProperties(service, &props, NULL, 0);
    if (kr == KERN_SUCCESS) {

      uint32_t location_id = 0;
      uint16_t usb_address = 0;
      uint16_t vendor_id = 0;
      uint16_t product_id = 0;

      std::string vendor;
      std::string product;

      /* location_id */
      if (CFDictionaryGetValueIfPresent(props, CFSTR(kUSBDevicePropertyLocationID), &v)
          && CFGetTypeID(v) == CFNumberGetTypeID()
          && CFNumberGetValue((CFNumberRef)v, kCFNumberSInt64Type, &i64)
          && i64 < UINT32_MAX) {

        location_id = (uint32_t)i64;
      }

      /* usb_address */
      if (CFDictionaryGetValueIfPresent(props, CFSTR(kUSBDevicePropertyAddress), &v)
          && CFGetTypeID(v) == CFNumberGetTypeID()
          && CFNumberGetValue((CFNumberRef)v, kCFNumberSInt32Type, &i32)
          && i32 <= UINT16_MAX) {

        usb_address = i32;
      }

      /* vendor_id */
      if (CFDictionaryGetValueIfPresent(props, CFSTR(kUSBVendorID), &v)
          && CFGetTypeID(v) == CFNumberGetTypeID()
          && CFNumberGetValue((CFNumberRef)v, kCFNumberSInt32Type, &i32)
          && i32 <= UINT16_MAX) {

        vendor_id = i32;
      }

      /* product_id */
      if (CFDictionaryGetValueIfPresent(props, CFSTR(kUSBProductID), &v)
          && CFGetTypeID(v) == CFNumberGetTypeID()
          && CFNumberGetValue((CFNumberRef)v, kCFNumberSInt32Type, &i32)
          && i32 <= UINT16_MAX) {

        product_id = i32;
      }

      /* vendor */
      if (CFDictionaryGetValueIfPresent(props, CFSTR(kUSBVendorString), &v)
          && CFGetTypeID(v) == CFStringGetTypeID()) {

        std_string_from_cfstring(vendor, (CFStringRef)v);
      }

      /* product */
      if (CFDictionaryGetValueIfPresent(props, CFSTR(kUSBProductString), &v)
          && CFGetTypeID(v) == CFStringGetTypeID()) {

        std_string_from_cfstring(product, (CFStringRef)v);
      }

      /* no more attributes */

      if (check_vid_pid(vendor_id, product_id)) {
        usb_device::add(this, service, location_id>>24, usb_address, vendor, product);
      }

      CFRelease(props);
      props = NULL;
    }

    IOObjectRelease(service);
  }
}


void agent_app::init_devices_cb(void *p, io_iterator_t iter) {
  static_cast<agent_app *>(p)->init_devices(iter);
}


void agent_app::setup_launchd_listeners_helper(
  launch_data_t sockets, const char *name, void *p) {

  auto *self = static_cast<agent_app *>(p);
  size_t i, n = launch_data_array_get_count(sockets);

  for (i = 0; i < n; i++) {
    int fd = launch_data_get_fd(launch_data_array_get_index(sockets, i));

    if (fd == -1) {
      continue;
    }

    fprintf(stderr, "add listener (%s)\n", name);

    std::unique_ptr<dispatch_source> h (
      new dispatch_source(DISPATCH_SOURCE_TYPE_READ, fd));

    h->set_event_handler(^{

      try {
        client::add(self, fd);
      }
      catch (out_of_clients &) {
        /* OK, no more clients */
      }
    });

    h->resume();
    self->other_sources.push_back(std::move(h));
  }
}


GPHOTO_ENABLER_NS_END

