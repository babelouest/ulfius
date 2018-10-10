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

#define U_DISABLE_WEBSOCKET
#include <ulfius.h>

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
  
  struct _u_map headers, url_params, post_params, req_headers;
  char * string_body = "param1=one&param2=two";
  json_t * json_body = json_object();
  struct _u_response response;
  int res;
  
  u_map_init(&headers);
  
  u_map_init(&url_params);
  u_map_put(&url_params, "test", "one");
  u_map_put(&url_params, "other_test", "two");
  
  u_map_init(&post_params);
  u_map_put(&post_params, "third_test", "three");
  u_map_put(&post_params, "fourth_test", "four");
  u_map_put(&post_params, "extreme_test", "Here ! are %9_ some $ ö\\)]= special châraçters");
  
  u_map_init(&req_headers);
  u_map_put(&req_headers, "Content-Type", MHD_HTTP_POST_ENCODING_FORM_URLENCODED);
  
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
  req_list[0].http_verb = o_strdup("GET");
  req_list[0].http_url = o_strdup(SERVER_URL_PREFIX "/get/");
  req_list[0].timeout = 20;
  u_map_copy_into(req_list[0].map_url, &url_params);
  
  // No parameters
  req_list[1].http_verb = o_strdup("DELETE");
  req_list[1].http_url = o_strdup(SERVER_URL_PREFIX "/delete/");
  req_list[1].timeout = 20;
  
  // Parameters in post_map and string_body
  req_list[2].http_verb = o_strdup("POST");
  req_list[2].http_url = o_strdup(SERVER_URL_PREFIX "/post/param/");
  req_list[2].timeout = 20;
  u_map_copy_into(req_list[2].map_post_body, &post_params);
  req_list[2].binary_body = o_strdup(string_body);
  req_list[2].binary_body_length = o_strlen(string_body);
  
  // Paremeters in string body, header MHD_HTTP_POST_ENCODING_FORM_URLENCODED
  req_list[3].http_verb = o_strdup("POST");
  req_list[3].http_url = o_strdup(SERVER_URL_PREFIX "/post/plain/");
  req_list[3].timeout = 20;
  u_map_copy_into(req_list[3].map_header, &req_headers);
  req_list[3].binary_body = o_strdup(string_body);
  req_list[3].binary_body_length = o_strlen(string_body);
  
  // Parameters in json_body
  req_list[4].http_verb = o_strdup("POST");
  req_list[4].http_url = o_strdup(SERVER_URL_PREFIX "/post/json/");
  req_list[4].timeout = 20;
  u_map_copy_into(req_list[4].map_url, &url_params);
  ulfius_set_json_body_request(&req_list[4], json_body);
  
  // Paremeters in string body, header MHD_HTTP_POST_ENCODING_FORM_URLENCODED
  req_list[5].http_verb = o_strdup("PUT");
  req_list[5].http_url = o_strdup(SERVER_URL_PREFIX "/put/plain/");
  req_list[5].timeout = 20;
  u_map_copy_into(req_list[5].map_header, &req_headers);
  req_list[5].binary_body = o_strdup(string_body);
  req_list[5].binary_body_length = o_strlen(string_body);
  
  // Parameters in json_body
  req_list[6].http_verb = o_strdup("PUT");
  req_list[6].http_url = o_strdup(SERVER_URL_PREFIX "/put/json/");
  req_list[6].timeout = 20;
  u_map_copy_into(req_list[6].map_url, &url_params);
  ulfius_set_json_body_request(&req_list[6], json_body);
  
  // Parameters in post_map
  req_list[7].http_verb = o_strdup("POST");
  req_list[7].http_url = o_strdup(SERVER_URL_PREFIX "/post/param/");
  req_list[7].timeout = 20;
  u_map_copy_into(req_list[6].map_post_body, &post_params);
  
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
  u_map_clean(&headers);
  u_map_clean(&url_params);
  u_map_clean(&post_params);
  u_map_clean(&req_headers);
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
