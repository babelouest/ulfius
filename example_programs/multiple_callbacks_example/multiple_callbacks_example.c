/**
 * 
 * Ulfius Framework example program
 * 
 * This example program describes the multiple callback feature
 * that are available in ulfius 2.0
 * 
 * Copyright 2017 Nicolas Mora <mail@babelouest.org>
 * 
 * License MIT
 *
 */

#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <ulfius.h>

#define PORT 6875
#define PREFIX "/multiple"

#define USER           "test"
#define PASSWORD       "testpassword"
#define DEFAULT_REALM  "default_realm"
#define SPECIFIC_REALM "specific_realm"

/**
 * callback functions declaration
 */
int callback_multiple_level_zero (const struct _u_request * request, struct _u_response * response, void * user_data);

int callback_multiple_level_one (const struct _u_request * request, struct _u_response * response, void * user_data);

int callback_multiple_level_two (const struct _u_request * request, struct _u_response * response, void * user_data);

int callback_multiple_level_three (const struct _u_request * request, struct _u_response * response, void * user_data);

int callback_multiple_level_one_complete (const struct _u_request * request, struct _u_response * response, void * user_data);

int callback_multiple_level_final (const struct _u_request * request, struct _u_response * response, void * user_data);

int callback_multiple_level_auth_check (const struct _u_request * request, struct _u_response * response, void * user_data);

