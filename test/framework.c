/* Public domain, no copyright. Use at your own risk. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#ifndef _WIN32
	#include <netinet/in.h>
#else
	#include <unistd.h>
#endif
#include <inttypes.h>

#include <check.h>
#include "../include/ulfius.h"

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

int callback_function_empty(const struct _u_request * request, struct _u_response * response, void * user_data) {
  return U_CALLBACK_CONTINUE;
}

int callback_function_return_url(const struct _u_request * request, struct _u_response * response, void * user_data) {
  ulfius_set_string_body_response(response, 200, request->http_url);
  return U_CALLBACK_CONTINUE;
}

int callback_function_error(const struct _u_request * request, struct _u_response * response, void * user_data) {
  return U_CALLBACK_ERROR;
}

int callback_function_unauthorized(const struct _u_request * request, struct _u_response * response, void * user_data) {
  return U_CALLBACK_UNAUTHORIZED;
}

int callback_function_check_auth(const struct _u_request * request, struct _u_response * response, void * user_data) {
  if (o_strcmp("user", request->auth_basic_user) == 0 && o_strcmp("password", request->auth_basic_password) == 0) {
    return U_CALLBACK_CONTINUE;
  } else {
    return U_CALLBACK_UNAUTHORIZED;
  }
}

int callback_function_param(const struct _u_request * request, struct _u_response * response, void * user_data) {
  char * param3, * body;
  
  if (u_map_has_key(request->map_url, "param3")) {
    param3 = msprintf(", param3 is %s", u_map_get(request->map_url, "param3"));
  } else {
    param3 = o_strdup("");
  }
  body = msprintf("param1 is %s, param2 is %s%s", u_map_get(request->map_url, "param1"), u_map_get(request->map_url, "param2"), param3);
  ulfius_set_string_body_response(response, 200, body);
  o_free(body);
  o_free(param3);
  return U_CALLBACK_CONTINUE;
}

int callback_function_body_param(const struct _u_request * request, struct _u_response * response, void * user_data) {
  char * body;
  
  body = msprintf("param1 is %s, param2 is %s", u_map_get(request->map_post_body, "param1"), u_map_get(request->map_post_body, "param2"));
  ulfius_set_string_body_response(response, 200, body);
  o_free(body);
  return U_CALLBACK_CONTINUE;
}

int callback_function_header_param(const struct _u_request * request, struct _u_response * response, void * user_data) {
  char * body;
  
  body = msprintf("param1 is %s, param2 is %s", u_map_get(request->map_header, "param1"), u_map_get(request->map_header, "param2"));
  ulfius_set_string_body_response(response, 200, body);
  o_free(body);
  return U_CALLBACK_CONTINUE;
}

int callback_function_cookie_param(const struct _u_request * request, struct _u_response * response, void * user_data) {
  char * body;
  
  body = msprintf("param1 is %s", u_map_get(request->map_cookie, "param1"));
  ulfius_set_string_body_response(response, 200, body);
  ulfius_add_cookie_to_response(response, "param2", "value_cookie", NULL, 100, "localhost", "/cookie", 0, 1);
  o_free(body);
  return U_CALLBACK_CONTINUE;
}

int callback_function_multiple_continue(const struct _u_request * request, struct _u_response * response, void * user_data) {
  if (response->binary_body != NULL) {
    char * body = msprintf("%.*s\n%s", response->binary_body_length, (char*)response->binary_body, request->http_url);
    ulfius_set_string_body_response(response, 200, body);
    o_free(body);
  } else {
    ulfius_set_string_body_response(response, 200, request->http_url);
  }
  return U_CALLBACK_CONTINUE;
}

int callback_function_multiple_complete(const struct _u_request * request, struct _u_response * response, void * user_data) {
  if (response->binary_body != NULL) {
    char * body = msprintf("%.*s\n%s", response->binary_body_length, (char*)response->binary_body, request->http_url);
    ulfius_set_string_body_response(response, 200, body);
    o_free(body);
  } else {
    ulfius_set_string_body_response(response, 200, request->http_url);
  }
  return U_CALLBACK_COMPLETE;
}


ssize_t stream_data (void * cls, uint64_t pos, char * buf, size_t max) {
  usleep(100);
  if (pos <= 100) {
      snprintf(buf, max, "%s %" PRIu64 "\n", (char *)cls, pos + 1);
      return o_strlen(buf);
  } else {
    return MHD_CONTENT_READER_END_OF_STREAM;
  }
}

void free_stream_data(void * cls) {
  o_free(cls);
}

int callback_function_stream (const struct _u_request * request, struct _u_response * response, void * user_data) {
  ulfius_set_stream_response(response, 200, stream_data, free_stream_data, U_STREAM_SIZE_UNKOWN, 32 * 1024, o_strdup("stream test"));
  return U_OK;
}

size_t my_write_body(void * contents, size_t size, size_t nmemb, void * user_data) {
  ck_assert_int_eq(o_strncmp((char *)contents, "stream test ", o_strlen("stream test ")), 0);
  ck_assert_int_ne(strtol((char *)contents + o_strlen("stream test "), NULL, 10), 0);
  return size * nmemb;
}

START_TEST(test_ulfius_simple_endpoint)
{
  struct _u_instance u_instance;
  struct _u_request request;
  struct _u_response response;
  
  ck_assert_int_eq(ulfius_init_instance(&u_instance, 8080, NULL, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "empty", NULL, 0, &callback_function_empty, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "POST", "empty", NULL, 0, &callback_function_empty, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "PUT", "empty", NULL, 0, &callback_function_empty, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "DELETE", "empty", NULL, 0, &callback_function_empty, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "url", NULL, 0, &callback_function_return_url, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "*", "url", NULL, 0, &callback_function_return_url, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "error", NULL, 0, &callback_function_error, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "unauthorized", NULL, 0, &callback_function_unauthorized, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "check_auth", NULL, 0, &callback_function_check_auth, NULL), U_OK);
  ck_assert_int_eq(ulfius_start_framework(&u_instance), U_OK);
  
  ulfius_init_request(&request);
  request.http_url = o_strdup("http://localhost:8080/nope");
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 404);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  ulfius_init_request(&request);
  request.http_url = o_strdup("http://localhost:8080/empty");
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  ulfius_init_request(&request);
  request.http_verb = o_strdup("POST");
  request.http_url = o_strdup("http://localhost:8080/empty");
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  ulfius_init_request(&request);
  request.http_verb = o_strdup("PUT");
  request.http_url = o_strdup("http://localhost:8080/empty");
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  ulfius_init_request(&request);
  request.http_verb = o_strdup("DELETE");
  request.http_url = o_strdup("http://localhost:8080/empty");
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  ulfius_init_request(&request);
  request.http_verb = o_strdup("OPTION");
  request.http_url = o_strdup("http://localhost:8080/empty");
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 404);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  ulfius_init_request(&request);
  request.http_url = o_strdup("http://localhost:8080/url");
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_eq(o_strncmp(response.binary_body, "/url", o_strlen("/url")), 0);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  ulfius_init_request(&request);
  request.http_verb = o_strdup("OPTION");
  request.http_url = o_strdup("http://localhost:8080/url");
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_eq(o_strncmp(response.binary_body, "/url", o_strlen("/url")), 0);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  ulfius_init_request(&request);
  request.http_verb = o_strdup("test");
  request.http_url = o_strdup("http://localhost:8080/url");
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_eq(o_strncmp(response.binary_body, "/url", o_strlen("/url")), 0);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  ulfius_init_request(&request);
  request.http_url = o_strdup("http://localhost:8080/error");
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 500);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  ulfius_init_request(&request);
  request.http_url = o_strdup("http://localhost:8080/unauthorized");
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 401);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  ulfius_init_request(&request);
  request.http_url = o_strdup("http://localhost:8080/check_auth");
  request.auth_basic_user = o_strdup("nope");
  request.auth_basic_password = o_strdup("nope");
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 401);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  ulfius_init_request(&request);
  request.http_url = o_strdup("http://localhost:8080/check_auth");
  request.auth_basic_user = o_strdup("user");
  request.auth_basic_password = o_strdup("password");
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  ulfius_clean_instance(&u_instance);
}
END_TEST

START_TEST(test_ulfius_endpoint_parameters)
{
  struct _u_instance u_instance;
  struct _u_request request;
  struct _u_response response;
  
  ck_assert_int_eq(ulfius_init_instance(&u_instance, 8080, NULL, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "param", "/:param1/@param2/", 0, &callback_function_param, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "param", "/:param1/@param2/:param3/:param3", 0, &callback_function_param, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "POST", "param", NULL, 0, &callback_function_body_param, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "header", NULL, 0, &callback_function_header_param, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "cookie", NULL, 0, &callback_function_cookie_param, NULL), U_OK);
  ck_assert_int_eq(ulfius_start_framework(&u_instance), U_OK);
  
  ulfius_init_request(&request);
  request.http_url = o_strdup("http://localhost:8080/param/value1/value2");
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_eq(o_strncmp(response.binary_body, "param1 is value1, param2 is value2", o_strlen("param1 is value1, param2 is value2")), 0);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  ulfius_init_request(&request);
  request.http_url = o_strdup("http://localhost:8080/param/value1/value2/value3.1/value3.2");
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_eq(o_strncmp(response.binary_body, "param1 is value1, param2 is value2, param3 is value3.1,value3.2", o_strlen("param1 is value1, param2 is value2, param3 is value3.1,value3.2")), 0);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  ulfius_init_request(&request);
  request.http_verb = o_strdup("POST");
  request.http_url = o_strdup("http://localhost:8080/param/");
  u_map_put(request.map_post_body, "param1", "value3");
  u_map_put(request.map_post_body, "param2", "value4");
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_eq(o_strncmp(response.binary_body, "param1 is value3, param2 is value4", o_strlen("param1 is value3, param2 is value4")), 0);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  ulfius_init_request(&request);
  request.http_url = o_strdup("http://localhost:8080/header");
  u_map_put(request.map_header, "param1", "value5");
  u_map_put(request.map_header, "param2", "value6");
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_eq(o_strncmp(response.binary_body, "param1 is value5, param2 is value6", o_strlen("param1 is value5, param2 is value6")), 0);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  ulfius_init_request(&request);
  request.http_url = o_strdup("http://localhost:8080/cookie");
  u_map_put(request.map_cookie, "param1", "value7");
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_eq(o_strncmp(response.binary_body, "param1 is value7", o_strlen("param1 is value7")), 0);
  ck_assert_str_eq(u_map_get(response.map_header, "Set-Cookie"), "param2=value_cookie; Max-Age=100; Domain=localhost; Path=/cookie; HttpOnly");
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  ulfius_clean_instance(&u_instance);
}
END_TEST

START_TEST(test_ulfius_endpoint_injection)
{
  struct _u_instance u_instance;
  struct _u_request request;
  struct _u_response response;
  
  ck_assert_int_eq(ulfius_init_instance(&u_instance, 8080, NULL, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "inject1", NULL, 0, &callback_function_empty, NULL), U_OK);
  ck_assert_int_eq(ulfius_start_framework(&u_instance), U_OK);
  
  ulfius_init_request(&request);
  request.http_url = o_strdup("http://localhost:8080/inject1");
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  ulfius_init_request(&request);
  request.http_url = o_strdup("http://localhost:8080/inject2");
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 404);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "inject2", NULL, 0, &callback_function_empty, NULL), U_OK);
  
  ulfius_init_request(&request);
  request.http_url = o_strdup("http://localhost:8080/inject2");
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  ck_assert_int_eq(ulfius_remove_endpoint_by_val(&u_instance, "GET", "inject2", NULL), U_OK);
  
  ulfius_init_request(&request);
  request.http_url = o_strdup("http://localhost:8080/inject2");
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 404);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  ulfius_clean_instance(&u_instance);
}
END_TEST

START_TEST(test_ulfius_endpoint_multiple)
{
  struct _u_instance u_instance;
  struct _u_request request;
  struct _u_response response;
  
  ck_assert_int_eq(ulfius_init_instance(&u_instance, 8080, NULL, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "multiple", "*", 0, &callback_function_multiple_continue, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "multiple", "/:param1/*", 1, &callback_function_multiple_continue, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "multiple", "/:param1/:param2/*", 2, &callback_function_multiple_continue, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "multiple", "/:param1/:param2/:param3", 3, &callback_function_multiple_continue, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "multiple_complete", "*", 0, &callback_function_multiple_continue, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "multiple_complete", "/:param1/*", 1, &callback_function_multiple_complete, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "multiple_complete", "/:param1/:param2/*", 2, &callback_function_multiple_continue, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "multiple_complete", "/:param1/:param2/:param3", 3, &callback_function_multiple_continue, NULL), U_OK);
  ck_assert_int_eq(ulfius_start_framework(&u_instance), U_OK);
  
  ulfius_init_request(&request);
  request.http_url = o_strdup("http://localhost:8080/multiple");
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_eq(o_strncmp(response.binary_body, "/multiple", o_strlen("/multiple")), 0);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  ulfius_init_request(&request);
  request.http_url = o_strdup("http://localhost:8080/multiple/value1");
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_eq(o_strncmp(response.binary_body, "/multiple/value1\n/multiple/value1", o_strlen("/multiple/value1\n/multiple/value1")), 0);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  ulfius_init_request(&request);
  request.http_url = o_strdup("http://localhost:8080/multiple/value1/value2");
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_eq(o_strncmp(response.binary_body, "/multiple/value1/value2\n/multiple/value1/value2\n/multiple/value1/value2", o_strlen("/multiple/value1/value2\n/multiple/value1/value2\n/multiple/value1/value2")), 0);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  ulfius_init_request(&request);
  request.http_url = o_strdup("http://localhost:8080/multiple/value1/value2/value3");
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_eq(o_strncmp(response.binary_body, "/multiple/value1/value2/value3\n/multiple/value1/value2/value3\n/multiple/value1/value2/value3\n/multiple/value1/value2/value3", o_strlen("/multiple/value1/value2/value3\n/multiple/value1/value2/value3\n/multiple/value1/value2/value3\n/multiple/value1/value2/value3")), 0);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  ulfius_init_request(&request);
  request.http_url = o_strdup("http://localhost:8080/nope/value1/value2/value3/value4");
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 404);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  ulfius_init_request(&request);
  request.http_url = o_strdup("http://localhost:8080/multiple_complete/value1/value2/value3");
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_eq(o_strncmp(response.binary_body, "/multiple_complete/value1/value2/value3\n/multiple_complete/value1/value2/value3", o_strlen("/multiple_complete/value1/value2/value3\n/multiple_complete/value1/value2/value3")), 0);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  ulfius_clean_instance(&u_instance);
}
END_TEST

START_TEST(test_ulfius_endpoint_stream)
{
  struct _u_instance u_instance;
  struct _u_request request;
  struct _u_response response;
  
  ck_assert_int_eq(ulfius_init_instance(&u_instance, 8080, NULL, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "stream", NULL, 0, &callback_function_stream, NULL), U_OK);
  ck_assert_int_eq(ulfius_start_framework(&u_instance), U_OK);
  
  ulfius_init_request(&request);
  request.http_url = o_strdup("http://localhost:8080/stream");
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_streaming_request(&request, &response, my_write_body, NULL), U_OK);
  ck_assert_int_eq(response.status, 200);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  ulfius_clean_instance(&u_instance);
}
END_TEST

static Suite *ulfius_suite(void)
{
	Suite *s;
	TCase *tc_core;

	s = suite_create("Ulfius framework function tests");
	tc_core = tcase_create("test_ulfius_framework");
	tcase_add_test(tc_core, test_ulfius_simple_endpoint);
	tcase_add_test(tc_core, test_ulfius_endpoint_parameters);
	tcase_add_test(tc_core, test_ulfius_endpoint_injection);
	tcase_add_test(tc_core, test_ulfius_endpoint_multiple);
	tcase_add_test(tc_core, test_ulfius_endpoint_stream);
	tcase_set_timeout(tc_core, 30);
	suite_add_tcase(s, tc_core);

	return s;
}

int main(int argc, char *argv[])
{
  int number_failed;
  Suite *s;
  SRunner *sr;
  y_init_logs("Ulfius", Y_LOG_MODE_CONSOLE, Y_LOG_LEVEL_DEBUG, NULL, "Starting Ulfius core tests");
  s = ulfius_suite();
  sr = srunner_create(s);

  srunner_run_all(sr, CK_VERBOSE);
  number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);
  
  y_close_logs();
	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
