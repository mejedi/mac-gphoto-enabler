#include "hub.hh"
#include "abstract_client.hh"
#include "abstract_device.hh"

GPHOTO_ENABLER_NS_BEGIN


hub::hub() {
  TAILQ_INIT(&device_list);
  LIST_INIT(&client_list);
}


hub::~hub() {
  abstract_device *d;
  abstract_client *cl;

  while ((d = TAILQ_FIRST(&device_list))) {
    d->destroy();
  }

  while ((cl = LIST_FIRST(&client_list))) {
    cl->destroy();
  }

}


void hub::broadcast(const std::string &message) {
  abstract_client *cl;

  LIST_FOREACH(cl, &client_list, client_list_entry) {
    cl->write(message);
  }
}


void hub::write_snapshot(abstract_client *cl) {
  abstract_device *d;

  TAILQ_FOREACH(d, &device_list, device_list_entry) {
    d->write_snapshot(cl);
  }
}


GPHOTO_ENABLER_NS_END

