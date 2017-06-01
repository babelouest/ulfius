/**
 * 
 * Ulfius Framework injection_example program
 * 
 * This example program describes the endpoints injections
 * 
 * Copyright 2016-2017 Nicolas Mora <mail@babelouest.org>
 * 
 * License MIT
 *
 */

#include <string.h>
#include <jansson.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define U_DISABLE_JANSSON
#define U_DISABLE_CURL
#define U_DISABLE_WEBSOCKET
#include "../../src/ulfius.h"

#define PORT 4528
#define PREFIX "/inject"

/**
 * callback functions declaration
 */
int callback_first (const struct _u_request * request, struct _u_response * response, void * user_data);

int callback_second (const struct _u_request * request, struct _u_response * response, void * user_data);

int callback_third (const struct _u_request * request, struct _u_response * response, void * user_data);

int callback_fourth (const struct _u_request * request, struct _u_response * response, void * user_data);

int callback_fifth (const struct _u_request * request, struct _u_response * response, void * user_data);

int main (int argc, char **argv) {
  // Initialize the instance
  struct _u_instance instance;
  
  y_init_logs("injection_example", Y_LOG_MODE_CONSOLE, Y_LOG_LEVEL_DEBUG, NULL, "Starting injection_example");
  
  if (ulfius_init_instance(&instance, PORT, NULL, NULL) != U_OK) {
    y_log_message(Y_LOG_LEVEL_ERROR, "Error ulfius_init_instance, abort");
    return(1);
  }
  
  // Endpoint list declaration
  ulfius_add_endpoint_by_val(&instance, "GET", PREFIX, "/first", 1, &callback_first, NULL);
  ulfius_add_endpoint_by_val(&instance, "GET", PREFIX, "/second", 1, &callback_second, NULL);
  ulfius_add_endpoint_by_val(&instance, "GET", PREFIX, "/third", 1, &callback_third, NULL);
  
  // Start the framework
  if (ulfius_start_framework(&instance) == U_OK) {
    y_log_message(Y_LOG_LEVEL_DEBUG, "Start framework on port %d", instance.port);
    
    y_log_message(Y_LOG_LEVEL_DEBUG, "Press <enter> to inject %s/fourth endpoint", PREFIX);
    getchar();
    ulfius_add_endpoint_by_val(&instance, "GET", PREFIX, "/fourth", 1, &callback_fourth, NULL);
    
    y_log_message(Y_LOG_LEVEL_DEBUG, "Press <enter> to inject %s/fifth endpoint", PREFIX);
    getchar();
    ulfius_add_endpoint_by_val(&instance, "GET", PREFIX, "/fifth", 1, &callback_fifth, NULL);
    
    y_log_message(Y_LOG_LEVEL_DEBUG, "Press <enter> to remove %s/fourth endpoint", PREFIX);
    getchar();
    ulfius_remove_endpoint_by_val(&instance, "GET", PREFIX, "/fourth");
    
    y_log_message(Y_LOG_LEVEL_DEBUG, "Press <enter> to quit the application");
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
 * Callback callback_first
 */
int callback_first (const struct _u_request * request, struct _u_response * response, void * user_data) {
  ulfius_set_string_body_response(response, 200, "Hello World from callback_first!");
  return U_CALLBACK_CONTINUE;
}

/**
 * Callback callback_second
 */
int callback_second (const struct _u_request * request, struct _u_response * response, void * user_data) {
  ulfius_set_string_body_response(response, 200, "Hello World from callback_second!");
  return U_CALLBACK_CONTINUE;
}

/**
 * Callback callback_third
 */
int callback_third (const struct _u_request * request, struct _u_response * response, void * user_data) {
  ulfius_set_string_body_response(response, 200, "Hello World from callback_third!");
  return U_CALLBACK_CONTINUE;
}

/**
 * Callback callback_fourth
 */
int callback_fourth (const struct _u_request * request, struct _u_response * response, void * user_data) {
  ulfius_set_string_body_response(response, 200, "Hello World from callback_fourth!");
  return U_CALLBACK_CONTINUE;
}

/**
 * Callback callback_fifth
 */
int callback_fifth (const struct _u_request * request, struct _u_response * response, void * user_data) {
  ulfius_set_string_body_response(response, 200, "Hello World from callback_fifth!");
  return U_CALLBACK_CONTINUE;
}
