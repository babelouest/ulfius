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
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "../../src/ulfius.h"

#define PORT 8537
#define PREFIX "/test"
#define PREFIXJSON "/testjson"
#define PREFIXCOOKIE "/testcookie"

/**
 * callback functions declaration
 */
int callback_get_test (const struct _u_request * request, struct _u_response * response, void * user_data);

int callback_post_test (const struct _u_request * request, struct _u_response * response, void * user_data);

int callback_all_test_foo (const struct _u_request * request, struct _u_response * response, void * user_data);

int callback_get_jsontest (const struct _u_request * request, struct _u_response * response, void * user_data);

int callback_get_cookietest (const struct _u_request * request, struct _u_response * response, void * user_data);

/**
 * decode a u_map into a string
 */
char * print_map(const struct _u_map * map) {
  char * line, * to_return = NULL;
  const char **keys, * value;
  int len, i;
  if (map != NULL) {
    keys = u_map_enum_keys(map);
    for (i=0; keys[i] != NULL; i++) {
      value = u_map_get(map, keys[i]);
      len = snprintf(NULL, 0, "key is %s, value is %s", keys[i], value);
      line = malloc((len+1)*sizeof(char));
      snprintf(line, (len+1), "key is %s, value is %s", keys[i], value);
      if (to_return != NULL) {
        len = strlen(to_return) + strlen(line) + 1;
        to_return = realloc(to_return, (len+1)*sizeof(char));
        if (strlen(to_return) > 0) {
          strcat(to_return, "\n");
        }
      } else {
                to_return = malloc((strlen(line) + 1)*sizeof(char));
                to_return[0] = 0;
      }
      strcat(to_return, line);
      free(line);
    }
    return to_return;
  } else {
    return NULL;
  }
}

int main (int argc, char **argv) {
  
  // Endpoint list declaration
  // The last line is mandatory to mark the end of the array
  struct _u_endpoint endpoint_list[] = {
    {"GET", PREFIX, NULL, &callback_get_test, NULL},
    {"POST", PREFIX, NULL, &callback_post_test, NULL},
    {"GET", PREFIX, "/:foo", &callback_all_test_foo, "user data 1"},
    {"POST", PREFIX, "/:foo", &callback_all_test_foo, "user data 2"},
    {"PUT", PREFIX, "/:foo", &callback_all_test_foo, "user data 3"},
    {"DELETE", PREFIX, "/:foo", &callback_all_test_foo, "user data 4"},
    {"PUT", PREFIXJSON, NULL, &callback_get_jsontest, NULL},
    {"GET", PREFIXCOOKIE, "/:lang/:extra", &callback_get_cookietest, NULL},
    {NULL, NULL, NULL, NULL, NULL}
  };
  
  // Set the framework port number
  struct _u_instance instance;
  instance.port = PORT;
  instance.bind_address = NULL;
  
  y_init_logs("simple_example", Y_LOG_MODE_CONSOLE, Y_LOG_LEVEL_DEBUG, NULL, "Starting simple_example");
  
  // Start the framework
  if (ulfius_init_framework(&instance, endpoint_list) == U_OK) {
    printf("Start framework on port %d\n", instance.port);
    
    // Wait for the user to press <enter> on the console to quit the application
    getchar();
  } else {
    printf("Error starting framework\n");
  }
  printf("End framework\n");
  
  y_close_logs();
  
  return ulfius_stop_framework(&instance);
}

/**
 * Callback function that put a "Hello World!" string in the response
 */
int callback_get_test (const struct _u_request * request, struct _u_response * response, void * user_data) {
  response->string_body = strdup("Hello World!");
  response->status = 200;
  return U_OK;
}

/**
 * Callback function that put a "Hello World!" and the post parameters send by the client in the response
 */
int callback_post_test (const struct _u_request * request, struct _u_response * response, void * user_data) {
  int len;
  char * post_params = print_map(request->map_post_body);
  len = snprintf(NULL, 0, "Hello World!\n%s", post_params);
  response->string_body = malloc((len+1)*sizeof(char));
  snprintf(response->string_body, (len+1), "Hello World!\n%s", post_params);
  free(post_params);
  response->status = 200;
  return U_OK;
}

/**
 * Callback function that put "Hello World!" and all the data sent by the client in the response as string (http method, url, params, cookies, headers, post, json, and user specific data in the response
 */
int callback_all_test_foo (const struct _u_request * request, struct _u_response * response, void * user_data) {
  char * url_params = print_map(request->map_url), * headers = print_map(request->map_header), * cookies = print_map(request->map_cookie), 
        * post_params = print_map(request->map_post_body), * json_params = json_dumps(request->json_body, JSON_INDENT(2));
  int len;
  len = snprintf(NULL, 0, "Hello World!\n\n  method is %s\n  url is %s\n\n  parameters from the url are \n%s\n\n  cookies are \n%s\n\n  headers are \n%s\n\n  post parameters are \n%s\n\n  json body parameters are \n%s\n\n  user data is %s\n\nclient address is %s\n\n",
                                  request->http_verb, request->http_url, url_params, cookies, headers, post_params, json_params, (char *)user_data, inet_ntoa(((struct sockaddr_in *)request->client_address)->sin_addr));
  response->string_body = malloc((len+1)*sizeof(char));
  snprintf(response->string_body, (len+1), "Hello World!\n\n  method is %s\n  url is %s\n\n  parameters from the url are \n%s\n\n  cookies are \n%s\n\n  headers are \n%s\n\n  post parameters are \n%s\n\n  json body parameters are \n%s\n\n  user data is %s\n\nclient address is %s\n\n",
                                  request->http_verb, request->http_url, url_params, cookies, headers, post_params, json_params, (char *)user_data, inet_ntoa(((struct sockaddr_in *)request->client_address)->sin_addr));
  response->status = 200;
  
  free(url_params);
  free(headers);
  free(cookies);
  free(post_params);
  free(json_params);
  return U_OK;
}

/**
 * Callback function that put "Hello World!", the http method and the url in a json response
 */
int callback_get_jsontest (const struct _u_request * request, struct _u_response * response, void * user_data) {
  response->json_body = json_object();
  json_object_set_new(response->json_body, "message", json_string("Hello World!"));
  json_object_set_new(response->json_body, "method", json_string(request->http_verb));
  json_object_set_new(response->json_body, "url", json_string(request->http_url));
  if (!request->json_error) {
    json_object_set(response->json_body, "request", request->json_body);
  } else {
    json_object_set_new(response->json_body, "request", json_string("Error parsing request"));
  }
  response->status = 200;
  return U_OK;
}

/**
 * Callback function that sets cookies in the response
 * The counter cookie is incremented every time the client reloads this url
 */
int callback_get_cookietest (const struct _u_request * request, struct _u_response * response, void * user_data) {
  const char * lang = u_map_get(request->map_url, "lang"), * extra = u_map_get(request->map_url, "extra"), 
             * counter = u_map_get(request->map_cookie, "counter");
  char new_counter[8];
  int i_counter;
  
  if (counter == NULL) {
    i_counter = 0;
  } else {
    i_counter = strtol(counter, NULL, 10);
    i_counter++;
  }
  snprintf(new_counter, 7, "%d", i_counter);
  ulfius_add_cookie_to_response(response, "lang", lang, NULL, 0, NULL, NULL, 0, 0);
  ulfius_add_cookie_to_response(response, "extra", extra, NULL, 0, NULL, NULL, 0, 0);
  ulfius_add_cookie_to_response(response, "counter", new_counter, NULL, 0, NULL, NULL, 0, 0);
  response->string_body = strdup("Cookies set");
  
  return U_OK;
}
