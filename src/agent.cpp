#include <fmt/core.h>

#include <server.h>

int main() {
  // read config

  // choose port

  // if lighthouse:
  //   start server
  Server server(41020);
  server.port(8080).run();
  // else:
  //   use client (with pki defined in config) to fetch static host map

  // fill in blanks in config (listen port, static host map)
  // run nebula child

  // on SIGHUP or several hour timeout, re-run config building step
  // on any config change, SIGHUP child

  return 0;
}
