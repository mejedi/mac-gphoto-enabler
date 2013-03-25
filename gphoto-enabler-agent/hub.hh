#ifndef GPHOTO_ENABLER_HUB_HH_
#define GPHOTO_ENABLER_HUB_HH_

#include "utility.hh"

#include <sys/queue.h>

#include <string>

GPHOTO_ENABLER_NS_BEGIN


class abstract_client;

class abstract_device;

LIST_HEAD(client_list_head, abstract_client);

TAILQ_HEAD(device_list_head, abstract_device);


/*
 * Hub owns clients and devices.  Hub implements broadcast() service to
 * deliver a message to all clients.  Write_snapshot() method creates a
 * snapshot of the device list and sends it to the specified client.
 */
class hub: non_copyable {
  friend class abstract_device;
  friend class abstract_client;

public:
  hub();
  ~hub();
  void broadcast(const std::string &message);
  void write_snapshot(abstract_client *cl);

private:
  device_list_head           device_list;
  client_list_head           client_list;
};


GPHOTO_ENABLER_NS_END

#endif

