/* Public domain, no copyright. Use at your own risk. */

#ifndef _WIN32
#include <netinet/in.h>
#endif

#include <check.h>
#include <errno.h>
#include <jansson.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ulfius.h>

#define HTTP_PROTOCOL "http_protocol"
#define HTTP_VERB "http_verb"
#define HTTP_URL "http_url"
#define HTTP_URL_APPEND "http_url_append"
#define URL_PATH "url_path"
#define PROXY "proxy"
#define CA_PATH "ca_path"
#define TIMEOUT 1
#define AUTH_BASIC_USER "auth_basic_user"
#define AUTH_BASIC_PASSWORD "auth_basic_password"
#define MAP_URL_KEY "map_url_key"
#define MAP_URL_VALUE "map_url_value"
#define MAP_HEADER_KEY "map_header_key"
#define MAP_HEADER_VALUE "map_header_value"
#define MAP_COOKIE_KEY "map_cookie_key"
#define MAP_COOKIE_VALUE "map_cookie_value"
#define MAP_POST_BODY_KEY "map_post_body_key"
#define MAP_POST_BODY_VALUE "map_post_body_value"
#define BINARY_BODY "binary_body"
#define BINARY_BODY_LEN o_strlen(BINARY_BODY)
#define STRING_BODY "string_body"
#define CALLBACK_POSITION 2
#define CLIENT_CERT_FILE "client_cert_file"
#define CLIENT_KEY_FILE "client_key_file"
#define CLIENT_KEY_PASSWORD "client_key_password"
#define STATUS 3
#define PROTOCOL "protocol"
#define AUTH_REALM "auth_realm"
#define STREAM_SIZE 4
#define STREAM_BLOCK_SIZE 5
#define STREAM_USER_DATA 0x06
#define SHARED_DATA 0x42
#define COOKIE_KEY "key"
#define COOKIE_VALUE "value"
#define COOKIE_EXPIRES "expires"
#define COOKIE_MAX_AGE 9
#define COOKIE_DOMAIN "domain"
#define COOKIE_PATH "path"
#define COOKIE_SECURE 1
#define COOKIE_HTTP_ONLY 1
#define WEBSOCKET_MANAGER_USER_DATA 0x10
#define WEBSOCKET_INCOMING_USER_DATA 0x11
#define WEBSOCKET_ONCLOSE_USER_DATA 0x12

int callback_function_empty(const struct _u_request * request, struct _u_response * response, void * user_data) {
  return U_CALLBACK_CONTINUE;
}

int callback_function_return_request_body(const struct _u_request * request, struct _u_response * response, void * user_data) {
  ulfius_set_binary_body_response(response, 200, request->binary_body, request->binary_body_length);
  return U_CALLBACK_CONTINUE;
}

ssize_t stream_callback_empty (void * stream_user_data, uint64_t offset, char * out_buf, size_t max) {
  return 0;
}

#ifndef U_DISABLE_WEBSOCKET
void websocket_manager_callback_empty (const struct _u_request * request, struct _websocket_manager * websocket_manager, void * websocket_manager_user_data) {
}

void websocket_incoming_message_callback_empty (const struct _u_request * request, struct _websocket_manager * websocket_manager, const struct _websocket_message * message, void * websocket_incoming_user_data) {
}

void websocket_onclose_callback_empty (const struct _u_request * request, struct _websocket_manager * websocket_manager, void * websocket_onclose_user_data) {
}
#endif

void stream_callback_empty_free (void * stream_user_data) {
}

#ifndef _WIN32
START_TEST(test_ulfius_init_instance)
{
  struct _u_instance u_instance;
  struct sockaddr_in listen;
#if MHD_VERSION >= 0x00095208
  struct sockaddr_in6 listen6;
#endif

  listen.sin_family = AF_INET;
  listen.sin_addr.s_addr = htonl (INADDR_ANY);

#if MHD_VERSION >= 0x00095208
  listen6.sin6_family = AF_INET6;
  listen6.sin6_addr = in6addr_any;
#endif

  ck_assert_int_eq(ulfius_init_instance(&u_instance, 80, NULL, NULL), U_OK);
  ulfius_clean_instance(&u_instance);
  ck_assert_int_eq(ulfius_init_instance(&u_instance, 0, NULL, NULL), U_ERROR_PARAMS);
  ck_assert_int_eq(ulfius_init_instance(&u_instance, 99999, NULL, NULL), U_ERROR_PARAMS);

  ck_assert_int_eq(ulfius_init_instance(&u_instance, 80, &listen, NULL), U_OK);
  ulfius_clean_instance(&u_instance);
  ck_assert_int_eq(ulfius_init_instance(&u_instance, 80, NULL, "test_ulfius"), U_OK);
  ulfius_clean_instance(&u_instance);

#if MHD_VERSION >= 0x00095208
  ck_assert_int_eq(ulfius_init_instance_ipv6(&u_instance, 80, &listen6, U_USE_IPV6, NULL), U_OK);
  ulfius_clean_instance(&u_instance);
  ck_assert_int_eq(ulfius_init_instance_ipv6(&u_instance, 80, &listen6, U_USE_ALL, NULL), U_OK);
  ulfius_clean_instance(&u_instance);

  ck_assert_int_eq(ulfius_init_instance_ipv6(&u_instance, 80, NULL, U_USE_IPV6, NULL), U_OK);
  ulfius_clean_instance(&u_instance);
  ck_assert_int_eq(ulfius_init_instance_ipv6(&u_instance, 80, NULL, U_USE_ALL, NULL), U_OK);
  ulfius_clean_instance(&u_instance);

  ck_assert_int_ne(ulfius_init_instance_ipv6(&u_instance, 80, NULL, U_USE_IPV4, NULL), U_OK);
  ck_assert_int_ne(ulfius_init_instance_ipv6(&u_instance, 80, NULL, 0, NULL), U_OK);
#endif
}
END_TEST
#else
START_TEST(test_ulfius_init_instance)
{
  struct _u_instance u_instance;

  ck_assert_int_eq(ulfius_init_instance(&u_instance, 80, NULL, NULL), U_OK);
  ulfius_clean_instance(&u_instance);
  ck_assert_int_eq(ulfius_init_instance(&u_instance, 0, NULL, NULL), U_ERROR_PARAMS);
  ck_assert_int_eq(ulfius_init_instance(&u_instance, 99999, NULL, NULL), U_ERROR_PARAMS);

  ck_assert_int_eq(ulfius_init_instance(&u_instance, 80, NULL, "test_ulfius"), U_OK);
  ulfius_clean_instance(&u_instance);
}
END_TEST
#endif

