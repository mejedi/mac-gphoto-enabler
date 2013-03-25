#ifndef GPHOTO_ENABLER_CLIENT_HH_
#define GPHOTO_ENABLER_CLIENT_HH_

#include "agent_app.hh"
#include "abstract_client.hh"

GPHOTO_ENABLER_NS_BEGIN


class client: public abstract_client {

public:
  static void add(agent_app *app, int listen_fd);

private:
  client(agent_app *app, int listen_fd);
  virtual void destroy();

  agent_app                 *owner;
};


GPHOTO_ENABLER_NS_END

#endif

