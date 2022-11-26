/**
 * 
 * Ulfius Framework example program
 * 
 * This example program send several requests to describe
 * the function ulfius_request_http behaviour
 *  
 * Copyright 2015-2022 Nicolas Mora <mail@babelouest.org>
 * 
 * License MIT
 *
 * How-to generate certificates using openssl for local tests only
 * 
 * Server key and certificate
 * openssl req -x509 -nodes -days 365 -newkey rsa:2048 -keyout server.key -out server.crt
 * 
 * Certificate authority
 * openssl genrsa -out ca.key 4096
 * openssl req -new -x509 -days 365 -key ca.key -out ca.crt
 * 
 * Run auth_server with the following command
 * $ ./auth_server server.key server.crt ca.crt
 * 
 * Client Key and CSR
 * openssl genrsa -out client.key 4096
 * openssl req -new -key client.key -out client.csr
 * openssl x509 -req -days 365 -in client.csr -CA ca.crt -CAkey ca.key -set_serial 01 -out client.crt
 * 
 * Run auth_client with the following command
 * ./auth_client client.crt client.key <password>
 *
 */

#include <stdio.h>
#include <string.h>
#include <ulfius.h>

#include "u_example.h"

#if defined(U_DISABLE_CURL)
#error You must build ulfius with libcurl support enabled to compile this example, check the install documentation
#else

#define SERVER_URL "http://localhost:2884/auth/basic"
#define SERVER_URL_DEFAULT "http://localhost:2884/auth/default"
#define SERVER_URL_CLIENT_CERT "https://localhost:2884/auth/client_cert"

void print_response(struct _u_response * response) {
  if (response != NULL) {
    char response_body[response->binary_body_length + 1];
    o_strncpy(response_body, response->binary_body, response->binary_body_length);
    response_body[response->binary_body_length] = '\0';
    printf("status is\n%ld\n\nstring body is \n%s\n\n",
           response->status, response_body);
  }
}

