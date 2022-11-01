/**
 * 
 * Ulfius Framework example program
 * 
 * This example program describes the main features 
 * that are available in a callback function
 * 
 * Copyright 2015-2022 Nicolas Mora <mail@babelouest.org>
 * 
 * License MIT
 *
 */

#ifndef _WIN32
  #include <sys/socket.h>

  #include <arpa/inet.h>
  #include <netinet/in.h>
#endif

#include <string.h>
#include <ulfius.h>

#include "u_example.h"

#define PORT 8537
#define PREFIX "/test"
#define PREFIXJSON "/testjson"
#define PREFIXCOOKIE "/testcookie"

/**
 * callback functions declaration
 */
int callback_get_test (const struct _u_request * request, struct _u_response * response, void * user_data);

int callback_get_empty_response (const struct _u_request * request, struct _u_response * response, void * user_data);

int callback_post_test (const struct _u_request * request, struct _u_response * response, void * user_data);

int callback_all_test_foo (const struct _u_request * request, struct _u_response * response, void * user_data);

int callback_get_cookietest (const struct _u_request * request, struct _u_response * response, void * user_data);

int callback_default (const struct _u_request * request, struct _u_response * response, void * user_data);

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
      line = o_malloc((size_t)(len+1));
      snprintf(line, (size_t)(len+1), "key is %s, value is %s", keys[i], value);
      if (to_return != NULL) {
        len = (int)(o_strlen(to_return) + o_strlen(line) + 1);
        to_return = o_realloc(to_return, (size_t)(len+1));
        if (o_strlen(to_return) > 0) {
          strcat(to_return, "\n");
        }
      } else {
        to_return = o_malloc((o_strlen(line) + 1));
        to_return[0] = 0;
      }
      strcat(to_return, line);
      o_free(line);
    }
    return to_return;
  } else {
    return NULL;
  }
}

static char * read_file(const char * filename) {
  char * buffer = NULL;
  long length;
  FILE * f;
  if (filename != NULL) {
    f = fopen (filename, "rb");
    if (f) {
      fseek (f, 0, SEEK_END);
      length = ftell (f);
      fseek (f, 0, SEEK_SET);
      buffer = o_malloc ((size_t)(length + 1));
      if (buffer != NULL) {
        fread (buffer, 1, (size_t)length, f);
        buffer[length] = '\0';
      }
      fclose (f);
    }
    return buffer;
  } else {
    return NULL;
  }
}

int main (int argc, char **argv) {
  int ret;
  
  // Set the framework port number
  struct _u_instance instance;
  
  y_init_logs("simple_example", Y_LOG_MODE_CONSOLE, Y_LOG_LEVEL_DEBUG, NULL, "Starting simple_example");
  
  if (ulfius_init_instance(&instance, PORT, NULL, NULL) != U_OK) {
    y_log_message(Y_LOG_LEVEL_ERROR, "Error ulfius_init_instance, abort");
    return(1);
  }
  
  u_map_put(instance.default_headers, "Access-Control-Allow-Origin", "*");
  
  // Maximum body size sent by the client is 1 Kb
  instance.max_post_body_size = 1024;
  
  // Endpoint list declaration
  ulfius_add_endpoint_by_val(&instance, "GET", PREFIX, NULL, 0, &callback_get_test, NULL);
  ulfius_add_endpoint_by_val(&instance, "GET", PREFIX, "/empty", 0, &callback_get_empty_response, NULL);
  ulfius_add_endpoint_by_val(&instance, "GET", PREFIX, "/multiple/:multiple/:multiple/:not_multiple", 0, &callback_all_test_foo, NULL);
  ulfius_add_endpoint_by_val(&instance, "POST", PREFIX, NULL, 0, &callback_post_test, NULL);
  ulfius_add_endpoint_by_val(&instance, "GET", PREFIX, "/param/:foo", 0, &callback_all_test_foo, "user data 1");
  ulfius_add_endpoint_by_val(&instance, "POST", PREFIX, "/param/:foo", 0, &callback_all_test_foo, "user data 2");
  ulfius_add_endpoint_by_val(&instance, "PUT", PREFIX, "/param/:foo", 0, &callback_all_test_foo, "user data 3");
  ulfius_add_endpoint_by_val(&instance, "DELETE", PREFIX, "/param/:foo", 0, &callback_all_test_foo, "user data 4");
  ulfius_add_endpoint_by_val(&instance, "GET", PREFIXCOOKIE, "/:lang/:extra", 0, &callback_get_cookietest, NULL);
  
  // default_endpoint declaration
  ulfius_set_default_endpoint(&instance, &callback_default, NULL);
  
  // Start the framework
  if (argc == 4 && o_strcmp("-secure", argv[1]) == 0) {
    // If command-line options are -secure <key_file> <cert_file>, then open an https connection
    char * key_pem = read_file(argv[2]), * cert_pem = read_file(argv[3]);
    ret = ulfius_start_secure_framework(&instance, key_pem, cert_pem);
    o_free(key_pem);
    o_free(cert_pem);
  } else {
    // Open an http connection
    ret = ulfius_start_framework(&instance);
  }
  
  if (ret == U_OK) {
    y_log_message(Y_LOG_LEVEL_DEBUG, "Start %sframework on port %d", ((argc == 4 && o_strcmp("-secure", argv[1]) == 0)?"secure ":""), instance.port);
    
    // Wait for the user to press <enter> on the console to quit the application
    getchar();
  } else {
    y_log_message(Y_LOG_LEVEL_DEBUG, "Error starting framework");
  }
  y_log_message(Y_LOG_LEVEL_DEBUG, "End framework");
  
  y_close_logs();
  
  ulfius_stop_framework(&instance);
  ulfius_clean_instance(&instance);
  
  return 0;
}

