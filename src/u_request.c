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
  
  if (to_return != NULL) {
    *to_return = NULL;
    if (url != NULL) {
      url_cpy = url_cpy_addr = u_strdup(url);
      if (to_return != NULL) {
        to_return[0] = NULL;
        cur_word = strtok_r( url_cpy, ULFIUS_URL_SEPARATOR, &saveptr );
        while (cur_word != NULL) {
          if (0 != strcmp("", cur_word) && cur_word[0] != '?') {
            to_return = realloc(to_return, (counter+1)*sizeof(char*));
            if (to_return == NULL) {
              break;
            }
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
  }
  return to_return;
}

/**
 * endpoint_match
 * return the endpoint matching the url called with the proper http method
 * return NULL if no endpoint is found
 */
struct _u_endpoint * endpoint_match(const char * method, const char * url, struct _u_endpoint * endpoint_list) {
  char ** splitted_url, ** splitted_url_format;
  struct _u_endpoint * endpoint = NULL;
  int i;
  
  if (method != NULL && url != NULL && endpoint_list != NULL) {
    splitted_url = split_url(url);
    for (i=0; (splitted_url != NULL && endpoint_list[i].http_method != NULL && endpoint_list[i].url_format != NULL); i++) {
      if (0 == strcmp(endpoint_list[i].http_method, method) || endpoint_list[i].http_method[0] == '*') {
        splitted_url_format = split_url(endpoint_list[i].url_format);
        if (splitted_url_format != NULL && url_format_match((const char **)splitted_url, (const char **)splitted_url_format)) {
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
 * url_format_match
 * return true if splitted_url matches splitted_url_format
 * false otherwise
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
 * parse_url
 * fills map with the keys/values defined in the url that are described in the endpoint format url
 * return U_OK on success
 */
int parse_url(const char * url, const struct _u_endpoint * endpoint, struct _u_map * map) {
  char * saveptr = NULL, * cur_word = NULL, * url_cpy = NULL, * url_cpy_addr = NULL;
  char * saveptr_format = NULL, * cur_word_format = NULL, * url_format_cpy = NULL, * url_format_cpy_addr = NULL;

  if (map != NULL && endpoint != NULL) {
    url_cpy = url_cpy_addr = u_strdup(url);
    url_format_cpy = url_format_cpy_addr = u_strdup(endpoint->url_format);
    cur_word = strtok_r( url_cpy, ULFIUS_URL_SEPARATOR, &saveptr );
    cur_word_format = strtok_r( url_format_cpy, ULFIUS_URL_SEPARATOR, &saveptr_format );
    while (cur_word_format != NULL && cur_word != NULL) {
      if (cur_word_format[0] == ':' || cur_word_format[0] == '@') {
        if (u_map_put(map, cur_word_format+1, cur_word) != U_OK) {
          url_cpy_addr = NULL;
          url_format_cpy_addr = NULL;
          return U_ERROR_MEMORY;
        }
      }
      cur_word = strtok_r( NULL, ULFIUS_URL_SEPARATOR, &saveptr );
      cur_word_format = strtok_r( NULL, ULFIUS_URL_SEPARATOR, &saveptr_format );
    }
    free(url_cpy_addr);
    free(url_format_cpy_addr);
    url_cpy_addr = NULL;
    url_format_cpy_addr = NULL;
    return U_OK;
  } else {
    return U_ERROR_PARAMS;
  }
}

/**
 * ulfius_init_request
 * Initialize a request structure by allocating inner elements
 * return U_OK on success
 */
int ulfius_init_request(struct _u_request * request) {
  if (request != NULL) {
    request->map_url = malloc(sizeof(struct _u_map));
    request->map_header = malloc(sizeof(struct _u_map));
    request->map_cookie = malloc(sizeof(struct _u_map));
    request->map_post_body = malloc(sizeof(struct _u_map));
    if (request->map_post_body == NULL || request->map_cookie == NULL || 
        request->map_url == NULL || request->map_header == NULL) {
      ulfius_clean_request(request);
      return U_ERROR_MEMORY;
    }
    u_map_init(request->map_url);
    u_map_init(request->map_header);
    u_map_init(request->map_cookie);
    u_map_init(request->map_post_body);
    request->http_verb = NULL;
    request->http_url = NULL;
    request->client_address = NULL;
    request->json_body = NULL;
    request->json_error = 0;
    request->binary_body = NULL;
    request->binary_body_length = 0;
    return U_OK;
  } else {
    return U_ERROR_PARAMS;
  }
}

/**
 * ulfius_clean_request
 * clean the specified request's inner elements
 * user must free the parent pointer if needed after clean
 * or use ulfius_clean_request_full
 * return U_OK on success
 */
int ulfius_clean_request(struct _u_request * request) {
  if (request != NULL) {
    free(request->http_verb);
    free(request->http_url);
    free(request->client_address);
    u_map_clean_full(request->map_url);
    u_map_clean_full(request->map_header);
    u_map_clean_full(request->map_cookie);
    u_map_clean_full(request->map_post_body);
    free(request->binary_body);
    json_decref(request->json_body);
    request->http_verb = NULL;
    request->http_url = NULL;
    request->client_address = NULL;
    request->map_url = NULL;
    request->map_header = NULL;
    request->map_cookie = NULL;
    request->map_post_body = NULL;
    request->json_body = NULL;
    request->binary_body = NULL;
    return U_OK;
  } else {
    return U_ERROR_PARAMS;
  }
}

/**
 * ulfius_clean_request_full
 * clean the specified request and all its elements
 * return U_OK on success
 */
int ulfius_clean_request_full(struct _u_request * request) {
  if (ulfius_clean_request(request) == U_OK) {
    free(request);
    return U_OK;
  } else {
    return U_ERROR_PARAMS;
  }
}

/**
 * create a new request based on the source elements
 * returned value must be free'd
 */
struct _u_request * ulfius_duplicate_request(const struct _u_request * request) {
  struct _u_request * new_request = NULL;
  if (request != NULL) {
    new_request = malloc(sizeof(struct _u_request));
    if (new_request == NULL) {
      return NULL;
    }
    if (ulfius_init_request(new_request) == U_OK) {
      new_request->http_verb = u_strdup(request->http_verb);
      new_request->http_url = u_strdup(request->http_url);
      if (request->client_address != NULL) {
        new_request->client_address = malloc(sizeof(struct sockaddr));
        if (new_request->client_address == NULL) {
        ulfius_clean_request_full(new_request);
        return NULL;
      }
        memcpy(new_request->client_address, request->client_address, sizeof(struct sockaddr));
      }
      u_map_clean_full(new_request->map_url);
      u_map_clean_full(new_request->map_header);
      u_map_clean_full(new_request->map_cookie);
      u_map_clean_full(new_request->map_post_body);
      new_request->map_url = u_map_copy(request->map_url);
      new_request->map_header = u_map_copy(request->map_header);
      new_request->map_cookie = u_map_copy(request->map_cookie);
      new_request->map_post_body = u_map_copy(request->map_post_body);
      new_request->json_body = json_copy(request->json_body);
      new_request->json_error = request->json_error;
      if ((new_request->map_url == NULL && request->map_url != NULL) || 
          (new_request->map_header == NULL && request->map_header != NULL) || 
          (new_request->map_cookie == NULL && request->map_cookie != NULL) || 
          (new_request->map_post_body == NULL && request->map_post_body != NULL) || 
          (new_request->json_body == NULL && request->json_body != NULL)) {
        ulfius_clean_request_full(new_request);
        return NULL;
      }
      if (request->binary_body != NULL && request->binary_body_length > 0) {
        new_request->binary_body = malloc(request->binary_body_length);
        if (new_request->binary_body == NULL) {
          ulfius_clean_request_full(new_request);
          return NULL;
        }
        memcpy(new_request->binary_body, request->binary_body, request->binary_body_length);
      }
      new_request->binary_body_length = request->binary_body_length;
    } else {
      free(new_request);
      new_request = NULL;
    }
  }
  return new_request;
}
