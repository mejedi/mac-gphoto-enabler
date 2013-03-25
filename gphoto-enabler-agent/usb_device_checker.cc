#include "usb_device_checker.hh"

#include <gphoto2/gphoto2.h>

GPHOTO_ENABLER_NS_BEGIN


usb_device_checker::usb_device_checker() {
  GPContext            *context = gp_context_new();
  CameraAbilitiesList  *list = NULL;

  if (context
      && gp_abilities_list_new(&list) == GP_OK
      && gp_abilities_list_load(list, context) == GP_OK) {

    int i, n = gp_abilities_list_count(list);
    for (i = 0; i < n; i++) {

      CameraAbilities ca;
      gp_abilities_list_get_abilities(list, i, &ca);
      if (ca.port & GP_PORT_USB) {
        known_models.insert(std::pair<int,int>(ca.usb_vendor, ca.usb_product));
      }
    }
  }

  if (list)
    gp_abilities_list_free(list);
  if (context)
    gp_context_unref(context);
}


bool usb_device_checker::check_vid_pid(int vid, int pid) {
  return known_models.empty()
    || known_models.find(std::pair<int,int>(vid,pid)) != known_models.end();
}


GPHOTO_ENABLER_NS_END

