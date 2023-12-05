#include <cstdio>

#include "zenoh.h"

int main()
{
  z_owned_config_t config = zc_config_from_file("zenoh-session-config.json");

  z_owned_session_t session;

  session = z_open(z_move(config));

  z_close(z_move(session));

  return 0;
}