START_TEST(test_ulfius_request)
{
  struct _u_request req1, req2, * req3;
#ifndef U_DISABLE_JANSSON
  json_t * j_body = json_pack("{ss}", "test", "body"), * j_body2 = NULL;
  char * str_body = json_dumps(j_body, JSON_COMPACT);
#endif
  
  ck_assert_int_eq(ulfius_init_request(&req1), U_OK);
  ck_assert_ptr_eq(req1.http_protocol, NULL);
  ck_assert_ptr_eq(req1.http_verb, NULL);
  ck_assert_ptr_eq(req1.http_url, NULL);
  ck_assert_ptr_eq(req1.url_path, NULL);
  ck_assert_ptr_eq(req1.proxy, NULL);
#if MHD_VERSION >= 0x00095208
  ck_assert_int_eq(req1.network_type, U_USE_ALL);
#endif
  ck_assert_int_eq(req1.check_server_certificate, 1);
  ck_assert_int_eq(req1.check_server_certificate_flag, (U_SSL_VERIFY_PEER|U_SSL_VERIFY_HOSTNAME));
  ck_assert_int_eq(req1.check_proxy_certificate, 1);
  ck_assert_int_eq(req1.check_proxy_certificate_flag, (U_SSL_VERIFY_PEER|U_SSL_VERIFY_HOSTNAME));
  ck_assert_ptr_eq(req1.ca_path, NULL);
  ck_assert_int_eq(req1.timeout, 0);
  ck_assert_ptr_eq(req1.client_address, NULL);
  ck_assert_ptr_eq(req1.auth_basic_user, NULL);
  ck_assert_ptr_eq(req1.auth_basic_password, NULL);
  ck_assert_int_eq(u_map_count(req1.map_url), 0);
  ck_assert_int_eq(u_map_count(req1.map_header), 0);
  ck_assert_int_eq(u_map_count(req1.map_cookie), 0);
  ck_assert_int_eq(u_map_count(req1.map_post_body), 0);
  ck_assert_ptr_eq(req1.binary_body, NULL);
  ck_assert_int_eq(req1.binary_body_length, 0);
  ck_assert_int_eq(req1.callback_position, 0);
#ifndef U_DISABLE_GNUTLS
  ck_assert_ptr_eq(req1.client_cert, NULL);
  ck_assert_ptr_eq(req1.client_cert_file, NULL);
  ck_assert_ptr_eq(req1.client_key_file, NULL);
  ck_assert_ptr_eq(req1.client_key_password, NULL);
#endif

  req1.http_protocol = o_strdup(HTTP_PROTOCOL);
  req1.http_verb = o_strdup(HTTP_VERB);
  req1.http_url = o_strdup(HTTP_URL);
  req1.url_path = o_strdup(URL_PATH);
  req1.proxy = o_strdup(PROXY);
  req1.ca_path = o_strdup(CA_PATH);
#if MHD_VERSION >= 0x00095208
  req1.network_type = U_USE_IPV4;
#endif
  req1.check_server_certificate = 0;
  req1.check_server_certificate_flag = U_SSL_VERIFY_PEER;
  req1.check_proxy_certificate = 0;
  req1.check_proxy_certificate_flag = U_SSL_VERIFY_PEER;
  req1.timeout = TIMEOUT;
  req1.auth_basic_user = o_strdup(AUTH_BASIC_USER);
  req1.auth_basic_password = o_strdup(AUTH_BASIC_PASSWORD);
  u_map_put(req1.map_url, MAP_URL_KEY, MAP_URL_VALUE);
  u_map_put(req1.map_header, MAP_HEADER_KEY, MAP_HEADER_VALUE);
  u_map_put(req1.map_cookie, MAP_COOKIE_KEY, MAP_COOKIE_VALUE);
  u_map_put(req1.map_post_body, MAP_POST_BODY_KEY, MAP_POST_BODY_VALUE);
  req1.binary_body = o_strdup(BINARY_BODY);
  req1.binary_body_length = o_strlen(BINARY_BODY) + sizeof(char);
  req1.callback_position = CALLBACK_POSITION;
#ifndef U_DISABLE_GNUTLS
  req1.client_cert_file = o_strdup(CLIENT_CERT_FILE);
  req1.client_key_file = o_strdup(CLIENT_KEY_FILE);
  req1.client_key_password = o_strdup(CLIENT_KEY_PASSWORD);
#endif
  
  // Test ulfius_copy_request
  ck_assert_int_eq(ulfius_init_request(&req2), U_OK);
  ck_assert_int_eq(ulfius_copy_request(&req2, &req1), U_OK);

  ck_assert_str_eq(req2.http_protocol, HTTP_PROTOCOL);
  ck_assert_str_eq(req2.http_verb, HTTP_VERB);
  ck_assert_str_eq(req2.http_url, HTTP_URL);
  ck_assert_str_eq(req2.url_path, URL_PATH);
  ck_assert_str_eq(req2.proxy, PROXY);
#if MHD_VERSION >= 0x00095208
  ck_assert_int_eq(req2.network_type, U_USE_IPV4);
#endif
  ck_assert_int_eq(req2.check_server_certificate, 0);
  ck_assert_int_eq(req2.check_server_certificate_flag, U_SSL_VERIFY_PEER);
  ck_assert_int_eq(req2.check_proxy_certificate, 0);
  ck_assert_int_eq(req2.check_proxy_certificate_flag, U_SSL_VERIFY_PEER);
  ck_assert_str_eq(req2.ca_path, CA_PATH);
  ck_assert_int_eq(req2.timeout, TIMEOUT);
  //ck_assert_ptr_eq(req2.client_address, NULL);
  ck_assert_str_eq(req2.auth_basic_user, AUTH_BASIC_USER);
  ck_assert_str_eq(req2.auth_basic_password, AUTH_BASIC_PASSWORD);
  ck_assert_int_eq(u_map_count(req2.map_url), 1);
  ck_assert_str_eq(u_map_get(req2.map_url, MAP_URL_KEY), MAP_URL_VALUE);
  ck_assert_int_eq(u_map_count(req2.map_header), 1);
  ck_assert_str_eq(u_map_get(req2.map_header, MAP_HEADER_KEY), MAP_HEADER_VALUE);
  ck_assert_int_eq(u_map_count(req2.map_cookie), 1);
  ck_assert_str_eq(u_map_get(req2.map_cookie, MAP_COOKIE_KEY), MAP_COOKIE_VALUE);
  ck_assert_int_eq(u_map_count(req2.map_post_body), 1);
  ck_assert_str_eq(u_map_get(req2.map_post_body, MAP_POST_BODY_KEY), MAP_POST_BODY_VALUE);
  ck_assert_ptr_ne(req1.binary_body, NULL);
  ck_assert_int_eq(req1.binary_body_length, o_strlen(BINARY_BODY) + sizeof(char));
  ck_assert_ptr_ne(req2.binary_body, NULL);
  ck_assert_int_eq(req2.binary_body_length, o_strlen(BINARY_BODY) + sizeof(char));
  ck_assert_int_eq(req2.callback_position, CALLBACK_POSITION);
#ifndef U_DISABLE_GNUTLS
  //ck_assert_ptr_eq(req2.client_cert, NULL);
  ck_assert_str_eq(req2.client_cert_file, CLIENT_CERT_FILE);
  ck_assert_str_eq(req2.client_key_file, CLIENT_KEY_FILE);
  ck_assert_str_eq(req2.client_key_password, CLIENT_KEY_PASSWORD);
#endif
  
  // Test ulfius_duplicate_request
  ck_assert_ptr_ne((req3 = ulfius_duplicate_request(&req1)), NULL);
  ck_assert_str_eq(req3->http_protocol, HTTP_PROTOCOL);
  ck_assert_str_eq(req3->http_verb, HTTP_VERB);
  ck_assert_str_eq(req3->http_url, HTTP_URL);
  ck_assert_str_eq(req3->url_path, URL_PATH);
  ck_assert_str_eq(req3->proxy, PROXY);
#if MHD_VERSION >= 0x00095208
  ck_assert_int_eq(req3->network_type, U_USE_IPV4);
#endif
  ck_assert_int_eq(req3->check_server_certificate, 0);
  ck_assert_int_eq(req3->check_server_certificate_flag, U_SSL_VERIFY_PEER);
  ck_assert_int_eq(req3->check_proxy_certificate, 0);
  ck_assert_int_eq(req3->check_proxy_certificate_flag, U_SSL_VERIFY_PEER);
  ck_assert_str_eq(req3->ca_path, CA_PATH);
  ck_assert_int_eq(req3->timeout, TIMEOUT);
  //ck_assert_ptr_eq(req3->client_address, NULL);
  ck_assert_str_eq(req3->auth_basic_user, AUTH_BASIC_USER);
  ck_assert_str_eq(req3->auth_basic_password, AUTH_BASIC_PASSWORD);
  ck_assert_int_eq(u_map_count(req3->map_url), 1);
  ck_assert_str_eq(u_map_get(req3->map_url, MAP_URL_KEY), MAP_URL_VALUE);
  ck_assert_int_eq(u_map_count(req3->map_header), 1);
  ck_assert_str_eq(u_map_get(req3->map_header, MAP_HEADER_KEY), MAP_HEADER_VALUE);
  ck_assert_int_eq(u_map_count(req3->map_cookie), 1);
  ck_assert_str_eq(u_map_get(req3->map_cookie, MAP_COOKIE_KEY), MAP_COOKIE_VALUE);
  ck_assert_int_eq(u_map_count(req3->map_post_body), 1);
  ck_assert_str_eq(u_map_get(req3->map_post_body, MAP_POST_BODY_KEY), MAP_POST_BODY_VALUE);
  ck_assert_ptr_ne(req3->binary_body, NULL);
  ck_assert_int_eq(req3->binary_body_length, o_strlen(BINARY_BODY) + sizeof(char));
  ck_assert_int_eq(req3->callback_position, CALLBACK_POSITION);
#ifndef U_DISABLE_GNUTLS
  //ck_assert_ptr_eq(req3->client_cert, NULL);
  ck_assert_str_eq(req3->client_cert_file, CLIENT_CERT_FILE);
  ck_assert_str_eq(req3->client_key_file, CLIENT_KEY_FILE);
  ck_assert_str_eq(req3->client_key_password, CLIENT_KEY_PASSWORD);
#endif

  // Test ulfius_set_*_request
  ulfius_clean_request(&req1);
  ulfius_init_request(&req1);
  ck_assert_int_eq(ulfius_set_string_body_request(&req1, STRING_BODY), U_OK);
  ck_assert_str_eq(req1.binary_body, STRING_BODY);
  ck_assert_int_eq(req1.binary_body_length, o_strlen(STRING_BODY));
  
  ck_assert_int_eq(ulfius_set_empty_body_request(&req1), U_OK);
  ck_assert_int_eq(req1.binary_body_length, 0);
  ck_assert_ptr_eq(req1.binary_body, NULL);
  
  ck_assert_int_eq(ulfius_set_binary_body_request(&req1, BINARY_BODY, BINARY_BODY_LEN), U_OK);
  ck_assert_ptr_ne(req1.binary_body, NULL);
  ck_assert_int_eq(req1.binary_body_length, BINARY_BODY_LEN);

#ifndef U_DISABLE_JANSSON
  ck_assert_int_eq(ulfius_set_json_body_request(&req1, j_body), U_OK);
  ck_assert_str_eq(req1.binary_body, str_body);
  j_body2 = ulfius_get_json_body_request(&req1, NULL);
  ck_assert_int_eq(json_equal(j_body, j_body2), 1);
  o_free(str_body);
  json_decref(j_body);
  json_decref(j_body2);
#endif

  ulfius_clean_request(&req1);
  ulfius_clean_request(&req2);
  ulfius_clean_request_full(req3);
}
END_TEST

