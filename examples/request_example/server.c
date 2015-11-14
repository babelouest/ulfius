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

int main (int argc, char **argv) {
  
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

  // Start the framework
  ulfius_init_framework(&instance, endpoint_list);
  printf("Start framework on port %d\n", instance.port);
  
  // Wait for the user to press <enter> on the console to quit the application
  printf("Press <enter> to quit server\n");
  getchar();
  printf("End framework\n");
  return !ulfius_stop_framework(&instance);
}