int main (int argc, char **argv) {
  
  struct _u_response response;
  int res;
  struct _u_request req_list[6];
  
  y_init_logs("auth_client", Y_LOG_MODE_CONSOLE, Y_LOG_LEVEL_DEBUG, NULL, "logs start");
  
#ifndef U_DISABLE_GNUTLS
  if (argc <= 2) {
#endif
    ulfius_init_request(&req_list[0]);
    ulfius_set_request_properties(&req_list[0],
                                  U_OPT_HTTP_VERB, "GET",
                                  U_OPT_HTTP_URL, SERVER_URL,
                                  U_OPT_NONE); // Required to close the parameters list
  
    ulfius_init_request(&req_list[1]);
    ulfius_set_request_properties(&req_list[1],
                                  U_OPT_HTTP_VERB, "GET",
                                  U_OPT_HTTP_URL, SERVER_URL,
                                  U_OPT_AUTH_BASIC_USER, "test",
                                  U_OPT_AUTH_BASIC_PASSWORD, "testpassword",
                                  U_OPT_NONE); // Required to close the parameters list
  
    ulfius_init_request(&req_list[2]);
    ulfius_set_request_properties(&req_list[2],
                                  U_OPT_HTTP_VERB, "GET",
                                  U_OPT_HTTP_URL, SERVER_URL,
                                  U_OPT_AUTH_BASIC_USER, "test",
                                  U_OPT_AUTH_BASIC_PASSWORD, "wrongpassword",
                                  U_OPT_NONE); // Required to close the parameters list
  
    ulfius_init_request(&req_list[3]);
    ulfius_set_request_properties(&req_list[3],
                                  U_OPT_HTTP_VERB, "GET",
                                  U_OPT_HTTP_URL, SERVER_URL,
                                  U_OPT_HTTP_URL_APPEND, "/404",
                                  U_OPT_NONE); // Required to close the parameters list

    ulfius_init_request(&req_list[4]);
    ulfius_set_request_properties(&req_list[4],
                                  U_OPT_HTTP_VERB, "GET",
                                  U_OPT_HTTP_URL, SERVER_URL_DEFAULT,
                                  U_OPT_AUTH_BASIC_USER, "test",
                                  U_OPT_AUTH_BASIC_PASSWORD, "testpassword",
                                  U_OPT_NONE); // Required to close the parameters list
  
    ulfius_init_request(&req_list[5]);
    ulfius_set_request_properties(&req_list[5],
                                  U_OPT_HTTP_VERB, "GET",
                                  U_OPT_HTTP_URL, SERVER_URL_DEFAULT,
                                  U_OPT_AUTH_BASIC_USER, "test",
                                  U_OPT_AUTH_BASIC_PASSWORD, "wrongpassword",
                                  U_OPT_NONE); // Required to close the parameters list
    
    printf("Press <enter> to run auth tests no authentication\n");
    getchar();
    ulfius_init_response(&response);
    res = ulfius_send_http_request(&req_list[0], &response);
    if (res == U_OK) {
      print_response(&response);
    } else {
      printf("Error in http request: %d\n", res);
    }
    ulfius_clean_response(&response);
  
    printf("Press <enter> to run auth tests success authentication\n");
    getchar();
    ulfius_init_response(&response);
    res = ulfius_send_http_request(&req_list[1], &response);
    if (res == U_OK) {
      print_response(&response);
    } else {
      printf("Error in http request: %d\n", res);
    }
    ulfius_clean_response(&response);
  
    printf("Press <enter> to run auth tests error authentication\n");
    getchar();
    ulfius_init_response(&response);
    res = ulfius_send_http_request(&req_list[2], &response);
    if (res == U_OK) {
      print_response(&response);
    } else {
      printf("Error in http request: %d\n", res);
    }
    ulfius_clean_response(&response);
  
    printf("Press <enter> to run auth tests 404\n");
    getchar();
    ulfius_init_response(&response);
    res = ulfius_send_http_request(&req_list[3], &response);
    if (res == U_OK) {
      print_response(&response);
    } else {
      printf("Error in http request: %d\n", res);
    }
    ulfius_clean_response(&response);

    printf("Press <enter> to run default auth tests success authentication\n");
    getchar();
    ulfius_init_response(&response);
    res = ulfius_send_http_request(&req_list[4], &response);
    if (res == U_OK) {
      print_response(&response);
    } else {
      printf("Error in http request: %d\n", res);
    }
    ulfius_clean_response(&response);
  
    printf("Press <enter> to run default auth tests error authentication\n");
    getchar();
    ulfius_init_response(&response);
    res = ulfius_send_http_request(&req_list[5], &response);
    if (res == U_OK) {
      print_response(&response);
    } else {
      printf("Error in http request: %d\n", res);
    }
    ulfius_clean_response(&response);
    ulfius_clean_request(&req_list[0]);
    ulfius_clean_request(&req_list[1]);
    ulfius_clean_request(&req_list[2]);
    ulfius_clean_request(&req_list[3]);
    ulfius_clean_request(&req_list[4]);
    ulfius_clean_request(&req_list[5]);
#ifndef U_DISABLE_GNUTLS
  } else {
    ulfius_init_request(&req_list[0]);
    ulfius_set_request_properties(&req_list[0],
                                  U_OPT_HTTP_VERB, "GET",
                                  U_OPT_HTTP_URL, SERVER_URL_CLIENT_CERT,
                                  U_OPT_CHECK_SERVER_CERTIFICATE, 0,
                                  U_OPT_CLIENT_CERT_FILE, argv[1],
                                  U_OPT_CLIENT_KEY_FILE, argv[2],
                                  U_OPT_CLIENT_KEY_PASSWORD, argc>=4?argv[3]:NULL,
                                  U_OPT_NONE); // Required to close the parameters list

    printf("Press <enter> to run client certificate authentication test\n");
    getchar();
    ulfius_init_response(&response);
    res = ulfius_send_http_request(&req_list[0], &response);
    if (res == U_OK) {
      print_response(&response);
    } else {
      printf("Error in http request: %d\n", res);
    }
    ulfius_clean_response(&response);
    ulfius_clean_request(&req_list[0]);
  }
#endif
  
  y_close_logs();
  return 0;
}
#endif
