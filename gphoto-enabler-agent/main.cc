#include "agent_app.hh"

#include <signal.h>

#include <CoreFoundation/CoreFoundation.h>

using namespace gphoto_enabler;


int main(int argc, char **argv) {

  agent_app app;

  app.setup_launchd_listeners();
  app.setup_device_matching();

  signal(SIGPIPE, SIG_IGN);

  /* SIGTERM */
  dispatch_source sigterm_source(DISPATCH_SOURCE_TYPE_SIGNAL, SIGTERM, 0);
  sigterm_source.set_event_handler(^{
    fprintf(stderr, "got SIGTERM, terminating\n");
    CFRunLoopStop(CFRunLoopGetMain());
  });
  sigterm_source.resume();
  signal(SIGTERM, SIG_IGN);

  /*
   * dispatch_main() never returns hence using CFRunLoopRun() instead
   */
  CFRunLoopRun();

  return EXIT_SUCCESS;
}

