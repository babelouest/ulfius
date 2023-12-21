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
 */

#include <jansson.h>
#include <string.h>
#include <ulfius.h>

#include "u_example.h"

#define SERVER_URL_PREFIX "http://localhost:7778/curl"

#if defined(U_DISABLE_CURL) || defined(U_DISABLE_JANSSON)
#error You must build ulfius with libcurl and jansson support enabled to compile this example, check the install documentation
#else

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

void print_response(struct _u_response * response) {
  if (response != NULL) {
    char * headers = print_map(response->map_header);
    char response_body[response->binary_body_length + 1];
    o_strncpy(response_body, (const char *)response->binary_body, response->binary_body_length);
    response_body[response->binary_body_length] = '\0';
    printf("protocol is\n%s\n\n  headers are \n%s\n\n  body is \n%s\n\n",
           response->protocol, headers, response_body);
    o_free(headers);
  }
}

void print_response_limit(struct _u_response * response) {
  if (response != NULL) {
    char * headers = print_map(response->map_header);
    char response_body[response->binary_body_length + 1];
    o_strncpy(response_body, (const char *)response->binary_body, response->binary_body_length);
    response_body[response->binary_body_length] = '\0';
    printf("protocol is\n%s\n\n  headers (%u) are \n%s\n\n  body (%zu) is \n'%s'\n\n",
           response->protocol, u_map_count(response->map_header), headers, response->binary_body_length, response_body);
    o_free(headers);
  }
}