/**
 * Callback function that put a "Hello World!" string in the response
 */
int callback_get_test (const struct _u_request * request, struct _u_response * response, void * user_data) {
  (void)(request);
  (void)(user_data);
  ulfius_set_string_body_response(response, 200, "Hello World!");
  return U_CALLBACK_CONTINUE;
}

/**
 * Callback function that put an empty response and a status 200
 */
int callback_get_empty_response (const struct _u_request * request, struct _u_response * response, void * user_data) {
  (void)(request);
  (void)(user_data);
  (void)(response);
  return U_CALLBACK_CONTINUE;
}

/**
 * Callback function that put a "Hello World!" and the post parameters send by the client in the response
 */
int callback_post_test (const struct _u_request * request, struct _u_response * response, void * user_data) {
  char * post_params = print_map(request->map_post_body);
  char * response_body = msprintf("Hello World!\n%s", post_params);
  (void)(user_data);
  ulfius_set_string_body_response(response, 200, response_body);
  o_free(response_body);
  o_free(post_params);
  return U_CALLBACK_CONTINUE;
}

/**
 * Callback function that put "Hello World!" and all the data sent by the client in the response as string (http method, url, params, cookies, headers, post, json, and user specific data in the response
 */
int callback_all_test_foo (const struct _u_request * request, struct _u_response * response, void * user_data) {
  char * url_params = print_map(request->map_url), * headers = print_map(request->map_header), * cookies = print_map(request->map_cookie), 
        * post_params = print_map(request->map_post_body);
#ifndef _WIN32
  char * response_body = msprintf("Hello World!\n\n  method is %s\n  url is %s\n\n  parameters from the url are \n%s\n\n  cookies are \n%s\n\n  headers are \n%s\n\n  post parameters are \n%s\n\n  user data is %s\n\nclient address is %s\n\n",
                                  request->http_verb, request->http_url, url_params, cookies, headers, post_params, (char *)user_data, inet_ntoa(((struct sockaddr_in *)request->client_address)->sin_addr));
#else
  char * response_body = msprintf("Hello World!\n\n  method is %s\n  url is %s\n\n  parameters from the url are \n%s\n\n  cookies are \n%s\n\n  headers are \n%s\n\n  post parameters are \n%s\n\n  user data is %s\n\n",
                                  request->http_verb, request->http_url, url_params, cookies, headers, post_params, (char *)user_data);
#endif
  ulfius_set_string_body_response(response, 200, response_body);
  o_free(url_params);
  o_free(headers);
  o_free(cookies);
  o_free(post_params);
  o_free(response_body);
  return U_CALLBACK_CONTINUE;
}

/**
 * Callback function that sets cookies in the response
 * The counter cookie is incremented every time the client reloads this url
 */
int callback_get_cookietest (const struct _u_request * request, struct _u_response * response, void * user_data) {
  (void)(user_data);
  const char * lang = u_map_get(request->map_url, "lang"), * extra = u_map_get(request->map_url, "extra"), 
             * counter = u_map_get(request->map_cookie, "counter");
  char new_counter[8];
  int i_counter;
  
  if (counter == NULL) {
    i_counter = 0;
  } else {
    i_counter = (int)strtol(counter, NULL, 10);
    i_counter++;
  }
  snprintf(new_counter, 7, "%d", i_counter);
  ulfius_add_cookie_to_response(response, "lang", lang, NULL, 0, NULL, NULL, 0, 0);
  ulfius_add_cookie_to_response(response, "extra", extra, NULL, 0, NULL, NULL, 0, 0);
  ulfius_add_cookie_to_response(response, "counter", new_counter, NULL, 0, NULL, NULL, 0, 0);
  ulfius_set_string_body_response(response, 200, "Cookies set!");
  
  return U_CALLBACK_CONTINUE;
}

/**
 * Default callback function called if no endpoint has a match
 */
int callback_default (const struct _u_request * request, struct _u_response * response, void * user_data) {
  (void)(request);
  (void)(user_data);
  ulfius_set_string_body_response(response, 404, "Page not found, do what you want");
  return U_CALLBACK_CONTINUE;
}