START_TEST(test_ulfius_set_request_properties)
{
  struct _u_request req;
  
  ck_assert_int_eq(ulfius_init_request(&req), U_OK);
  ck_assert_int_eq(ulfius_set_request_properties(&req,
                                                 U_OPT_HTTP_VERB, HTTP_VERB,
                                                 U_OPT_HTTP_URL, HTTP_URL,
                                                 U_OPT_HTTP_URL_APPEND, HTTP_URL_APPEND,
                                                 U_OPT_HTTP_PROXY, PROXY,
#if MHD_VERSION >= 0x00095208
                                                 U_OPT_NETWORK_TYPE, U_USE_IPV4,
#endif
                                                 U_OPT_CHECK_SERVER_CERTIFICATE, 0,
                                                 U_OPT_CHECK_SERVER_CERTIFICATE_FLAG, U_SSL_VERIFY_PEER,
                                                 U_OPT_CHECK_PROXY_CERTIFICATE, 0,
                                                 U_OPT_CHECK_PROXY_CERTIFICATE_FLAG, U_SSL_VERIFY_HOSTNAME,
                                                 U_OPT_FOLLOW_REDIRECT, 1,
                                                 U_OPT_CA_PATH, CA_PATH,
                                                 U_OPT_TIMEOUT, TIMEOUT,
                                                 U_OPT_AUTH_BASIC_USER, AUTH_BASIC_USER,
                                                 U_OPT_AUTH_BASIC_PASSWORD, AUTH_BASIC_PASSWORD,
                                                 U_OPT_URL_PARAMETER, MAP_URL_KEY, MAP_URL_VALUE,
                                                 U_OPT_HEADER_PARAMETER, MAP_HEADER_KEY, MAP_HEADER_VALUE,
                                                 U_OPT_COOKIE_PARAMETER, MAP_COOKIE_KEY, MAP_COOKIE_VALUE,
                                                 U_OPT_POST_BODY_PARAMETER, MAP_POST_BODY_KEY, MAP_POST_BODY_VALUE,
                                                 U_OPT_BINARY_BODY, BINARY_BODY, BINARY_BODY_LEN,
#ifndef U_DISABLE_GNUTLS
                                                 U_OPT_CLIENT_CERT_FILE, CLIENT_CERT_FILE,
                                                 U_OPT_CLIENT_KEY_FILE, CLIENT_KEY_FILE,
                                                 U_OPT_CLIENT_KEY_PASSWORD, CLIENT_KEY_PASSWORD,
#endif
                                                 U_OPT_NONE), U_OK);

  ck_assert_str_eq(req.http_verb, HTTP_VERB);
  ck_assert_str_eq(req.http_url, HTTP_URL HTTP_URL_APPEND);
  ck_assert_str_eq(req.proxy, PROXY);
#if MHD_VERSION >= 0x00095208
  ck_assert_int_eq(req.network_type, U_USE_IPV4);
#endif
  ck_assert_int_eq(req.check_server_certificate, 0);
  ck_assert_int_eq(req.check_server_certificate_flag, U_SSL_VERIFY_PEER);
  ck_assert_int_eq(req.check_proxy_certificate, 0);
  ck_assert_int_eq(req.check_proxy_certificate_flag, U_SSL_VERIFY_HOSTNAME);
  ck_assert_int_eq(req.follow_redirect, 1);
  ck_assert_str_eq(req.ca_path, CA_PATH);
  ck_assert_int_eq(req.timeout, TIMEOUT);
  ck_assert_str_eq(req.auth_basic_user, AUTH_BASIC_USER);
  ck_assert_str_eq(req.auth_basic_password, AUTH_BASIC_PASSWORD);
  ck_assert_int_eq(u_map_count(req.map_url), 1);
  ck_assert_str_eq(u_map_get(req.map_url, MAP_URL_KEY), MAP_URL_VALUE);
  ck_assert_int_eq(u_map_count(req.map_header), 1);
  ck_assert_str_eq(u_map_get(req.map_header, MAP_HEADER_KEY), MAP_HEADER_VALUE);
  ck_assert_int_eq(u_map_count(req.map_cookie), 1);
  ck_assert_str_eq(u_map_get(req.map_cookie, MAP_COOKIE_KEY), MAP_COOKIE_VALUE);
  ck_assert_int_eq(u_map_count(req.map_post_body), 1);
  ck_assert_str_eq(u_map_get(req.map_post_body, MAP_POST_BODY_KEY), MAP_POST_BODY_VALUE);
  ck_assert_ptr_ne(req.binary_body, NULL);
  ck_assert_int_eq(req.binary_body_length, o_strlen(BINARY_BODY));
#ifndef U_DISABLE_GNUTLS
  ck_assert_str_eq(req.client_cert_file, CLIENT_CERT_FILE);
  ck_assert_str_eq(req.client_key_file, CLIENT_KEY_FILE);
  ck_assert_str_eq(req.client_key_password, CLIENT_KEY_PASSWORD);
#endif

  ck_assert_int_eq(ulfius_set_request_properties(&req,
                                                 U_OPT_STRING_BODY, STRING_BODY,
                                                 U_OPT_URL_PARAMETER_REMOVE, MAP_URL_KEY,
                                                 U_OPT_HEADER_PARAMETER_REMOVE, MAP_HEADER_KEY,
                                                 U_OPT_COOKIE_PARAMETER_REMOVE, MAP_COOKIE_KEY,
                                                 U_OPT_POST_BODY_PARAMETER_REMOVE, MAP_POST_BODY_KEY,
                                                 U_OPT_NONE), U_OK);
  ck_assert_ptr_ne(req.binary_body, NULL);
  ck_assert_int_eq(req.binary_body_length, o_strlen(STRING_BODY));
  ck_assert_int_eq(u_map_count(req.map_url), 0);
  ck_assert_int_eq(u_map_count(req.map_header), 0);
  ck_assert_int_eq(u_map_count(req.map_cookie), 0);
  ck_assert_int_eq(u_map_count(req.map_post_body), 0);

#ifndef U_DISABLE_JANSSON
  json_t * j_body = json_pack("{ss}", "result", "body");
  ck_assert_int_eq(ulfius_set_request_properties(&req, U_OPT_JSON_BODY, j_body, U_OPT_NONE), U_OK);
  ck_assert_str_eq(req.binary_body, "{\"result\":\"body\"}");
  json_decref(j_body);
#endif

  // Test memory leaks on multiple ulfius_set_request_properties for the same properties
  ck_assert_int_eq(ulfius_set_request_properties(&req,
                                                 U_OPT_HTTP_VERB, HTTP_VERB,
                                                 U_OPT_HTTP_URL, HTTP_URL,
                                                 U_OPT_HTTP_URL_APPEND, HTTP_URL_APPEND,
                                                 U_OPT_HTTP_PROXY, PROXY,
#if MHD_VERSION >= 0x00095208
                                                 U_OPT_NETWORK_TYPE, U_USE_IPV4,
#endif
                                                 U_OPT_CHECK_SERVER_CERTIFICATE, 0,
                                                 U_OPT_CHECK_SERVER_CERTIFICATE_FLAG, U_SSL_VERIFY_PEER,
                                                 U_OPT_CHECK_PROXY_CERTIFICATE, 0,
                                                 U_OPT_CHECK_PROXY_CERTIFICATE_FLAG, U_SSL_VERIFY_HOSTNAME,
                                                 U_OPT_FOLLOW_REDIRECT, 1,
                                                 U_OPT_CA_PATH, CA_PATH,
                                                 U_OPT_TIMEOUT, TIMEOUT,
                                                 U_OPT_AUTH_BASIC_USER, AUTH_BASIC_USER,
                                                 U_OPT_AUTH_BASIC_PASSWORD, AUTH_BASIC_PASSWORD,
                                                 U_OPT_URL_PARAMETER, MAP_URL_KEY, MAP_URL_VALUE,
                                                 U_OPT_HEADER_PARAMETER, MAP_HEADER_KEY, MAP_HEADER_VALUE,
                                                 U_OPT_COOKIE_PARAMETER, MAP_COOKIE_KEY, MAP_COOKIE_VALUE,
                                                 U_OPT_POST_BODY_PARAMETER, MAP_POST_BODY_KEY, MAP_POST_BODY_VALUE,
                                                 U_OPT_BINARY_BODY, BINARY_BODY, BINARY_BODY_LEN,
#ifndef U_DISABLE_GNUTLS
                                                 U_OPT_CLIENT_CERT_FILE, CLIENT_CERT_FILE,
                                                 U_OPT_CLIENT_KEY_FILE, CLIENT_KEY_FILE,
                                                 U_OPT_CLIENT_KEY_PASSWORD, CLIENT_KEY_PASSWORD,
#endif
                                                 U_OPT_NONE), U_OK);

  ck_assert_str_eq(req.http_verb, HTTP_VERB);
  ck_assert_str_eq(req.http_url, HTTP_URL HTTP_URL_APPEND);
  ck_assert_str_eq(req.proxy, PROXY);
#if MHD_VERSION >= 0x00095208
  ck_assert_int_eq(req.network_type, U_USE_IPV4);
#endif
  ck_assert_int_eq(req.check_server_certificate, 0);
  ck_assert_int_eq(req.check_server_certificate_flag, U_SSL_VERIFY_PEER);
  ck_assert_int_eq(req.check_proxy_certificate, 0);
  ck_assert_int_eq(req.check_proxy_certificate_flag, U_SSL_VERIFY_HOSTNAME);
  ck_assert_int_eq(req.follow_redirect, 1);
  ck_assert_str_eq(req.ca_path, CA_PATH);
  ck_assert_int_eq(req.timeout, TIMEOUT);
  ck_assert_str_eq(req.auth_basic_user, AUTH_BASIC_USER);
  ck_assert_str_eq(req.auth_basic_password, AUTH_BASIC_PASSWORD);
  ck_assert_int_eq(u_map_count(req.map_url), 1);
  ck_assert_str_eq(u_map_get(req.map_url, MAP_URL_KEY), MAP_URL_VALUE);
#ifndef U_DISABLE_JANSSON
  ck_assert_int_eq(u_map_count(req.map_header), 2); // contains keys MAP_HEADER_KEY and "Content-Type"
#else
  ck_assert_int_eq(u_map_count(req.map_header), 1);
#endif
  ck_assert_str_eq(u_map_get(req.map_header, MAP_HEADER_KEY), MAP_HEADER_VALUE);
  ck_assert_int_eq(u_map_count(req.map_cookie), 1);
  ck_assert_str_eq(u_map_get(req.map_cookie, MAP_COOKIE_KEY), MAP_COOKIE_VALUE);
  ck_assert_int_eq(u_map_count(req.map_post_body), 1);
  ck_assert_str_eq(u_map_get(req.map_post_body, MAP_POST_BODY_KEY), MAP_POST_BODY_VALUE);
  ck_assert_ptr_ne(req.binary_body, NULL);
  ck_assert_int_eq(req.binary_body_length, o_strlen(BINARY_BODY));
#ifndef U_DISABLE_GNUTLS
  ck_assert_str_eq(req.client_cert_file, CLIENT_CERT_FILE);
  ck_assert_str_eq(req.client_key_file, CLIENT_KEY_FILE);
  ck_assert_str_eq(req.client_key_password, CLIENT_KEY_PASSWORD);
#endif

  ck_assert_int_eq(ulfius_set_request_properties(&req, U_OPT_AUTH_BASIC, NULL, NULL, U_OPT_NONE), U_OK);
  ck_assert_ptr_eq(req.auth_basic_user, NULL);
  ck_assert_ptr_eq(req.auth_basic_password, NULL);
  ck_assert_int_eq(ulfius_set_request_properties(&req, U_OPT_AUTH_BASIC, AUTH_BASIC_USER, AUTH_BASIC_PASSWORD, U_OPT_NONE), U_OK);
  ck_assert_str_eq(req.auth_basic_user, AUTH_BASIC_USER);
  ck_assert_str_eq(req.auth_basic_password, AUTH_BASIC_PASSWORD);

  ulfius_clean_request(&req);

}
END_TEST

