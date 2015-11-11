/**
 * 
 * Ulfius Framework example program
 * 
 * This example program send several requests to describe
 * the function ulfius_request_http behaviour
 *  
 * Copyright 2015 Nicolas Mora <mail@babelouest.org>
 * 
 * License MIT
 *
 */

#include <string.h>
#include <jansson.h>

#include "../../src/ulfius.h"

#define PORT 7778

/**
 * decode a u_map into a string
 */
char * print_map(const struct _u_map * map) {
  char * line, * to_return = NULL, **keys, * value;
  int len, i;
  if (map != NULL) {
    keys = u_map_enum_keys(map);
    for (i=0; keys[i] != NULL; i++) {
      value = u_map_get(map, keys[i]);
      len = snprintf(NULL, 0, "key is %s, value is %s\n", keys[i], value);
      line = malloc((len+1)*sizeof(char));
      snprintf(line, (len+1), "key is %s, value is %s\n", keys[i], value);
      free(value);
      if (to_return != NULL) {
        len = strlen(to_return) + strlen(line) + 1;
        to_return = realloc(to_return, (len+1)*sizeof(char));
      } else {
        to_return = malloc((strlen(line) + 1)*sizeof(char));
        to_return[0] = 0;
      }
      strcat(to_return, line);
      free(line);
    }
    u_map_clean_enum(keys);
    return to_return;
  } else {
    return NULL;
  }
}

/**
 * Callback function that put "Hello World!" and all the data sent by the client in the response as string (http method, url, params, cookies, headers, post, json, and user specific data in the response
 */
int callback (const struct _u_request * request, struct _u_response * response, void * user_data) {
  char * url_params = print_map(request->map_url), * headers = print_map(request->map_header), * cookies = print_map(request->map_cookie), 
        * post_params = print_map(request->map_post_body), * json_params = json_dumps(request->json_body, JSON_INDENT(2));
  response->string_body = u_strdup("ok");
  response->status = 200;
  printf("Method is %s\n  url is %s\n\n  parameters from the url are \n%s\n\n  cookies are \n%s\n\n  headers are \n%s\n\n  post parameters are \n%s\n\n  json body parameters are \n%s\n\n  user data is %s\n\n",
                                  request->http_verb, request->http_url, url_params, cookies, headers, post_params, json_params, (char *)user_data);
  
  ulfius_add_cookie_to_response(response, "cook", "ie", NULL, 5000, NULL, NULL, 0, 0);
  free(url_params);
  free(headers);
  free(cookies);
  free(post_params);
  free(json_params);
  return 0;
}

void print_response(struct _u_response * response) {
  if (response != NULL) {
    char * headers = print_map(response->map_header), * json_body = json_dumps(response->json_body, JSON_INDENT(2));
    printf("protocol is\n%s\n\n  headers are \n%s\n\n  json body is \n%s\n\n  string body is \n%s\n\n",
           response->protocol, headers, json_body, (char *)response->string_body);
    free(headers);
    free(json_body);
  }
}

int main (int argc, char **argv) {
  
  struct _u_map headers;
  char * string_body = "param1=one&param2=two";
  json_t * json_body = json_object();
  json_object_set_new(json_body, "param1", json_string("one"));
  json_object_set_new(json_body, "param2", json_string("two"));
  
  struct _u_request req_list[] = {
    {"GET", "http://localhost:7778/curl/get/", NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL, 0},
    {"DELETE", "http://localhost:7778/curl/delete/", NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL, 0},
    {"POST", "http://localhost:7778/curl/post/plain/", NULL, NULL, NULL, NULL, NULL, NULL, 0, string_body, 0},
    {"POST", "http://localhost:7778/curl/post/json/", NULL, NULL, NULL, NULL, NULL, json_body, 0, NULL, 0},
    {"PUT", "http://localhost:7778/curl/put/plain", NULL, NULL, NULL, NULL, NULL, NULL, 0, string_body, 0},
    {"PUT", "http://localhost:7778/curl/put/json", NULL, NULL, NULL, NULL, NULL, json_body, 0, NULL, 0}
  };
  
  struct _u_response response;
  
  struct _u_endpoint endpoint_list[] = {
    {"GET", "/curl/*", &callback, NULL},
    {"DELETE", "/curl/*", &callback, NULL},
    {"POST", "/curl/*", &callback, NULL},
    {"PUT", "/curl/*", &callback, NULL},
    {NULL, NULL, NULL, NULL}
  };
  
  // Set the framework port number
  struct _u_instance instance;
  instance.port = PORT;
  instance.bind_address = NULL;

  u_map_init(&headers);
  
  // Start the framework
  ulfius_init_framework(&instance, endpoint_list);
  printf("Start framework on port %d\n", instance.port);
  
  printf("Press <enter> to run get test\n");
  getchar();
  ulfius_init_response(&response);
  ulfius_request_http(&req_list[0], &response);
  print_response(&response);
  ulfius_clean_response(&response);
  
  printf("Press <enter> to run delete test\n");
  getchar();
  ulfius_init_response(&response);
  ulfius_request_http(&req_list[1], &response);
  print_response(&response);
  ulfius_clean_response(&response);
  
  printf("Press <enter> to run post plain test\n");
  getchar();
  ulfius_init_response(&response);
  ulfius_request_http(&req_list[2], &response);
  print_response(&response);
  ulfius_clean_response(&response);
  
  printf("Press <enter> to run post json test\n");
  getchar();
  ulfius_init_response(&response);
  ulfius_request_http(&req_list[3], &response);
  print_response(&response);
  ulfius_clean_response(&response);
  
  printf("Press <enter> to run put plain test\n");
  getchar();
  ulfius_init_response(&response);
  ulfius_request_http(&req_list[4], &response);
  print_response(&response);
  ulfius_clean_response(&response);
  
  printf("Press <enter> to run put json test\n");
  getchar();
  ulfius_init_response(&response);
  ulfius_request_http(&req_list[5], &response);
  print_response(&response);
  ulfius_clean_response(&response);
  
  // Wait for the user to press <enter> on the console to quit the application
  printf("Press <enter> to quit test\n");
  getchar();
  printf("End framework\n");
  json_decref(json_body);
  return !ulfius_stop_framework(&instance);
}
