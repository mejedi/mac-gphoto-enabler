#ifndef GPHOTO_ENABLER_USB_DEVICE_CHECKER_HH_
#define GPHOTO_ENABLER_USB_DEVICE_CHECKER_HH_

#include "utility.hh"

#include <stdint.h>

#include <set>

GPHOTO_ENABLER_NS_BEGIN


/*
 * Check whether a USB device is recognized by libgphoto
 */
class usb_device_checker {

public:
  usb_device_checker();
  bool check_vid_pid(int vid, int pid);

private:
  std::set<std::pair<int,int>> known_models;
};


GPHOTO_ENABLER_NS_END

#endif