START_TEST(test_ulfius_request_limits)
{
  struct _u_request request;
  struct _u_response response;
  struct _u_instance u_instance;

  ck_assert_int_eq(ulfius_init_instance(&u_instance, 8080, NULL, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", NULL, "echo", 0, &callback_function_return_request_body, NULL), U_OK);
  ck_assert_int_eq(ulfius_start_framework(&u_instance), U_OK);

  ulfius_init_request(&request);
  ulfius_init_response(&response);
  request.http_url = o_strdup("http://localhost:8080/echo");
  request.binary_body_length = 128;
  request.binary_body = o_malloc(request.binary_body_length*sizeof(char));
  ck_assert_ptr_ne(request.binary_body, NULL);
  memset(request.binary_body, '0', request.binary_body_length);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_eq(request.binary_body_length, response.binary_body_length);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  ulfius_init_request(&request);
  ulfius_init_response(&response);
  request.http_url = o_strdup("http://localhost:8080/echo");
  request.binary_body_length = 256;
  request.binary_body = o_malloc(request.binary_body_length*sizeof(char));
  ck_assert_ptr_ne(request.binary_body, NULL);
  memset(request.binary_body, '0', request.binary_body_length);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_eq(request.binary_body_length, response.binary_body_length);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  ulfius_init_request(&request);
  ulfius_init_response(&response);
  request.http_url = o_strdup("http://localhost:8080/echo");
  request.binary_body_length = 512;
  request.binary_body = o_malloc(request.binary_body_length*sizeof(char));
  ck_assert_ptr_ne(request.binary_body, NULL);
  memset(request.binary_body, '0', request.binary_body_length);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_eq(request.binary_body_length, response.binary_body_length);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  ulfius_init_request(&request);
  ulfius_init_response(&response);
  request.http_url = o_strdup("http://localhost:8080/echo");
  request.binary_body_length = 1024;
  request.binary_body = o_malloc(request.binary_body_length*sizeof(char));
  ck_assert_ptr_ne(request.binary_body, NULL);
  memset(request.binary_body, '0', request.binary_body_length);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_eq(request.binary_body_length, response.binary_body_length);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  ulfius_init_request(&request);
  ulfius_init_response(&response);
  request.http_url = o_strdup("http://localhost:8080/echo");
  request.binary_body_length = 2048;
  request.binary_body = o_malloc(request.binary_body_length*sizeof(char));
  ck_assert_ptr_ne(request.binary_body, NULL);
  memset(request.binary_body, '0', request.binary_body_length);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_eq(request.binary_body_length, response.binary_body_length);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  ulfius_init_request(&request);
  ulfius_init_response(&response);
  request.http_url = o_strdup("http://localhost:8080/echo");
  request.binary_body_length = 16*1024;
  request.binary_body = o_malloc(request.binary_body_length*sizeof(char));
  ck_assert_ptr_ne(request.binary_body, NULL);
  memset(request.binary_body, '0', request.binary_body_length);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_eq(request.binary_body_length, response.binary_body_length);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  ulfius_init_request(&request);
  ulfius_init_response(&response);
  request.http_url = o_strdup("http://localhost:8080/echo");
  request.binary_body_length = 2*1024*1024;
  request.binary_body = o_malloc(request.binary_body_length*sizeof(char));
  ck_assert_ptr_ne(request.binary_body, NULL);
  memset(request.binary_body, '0', request.binary_body_length);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_eq(request.binary_body_length, response.binary_body_length);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  ulfius_stop_framework(&u_instance);
  ulfius_clean_instance(&u_instance);
}
END_TEST

START_TEST(test_ulfius_response)
{
  struct _u_response resp1, resp2, * resp3;
#ifndef U_DISABLE_JANSSON
  json_t * j_body = json_pack("{ss}", "test", "body"), * j_body2 = NULL;
  char * str_body = json_dumps(j_body, JSON_COMPACT);
#endif
  
  ulfius_init_response(&resp1);
  ck_assert_int_eq(resp1.status, 200);
  ck_assert_ptr_eq(resp1.protocol, NULL);
  ck_assert_int_eq(u_map_count(resp1.map_header), 0);
  ck_assert_int_eq(resp1.nb_cookies, 0);
  ck_assert_ptr_eq(resp1.auth_realm, NULL);
  ck_assert_ptr_eq(resp1.binary_body, NULL);
  ck_assert_int_eq(resp1.binary_body_length, 0);
  ck_assert_ptr_eq(resp1.stream_callback, NULL);
  ck_assert_ptr_eq(resp1.stream_callback_free, NULL);
  ck_assert_int_eq(resp1.stream_size, U_STREAM_SIZE_UNKNOWN);
  ck_assert_int_eq(resp1.stream_block_size, ULFIUS_STREAM_BLOCK_SIZE_DEFAULT);
  ck_assert_ptr_eq(resp1.stream_user_data, NULL);
#ifndef U_DISABLE_WEBSOCKET
  ck_assert_ptr_eq(((struct _websocket_handle *)resp1.websocket_handle)->websocket_manager_callback, NULL);
  ck_assert_ptr_eq(((struct _websocket_handle *)resp1.websocket_handle)->websocket_manager_user_data, NULL);
  ck_assert_ptr_eq(((struct _websocket_handle *)resp1.websocket_handle)->websocket_incoming_message_callback, NULL);
  ck_assert_ptr_eq(((struct _websocket_handle *)resp1.websocket_handle)->websocket_incoming_user_data, NULL);
  ck_assert_ptr_eq(((struct _websocket_handle *)resp1.websocket_handle)->websocket_onclose_callback, NULL);
  ck_assert_ptr_eq(((struct _websocket_handle *)resp1.websocket_handle)->websocket_onclose_user_data, NULL);
#endif
  ck_assert_ptr_eq(resp1.shared_data, NULL);
  ck_assert_int_eq(resp1.timeout, 0);
  
  resp1.status = STATUS;
  resp1.protocol = o_strdup(PROTOCOL);
  ck_assert_int_eq(u_map_put(resp1.map_header, MAP_HEADER_KEY, MAP_HEADER_VALUE), U_OK);
  ck_assert_int_eq(ulfius_add_cookie_to_response(&resp1, COOKIE_KEY, COOKIE_VALUE, COOKIE_EXPIRES, COOKIE_MAX_AGE, COOKIE_DOMAIN, COOKIE_PATH, COOKIE_SECURE, COOKIE_HTTP_ONLY), U_OK);
  resp1.auth_realm = o_strdup(AUTH_REALM);
  
  ck_assert_int_eq(ulfius_set_stream_response(&resp1, STATUS, &stream_callback_empty, &stream_callback_empty_free, STREAM_SIZE, STREAM_BLOCK_SIZE, (void *)STREAM_USER_DATA), U_OK);
  ck_assert_ptr_eq(resp1.stream_callback, &stream_callback_empty);
  ck_assert_ptr_eq(resp1.stream_callback_free, &stream_callback_empty_free);
  ck_assert_int_eq(resp1.stream_size, STREAM_SIZE);
  ck_assert_int_eq(resp1.stream_block_size, STREAM_BLOCK_SIZE);
  ck_assert_ptr_eq(resp1.stream_user_data, (void *)STREAM_USER_DATA);
  
#ifndef U_DISABLE_WEBSOCKET
  ck_assert_int_eq(ulfius_set_websocket_response(&resp1, NULL, NULL, &websocket_manager_callback_empty, (void *)WEBSOCKET_MANAGER_USER_DATA, &websocket_incoming_message_callback_empty, (void *)WEBSOCKET_INCOMING_USER_DATA, &websocket_onclose_callback_empty, (void *)WEBSOCKET_ONCLOSE_USER_DATA), U_OK);
  ck_assert_ptr_eq(((struct _websocket_handle *)resp1.websocket_handle)->websocket_manager_callback, &websocket_manager_callback_empty);
  ck_assert_ptr_ne(((struct _websocket_handle *)resp1.websocket_handle)->websocket_manager_user_data, NULL);
  ck_assert_ptr_eq(((struct _websocket_handle *)resp1.websocket_handle)->websocket_incoming_message_callback, &websocket_incoming_message_callback_empty);
  ck_assert_ptr_ne(((struct _websocket_handle *)resp1.websocket_handle)->websocket_incoming_user_data, NULL);
  ck_assert_ptr_eq(((struct _websocket_handle *)resp1.websocket_handle)->websocket_onclose_callback, &websocket_onclose_callback_empty);
  ck_assert_ptr_ne(((struct _websocket_handle *)resp1.websocket_handle)->websocket_onclose_user_data, NULL);
#endif
  
  resp1.binary_body = o_strdup(BINARY_BODY);
  resp1.binary_body_length = BINARY_BODY_LEN;
  resp1.shared_data = (void *)SHARED_DATA;
  resp1.timeout = TIMEOUT;
  
  // Test ulfius_copy_response
  ulfius_init_response(&resp2);
  ck_assert_int_eq(ulfius_copy_response(&resp2, &resp1), U_OK);
  ck_assert_int_eq(resp2.status, STATUS);
  ck_assert_str_eq(resp2.protocol, PROTOCOL);
  ck_assert_int_eq(u_map_count(resp2.map_header), 1);
  ck_assert_str_eq(u_map_get(resp2.map_header, MAP_HEADER_KEY), MAP_HEADER_VALUE);
  ck_assert_int_eq(resp2.nb_cookies, 1);
  ck_assert_str_eq(resp2.auth_realm, AUTH_REALM);
  ck_assert_ptr_ne(resp2.binary_body, NULL);
  ck_assert_int_eq(resp2.binary_body_length, BINARY_BODY_LEN);
  ck_assert_ptr_eq(resp2.stream_callback, &stream_callback_empty);
  ck_assert_ptr_eq(resp2.stream_callback_free, &stream_callback_empty_free);
  ck_assert_int_eq(resp2.stream_size, STREAM_SIZE);
  ck_assert_int_eq(resp2.stream_block_size, STREAM_BLOCK_SIZE);
  ck_assert_ptr_ne(resp2.stream_user_data, NULL);
#ifndef U_DISABLE_WEBSOCKET
  ck_assert_ptr_eq(((struct _websocket_handle *)resp2.websocket_handle)->websocket_manager_callback, &websocket_manager_callback_empty);
  ck_assert_ptr_ne(((struct _websocket_handle *)resp2.websocket_handle)->websocket_manager_user_data, NULL);
  ck_assert_ptr_eq(((struct _websocket_handle *)resp2.websocket_handle)->websocket_incoming_message_callback, &websocket_incoming_message_callback_empty);
  ck_assert_ptr_ne(((struct _websocket_handle *)resp2.websocket_handle)->websocket_incoming_user_data, NULL);
  ck_assert_ptr_eq(((struct _websocket_handle *)resp2.websocket_handle)->websocket_onclose_callback, &websocket_onclose_callback_empty);
  ck_assert_ptr_ne(((struct _websocket_handle *)resp2.websocket_handle)->websocket_onclose_user_data, NULL);
#endif
  ck_assert_ptr_ne(resp2.shared_data, NULL);
  ck_assert_int_eq(resp2.timeout, TIMEOUT);
  
  // Test ulfius_duplicate_response
  resp3 = ulfius_duplicate_response(&resp1);
  ck_assert_int_eq(resp3->status, STATUS);
  ck_assert_str_eq(resp3->protocol, PROTOCOL);
  ck_assert_int_eq(u_map_count(resp3->map_header), 1);
  ck_assert_str_eq(u_map_get(resp3->map_header, MAP_HEADER_KEY), MAP_HEADER_VALUE);
  ck_assert_int_eq(resp3->nb_cookies, 1);
  ck_assert_str_eq(resp3->auth_realm, AUTH_REALM);
  ck_assert_ptr_ne(resp3->binary_body, NULL);
  ck_assert_int_eq(resp3->binary_body_length, BINARY_BODY_LEN);
  ck_assert_ptr_eq(resp3->stream_callback, &stream_callback_empty);
  ck_assert_ptr_eq(resp3->stream_callback_free, &stream_callback_empty_free);
  ck_assert_int_eq(resp3->stream_size, STREAM_SIZE);
  ck_assert_int_eq(resp3->stream_block_size, STREAM_BLOCK_SIZE);
  ck_assert_ptr_ne(resp3->stream_user_data, NULL);
#ifndef U_DISABLE_WEBSOCKET
  ck_assert_ptr_eq(((struct _websocket_handle *)resp3->websocket_handle)->websocket_manager_callback, &websocket_manager_callback_empty);
  ck_assert_ptr_ne(((struct _websocket_handle *)resp3->websocket_handle)->websocket_manager_user_data, NULL);
  ck_assert_ptr_eq(((struct _websocket_handle *)resp3->websocket_handle)->websocket_incoming_message_callback, &websocket_incoming_message_callback_empty);
  ck_assert_ptr_ne(((struct _websocket_handle *)resp3->websocket_handle)->websocket_incoming_user_data, NULL);
  ck_assert_ptr_eq(((struct _websocket_handle *)resp3->websocket_handle)->websocket_onclose_callback, &websocket_onclose_callback_empty);
  ck_assert_ptr_ne(((struct _websocket_handle *)resp3->websocket_handle)->websocket_onclose_user_data, NULL);
#endif
  ck_assert_ptr_ne(resp3->shared_data, NULL);
  ck_assert_int_eq(resp3->timeout, TIMEOUT);
  
  // Test ulfius_set_*_response
  ulfius_clean_response(&resp1);
  ulfius_init_response(&resp1);
  ck_assert_int_eq(ulfius_set_string_body_response(&resp1, STATUS, STRING_BODY), U_OK);
  ck_assert_ptr_ne(resp1.binary_body, NULL);
  ck_assert_int_eq(resp1.binary_body_length, o_strlen(STRING_BODY));
  
  ck_assert_int_eq(ulfius_set_empty_body_response(&resp1, STATUS), U_OK);
  ck_assert_int_eq(resp1.binary_body_length, 0);
  ck_assert_ptr_eq(resp1.binary_body, NULL);
  
  ck_assert_int_eq(ulfius_set_binary_body_response(&resp1, STATUS, BINARY_BODY, BINARY_BODY_LEN), U_OK);
  ck_assert_ptr_ne(resp1.binary_body, NULL);
  ck_assert_int_eq(resp1.binary_body_length, BINARY_BODY_LEN);

#ifndef U_DISABLE_JANSSON
  ck_assert_int_eq(ulfius_set_json_body_response(&resp1, STATUS, j_body), U_OK);
  ck_assert_str_eq(resp1.binary_body, str_body);
  j_body2 = ulfius_get_json_body_response(&resp1, NULL);
  ck_assert_int_eq(json_equal(j_body, j_body2), 1);
  o_free(str_body);
  json_decref(j_body);
  json_decref(j_body2);
#endif

  ulfius_clean_response(&resp1);
  ulfius_clean_response(&resp2);
  ulfius_clean_response_full(resp3);
}
END_TEST

START_TEST(test_ulfius_set_response_properties)
{
  struct _u_response resp;
  
  ck_assert_int_eq(ulfius_init_response(&resp), U_OK);
  
  ck_assert_int_eq(ulfius_set_response_properties(&resp,
                                                  U_OPT_STATUS, STATUS,
                                                  U_OPT_HEADER_PARAMETER, MAP_HEADER_KEY, MAP_HEADER_VALUE,
                                                  U_OPT_AUTH_REALM, AUTH_REALM,
                                                  U_OPT_BINARY_BODY, BINARY_BODY, BINARY_BODY_LEN,
                                                  U_OPT_SHARED_DATA, (void *)SHARED_DATA,
                                                  U_OPT_TIMEOUT, TIMEOUT,
                                                  U_OPT_NONE), U_OK);

  ck_assert_int_eq(resp.status, STATUS);
  ck_assert_int_eq(u_map_count(resp.map_header), 1);
  ck_assert_str_eq(u_map_get(resp.map_header, MAP_HEADER_KEY), MAP_HEADER_VALUE);
  ck_assert_str_eq(resp.auth_realm, AUTH_REALM);
  ck_assert_ptr_ne(resp.binary_body, NULL);
  ck_assert_int_eq(resp.binary_body_length, BINARY_BODY_LEN);
  ck_assert_ptr_eq(resp.shared_data, (void *)SHARED_DATA);
  ck_assert_int_eq(resp.timeout, TIMEOUT);
  
  ck_assert_int_eq(ulfius_set_response_properties(&resp,
                                                  U_OPT_STRING_BODY, STRING_BODY,
                                                  U_OPT_HEADER_PARAMETER_REMOVE, MAP_HEADER_KEY,
                                                  U_OPT_NONE), U_OK);
  ck_assert_ptr_ne(resp.binary_body, NULL);
  ck_assert_int_eq(resp.binary_body_length, o_strlen(STRING_BODY));
  ck_assert_int_eq(u_map_count(resp.map_header), 0);
  
#ifndef U_DISABLE_JANSSON
  json_t * j_body = json_pack("{ss}", "result", "body");
  ck_assert_int_eq(ulfius_set_response_properties(&resp, U_OPT_JSON_BODY, j_body, U_OPT_NONE), U_OK);
  ck_assert_str_eq(resp.binary_body, "{\"result\":\"body\"}");
  json_decref(j_body);
#endif

  // Test memory leaks on multiple ulfius_set_request_properties for the same properties
  ck_assert_int_eq(ulfius_set_response_properties(&resp,
                                                  U_OPT_STATUS, STATUS,
                                                  U_OPT_HEADER_PARAMETER, MAP_HEADER_KEY, MAP_HEADER_VALUE,
                                                  U_OPT_AUTH_REALM, AUTH_REALM,
                                                  U_OPT_BINARY_BODY, BINARY_BODY, BINARY_BODY_LEN,
                                                  U_OPT_SHARED_DATA, (void *)SHARED_DATA,
                                                  U_OPT_TIMEOUT, TIMEOUT,
                                                  U_OPT_NONE), U_OK);

  ck_assert_int_eq(resp.status, STATUS);
#ifndef U_DISABLE_JANSSON
  ck_assert_int_eq(u_map_count(resp.map_header), 2); // contains keys MAP_HEADER_KEY and "Content-Type"
#else
  ck_assert_int_eq(u_map_count(resp.map_header), 1);
#endif
  ck_assert_str_eq(u_map_get(resp.map_header, MAP_HEADER_KEY), MAP_HEADER_VALUE);
  ck_assert_str_eq(resp.auth_realm, AUTH_REALM);
  ck_assert_ptr_ne(resp.binary_body, NULL);
  ck_assert_int_eq(resp.binary_body_length, BINARY_BODY_LEN);
  ck_assert_ptr_eq(resp.shared_data, (void *)SHARED_DATA);
  ck_assert_int_eq(resp.timeout, TIMEOUT);
  
  ck_assert_int_eq(ulfius_clean_response(&resp), U_OK);
}
END_TEST

START_TEST(test_endpoint)
{
  struct _u_instance u_instance;
  struct _u_endpoint endpoint;
  endpoint.http_method = "nope";
  endpoint.url_prefix = NULL;
  endpoint.url_format = NULL;
  endpoint.priority = 0;
  endpoint.callback_function = NULL;

  ck_assert_int_eq(ulfius_init_instance(&u_instance, 80, NULL, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint(&u_instance, &endpoint), U_ERROR_PARAMS);
  endpoint.http_method = NULL;
  endpoint.url_prefix = "nope";
  ck_assert_int_eq(ulfius_add_endpoint(&u_instance, &endpoint), U_ERROR_PARAMS);
  endpoint.url_prefix = NULL;
  endpoint.url_format = "nope";
  ck_assert_int_eq(ulfius_add_endpoint(&u_instance, &endpoint), U_ERROR_PARAMS);
  
  endpoint.callback_function = &callback_function_empty;
  ck_assert_int_eq(ulfius_add_endpoint(&u_instance, &endpoint), U_ERROR_PARAMS);
  endpoint.url_format = NULL;
  endpoint.url_prefix = "nope";
  ck_assert_int_eq(ulfius_add_endpoint(&u_instance, &endpoint), U_ERROR_PARAMS);
  endpoint.url_prefix = NULL;
  endpoint.http_method = "nope";
  ck_assert_int_eq(ulfius_add_endpoint(&u_instance, &endpoint), U_ERROR_PARAMS);
  endpoint.http_method = o_strdup("test0");
  endpoint.url_prefix = o_strdup("test0");
  endpoint.url_format = o_strdup("test0");
  ck_assert_int_eq(ulfius_add_endpoint(&u_instance, &endpoint), U_OK);
  
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "nope", NULL, NULL, 0, NULL, NULL), U_ERROR_PARAMS);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, NULL, "nope", NULL, 0, NULL, NULL), U_ERROR_PARAMS);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, NULL, NULL, "nope", 0, NULL, NULL), U_ERROR_PARAMS);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, NULL, "nope", "nope", 0, NULL, NULL), U_ERROR_PARAMS);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "nope", NULL, "nope", 0, NULL, NULL), U_ERROR_PARAMS);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "nope", "nope", NULL, 0, NULL, NULL), U_ERROR_PARAMS);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "nope", NULL, NULL, 0, &callback_function_empty, NULL), U_ERROR_PARAMS);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, NULL, "nope", NULL, 0, &callback_function_empty, NULL), U_ERROR_PARAMS);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, NULL, NULL, "nope", 0, &callback_function_empty, NULL), U_ERROR_PARAMS);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, NULL, "nope", "nope", 0, &callback_function_empty, NULL), U_ERROR_PARAMS);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "test1", "test1", "test1", 0, &callback_function_empty, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "test2", NULL, "test2", 0, &callback_function_empty, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "test3", "test3", NULL, 0, &callback_function_empty, NULL), U_OK);
  ck_assert_int_eq(ulfius_remove_endpoint(&u_instance, &endpoint), U_OK);
  ck_assert_int_eq(ulfius_remove_endpoint(&u_instance, &endpoint), U_ERROR_NOT_FOUND);
  ck_assert_int_eq(ulfius_remove_endpoint_by_val(&u_instance, "nope", "nope", NULL), U_ERROR_NOT_FOUND);
  ck_assert_int_eq(ulfius_remove_endpoint_by_val(&u_instance, "nope", NULL, "nope"), U_ERROR_NOT_FOUND);
  ck_assert_int_eq(ulfius_remove_endpoint_by_val(&u_instance, "test1", "test1", "test1"), U_OK);
  ck_assert_int_eq(ulfius_remove_endpoint_by_val(&u_instance, "test2", NULL, "test2"), U_OK);
  ck_assert_int_eq(ulfius_remove_endpoint_by_val(&u_instance, "test3", "test3", NULL), U_OK);
  ck_assert_int_eq(ulfius_set_default_endpoint(&u_instance, NULL, NULL), U_ERROR_PARAMS);
  ck_assert_int_eq(ulfius_set_default_endpoint(&u_instance, &callback_function_empty, NULL), U_OK);
  
  o_free(endpoint.http_method);
  o_free(endpoint.url_prefix);
  o_free(endpoint.url_format);
  ulfius_clean_instance(&u_instance);
}
END_TEST