int main (void) {
  
  char * string_body = "param1=one&param2=two";
  json_t * json_body = json_object();
  struct _u_response response;
  int res;
  
  y_init_logs("Ulfius request", Y_LOG_MODE_CONSOLE, Y_LOG_LEVEL_DEBUG, NULL, "Starting Ulfius framework request tests");
  json_object_set_new(json_body, "param1", json_string("one"));
  json_object_set_new(json_body, "param2", json_string("two"));
  
  struct _u_request req_list[9];
  ulfius_init_request(&req_list[0]);
  ulfius_init_request(&req_list[1]);
  ulfius_init_request(&req_list[2]);
  ulfius_init_request(&req_list[3]);
  ulfius_init_request(&req_list[4]);
  ulfius_init_request(&req_list[5]);
  ulfius_init_request(&req_list[6]);
  ulfius_init_request(&req_list[7]);
  ulfius_init_request(&req_list[8]);
  
  // Parameters in url
  ulfius_set_request_properties(&req_list[0],
                                U_OPT_HTTP_VERB, "GET",
                                U_OPT_HTTP_URL, SERVER_URL_PREFIX,
                                U_OPT_HTTP_URL_APPEND, "/get/",
                                U_OPT_TIMEOUT, 20,
                                U_OPT_URL_PARAMETER, "test", "one",
                                U_OPT_URL_PARAMETER, "other_test", "two",
                                U_OPT_NONE); // Required to close the parameters list
  
  // No parameters
  ulfius_set_request_properties(&req_list[1],
                                U_OPT_HTTP_VERB, "DELETE",
                                U_OPT_HTTP_URL, SERVER_URL_PREFIX,
                                U_OPT_HTTP_URL_APPEND, "/delete/",
                                U_OPT_TIMEOUT, 20,
                                U_OPT_NONE); // Required to close the parameters list
  
  // Parameters in post_map and string_body
  ulfius_set_request_properties(&req_list[2],
                                U_OPT_HTTP_VERB, "POST",
                                U_OPT_HTTP_URL, SERVER_URL_PREFIX,
                                U_OPT_HTTP_URL_APPEND, "/post/param/",
                                U_OPT_TIMEOUT, 20,
                                U_OPT_BINARY_BODY, string_body, o_strlen(string_body),
                                U_OPT_POST_BODY_PARAMETER, "third_test", "three",
                                U_OPT_POST_BODY_PARAMETER, "fourth_test", "four",
                                U_OPT_POST_BODY_PARAMETER, "extreme_test", "Here ! are %9_ some $ ö\\)]= special châraçters",
                                U_OPT_NONE); // Required to close the parameters list
  
  // Paremeters in string body, header MHD_HTTP_POST_ENCODING_FORM_URLENCODED
  ulfius_set_request_properties(&req_list[3],
                                U_OPT_HTTP_VERB, "POST",
                                U_OPT_HTTP_URL, SERVER_URL_PREFIX,
                                U_OPT_HTTP_URL_APPEND, "/post/plain/",
                                U_OPT_TIMEOUT, 20,
                                U_OPT_BINARY_BODY, string_body, o_strlen(string_body),
                                U_OPT_HEADER_PARAMETER, "Content-Type", MHD_HTTP_POST_ENCODING_FORM_URLENCODED,
                                U_OPT_NONE); // Required to close the parameters list
  
  // Parameters in json_body
  ulfius_set_request_properties(&req_list[4],
                                U_OPT_HTTP_VERB, "POST",
                                U_OPT_HTTP_URL, SERVER_URL_PREFIX,
                                U_OPT_HTTP_URL_APPEND, "/post/json/",
                                U_OPT_TIMEOUT, 20,
                                U_OPT_JSON_BODY, json_body,
                                U_OPT_URL_PARAMETER, "test", "one",
                                U_OPT_URL_PARAMETER, "other_test", "two",
                                U_OPT_NONE); // Required to close the parameters list
  
  // Paremeters in string body, header MHD_HTTP_POST_ENCODING_FORM_URLENCODED
  ulfius_set_request_properties(&req_list[5],
                                U_OPT_HTTP_VERB, "PUT",
                                U_OPT_HTTP_URL, SERVER_URL_PREFIX,
                                U_OPT_HTTP_URL_APPEND, "/put/plain/",
                                U_OPT_TIMEOUT, 20,
                                U_OPT_BINARY_BODY, string_body, o_strlen(string_body),
                                U_OPT_HEADER_PARAMETER, "Content-Type", MHD_HTTP_POST_ENCODING_FORM_URLENCODED,
                                U_OPT_NONE); // Required to close the parameters list
  
  // Parameters in json_body
  ulfius_set_request_properties(&req_list[6],
                                U_OPT_HTTP_VERB, "PUT",
                                U_OPT_HTTP_URL, SERVER_URL_PREFIX,
                                U_OPT_HTTP_URL_APPEND, "/put/json/",
                                U_OPT_TIMEOUT, 20,
                                U_OPT_JSON_BODY, json_body,
                                U_OPT_URL_PARAMETER, "test", "one",
                                U_OPT_URL_PARAMETER, "other_test", "two",
                                U_OPT_NONE); // Required to close the parameters list
  
  // Parameters in post_map
  ulfius_set_request_properties(&req_list[7],
                                U_OPT_HTTP_VERB, "POST",
                                U_OPT_HTTP_URL, SERVER_URL_PREFIX,
                                U_OPT_HTTP_URL_APPEND, "/post/param/",
                                U_OPT_TIMEOUT, 20,
                                U_OPT_POST_BODY_PARAMETER, "third_test", "three",
                                U_OPT_POST_BODY_PARAMETER, "fourth_test", "four",
                                U_OPT_POST_BODY_PARAMETER, "extreme_test", "Here ! are %9_ some $ ö\\)]= special châraçters",
                                U_OPT_NONE); // Required to close the parameters list
  
  // Limit in response
  ulfius_set_request_properties(&req_list[8],
                                U_OPT_HTTP_VERB, "GET",
                                U_OPT_HTTP_URL, SERVER_URL_PREFIX,
                                U_OPT_HTTP_URL_APPEND, "/limit/",
                                U_OPT_TIMEOUT, 20,
                                U_OPT_URL_PARAMETER, "test", "one",
                                U_OPT_URL_PARAMETER, "other_test", "two",
                                U_OPT_NONE); // Required to close the parameters list

  /*printf("Press <enter> to run get test\n");
  getchar();
  ulfius_init_response(&response);
  res = ulfius_send_http_request(&req_list[0], &response);
  if (res == U_OK) {
    print_response(&response);
  } else {
    printf("Error in http request: %d\n", res);
  }
  ulfius_clean_response(&response);
  
  printf("Press <enter> to run get test with no interest on the response\n");
  getchar();
  printf("Request sent, result is %d\n", ulfius_send_http_request(&req_list[0], NULL));
  
  printf("Press <enter> to run delete test\n");
  getchar();
  ulfius_init_response(&response);
  res = ulfius_send_http_request(&req_list[1], &response);
  if (res == U_OK) {
    print_response(&response);
  } else {
    printf("Error in http request: %d\n", res);
  }
  ulfius_clean_response(&response);
  
  printf("Press <enter> to run post parameters test\n");
  getchar();
  ulfius_init_response(&response);
  res = ulfius_send_http_request(&req_list[2], &response);
  if (res == U_OK) {
    print_response(&response);
  } else {
    printf("Error in http request: %d\n", res);
  }
  ulfius_clean_response(&response);
  
  printf("Press <enter> to run post plain test\n");
  getchar();
  ulfius_init_response(&response);
  res = ulfius_send_http_request(&req_list[3], &response);
  if (res == U_OK) {
    print_response(&response);
  } else {
    printf("Error in http request: %d\n", res);
  }
  ulfius_clean_response(&response);
  
  printf("Press <enter> to run post json test\n");
  getchar();
  ulfius_init_response(&response);
  res = ulfius_send_http_request(&req_list[4], &response);
  if (res == U_OK) {
    print_response(&response);
  } else {
    printf("Error in http request: %d\n", res);
  }
  ulfius_clean_response(&response);
  
  printf("Press <enter> to run put plain test\n");
  getchar();
  ulfius_init_response(&response);
  res = ulfius_send_http_request(&req_list[5], &response);
  if (res == U_OK) {
    print_response(&response);
  } else {
    printf("Error in http request: %d\n", res);
  }
  ulfius_clean_response(&response);
  
  printf("Press <enter> to run put json test\n");
  getchar();
  ulfius_init_response(&response);
  res = ulfius_send_http_request(&req_list[6], &response);
  if (res == U_OK) {
    print_response(&response);
  } else {
    printf("Error in http request: %d\n", res);
  }
  ulfius_clean_response(&response);
  
  printf("Press <enter> to run post only test\n");
  getchar();
  ulfius_init_response(&response);
  res = ulfius_send_http_request(&req_list[7], &response);
  if (res == U_OK) {
    print_response(&response);
  } else {
    printf("Error in http request: %d\n", res);
  }
  ulfius_clean_response(&response);*/
  
  printf("Press <enter> to run response limit test\n");
  getchar();
  ulfius_init_response(&response);
  res = ulfius_send_http_request_with_limit(&req_list[8], &response, 32, 2);
  if (res == U_OK) {
    print_response_limit(&response);
  } else {
    printf("Error in http request: %d\n", res);
  }
  ulfius_clean_response(&response);
  
  // Wait for the user to press <enter> on the console to quit the application
  printf("Press <enter> to quit test\n");
  getchar();
  json_decref(json_body);
  ulfius_clean_request(&req_list[0]);
  ulfius_clean_request(&req_list[1]);
  ulfius_clean_request(&req_list[2]);
  ulfius_clean_request(&req_list[3]);
  ulfius_clean_request(&req_list[4]);
  ulfius_clean_request(&req_list[5]);
  ulfius_clean_request(&req_list[6]);
  ulfius_clean_request(&req_list[7]);
  ulfius_clean_request(&req_list[8]);
  y_close_logs();
  
  return 0;
}
#endif
