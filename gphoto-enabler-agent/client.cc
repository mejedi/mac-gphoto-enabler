#include "client.hh"

GPHOTO_ENABLER_NS_BEGIN


void client::add(agent_app *app, int listen_fd) {
  auto cl = new client(app, listen_fd);
  app->client_did_connect();
  app->get_hub()->write_snapshot(cl);
}


client::client(agent_app *app, int listen_fd)
: abstract_client(app->get_hub(), listen_fd), owner(app) {
  ;
}


void client::destroy() {
  owner->client_will_disconnect();
  delete this;
}


GPHOTO_ENABLER_NS_END

