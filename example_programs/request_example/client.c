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

#include <ulfius.h>
#include <u_example.h>

#define SERVER_URL_PREFIX "http://localhost:7778/curl"

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
        len = o_strlen(to_return) + o_strlen(line) + 1;
        to_return = o_realloc(to_return, (len+1)*sizeof(char));
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

void print_response(struct _u_response * response) {
  if (response != NULL) {
    char * headers = print_map(response->map_header);
    char response_body[response->binary_body_length + 1];
    o_strncpy(response_body, response->binary_body, response->binary_body_length);
    response_body[response->binary_body_length] = '\0';
    printf("protocol is\n%s\n\n  headers are \n%s\n\n  body is \n%s\n\n",
           response->protocol, headers, response_body);
    o_free(headers);
  }
}

int main (int argc, char **argv) {
  
  char * string_body = "param1=one&param2=two";
  json_t * json_body = json_object();
  struct _u_response response;
  int res;
  
  json_object_set_new(json_body, "param1", json_string("one"));
  json_object_set_new(json_body, "param2", json_string("two"));
  
  struct _u_request req_list[8];
  ulfius_init_request(&req_list[0]);
  ulfius_init_request(&req_list[1]);
  ulfius_init_request(&req_list[2]);
  ulfius_init_request(&req_list[3]);
  ulfius_init_request(&req_list[4]);
  ulfius_init_request(&req_list[5]);
  ulfius_init_request(&req_list[6]);
  ulfius_init_request(&req_list[7]);
  
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
  
  printf("Press <enter> to run get test\n");
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
  
  return 0;
}
