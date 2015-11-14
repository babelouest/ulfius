/**
 * 
 * Ulfius Framework
 * 
 * REST framework library
 * 
 * u_request.c: request related functions defintions
 * 
 * Copyright 2015 Nicolas Mora <mail@babelouest.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation;
 * version 2.1 of the License.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU GENERAL PUBLIC LICENSE for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */

#include "ulfius.h"

/**
 * Splits the url to an array of char *
 */
char ** split_url(const char * url) {
  char * saveptr=NULL, * cur_word=NULL, ** to_return = malloc(sizeof(char*)), * url_cpy = NULL, * url_cpy_addr = NULL;
  int counter = 1;
  
  *to_return = NULL;
  if (url != NULL) {
    url_cpy = url_cpy_addr = u_strdup(url);
    if (to_return != NULL) {
      to_return[0] = NULL;
      cur_word = strtok_r( url_cpy, ULFIUS_URL_SEPARATOR, &saveptr );
      while (cur_word != NULL) {
        if (0 != strcmp("", cur_word) && cur_word[0] != '?') {
          to_return = realloc(to_return, (counter+1)*sizeof(char*));
          to_return[counter-1] = u_strdup(cur_word);
          to_return[counter] = NULL;
          counter++;
        }
        cur_word = strtok_r( NULL, ULFIUS_URL_SEPARATOR, &saveptr );
      }
    }
    free(url_cpy_addr);
    url_cpy_addr = NULL;
  }
  return to_return;
}

/**
 * Look in all endpoints if one matches the call specified by verb and url
 */
struct _u_endpoint * endpoint_match(const char * method, const char * url, struct _u_endpoint * endpoint_list) {
  char ** splitted_url, ** splitted_url_format;
  struct _u_endpoint * endpoint = NULL;
  int i;
  
  if (method != NULL && url != NULL && endpoint_list != NULL) {
    splitted_url = split_url(url);
    for (i=0; (endpoint_list[i].http_method != NULL && endpoint_list[i].url_format != NULL); i++) {
      if (0 == strcmp(endpoint_list[i].http_method, method)) {
        splitted_url_format = split_url(endpoint_list[i].url_format);
        if (url_format_match((const char **)splitted_url, (const char **)splitted_url_format)) {
          endpoint = (endpoint_list + i);
        }
        u_map_clean_enum(splitted_url_format);
        splitted_url_format = NULL;
      }
    }
    u_map_clean_enum(splitted_url);
    splitted_url = NULL;
  }
  return endpoint;
}

/**
 * Compare the current url to the specified url_format
 * return 1 if it's a match, otherwise 0
 * Match means that url is contained in url_format
 */
int url_format_match(const char ** splitted_url, const char ** splitted_url_format) {
  int i;
  
  for (i=0; splitted_url_format[i] != NULL; i++) {
    if (splitted_url_format[i][0] == '*' && splitted_url_format[i+1] == NULL) {
      return 1;
    }
    if (splitted_url[i] == NULL || (splitted_url_format[i][0] != '@' && splitted_url_format[i][0] != ':' && 0 != strcmp(splitted_url_format[i], splitted_url[i]))) {
      return 0;
    }
  }
  return (splitted_url[i] == NULL && splitted_url_format[i] == NULL);
}

/**
 * Fill a map with url parameters using the format url
 */
int parse_url(const char * url, const struct _u_endpoint * endpoint, struct _u_map * map) {
  char * saveptr = NULL, * cur_word = NULL, * url_cpy = NULL, * url_cpy_addr = NULL;
  char * saveptr_format = NULL, * cur_word_format = NULL, * url_format_cpy = NULL, * url_format_cpy_addr = NULL;
  int ret = 0;

  if (map != NULL && endpoint != NULL) {
    url_cpy = url_cpy_addr = u_strdup(url);
    url_format_cpy = url_format_cpy_addr = u_strdup(endpoint->url_format);
    cur_word = strtok_r( url_cpy, ULFIUS_URL_SEPARATOR, &saveptr );
    cur_word_format = strtok_r( url_format_cpy, ULFIUS_URL_SEPARATOR, &saveptr_format );
    while (cur_word_format != NULL && cur_word != NULL) {
      if (cur_word_format[0] == ':' || cur_word_format[0] == '@') {
        u_map_put(map, cur_word_format+1, cur_word);
      }
      cur_word = strtok_r( NULL, ULFIUS_URL_SEPARATOR, &saveptr );
      cur_word_format = strtok_r( NULL, ULFIUS_URL_SEPARATOR, &saveptr_format );
    }
    ret = 1;
  }
  free(url_cpy_addr);
  url_cpy_addr = NULL;
  free(url_format_cpy_addr);
  url_format_cpy_addr = NULL;
  return ret;
}

