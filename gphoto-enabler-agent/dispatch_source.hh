#ifndef GPHOTO_ENABLER_DISPATCH_SOURCE_HH_
#define GPHOTO_ENABLER_DISPATCH_SOURCE_HH_

#include "utility.hh"

#include <dispatch/dispatch.h>
#include <stdint.h>

GPHOTO_ENABLER_NS_BEGIN


/*
 * RAII wrapper for libdispatch dispatch_source
 */
class dispatch_source: non_copyable {

  public:
    dispatch_source(
      dispatch_source_type_t type,
      uintptr_t handle,
      unsigned long mask = 0,
      dispatch_queue_t queue = 0);

    ~dispatch_source();

    /*
     * If already resumed subsequent calls to resume() are ignored.
     * Same for suspend()
     */
    void resume();
    void suspend();

    void set_event_handler(void (^block)(void));

  private:
    dispatch_source_t   source;
    bool                is_suspended;
};


GPHOTO_ENABLER_NS_END

#endif

