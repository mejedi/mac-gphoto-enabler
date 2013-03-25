#include "dispatch_source.hh"

GPHOTO_ENABLER_NS_BEGIN


dispatch_source::dispatch_source (
  dispatch_source_type_t type,
  uintptr_t handle,
  unsigned long mask,
  dispatch_queue_t queue) {

  source = dispatch_source_create(type, handle, mask, (queue ? queue : dispatch_get_main_queue()));
  /* FIXME error check */
  is_suspended = true;
}


dispatch_source::~dispatch_source() {
  dispatch_source_cancel(source);
  resume();
  dispatch_release(source);
}


void dispatch_source::resume() {
  if (is_suspended) {
    dispatch_resume(source);
    is_suspended = false;
  }
}


void dispatch_source::suspend() {
  if (!is_suspended) {
    dispatch_suspend(source);
    is_suspended = true;
  }
}


void dispatch_source::set_event_handler(void (^block)(void)) {
  dispatch_source_set_event_handler(source, block);
}


GPHOTO_ENABLER_NS_END