/**
 * ulfius_init_request
 * Initialize a request structure by allocating inner elements
 * return true if everything went fine, false otherwise
 */
int ulfius_init_request(struct _u_request * request) {
  if (request != NULL) {
    request->map_url = malloc(sizeof(struct _u_map));
    u_map_init(request->map_url);
    request->map_header = malloc(sizeof(struct _u_map));
    u_map_init(request->map_header);
    request->map_cookie = malloc(sizeof(struct _u_map));
    u_map_init(request->map_cookie);
    request->map_post_body = malloc(sizeof(struct _u_map));
    u_map_init(request->map_post_body);
    request->http_verb = NULL;
    request->http_url = NULL;
    request->client_address = NULL;
    request->json_body = NULL;
    request->json_error = 0;
    request->binary_body = NULL;
    request->binary_body_length = 0;
    return 1;
  } else {
    return 0;
  }
}

/**
 * ulfius_clean_request
 * clean the specified request's inner elements
 * user must free the parent pointer if needed after clean
 * or use ulfius_clean_request_full
 * return true if no error
 */
int ulfius_clean_request(struct _u_request * request) {
  if (request != NULL) {
    free(request->http_verb);
    request->http_verb = NULL;
    free(request->http_url);
    request->http_url = NULL;
    free(request->client_address);
    request->client_address = NULL;
    u_map_clean_full(request->map_url);
    request->map_url = NULL;
    u_map_clean_full(request->map_header);
    request->map_header = NULL;
    u_map_clean_full(request->map_cookie);
    request->map_cookie = NULL;
    u_map_clean_full(request->map_post_body);
    request->map_post_body = NULL;
    json_decref(request->json_body);
    request->json_body = NULL;
    free(request->binary_body);
    request->binary_body = NULL;
    return 1;
  } else {
    return 0;
  }
}

/**
 * ulfius_clean_request_full
 * clean the specified request and all its elements
 * return true if no error
 */
int ulfius_clean_request_full(struct _u_request * request) {
  if (ulfius_clean_request(request)) {
    free(request);
    return 1;
  } else {
    return 0;
  }
}

/**
 * create a new request based on the source elements
 * return value must be free'd
 */
struct _u_request * ulfius_duplicate_request(const struct _u_request * request) {
  struct _u_request * new_request = NULL;
  if (request != NULL) {
    new_request = malloc(sizeof(struct _u_request));
    ulfius_init_request(new_request);
    new_request->http_verb = u_strdup(request->http_verb);
    new_request->http_url = u_strdup(request->http_url);
    if (request->client_address != NULL) {
      new_request->client_address = malloc(sizeof(struct sockaddr));
      memcpy(new_request->client_address, request->client_address, sizeof(struct sockaddr));
    }
    u_map_clean_full(new_request->map_url);
    new_request->map_url = u_map_copy(request->map_url);
    u_map_clean_full(new_request->map_header);
    new_request->map_header = u_map_copy(request->map_header);
    u_map_clean_full(new_request->map_cookie);
    new_request->map_cookie = u_map_copy(request->map_cookie);
    u_map_clean_full(new_request->map_post_body);
    new_request->map_post_body = u_map_copy(request->map_post_body);
    new_request->json_body = json_copy(request->json_body);
    new_request->json_error = request->json_error;
    if (request->binary_body != NULL && request->binary_body_length > 0) {
      new_request->binary_body = malloc(request->binary_body_length);
      memcpy(new_request->binary_body, request->binary_body, request->binary_body_length);
    }
    new_request->binary_body_length = request->binary_body_length;
  }
  return new_request;
}
