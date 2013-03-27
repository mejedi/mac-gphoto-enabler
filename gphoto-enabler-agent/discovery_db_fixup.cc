#include "discovery_db_fixup.hh"

#include <limits.h>
#include <unistd.h>

GPHOTO_ENABLER_NS_BEGIN


std::unique_ptr<discovery_db_fixup> discovery_db_fixup::create() {
  char path_buf[PATH_MAX];

  snprintf(
    path_buf,
    sizeof path_buf,
    "/Library/Caches/com.apple.ImageCaptureNotifications.DeviceDiscoveryDatabase.%d",
    (int)getuid());

  return std::unique_ptr<discovery_db_fixup> (
    new discovery_db_fixup(path_buf));
}


discovery_db_fixup::discovery_db_fixup(const std::string &db_file_path_):
  db_file_path(db_file_path_), did_apply_patch(false) {

  fprintf(
    stderr, "patching device discovery database (%s)\n",
    db_file_path.c_str());

  did_apply_patch = true;
}


discovery_db_fixup::~discovery_db_fixup() {

  if (!did_apply_patch)
    return;

  fprintf(
    stderr,
    "reverting patch for device discovery database (%s)\n",
    db_file_path.c_str());
}


GPHOTO_ENABLER_NS_END

