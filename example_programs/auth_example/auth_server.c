/**
 * 
 * Ulfius Framework example program
 * 
 * This program implements basic auth example
 *  
 * Copyright 2016-2017 Nicolas Mora <mail@babelouest.org>
 * 
 * License MIT
 *
 */

#include <stdio.h>
#include <yder.h>
#include <orcania.h>

#define U_DISABLE_JANSSON
#define U_DISABLE_CURL
#define U_DISABLE_WEBSOCKET
#include <ulfius.h>

#define PORT 2884
#define PREFIX "/auth"

#define USER "test"
#define PASSWORD "testpassword"

/**
 * Auth function for basic authentication
 */
int auth_basic (const struct _u_request * request, struct _u_response * response, void * user_data) {
  y_log_message(Y_LOG_LEVEL_DEBUG, "basic auth user: %s", request->auth_basic_user);
  y_log_message(Y_LOG_LEVEL_DEBUG, "basic auth password: %s", request->auth_basic_password);
  y_log_message(Y_LOG_LEVEL_DEBUG, "basic auth param: %s", (char *)user_data);
  if (request->auth_basic_user != NULL && request->auth_basic_password != NULL && 
      0 == o_strcmp(request->auth_basic_user, USER) && 0 == o_strcmp(request->auth_basic_password, PASSWORD)) {
    return U_CALLBACK_CONTINUE;
  } else {
    ulfius_set_string_body_response(response, 401, "Error authentication");
    return U_CALLBACK_UNAUTHORIZED;
  }
}

/**
 * Callback function for basic authentication
 */
int callback_auth_basic (const struct _u_request * request, struct _u_response * response, void * user_data) {
  ulfius_set_string_body_response(response, 200, "Basic auth callback");
  return U_CALLBACK_CONTINUE;
}

int main (int argc, char **argv) {
  // Initialize the instance
  struct _u_instance instance;
  
  y_init_logs("auth_server", Y_LOG_MODE_CONSOLE, Y_LOG_LEVEL_DEBUG, NULL, "logs start");
  
  if (ulfius_init_instance(&instance, PORT, NULL, "auth_basic_default") != U_OK) {
    printf("Error ulfius_init_instance, abort\n");
    return(1);
  }
  
  // Endpoint list declaration
  ulfius_add_endpoint_by_val(&instance, "GET", PREFIX, "/basic", 0, &auth_basic, "auth param");
  ulfius_add_endpoint_by_val(&instance, "GET", PREFIX, "/basic", 1, &callback_auth_basic, NULL);
  ulfius_add_endpoint_by_val(&instance, "GET", PREFIX, "/default", 1, &callback_auth_basic, NULL);
  ulfius_add_endpoint_by_val(&instance, "GET", PREFIX, "/default", 0, &auth_basic, NULL);
  
  // Start the framework
  if (ulfius_start_framework(&instance) == U_OK) {
    printf("Start framework on port %u\n", instance.port);
    
    // Wait for the user to press <enter> on the console to quit the application
    printf("Press <enter> to quit server\n");
    getchar();
  } else {
    printf("Error starting framework\n");
  }

  printf("End framework\n");
  ulfius_stop_framework(&instance);
  ulfius_clean_instance(&instance);
  y_close_logs();
  
  return 0;
}
