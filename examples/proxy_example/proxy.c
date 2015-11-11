/**
 * 
 * Ulfius Framework example program
 * 
 * This example program describes the main features 
 * that are available in a callback function
 * 
 * Copyright 2015 Nicolas Mora <mail@babelouest.org>
 * 
 * License MIT
 *
 */

#include <string.h>
#include <jansson.h>

#include "../../src/ulfius.h"

#define PORT 7799
#define PROXY_DEST "http://www.w3.org"

/**
 * callback function declaration
 */
int callback_get (const struct _u_request * request, struct _u_response * response, void * user_data);

int main (int argc, char **argv) {
  
  // Endpoint list declaration
  // The last line is mandatory to mark the end of the array
  struct _u_endpoint endpoint_list[] = {
    {"GET", "*", &callback_get, NULL},
    {NULL, NULL, NULL, NULL}
  };
  
  // Set the framework port number
  struct _u_instance instance;
  instance.port = PORT;
  instance.bind_address = NULL;
  
  // Start the framework
  ulfius_init_framework(&instance, endpoint_list);
  printf("Start framework on port %d\n", instance.port);
  printf("Go to url / to proxify %s\n", PROXY_DEST);
  
  // Wait for the user to press <enter> on the console to quit the application
  getchar();
  
  printf("End framework\n");
	return ulfius_stop_framework(&instance);
}

/**
 * Callback function that put a "Hello World!" string in the response
 */
int callback_get (const struct _u_request * request, struct _u_response * response, void * user_data) {
	struct _u_request * req = ulfius_duplicate_request(request);
	struct _u_response * res = malloc(sizeof(struct _u_response));
  ulfius_init_response(res);
  int len;
  
  free(req->http_url);
  len = snprintf(NULL, 0, "%s%s", PROXY_DEST, request->http_url);
  req->http_url = malloc((len+1)*sizeof(char));
  snprintf(req->http_url, (len+1), "%s%s", PROXY_DEST, request->http_url);
  ulfius_request_http(req, res);
  ulfius_copy_response(response, res);
  ulfius_clean_response_full(res);
  ulfius_clean_request_full(req);
  return 0;
}