int callback_multiple_level_auth_data (const struct _u_request * request, struct _u_response * response, void * user_data);

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
      line = o_malloc((len+1)*sizeof(char));
      snprintf(line, (len+1), "key is %s, value is %s", keys[i], value);
      if (to_return != NULL) {
        len = o_strlen(to_return) + o_strlen(line) + 1;
        to_return = o_realloc(to_return, (len+1)*sizeof(char));
        if (o_strlen(to_return) > 0) {
          strcat(to_return, "\n");
        }
      } else {
        to_return = o_malloc((o_strlen(line) + 1)*sizeof(char));
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

int main (int argc, char **argv) {
  int ret;
  
  // Set the framework port number
  struct _u_instance instance;
  
  y_init_logs("multiple_callbacks_example", Y_LOG_MODE_CONSOLE, Y_LOG_LEVEL_DEBUG, NULL, "Starting multiple_callbacks_example");
  
  if (ulfius_init_instance(&instance, PORT, NULL, DEFAULT_REALM) != U_OK) {
    y_log_message(Y_LOG_LEVEL_ERROR, "Error ulfius_init_instance, abort");
    return(1);
  }
  
  u_map_put(instance.default_headers, "Access-Control-Allow-Origin", "*");
  
  // Maximum body size sent by the client is 1 Kb
  instance.max_post_body_size = 1024;
  
  // Endpoint list declaration
  ulfius_add_endpoint_by_val(&instance, "*", PREFIX, "/zero/*", 99, &callback_multiple_level_final, NULL);
  
  ulfius_add_endpoint_by_val(&instance, "GET", PREFIX, "/zero/*", 0, &callback_multiple_level_zero, NULL);
  ulfius_add_endpoint_by_val(&instance, "GET", PREFIX, "/zero/one/*", 1, &callback_multiple_level_one, NULL);
  ulfius_add_endpoint_by_val(&instance, "GET", PREFIX, "/zero/one/two/*", 2, &callback_multiple_level_two, NULL);
  ulfius_add_endpoint_by_val(&instance, "GET", PREFIX, "/zero/one/two/three/", 3, &callback_multiple_level_three, NULL);
  
  ulfius_add_endpoint_by_val(&instance, "GET", PREFIX, "/zero/onec/*", 1, &callback_multiple_level_one_complete, NULL);
  ulfius_add_endpoint_by_val(&instance, "GET", PREFIX, "/zero/onec/two/*", 2, &callback_multiple_level_two, NULL);
  
  ulfius_add_endpoint_by_val(&instance, "GET", PREFIX, "/auth/*", 1, &callback_multiple_level_auth_check, NULL);
  ulfius_add_endpoint_by_val(&instance, "PUT", PREFIX, "/auth/*", 1, &callback_multiple_level_auth_check, NULL);
  ulfius_add_endpoint_by_val(&instance, "*", PREFIX, "/auth/data/", 2, &callback_multiple_level_auth_data, NULL);
  
  // default_endpoint declaration
  ulfius_set_default_endpoint(&instance, &callback_default, NULL);
  
  // Start the framework
  ret = ulfius_start_framework(&instance);
  
  if (ret == U_OK) {
    y_log_message(Y_LOG_LEVEL_DEBUG, "Start framework on port %d", instance.port);
    
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
 * Callback function of level zero
 */
int callback_multiple_level_zero (const struct _u_request * request, struct _u_response * response, void * user_data) {
  ulfius_set_string_body_response(response, 200, "Level zero");
  response->shared_data = o_strdup("shared from level zero");
  return U_CALLBACK_CONTINUE;
}

/**
 * Callback function of level one
 */
int callback_multiple_level_one (const struct _u_request * request, struct _u_response * response, void * user_data) {
  char old_body[response->binary_body_length + 1], * new_body;
  
  memcpy(old_body, response->binary_body, response->binary_body_length);
  old_body[response->binary_body_length] = '\0';
  new_body = msprintf("%s\n%s", old_body, "Level one");
  ulfius_set_string_body_response(response, 200, new_body);
  o_free(new_body);
  y_log_message(Y_LOG_LEVEL_DEBUG, "shared_data is %s", response->shared_data);
  return U_CALLBACK_CONTINUE;
}

/**
 * Callback function of level one with complete return
 */
int callback_multiple_level_one_complete (const struct _u_request * request, struct _u_response * response, void * user_data) {
  char old_body[response->binary_body_length + 1], * new_body;
  
  memcpy(old_body, response->binary_body, response->binary_body_length);
  old_body[response->binary_body_length] = '\0';
  new_body = msprintf("%s\n%s", old_body, "Level one");
  ulfius_set_string_body_response(response, 200, new_body);
  o_free(new_body);
  y_log_message(Y_LOG_LEVEL_DEBUG, "shared_data is %s", response->shared_data);
  o_free(response->shared_data);
  return U_CALLBACK_COMPLETE;
}

/**
 * Callback function of level two
 */
int callback_multiple_level_two (const struct _u_request * request, struct _u_response * response, void * user_data) {
  char old_body[response->binary_body_length + 1], * new_body;
  
  memcpy(old_body, response->binary_body, response->binary_body_length);
  old_body[response->binary_body_length] = '\0';
  new_body = msprintf("%s\n%s", old_body, "Level two");
  ulfius_set_string_body_response(response, 200, new_body);
  o_free(new_body);
  y_log_message(Y_LOG_LEVEL_DEBUG, "shared_data is %s", response->shared_data);
  return U_CALLBACK_CONTINUE;
}

/**
 * Callback function of level three
 */
int callback_multiple_level_three (const struct _u_request * request, struct _u_response * response, void * user_data) {
  char old_body[response->binary_body_length + 1], * new_body;
  
  memcpy(old_body, response->binary_body, response->binary_body_length);
  old_body[response->binary_body_length] = '\0';
  new_body = msprintf("%s\n%s", old_body, "Level three");
  ulfius_set_string_body_response(response, 200, new_body);
  o_free(new_body);
  y_log_message(Y_LOG_LEVEL_DEBUG, "shared_data is %s", response->shared_data);
  return U_CALLBACK_CONTINUE;
}

/**
 * Final callback function called on multiple calls
 */
int callback_multiple_level_final (const struct _u_request * request, struct _u_response * response, void * user_data) {
  o_free(response->shared_data);
  return U_CALLBACK_COMPLETE;
}

/**
 * Check authentication
 */
int callback_multiple_level_auth_check (const struct _u_request * request, struct _u_response * response, void * user_data) {
  y_log_message(Y_LOG_LEVEL_DEBUG, "basic auth user: %s", request->auth_basic_user);
  y_log_message(Y_LOG_LEVEL_DEBUG, "basic auth password: %s", request->auth_basic_password);
  if (request->auth_basic_user != NULL && request->auth_basic_password != NULL && 
      0 == o_strcmp(request->auth_basic_user, USER) && 0 == o_strcmp(request->auth_basic_password, PASSWORD)) {
    return U_CALLBACK_CONTINUE;
  } else {
    if (0 == o_strcmp("PUT", request->http_verb)) {
      response->auth_realm = o_strdup(SPECIFIC_REALM);
    }
    ulfius_set_string_body_response(response, 401, "Error authentication");
    return U_CALLBACK_UNAUTHORIZED;
  }
}

/**
 * Send Hello World!
 */
int callback_multiple_level_auth_data (const struct _u_request * request, struct _u_response * response, void * user_data) {
  ulfius_set_string_body_response(response, 200, "Hello World!");
  return U_CALLBACK_CONTINUE;
}

/**
 * Default callback function called if no endpoint has a match
 */
int callback_default (const struct _u_request * request, struct _u_response * response, void * user_data) {
  ulfius_set_string_body_response(response, 404, "Page not found, do what you want");
  return U_CALLBACK_CONTINUE;
}
