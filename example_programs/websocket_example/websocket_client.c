/**
 * 
 * Ulfius Framework example program
 * 
 * This example program implements a websocket
 * 
 * Copyright 2017 Nicolas Mora <mail@babelouest.org>
 * 
 * License MIT
 *
 */

#include <string.h>
#include <unistd.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <orcania.h>
#include <yder.h>

#define U_DISABLE_JANSSON
#define U_DISABLE_CURL
#include "../../include/ulfius.h"

int main(int argc, char ** argv) {
  struct _u_request request;
  
  y_init_logs("websocket_client", Y_LOG_MODE_CONSOLE, Y_LOG_LEVEL_DEBUG, NULL, "Starting websocket_client");
  ulfius_init_request(&request);
  ulfius_init_websocket_request(&request, "http://localhost/websocket?arg=1#hash=3", "my_proto", NULL);
  ulfius_open_websocket_client_connection(&request, NULL, NULL, NULL, NULL, NULL, NULL);
  
  ulfius_clean_request(&request);
  y_close_logs();
  return 0;
}
