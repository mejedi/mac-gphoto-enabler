#ifndef GPHOTO_ENABLER_DISCOVERY_DB_FIXUP_
#define GPHOTO_ENABLER_DISCOVERY_DB_FIXUP_

#include "utility.hh"

#include <memory>
#include <string>

GPHOTO_ENABLER_NS_BEGIN


class discovery_db_fixup: non_copyable {

public:
  static std::unique_ptr<discovery_db_fixup> create();
  ~discovery_db_fixup();

private:
  discovery_db_fixup(const std::string &db_file_path);

  std::string                db_file_path;
};


GPHOTO_ENABLER_NS_END

#endif