START_TEST(test_endpoint_weirder)
{
  struct _u_instance u_instance;
  ck_assert_int_eq(ulfius_init_instance(&u_instance, 80, NULL, NULL), U_OK);
  
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "test1", "test1/test1/", "test1/test1/", 0, &callback_function_empty, NULL), U_OK);
  ck_assert_int_eq(u_instance.nb_endpoints, 1);
  ck_assert_int_eq(ulfius_remove_endpoint_by_val(&u_instance, "test1", "test1/test1", "test1/test1"), U_OK);
  ck_assert_int_eq(u_instance.nb_endpoints, 0);

  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "test1", "/test1/test1", "/test1/test1", 0, &callback_function_empty, NULL), U_OK);
  ck_assert_int_eq(u_instance.nb_endpoints, 1);
  ck_assert_int_eq(ulfius_remove_endpoint_by_val(&u_instance, "test1", "test1/test1", "test1/test1"), U_OK);
  ck_assert_int_eq(u_instance.nb_endpoints, 0);

  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "test1", "/test1/test1/", "/test1/test1/", 0, &callback_function_empty, NULL), U_OK);
  ck_assert_int_eq(u_instance.nb_endpoints, 1);
  ck_assert_int_eq(ulfius_remove_endpoint_by_val(&u_instance, "test1", "test1/test1", "test1/test1"), U_OK);
  ck_assert_int_eq(u_instance.nb_endpoints, 0);

  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "test1", "test1/test1", "test1/test1", 0, &callback_function_empty, NULL), U_OK);
  ck_assert_int_eq(u_instance.nb_endpoints, 1);
  ck_assert_int_eq(ulfius_remove_endpoint_by_val(&u_instance, "test1", "/test1/test1", "/test1/test1"), U_OK);
  ck_assert_int_eq(u_instance.nb_endpoints, 0);

  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "test1", "test1/test1", "test1/test1", 0, &callback_function_empty, NULL), U_OK);
  ck_assert_int_eq(u_instance.nb_endpoints, 1);
  ck_assert_int_eq(ulfius_remove_endpoint_by_val(&u_instance, "test1", "test1/test1/", "test1/test1/"), U_OK);
  ck_assert_int_eq(u_instance.nb_endpoints, 0);

  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "test1", "test1/test1", "test1/test1", 0, &callback_function_empty, NULL), U_OK);
  ck_assert_int_eq(u_instance.nb_endpoints, 1);
  ck_assert_int_eq(ulfius_remove_endpoint_by_val(&u_instance, "test1", "/test1/test1/", "/test1/test1/"), U_OK);
  ck_assert_int_eq(u_instance.nb_endpoints, 0);

  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "test1", "test1/test1/", "test1/test1/", 0, &callback_function_empty, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "test1", "/test1/test1", "/test1/test1", 0, &callback_function_empty, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "test1", "/test1/test1/", "/test1/test1/", 0, &callback_function_empty, NULL), U_OK);
  ck_assert_int_eq(u_instance.nb_endpoints, 3);
  ck_assert_int_eq(ulfius_remove_endpoint_by_val(&u_instance, "test1", "test1/test1", "test1/test1"), U_OK);
  ck_assert_int_eq(u_instance.nb_endpoints, 0);
  ck_assert_int_eq(ulfius_remove_endpoint_by_val(&u_instance, "test1", "test1/test1", "test1/test1"), U_ERROR_NOT_FOUND);
  
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "test1", "test1/test1", "test1/test1", 0, &callback_function_empty, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "test1", "test1/test1/", "test1/test1", 0, &callback_function_empty, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "test1", "/test1/test1", "test1/test1", 0, &callback_function_empty, NULL), U_OK);
  ck_assert_int_eq(u_instance.nb_endpoints, 3);
  ck_assert_int_eq(ulfius_remove_endpoint_by_val(&u_instance, "test1", "/test1/test1/", "/test1/test1/"), U_OK);
  ck_assert_int_eq(u_instance.nb_endpoints, 0);
  ck_assert_int_eq(ulfius_remove_endpoint_by_val(&u_instance, "test1", "/test1/test1/", "/test1/test1/"), U_ERROR_NOT_FOUND);
  
  ulfius_clean_instance(&u_instance);
}
END_TEST

