/**
 * 
 * Ulfius Framework example program
 * 
 * This example program get a stream data from the server
 * 
 * Copyright 2016 Nicolas Mora <mail@babelouest.org>
 * 
 * License MIT
 *
 */

#include <stdio.h>
#include <string.h>

#define U_DISABLE_JANSSON
#define U_DISABLE_WEBSOCKET
#include "../../src/ulfius.h"

#define URL "http://localhost:7876/stream"

size_t my_write_body(void * contents, size_t size, size_t nmemb, void * user_data) {
  printf("got %s", (char *)contents);
  
  return size * nmemb;
}

size_t my_write_meta_body(void * contents, size_t size, size_t nmemb, void * user_data) {
  printf("got %zu %zu\n", size, nmemb);
  
  return size * nmemb;
}

int main(void) {
  struct _u_request request;
  struct _u_response response;
  int res;
  
  ulfius_init_response(&response);
  ulfius_init_request(&request);
  request.http_verb = o_strdup("GET");
  request.http_url = o_strdup(URL);
  
  printf("Press <enter> to run stream test\n");
  getchar();
  res = ulfius_send_http_streaming_request(&request, &response, my_write_body, NULL);
  if (res == U_OK) {
    printf("ulfius_send_http_streaming_request OK\n");
  } else {
    printf("Error in http request: %d\n", res);
  }
  ulfius_clean_response(&response);
  
  printf("Press <enter> to run stream audio test\n");
  o_free(request.http_url);
  request.http_url = o_strdup(URL "/audio");
  getchar();
  res = ulfius_send_http_streaming_request(&request, &response, my_write_meta_body, NULL);
  if (res == U_OK) {
    printf("ulfius_send_http_streaming_request OK\n");
  } else {
    printf("Error in http request: %d\n", res);
  }
  ulfius_clean_response(&response);
  
  printf("Press <enter> to run no stream test\n");
  o_free(request.http_url);
  request.http_url = o_strdup("http://www.w3.org/");
  getchar();
  res = ulfius_send_http_request(&request, &response);
  if (res == U_OK) {
    printf("ulfius_send_http_request OK\n");
  } else {
    printf("Error in http request: %d\n", res);
  }
  ulfius_clean_response(&response);
  
  ulfius_clean_request(&request);
  return 0;
}
