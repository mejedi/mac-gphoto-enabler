#ifndef GPHOTO_ENABLER_UTILITY_HH_
#define GPHOTO_ENABLER_UTILITY_HH_


#define GPHOTO_ENABLER_NS_BEGIN   namespace gphoto_enabler {
#define GPHOTO_ENABLER_NS_END     }

GPHOTO_ENABLER_NS_BEGIN


struct non_copyable {

  non_copyable() {}
  non_copyable(const non_copyable &) = delete;
  const non_copyable & operator=(const non_copyable &) = delete;
};


GPHOTO_ENABLER_NS_END

#endif