START_TEST(test_ulfius_start_instance)
{
  struct _u_instance u_instance;
  struct MHD_OptionItem mhd_ops[3];
#ifndef U_DISABLE_WEBSOCKET
  unsigned int mhd_flags = MHD_USE_THREAD_PER_CONNECTION|MHD_ALLOW_UPGRADE;
#else
  unsigned int mhd_flags = MHD_USE_THREAD_PER_CONNECTION;
#endif

  ck_assert_int_eq(ulfius_init_instance(&u_instance, 8081, NULL, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", NULL, "test", 0, &callback_function_empty, NULL), U_OK);
  ck_assert_int_eq(ulfius_start_framework(&u_instance), U_OK);
  ck_assert_int_eq(ulfius_stop_framework(&u_instance), U_OK);
  ck_assert_int_eq(ulfius_start_framework(NULL), U_ERROR_PARAMS);
  ck_assert_int_eq(ulfius_start_secure_framework(NULL, NULL, NULL), U_ERROR_PARAMS);
  ck_assert_int_eq(ulfius_start_secure_framework(&u_instance, "error", "error"), U_ERROR_LIBMHD);
  ck_assert_int_eq(ulfius_start_secure_framework(&u_instance, "-----BEGIN PRIVATE KEY-----\
MIIEvwIBADANBgkqhkiG9w0BAQEFAASCBKkwggSlAgEAAoIBAQDr90HrswgEmln/\
rXeNqYq0boIvas5wu27hmeHDdGGKtkCWIWGAo9GUy45xqsI4mDl3bOWS+pmb/3yi\
+nhe+BmYHvEqUFo1JfUcVMxaNEbdd9REytMjKdOS+kkLf++BBRoZI/g8DggIu+Ri\
dOSypk+pUECyQxROsyCrB/FgXuKbyC4QNl7fqZxMSpzw7jsWCZiwFv4pu8kMqzDG\
2wTl/r/4STyK4Pj2TVa/JVzbZbH7VfcjT8MdMsXvKhlmPywjbqo70Hnmt3cnakYF\
X+07ncx/5mjYYd3eSFgiNXr7WNw2rhFKtfTUcjrqSw9FDxmHFWUU76mwJyUo02N9\
ViakSoQpAgMBAAECggEBAJp/VBwdJpzM6yxqyaJpZbXpvTeKuQw6zMjN1nIBG3SV\
DAjAZnSxziGcffGSmoQvt0CoflAT4MuxJkwXrwSPcUKWz9Sis82kwq4AH6TYIaYU\
NVmtazzUwAC1+2maJJjXXFUlpfy8Oypsy4ZjfvIxzmrPbuzI2t0Ej9kr5DDzL3BL\
CWQ/U7w7y4KC0Pnq1ueIzM+UJIfvI0ldUcXHWsAnjyQzwgFBC35qDOfDTw0YUJv+\
ElfFFcGYCA+9wlQyhM/zhAWqKgZ2mwAS6WykgbSc7j4NDjlmZwf4ZuTxbDUV1kBX\
pPH21snqO42CFpw9hRUAA0W0XydCIfUhH8/6tH9enQECgYEA+rM9f6cUk3c7aLWs\
hnauVqJuyGhgCkMyF9sSxgfcs87OVLNuGgaTIfwcT/7oxAY8G7sY44cbk1ZRhh7y\
6kf01xqiJeXxBQei1qiJxMb2gukvpeY81s2Mg9og5d9qbEhLzp8TdiRJHxLIiGwF\
xOM69CpugKN4T0Zum7EBGeSvmBECgYEA8PRG5SRTE4JwzGtLuTbMbjYTqyEjXAkB\
zo33a92znA0EXEeLCl845EUgzUkSkeN/T/uyWRjj0hrPU99UaaXHt3bc+lrDHrc7\
WFAR3QoAfFFJPIqqwiHcBDdTeAozQ8IOqFIxspl72RukuRdeQR3EdfcF9TUZyUbU\
k8SuRioggpkCgYA2scgnA3KvwYGKlKgxJc9fQ0zcGDlrw8E4BymPXsO9zs6hGAxb\
TTfoYDJlGX361kli22zQpvdTK6/ZjQL+LfiyvTLHBeWRbVsPbfGwpp+9a9ZjYVnA\
m1OeqIYo4Jc9TICNcZMzYTM6vkRVzwtrKw//mQpGsmNbGEilWvaciZHtoQKBgQDo\
FDBQtir6SJIConm9/ETtBlLtai6Xj+lYnK6qC1DaxkLj6tjF9a9jVh3g/DfRopBW\
ZnSCkpGkJcR54Up5s35ofCkdTdxPsmaLihuaje6nztc+Y8VS1LAIs41GunRkF/5s\
KzbI8kIyfAitag+Toms+v93SLwIWNo27gh3lYOANSQKBgQDIidSO3fzB+jzJh7R0\
Yy9ADWbBsLxc8u+sBdxmZBGl+l4YZWNPlQsnsafwcpJWT3le6N7Ri3iuOZw9KiGe\
QDkc7olxUZZ3pshg+cOORK6jVE8v6FeUlLnxpeAWa4C4JDawGPTOBct6bVBl5sxi\
7GaqDcEK1TSxc4cUaiiPDNNXQA==\
-----END PRIVATE KEY-----", "-----BEGIN CERTIFICATE-----\
MIIDhTCCAm2gAwIBAgIJANrO2RnCbURLMA0GCSqGSIb3DQEBCwUAMFkxCzAJBgNV\
BAYTAkFVMRMwEQYDVQQIDApTb21lLVN0YXRlMSEwHwYDVQQKDBhJbnRlcm5ldCBX\
aWRnaXRzIFB0eSBMdGQxEjAQBgNVBAMMCWxvY2FsaG9zdDAeFw0xNzA0MjgxNTA0\
NDVaFw0xODA0MjgxNTA0NDVaMFkxCzAJBgNVBAYTAkFVMRMwEQYDVQQIDApTb21l\
LVN0YXRlMSEwHwYDVQQKDBhJbnRlcm5ldCBXaWRnaXRzIFB0eSBMdGQxEjAQBgNV\
BAMMCWxvY2FsaG9zdDCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAOv3\
QeuzCASaWf+td42pirRugi9qznC7buGZ4cN0YYq2QJYhYYCj0ZTLjnGqwjiYOXds\
5ZL6mZv/fKL6eF74GZge8SpQWjUl9RxUzFo0Rt131ETK0yMp05L6SQt/74EFGhkj\
+DwOCAi75GJ05LKmT6lQQLJDFE6zIKsH8WBe4pvILhA2Xt+pnExKnPDuOxYJmLAW\
/im7yQyrMMbbBOX+v/hJPIrg+PZNVr8lXNtlsftV9yNPwx0yxe8qGWY/LCNuqjvQ\
eea3dydqRgVf7TudzH/maNhh3d5IWCI1evtY3DauEUq19NRyOupLD0UPGYcVZRTv\
qbAnJSjTY31WJqRKhCkCAwEAAaNQME4wHQYDVR0OBBYEFPFfmGA3jO9koBZNGNZC\
T/dZHZyHMB8GA1UdIwQYMBaAFPFfmGA3jO9koBZNGNZCT/dZHZyHMAwGA1UdEwQF\
MAMBAf8wDQYJKoZIhvcNAQELBQADggEBAIc8Yuom4vz82izNEV+9bcCvuabcVwLH\
Qgpv5Nzy/W+1hDoqfMfKNwOSdUB7jZoDaNDG1WhjKGGCLTAx4Hx+q1LwUXvu4Bs1\
woocge65bl85h10l2TxxnlT5BIJezm5r3NiZSwOK2zxxIEyL4vh+b/xqQblBEkR3\
e4/A4Ugn9Egh8GdpF4klGp4MjjpRyAVI7BDaleAhvDSfPmm7ylHJ2y7CLI9ApOQY\
glwRuTmowAZQtaSiE1Ox7QtWj858HDzzTZyFWRG/MNqQptn7AMTPJv3DivNfDNPj\
fYxFAheH3CjryHqqR9DD+d9396W8mqEaUp+plMwSjpcTDSR4rEQkUJg=\
-----END CERTIFICATE-----"), U_OK);
  ck_assert_int_eq(ulfius_stop_framework(&u_instance), U_OK);
  
  mhd_ops[0].option = MHD_OPTION_NOTIFY_COMPLETED;
  mhd_ops[0].value = (intptr_t)mhd_request_completed;
  mhd_ops[0].ptr_value = NULL;

  mhd_ops[1].option = MHD_OPTION_URI_LOG_CALLBACK;
  mhd_ops[1].value = (intptr_t)ulfius_uri_logger;
  mhd_ops[1].ptr_value = NULL;

  mhd_ops[2].option = MHD_OPTION_END;
  mhd_ops[2].value = 0;
  mhd_ops[2].ptr_value = NULL;
  ck_assert_int_eq(ulfius_start_framework_with_mhd_options(&u_instance, mhd_flags, mhd_ops), U_OK);
  ck_assert_int_eq(ulfius_stop_framework(&u_instance), U_OK);
  
  ulfius_clean_instance(&u_instance);
}
END_TEST

START_TEST(test_url_encode_decode)
{
  char * raw = "Hëllô Ulfius%3B$#!/?*[]", * raw_encoded = "H%C3%ABll%C3%B4+Ulfius%253B$%23!%2F%3F*%5B%5D", * easy_raw = "grut_1234", * result;
  
  // Test ulfius_url_encode
  ck_assert_ptr_eq(ulfius_url_encode(NULL), NULL);
  ck_assert_ptr_ne((result = ulfius_url_encode(raw)), NULL);
  ck_assert_str_eq(result, raw_encoded);
  o_free(result);
  ck_assert_ptr_ne((result = ulfius_url_encode(easy_raw)), NULL);
  ck_assert_str_eq(result, easy_raw);
  o_free(result);
  
  // Test ulfius_url_decode
  ck_assert_ptr_eq(ulfius_url_decode(NULL), NULL);
  ck_assert_ptr_ne((result = ulfius_url_decode(raw_encoded)), NULL);
  ck_assert_str_eq(result, raw);
  o_free(result);
  ck_assert_ptr_ne((result = ulfius_url_decode(easy_raw)), NULL);
  ck_assert_str_eq(result, easy_raw);
  o_free(result);
}
END_TEST

static Suite *ulfius_suite(void)
{
	Suite *s;
	TCase *tc_core;

	s = suite_create("Ulfius core function tests");
	tc_core = tcase_create("test_ulfius_core");
	tcase_add_test(tc_core, test_ulfius_init_instance);
	tcase_add_test(tc_core, test_ulfius_request);
	tcase_add_test(tc_core, test_ulfius_set_request_properties);
	tcase_add_test(tc_core, test_ulfius_request_limits);
	tcase_add_test(tc_core, test_ulfius_response);
	tcase_add_test(tc_core, test_ulfius_set_response_properties);
	tcase_add_test(tc_core, test_endpoint);
	tcase_add_test(tc_core, test_endpoint_weirder);
	tcase_add_test(tc_core, test_ulfius_start_instance);
	tcase_add_test(tc_core, test_url_encode_decode);
	tcase_set_timeout(tc_core, 30);
	suite_add_tcase(s, tc_core);

	return s;
}

int main(int argc, char *argv[])
{
  int number_failed;
  Suite *s;
  SRunner *sr;
  //y_init_logs("Ulfius", Y_LOG_MODE_CONSOLE, Y_LOG_LEVEL_DEBUG, NULL, "Starting Ulfius core tests");
  ulfius_global_init();
  s = ulfius_suite();
  sr = srunner_create(s);

  srunner_run_all(sr, CK_VERBOSE);
  number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);
  
  ulfius_global_close();
  //y_close_logs();
	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
