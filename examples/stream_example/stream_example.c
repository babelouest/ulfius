/**
 * 
 * Ulfius Framework example program
 * 
 * This example program streams some data
 * 
 * Copyright 2016 Nicolas Mora <mail@babelouest.org>
 * 
 * License MIT
 *
 */

#include <string.h>
#include <unistd.h>
#include <inttypes.h>

#include "../../src/ulfius.h"

#define PORT 7876
#define PREFIX "/stream"

/**
 * callback functions declaration
 */
int callback_get_stream (const struct _u_request * request, struct _u_response * response, void * user_data);

ssize_t stream_data (void * cls, uint64_t pos, char *buf, size_t max);

void free_stream_data(void * cls);

int main (int argc, char **argv) {
  int ret;
  
  // Set the framework port number
  struct _u_instance instance;
  
  if (ulfius_init_instance(&instance, PORT, NULL) != U_OK) {
    printf("Error ulfius_init_instance, abort\n");
    return(1);
  }
  
  u_map_put(instance.default_headers, "Access-Control-Allow-Origin", "*");
  
  // Endpoint list declaration
  ulfius_add_endpoint_by_val(&instance, "GET", PREFIX, NULL, NULL, NULL, NULL, &callback_get_stream, NULL);
  
  // Start the framework
  ret = ulfius_start_framework(&instance);
  
  if (ret == U_OK) {
    printf("Start framework on port %d\n", instance.port);
    
    // Wait for the user to press <enter> on the console to quit the application
    getchar();
  } else {
    printf("Error starting framework\n");
  }
  printf("End framework\n");
  
  ulfius_stop_framework(&instance);
  ulfius_clean_instance(&instance);
  
  return 0;
}

/**
 * Callback function that put a "Hello World!" string in the response
 */
int callback_get_stream (const struct _u_request * request, struct _u_response * response, void * user_data) {
  ulfius_set_stream_response(response, 200, stream_data, free_stream_data, -1, 32 * 1024, strdup("stream test"));
  return U_OK;
}

ssize_t stream_data (void * cls, uint64_t pos, char * buf, size_t max) {
  printf("stream data %" PRIu64 " %zu\n", pos, max);
  sleep(1);
  if (pos <= 100) {
      snprintf(buf, max, "%s %" PRIu64 "\n", (char *)cls, pos);
      return strlen(buf);
  } else {
    return ULFIUS_STREAM_ERROR;
  }
}

void free_stream_data(void * cls) {
  printf("clean data\n");
  free(cls);
}
