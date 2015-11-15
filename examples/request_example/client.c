/**
 * 
 * Ulfius Framework example program
 * 
 * This example program send several requests to describe
 * the function ulfius_request_http behaviour
 *  
 * Copyright 2015 Nicolas Mora <mail@babelouest.org>
 * 
 * License MIT
 *
 */

#include <string.h>
#include <jansson.h>

#include "../../src/ulfius.h"

#define SERVER_URL_PREFIX "http://localhost:7778/curl"

/**
 * decode a u_map into a string
 */
char * print_map(const struct _u_map * map) {
  char * line, * to_return = NULL, **keys, * value;
  int len, i;
  if (map != NULL) {
    keys = u_map_enum_keys(map);
    for (i=0; keys[i] != NULL; i++) {
      value = u_map_get(map, keys[i]);
      len = snprintf(NULL, 0, "key is %s, value is %s\n", keys[i], value);
      line = malloc((len+1)*sizeof(char));
      snprintf(line, (len+1), "key is %s, value is %s\n", keys[i], value);
      free(value);
      if (to_return != NULL) {
        len = strlen(to_return) + strlen(line) + 1;
        to_return = realloc(to_return, (len+1)*sizeof(char));
      } else {
        to_return = malloc((strlen(line) + 1)*sizeof(char));
        to_return[0] = 0;
      }
      strcat(to_return, line);
      free(line);
    }
    u_map_clean_enum(keys);
    return to_return;
  } else {
    return NULL;
  }
}

void print_response(struct _u_response * response) {
  if (response != NULL) {
    char * headers = print_map(response->map_header), * json_body = json_dumps(response->json_body, JSON_INDENT(2));
    printf("protocol is\n%s\n\n  headers are \n%s\n\n  json body is \n%s\n\n  string body is \n%s\n\n",
           response->protocol, headers, json_body, (char *)response->string_body);
    free(headers);
    free(json_body);
  }
}

int main (int argc, char **argv) {
  
  struct _u_map headers, url_params, post_params;
  char * string_body = "param1=one&param2=two";
  json_t * json_body = json_object();
  struct _u_response response;
  
  u_map_init(&headers);
  
  u_map_init(&url_params);
  u_map_put(&url_params, "test", "one");
  u_map_put(&url_params, "other_test", "two");
  
  u_map_init(&post_params);
  u_map_put(&post_params, "third_test", "three");
  u_map_put(&post_params, "fourth_test", "four");
  u_map_put(&post_params, "extreme_test", "Here ! are %9_ some $ ö\\)]= special châraçters");
  
  json_object_set_new(json_body, "param1", json_string("one"));
  json_object_set_new(json_body, "param2", json_string("two"));
  
  struct _u_request req_list[] = {
    {"GET", SERVER_URL_PREFIX "/get/", NULL, &url_params, NULL, NULL, NULL, NULL, 0, NULL, 0},
    {"DELETE", SERVER_URL_PREFIX "/delete/", NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL, 0},
    {"POST", SERVER_URL_PREFIX "/post/param/", NULL, NULL, NULL, NULL, &post_params, NULL, 0, string_body, 0},
    {"POST", SERVER_URL_PREFIX "/post/plain/", NULL, NULL, NULL, NULL, NULL, NULL, 0, string_body, 0},
    {"POST", SERVER_URL_PREFIX "/post/json/", NULL, NULL, NULL, NULL, NULL, json_body, 0, NULL, 0},
    {"PUT", SERVER_URL_PREFIX "/put/plain", NULL, NULL, NULL, NULL, NULL, NULL, 0, string_body, 0},
    {"PUT", SERVER_URL_PREFIX "/put/json", NULL, NULL, NULL, NULL, NULL, json_body, 0, NULL, 0}
  };
  
  printf("Press <enter> to run get test\n");
  getchar();
  ulfius_init_response(&response);
  ulfius_send_http_request(&req_list[0], &response);
  print_response(&response);
  ulfius_clean_response(&response);
  
  printf("Press <enter> to run get test with no interest on the response\n");
  getchar();
  printf("Request sent, result is %d\n", ulfius_send_http_request(&req_list[0], NULL));
  
  printf("Press <enter> to run delete test\n");
  getchar();
  ulfius_init_response(&response);
  ulfius_send_http_request(&req_list[1], &response);
  print_response(&response);
  ulfius_clean_response(&response);
  
  printf("Press <enter> to run post parameters test\n");
  getchar();
  ulfius_init_response(&response);
  ulfius_send_http_request(&req_list[2], &response);
  print_response(&response);
  ulfius_clean_response(&response);
  
  printf("Press <enter> to run post plain test\n");
  getchar();
  ulfius_init_response(&response);
  ulfius_send_http_request(&req_list[3], &response);
  print_response(&response);
  ulfius_clean_response(&response);
  
  printf("Press <enter> to run post json test\n");
  getchar();
  ulfius_init_response(&response);
  ulfius_send_http_request(&req_list[4], &response);
  print_response(&response);
  ulfius_clean_response(&response);
  
  printf("Press <enter> to run put plain test\n");
  getchar();
  ulfius_init_response(&response);
  ulfius_send_http_request(&req_list[5], &response);
  print_response(&response);
  ulfius_clean_response(&response);
  
  printf("Press <enter> to run put json test\n");
  getchar();
  ulfius_init_response(&response);
  ulfius_send_http_request(&req_list[6], &response);
  print_response(&response);
  ulfius_clean_response(&response);
  
  // Wait for the user to press <enter> on the console to quit the application
  printf("Press <enter> to quit test\n");
  getchar();
  json_decref(json_body);
  u_map_clean(&headers);
  u_map_clean(&url_params);
  u_map_clean(&post_params);
  
  return 0;
}
