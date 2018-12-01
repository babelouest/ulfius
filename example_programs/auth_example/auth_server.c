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
#include <gnutls/gnutls.h>
#include <gnutls/x509.h>

#include <yder.h>
#include <orcania.h>

#include <ulfius.h>

#define PORT 2884
#define PREFIX "/auth"

#define USER "test"
#define PASSWORD "testpassword"

char * read_file(const char * filename) {
  char * buffer = NULL;
  long length;
  FILE * f = fopen (filename, "rb");
  if (f != NULL) {
    fseek (f, 0, SEEK_END);
    length = ftell (f);
    fseek (f, 0, SEEK_SET);
    buffer = o_malloc (length + 1);
    if (buffer != NULL) {
      fread (buffer, 1, length, f);
      buffer[length] = '\0';
    }
    fclose (f);
  }
  return buffer;
}

/**
 * Auth function for basic authentication
 */
int callback_auth_basic_body (const struct _u_request * request, struct _u_response * response, void * user_data) {
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

#ifndef U_DISABLE_WEBSOCKET
/**
 * Callback function on client certificate authentication
 */
int callback_auth_client_cert (const struct _u_request * request, struct _u_response * response, void * user_data) {
  char * buf;
  size_t lbuf = 0;

  if (request->client_cert != NULL) {
    gnutls_x509_crt_get_dn(request->client_cert, NULL, &lbuf);
    buf = o_malloc(lbuf + 1);
    if (buf != NULL) {
      gnutls_x509_crt_get_dn(request->client_cert, buf, &lbuf);
      buf[lbuf] = '\0';
      y_log_message(Y_LOG_LEVEL_DEBUG, "dn of the client: %s", buf);
      ulfius_set_string_body_response(response, 200, buf);
      o_free(buf);
    }
  } else {
    ulfius_set_string_body_response(response, 400, "Invalid client certificate");
  }
  return U_CALLBACK_CONTINUE;
}
#endif

int main (int argc, char **argv) {
  // Initialize the instance
  struct _u_instance instance;
  
  y_init_logs("auth_server", Y_LOG_MODE_CONSOLE, Y_LOG_LEVEL_DEBUG, NULL, "logs start");
  
  if (ulfius_init_instance(&instance, PORT, NULL, "auth_basic_default") != U_OK) {
    printf("Error ulfius_init_instance, abort\n");
    return(1);
  }
  
  // Endpoint list declaration
  ulfius_add_endpoint_by_val(&instance, "GET", PREFIX, "/basic", 0, &callback_auth_basic_body, "auth param");
  ulfius_add_endpoint_by_val(&instance, "GET", PREFIX, "/basic", 1, &callback_auth_basic, NULL);
  ulfius_add_endpoint_by_val(&instance, "GET", PREFIX, "/default", 1, &callback_auth_basic, NULL);
  ulfius_add_endpoint_by_val(&instance, "GET", PREFIX, "/default", 0, &callback_auth_basic_body, NULL);
#ifndef U_DISABLE_WEBSOCKET
  if (argc > 3) {
    ulfius_add_endpoint_by_val(&instance, "GET", PREFIX, "/client_cert", 0, &callback_auth_client_cert, NULL);
  }
#endif
  
#ifndef U_DISABLE_WEBSOCKET
  // Start the framework
  if (argc > 3) {
    char * server_key = read_file(argv[1]), * server_pem = read_file(argv[2]), * root_ca_pem = read_file(argv[3]);
    if (ulfius_start_secure_client_cert_framework(&instance, server_key, server_pem, root_ca_pem) == U_OK) {
      printf("Start secure framework on port %u\n", instance.port);
    
      // Wait for the user to press <enter> on the console to quit the application
      printf("Press <enter> to quit server\n");
      getchar();
    } else {
      printf("Error starting secure framework\n");
    }
    o_free(server_key);
    o_free(server_pem);
    o_free(root_ca_pem);
  } else if (ulfius_start_framework(&instance) == U_OK) {
#endif
    printf("Start framework on port %u\n", instance.port);
    
    // Wait for the user to press <enter> on the console to quit the application
    printf("Press <enter> to quit server\n");
    getchar();
#ifndef U_DISABLE_WEBSOCKET
  } else {
    printf("Error starting framework\n");
  }
#endif

  printf("End framework\n");
  ulfius_stop_framework(&instance);
  ulfius_clean_instance(&instance);
  y_close_logs();
  
  return 0;
}
