#ifndef GPHOTO_ENABLER_AGENT_APP_HH_
#define GPHOTO_ENABLER_AGENT_APP_HH_

#include "utility.hh"
#include "hub.hh"
#include "usb_device_checker.hh"
#include "dispatch_source.hh"

#include <IOKit/IOKitLib.h>

#include <launch.h>

#include <vector>
#include <memory>


GPHOTO_ENABLER_NS_BEGIN


/*
 * Primary application class
 */
class agent_app {

public:
  agent_app();
  ~agent_app();

  void client_did_connect();
  void client_will_disconnect();
  void setup_device_matching();
  void setup_launchd_listeners();

  bool check_vid_pid(int vid, int pid) {
    return checker.check_vid_pid(vid, pid);
  }

  hub *get_hub() { return &hub; }

  IONotificationPortRef get_notification_port() { return notification_port; }

private:
  void init_devices(io_iterator_t iter);

  static void init_devices_cb(void *p, io_iterator_t iter);

  static void setup_launchd_listeners_helper(
    launch_data_t sockets, const char *name, void *p);

  int                        clients_count;
  hub                        hub;
  IONotificationPortRef      notification_port;
  usb_device_checker         checker;
  // dispatch_source         timed_killer;
  std::vector<std::unique_ptr<dispatch_source>>
                             other_sources;
};


GPHOTO_ENABLER_NS_END

#endif

