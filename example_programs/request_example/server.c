/**
 * 
 * Ulfius Framework example program
 * 
 * This example program send several requests to describe
 * the function ulfius_request_http behaviour
 *  
 * Copyright 2015-2017 Nicolas Mora <mail@babelouest.org>
 * 
 * License MIT
 *
 */

#include <string.h>
#include <jansson.h>

#define U_DISABLE_WEBSOCKET
#include "../../src/ulfius.h"

#define PORT 7778
#define PREFIX "/curl"

/**
 * decode a u_map into a string
 */
char * print_map(const struct _u_map * map) {
  char * line, * to_return = NULL;
  const char **keys;
  int len, i;
  if (map != NULL) {
    keys = u_map_enum_keys(map);
    for (i=0; keys[i] != NULL; i++) {
      len = snprintf(NULL, 0, "key is %s, value is %s\n", keys[i], u_map_get(map, keys[i]));
      line = o_malloc((len+1)*sizeof(char));
      snprintf(line, (len+1), "key is %s, value is %s\n", keys[i], u_map_get(map, keys[i]));
      if (to_return != NULL) {
        len = strlen(to_return) + strlen(line) + 1;
        to_return = o_realloc(to_return, (len+1)*sizeof(char));
      } else {
        to_return = o_malloc((strlen(line) + 1)*sizeof(char));
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

/**
 * Callback function that put "Hello World!" and all the data sent by the client in the response as string (http method, url, params, cookies, headers, post, json, and user specific data in the response
 */
int callback (const struct _u_request * request, struct _u_response * response, void * user_data) {
  char * url_params = print_map(request->map_url), * headers = print_map(request->map_header), * cookies = print_map(request->map_cookie), 
        * post_params = print_map(request->map_post_body);
  char request_body[request->binary_body_length + 1];
  strncpy(request_body, request->binary_body, request->binary_body_length);
  request_body[request->binary_body_length] = '\0';
  response->binary_body = o_strdup("ok");
  response->binary_body_length = strlen("ok");
  response->status = 200;
  printf("######################################################\n###################### Callback ######################\n######################################################\n\nMethod is %s\n  url is %s\n\n  parameters from the url are \n%s\n\n  cookies are \n%s\n\n  headers are \n%s\n\n  post parameters are \n%s\n\n  raw body is \n%s\n\n  user data is %s\n\n",
                                  request->http_verb, request->http_url, url_params, cookies, headers, post_params, request_body, (char *)user_data);
  
  ulfius_add_cookie_to_response(response, "cook", "ie", NULL, 5000, NULL, NULL, 0, 0);
  u_map_put(response->map_header, "multiple-key", "value 1");
  u_map_put(response->map_header, "Multiple-Key", "value 2");
  o_free(url_params);
  o_free(headers);
  o_free(cookies);
  o_free(post_params);
  return U_CALLBACK_CONTINUE;
}

int main (int argc, char **argv) {
  
  // Initialize the instance
  struct _u_instance instance;
  
  if (ulfius_init_instance(&instance, PORT, NULL, NULL) != U_OK) {
    y_log_message(Y_LOG_LEVEL_ERROR, "Error ulfius_init_instance, abort");
    return(1);
  }
  
  // Endpoint list declaration
  ulfius_add_endpoint_by_val(&instance, "*", PREFIX, "*", 0, &callback, NULL);
  
  // Start the framework
  if (ulfius_start_framework(&instance) == U_OK) {
    printf("Start framework on port %d\n", instance.port);
    
    // Wait for the user to press <enter> on the console to quit the application
    printf("Press <enter> to quit server\n");
    getchar();
  } else {
    printf("Error starting framework\n");
  }

  printf("End framework\n");
  ulfius_stop_framework(&instance);
  ulfius_clean_instance(&instance);
  
  return 0;
}
