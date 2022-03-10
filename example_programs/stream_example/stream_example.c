/**
 * 
 * Ulfius Framework example program
 * 
 * This example program streams some data
 * 
 * Copyright 2016-2022 Nicolas Mora <mail@babelouest.org>
 * 
 * License MIT
 *
 */

#include <sys/stat.h>
#include <sys/types.h>

#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <ulfius.h>
#include <unistd.h>

#include "u_example.h"

#define PORT 7876
#define PREFIX "/stream"

/**
 * callback functions declaration
 */
int callback_get_stream (const struct _u_request * request, struct _u_response * response, void * user_data);

int callback_get_audio_stream (const struct _u_request * request, struct _u_response * response, void * user_data);

ssize_t stream_data (void * cls, uint64_t pos, char *buf, size_t max);
ssize_t stream_audio_file (void * cls, uint64_t pos, char *buf, size_t max);

void free_stream_data(void * cls);
void free_stream_audio_file(void * cls);

int main (int argc, char **argv) {
  int ret;
  
  // Set the framework port number
  struct _u_instance instance;
  
  if (ulfius_init_instance(&instance, PORT, NULL, NULL) != U_OK) {
    printf("Error ulfius_init_instance, abort\n");
    return(1);
  }
  
  u_map_put(instance.default_headers, "Access-Control-Allow-Origin", "*");
  
  // Endpoint list declaration
  ulfius_add_endpoint_by_val(&instance, "GET", PREFIX, NULL, 0, &callback_get_stream, NULL);
  if (argc > 1) {
    ulfius_add_endpoint_by_val(&instance, "GET", PREFIX, "/audio", 0, &callback_get_audio_stream, argv[1]);
  }
  
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
 * Callback function
 */
int callback_get_stream (const struct _u_request * request, struct _u_response * response, void * user_data) {
  ulfius_set_stream_response(response, 200, stream_data, free_stream_data, U_STREAM_SIZE_UNKNOWN, 32 * 1024, o_strdup("stream test"));
  return U_CALLBACK_CONTINUE;
}

int callback_get_audio_stream (const struct _u_request * request, struct _u_response * response, void * user_data) {
  FILE * file = fopen((char *)user_data, "rb");
  int fd;
  struct stat buf;
  printf("stream audio file\n");
  
  if (file != NULL) {

    fd = fileno (file);
    if (-1 == fd) {
      fclose (file);
      return U_ERROR; /* internal error */
    }
    if ( (0 != fstat (fd, &buf)) || (! S_ISREG (buf.st_mode)) ) {
      /* not a regular file, refuse to serve */
      fclose (file);
      file = NULL;
      return U_ERROR;
    }
    u_map_put(response->map_header, "Content-Type", "audio/mpeg");
    ulfius_set_stream_response(response, 200, stream_audio_file, free_stream_audio_file, buf.st_size, 32 * 1024, file);
    return U_CALLBACK_CONTINUE;
  } else {
    return U_CALLBACK_ERROR;
  }
}

ssize_t stream_data (void * cls, uint64_t pos, char * buf, size_t max) {
  printf("stream data %" PRIu64 " %zu\n", pos, max);
  sleep(1);
  if (pos <= 100) {
      snprintf(buf, max, "%s %" PRIu64 "\n", (char *)cls, pos);
      return o_strlen(buf);
  } else {
    return U_STREAM_END;
  }
}

ssize_t stream_audio_file (void * cls, uint64_t pos, char * buf, size_t max) {
  FILE *file = cls;

  (void) fseek (file, pos, SEEK_SET);
  return fread (buf, 1, max, file);
}

void free_stream_data(void * cls) {
  printf("clean data\n");
  o_free(cls);
}

void free_stream_audio_file(void * cls) {
  printf("clean file\n");

  FILE *file = cls;
  fclose (file);
}
