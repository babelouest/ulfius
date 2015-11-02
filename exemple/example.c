/**
 * 
 * Ulfius Framework example program
 * 
 * Copyright 2015 Nicolas Mora <mail@babelouest.org>
 * 
 * License MIT
 *
 */

#include <string.h>
#include <jansson.h>

#include "../src/ulfius.h"

#define PORT 8537

int callback_get_test (const struct _u_request * request, struct _u_response * response, void * user_data);

int callback_post_test (const struct _u_request * request, struct _u_response * response, void * user_data);

int callback_all_test_foo (const struct _u_request * request, struct _u_response * response, void * user_data);

int callback_get_jsontest (const struct _u_request * request, struct _u_response * response, void * user_data);

int callback_get_cookietest (const struct _u_request * request, struct _u_response * response, void * user_data);

char * print_map(const struct _u_map * map) {
	char * line, * to_return = NULL, **keys, * value;
	int len, i;
	if (map != NULL) {
		keys = u_map_enum_keys(map);
		for (i=0; keys[i] != NULL; i++) {
			value = u_map_get(map, keys[i]);
			len = snprintf(NULL, 0, "key is %s, value is %s", keys[i], value);
			line = malloc((len+1)*sizeof(char));
			snprintf(line, (len+1), "key is %s, value is %s", keys[i], value);
			free(value);
			if (to_return != NULL) {
				len = strlen(to_return) + strlen(line) + 1;
				to_return = realloc(to_return, (len+1)*sizeof(char));
				if (strlen(to_return) > 0) {
					strcat(to_return, "\n");
				}
			} else {
				to_return = malloc((strlen(line) + 1)*sizeof(char));
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

int main (int argc, char **argv) {
  struct _u_endpoint endpoint_list[] = {
    {"GET", "/test", NULL, &callback_get_test},
    {"POST", "/test", NULL, &callback_post_test},
    {"GET", "/test/:foo", "user data 1", &callback_all_test_foo},
    {"POST", "/test/:foo", "user data 2", &callback_all_test_foo},
    {"PUT", "/test/:foo", "user data 3", &callback_all_test_foo},
    {"DELETE", "/test/:foo", "user data 4", &callback_all_test_foo},
    {"GET", "/json/test", NULL, &callback_get_jsontest},
    {"GET", "/cookie/test/:lang/:extra", NULL, &callback_get_cookietest},
    {NULL, NULL, NULL, NULL}
  };
  
  struct _u_instance instance;
  instance.port = PORT;
  instance.bind_address = NULL;
  
  ulfius_init_framework(&instance, endpoint_list);
  printf("Start framework on port %d\n", instance.port);
  
  getchar();
  
  printf("End framework\n");
	return 0;
}

int callback_get_test (const struct _u_request * request, struct _u_response * response, void * user_data) {
  response->string_body = strdup("Hello World!");
  response->status = 200;
  return 0;
}

int callback_post_test (const struct _u_request * request, struct _u_response * response, void * user_data) {
  int len;
  char * post_params = print_map(request->map_post_body);
  len = snprintf(NULL, 0, "Hello World!\n%s", post_params);
  response->string_body = malloc((len+1)*sizeof(char));
  snprintf(response->string_body, (len+1), "Hello World!\n%s", post_params);
  free(post_params);
  response->status = 200;
  return 0;
}

int callback_all_test_foo (const struct _u_request * request, struct _u_response * response, void * user_data) {
  char * url_params = print_map(request->map_url), * headers = print_map(request->map_header), * cookies = print_map(request->map_cookie), 
				* post_params = print_map(request->map_post_body), * json_params = json_dumps(request->json_body, JSON_INDENT(2));
  int len;
  len = snprintf(NULL, 0, "Hello World!\n\n  method is %s\n  url is %s\n\n  parameters from the url are \n%s\n\n  cookies are \n%s\n\n  headers are \n%s\n\n  post parameters are \n%s\n\n  json body parameters are \n%s\n\n  user data is %s\n\n",
																	request->http_verb, request->http_url, url_params, cookies, headers, post_params, json_params, (char *)user_data);
  response->string_body = malloc((len+1)*sizeof(char));
  snprintf(response->string_body, (len+1), "Hello World!\n\n  method is %s\n  url is %s\n\n  parameters from the url are \n%s\n\n  cookies are \n%s\n\n  headers are \n%s\n\n  post parameters are \n%s\n\n  json body parameters are \n%s\n\n  user data is %s\n\n",
																	request->http_verb, request->http_url, url_params, cookies, headers, post_params, json_params, (char *)user_data);
  response->status = 200;
  
  free(url_params);
  free(headers);
  free(cookies);
  free(post_params);
  free(json_params);
  return 0;
}

int callback_get_jsontest (const struct _u_request * request, struct _u_response * response, void * user_data) {
  response->json_body = json_object();
  json_object_set_new(response->json_body, "message", json_string("Hello World!"));
  json_object_set_new(response->json_body, "method", json_string(request->http_verb));
  json_object_set_new(response->json_body, "url", json_string(request->http_url));
  response->status = 200;
  return 0;
}

int callback_get_cookietest (const struct _u_request * request, struct _u_response * response, void * user_data) {
	char * lang = u_map_get(request->map_url, "lang"), * extra = u_map_get(request->map_url, "extra"), 
				* counter = u_map_get(request->map_cookie, "counter"), new_counter[8];
	int i_counter;
	
	if (counter == NULL) {
		i_counter = 0;
	} else {
		i_counter = strtol(counter, NULL, 10);
		i_counter++;
	}
	snprintf(new_counter, 7, "%d", i_counter);
	u_map_put(response->map_cookie, "lang", lang);
	u_map_put(response->map_cookie, "extra", extra);
	u_map_put(response->map_cookie, "counter", new_counter);
	response->string_body = strdup("Cookies set");
	
	free(lang);
	free(extra);
	free(counter);
	
	return 0;
}
