/**
 * 
 * Ulfius Framework
 * 
 * REST framework library
 * 
 * u_request.c: request related functions defintions
 * 
 * Copyright 2015-2017 Nicolas Mora <mail@babelouest.org>
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
#include <stdlib.h>
#include <string.h>

#include "u_private.h"
#include "ulfius.h"

/**
 * Splits the url to an array of char *
 */
static char ** ulfius_split_url(const char * prefix, const char * url) {
  char * saveptr = NULL, * cur_word = NULL, ** to_return = o_malloc(sizeof(char*)), * url_cpy = NULL, * url_cpy_addr = NULL;
  int counter = 1;
  
  if (to_return != NULL) {
    to_return[0] = NULL;
    if (prefix != NULL) {
      url_cpy = url_cpy_addr = o_strdup(prefix);
      cur_word = strtok_r( url_cpy, ULFIUS_URL_SEPARATOR, &saveptr );
      while (cur_word != NULL) {
        to_return = o_realloc(to_return, (counter+1)*sizeof(char*));
        if (to_return == NULL) {
          y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for ulfius_split_url.to_return");
          break;
        }
        to_return[counter-1] = o_strdup(cur_word);
        if (to_return[counter-1] == NULL) {
          y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for ulfius_split_url.to_return[counter-1]");
          break;
        }
        to_return[counter] = NULL;
        counter++;
        cur_word = strtok_r( NULL, ULFIUS_URL_SEPARATOR, &saveptr );
      }
      o_free(url_cpy_addr);
      url_cpy_addr = NULL;
    }
    if (url != NULL) {
      url_cpy = url_cpy_addr = o_strdup(url);
      cur_word = strtok_r( url_cpy, ULFIUS_URL_SEPARATOR, &saveptr );
      while (cur_word != NULL) {
        if (0 != o_strcmp("", cur_word) && cur_word[0] != '?') {
          to_return = o_realloc(to_return, (counter+1)*sizeof(char*));
          if (to_return == NULL) {
            y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for ulfius_split_url.to_return");
            break;
          }
          to_return[counter-1] = o_strdup(cur_word);
          if (to_return[counter-1] == NULL) {
            y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for ulfius_split_url.to_return[counter-1]");
            break;
          }
          to_return[counter] = NULL;
          counter++;
        }
        cur_word = strtok_r( NULL, ULFIUS_URL_SEPARATOR, &saveptr );
      }
      o_free(url_cpy_addr);
      url_cpy_addr = NULL;
    }
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for ulfius_split_url.to_return");
  }
  return to_return;
}

/**
 * Compare two endoints by their priorities
 * Used by qsort to compare endpoints
 */
static int compare_endpoint_priorities(const void * a, const void * b) {
  struct _u_endpoint * e1 = *(struct _u_endpoint **)a, * e2 = *(struct _u_endpoint **)b;
  
  if (e1->priority < e2->priority) {
    return -1;
  } else if (e1->priority > e2->priority) {
    return 1;
  } else {
    return 0;
  }
}

/**
 * ulfius_url_format_match
 * return true if splitted_url matches splitted_url_format
 * false otherwise
 */
static int ulfius_url_format_match(const char ** splitted_url, const char ** splitted_url_format) {
  int i;
  
  for (i=0; splitted_url_format[i] != NULL; i++) {
    if (splitted_url_format[i][0] == '*' && splitted_url_format[i+1] == NULL) {
      return 1;
    }
    if (splitted_url[i] == NULL || (splitted_url_format[i][0] != '@' && splitted_url_format[i][0] != ':' && 0 != o_strcmp(splitted_url_format[i], splitted_url[i]))) {
      return 0;
    }
  }
  return (splitted_url[i] == NULL && splitted_url_format[i] == NULL);
}

/**
 * ulfius_endpoint_match
 * return the endpoint array matching the url called with the proper http method
 * the returned array always has its last value to NULL
 * return NULL on memory error
 * returned value must be free'd after use
 */
struct _u_endpoint ** ulfius_endpoint_match(const char * method, const char * url, struct _u_endpoint * endpoint_list) {
  char ** splitted_url, ** splitted_url_format;
  struct _u_endpoint ** endpoint_returned = o_malloc(sizeof(struct _u_endpoint *));
  int i;
  size_t count = 0;
  
  if (endpoint_returned == NULL) {
    y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for endpoint_returned");
  } else {
    endpoint_returned[0] = NULL;
    if (method != NULL && url != NULL && endpoint_list != NULL) {
      splitted_url = ulfius_split_url(url, NULL);
      for (i=0; (splitted_url != NULL && !ulfius_equals_endpoints(&(endpoint_list[i]), ulfius_empty_endpoint())); i++) {
        if (0 == o_strcasecmp(endpoint_list[i].http_method, method) || endpoint_list[i].http_method[0] == '*') {
          splitted_url_format = ulfius_split_url(endpoint_list[i].url_prefix, endpoint_list[i].url_format);
          if (splitted_url_format != NULL && ulfius_url_format_match((const char **)splitted_url, (const char **)splitted_url_format)) {
            endpoint_returned = o_realloc(endpoint_returned, (count+2)*sizeof(struct _u_endpoint *));
            if (endpoint_returned != NULL) {
              endpoint_returned[count] = (endpoint_list + i);
              endpoint_returned[count + 1] = NULL;
            } else {
              y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error reallocating memory for endpoint_returned");
            }
            count++;
            u_map_clean_enum(splitted_url_format);
            splitted_url_format = NULL;
          }
          u_map_clean_enum(splitted_url_format);
          splitted_url_format = NULL;
        }
      }
      u_map_clean_enum(splitted_url);
      splitted_url = NULL;
    }
  }
  qsort(endpoint_returned, count, sizeof(struct endpoint_list *), &compare_endpoint_priorities);
  return endpoint_returned;
}

/**
 * ulfius_parse_url
 * fills map with the keys/values defined in the url that are described in the endpoint format url
 * return U_OK on success
 */
int ulfius_parse_url(const char * url, const struct _u_endpoint * endpoint, struct _u_map * map) {
  char * saveptr = NULL, * cur_word = NULL, * url_cpy = NULL, * url_cpy_addr = NULL;
  char * saveptr_format = NULL, * saveptr_prefix = NULL, * cur_word_format = NULL, * url_format_cpy = NULL, * url_format_cpy_addr = NULL;

  if (map != NULL && endpoint != NULL) {
    url_cpy = url_cpy_addr = o_strdup(url);
    url_format_cpy = url_format_cpy_addr = o_strdup(endpoint->url_prefix);
    cur_word = strtok_r( url_cpy, ULFIUS_URL_SEPARATOR, &saveptr );
    if (endpoint->url_prefix != NULL && url_format_cpy == NULL) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for url_format_cpy");
    } else if (url_format_cpy != NULL) {
      cur_word_format = strtok_r( url_format_cpy, ULFIUS_URL_SEPARATOR, &saveptr_prefix );
    }
    while (cur_word_format != NULL && cur_word != NULL) {
      // Ignoring url_prefix words
      cur_word = strtok_r( NULL, ULFIUS_URL_SEPARATOR, &saveptr );
      cur_word_format = strtok_r( NULL, ULFIUS_URL_SEPARATOR, &saveptr_prefix );
    }
    o_free(url_format_cpy_addr);
    
    url_format_cpy = url_format_cpy_addr = o_strdup(endpoint->url_format);
    if (endpoint->url_format != NULL && url_format_cpy == NULL) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for url_format_cpy");
      return U_ERROR_MEMORY;
    } else if (url_format_cpy != NULL) {
      cur_word_format = strtok_r( url_format_cpy, ULFIUS_URL_SEPARATOR, &saveptr_format );
    }
    while (cur_word_format != NULL && cur_word != NULL) {
      if (cur_word_format[0] == ':' || cur_word_format[0] == '@') {
        if (u_map_has_key(map, cur_word_format+1)) {
          char * concat_url_param = msprintf("%s,%s", u_map_get(map, cur_word_format+1), cur_word);
          if (concat_url_param == NULL) {
            y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating resources for concat_url_param");
            o_free(url_cpy_addr);
            o_free(url_format_cpy_addr);
            return U_ERROR_MEMORY;
          } else if (u_map_put(map, cur_word_format+1, concat_url_param) != U_OK) {
            url_cpy_addr = NULL;
            url_format_cpy_addr = NULL;
            return U_ERROR_MEMORY;
          }
          o_free(concat_url_param);
        } else {
          if (u_map_put(map, cur_word_format+1, cur_word) != U_OK) {
            url_cpy_addr = NULL;
            url_format_cpy_addr = NULL;
            return U_ERROR_MEMORY;
          }
        }
      }
      cur_word = strtok_r( NULL, ULFIUS_URL_SEPARATOR, &saveptr );
      cur_word_format = strtok_r( NULL, ULFIUS_URL_SEPARATOR, &saveptr_format );
    }
    o_free(url_cpy_addr);
    o_free(url_format_cpy_addr);
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
    request->map_url = o_malloc(sizeof(struct _u_map));
    request->auth_basic_user = NULL;
    request->auth_basic_password = NULL;
    request->map_header = o_malloc(sizeof(struct _u_map));
    request->map_cookie = o_malloc(sizeof(struct _u_map));
    request->map_post_body = o_malloc(sizeof(struct _u_map));
    if (request->map_post_body == NULL || request->map_cookie == NULL || 
        request->map_url == NULL || request->map_header == NULL) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for request->map*");
      ulfius_clean_request(request);
      return U_ERROR_MEMORY;
    }
    u_map_init(request->map_url);
    u_map_init(request->map_header);
    u_map_init(request->map_cookie);
    u_map_init(request->map_post_body);
    request->http_protocol = NULL;
    request->http_verb = NULL;
    request->http_url = NULL;
    request->proxy = NULL;
    request->timeout = 0L;
    request->check_server_certificate = 1;
    request->client_address = NULL;
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
    o_free(request->http_protocol);
    o_free(request->http_verb);
    o_free(request->http_url);
    o_free(request->proxy);
    o_free(request->auth_basic_user);
    o_free(request->auth_basic_password);
    o_free(request->client_address);
    u_map_clean_full(request->map_url);
    u_map_clean_full(request->map_header);
    u_map_clean_full(request->map_cookie);
    u_map_clean_full(request->map_post_body);
    o_free(request->binary_body);
    request->http_protocol = NULL;
    request->http_verb = NULL;
    request->http_url = NULL;
    request->proxy = NULL;
    request->client_address = NULL;
    request->map_url = NULL;
    request->map_header = NULL;
    request->map_cookie = NULL;
    request->map_post_body = NULL;
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
    o_free(request);
    return U_OK;
  } else {
    return U_ERROR_PARAMS;
  }
}

/**
 * create a new request based on the source elements
 * returned value must be free'd after use
 */
struct _u_request * ulfius_duplicate_request(const struct _u_request * request) {
  struct _u_request * new_request = NULL;
  if (request != NULL) {
    new_request = o_malloc(sizeof(struct _u_request));
    if (new_request == NULL) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for new_request");
      return NULL;
    }
    if (ulfius_init_request(new_request) == U_OK) {
      new_request->http_protocol = o_strdup(request->http_protocol);
      new_request->http_verb = o_strdup(request->http_verb);
      new_request->http_url = o_strdup(request->http_url);
      new_request->proxy = o_strdup(request->proxy);
      if ((new_request->http_verb == NULL && request->http_verb != NULL) ||
          (new_request->http_url == NULL && request->http_url != NULL) ||
          (new_request->proxy == NULL && request->proxy != NULL) ||
          (new_request->http_protocol == NULL && request->http_protocol != NULL)) {
        y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for ulfius_duplicate_request");
        ulfius_clean_request_full(new_request);
        return NULL;
      }
      if (request->client_address != NULL) {
        new_request->client_address = o_malloc(sizeof(struct sockaddr));
        if (new_request->client_address == NULL) {
          y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for new_request->client_address");
          ulfius_clean_request_full(new_request);
          return NULL;
        }
        memcpy(new_request->client_address, request->client_address, sizeof(struct sockaddr));
      }
      new_request->check_server_certificate = request->check_server_certificate;
      new_request->timeout = request->timeout;
      new_request->auth_basic_user = o_strdup(request->auth_basic_user);
      new_request->auth_basic_password = o_strdup(request->auth_basic_password);
      u_map_clean_full(new_request->map_url);
      u_map_clean_full(new_request->map_header);
      u_map_clean_full(new_request->map_cookie);
      u_map_clean_full(new_request->map_post_body);
      new_request->map_url = u_map_copy(request->map_url);
      new_request->map_header = u_map_copy(request->map_header);
      new_request->map_cookie = u_map_copy(request->map_cookie);
      new_request->map_post_body = u_map_copy(request->map_post_body);
      if ((new_request->map_url == NULL && request->map_url != NULL) || 
          (new_request->map_header == NULL && request->map_header != NULL) || 
          (new_request->map_cookie == NULL && request->map_cookie != NULL) || 
          (new_request->map_post_body == NULL && request->map_post_body != NULL)) {
        ulfius_clean_request_full(new_request);
        return NULL;
      }
      if (request->binary_body != NULL && request->binary_body_length > 0) {
        new_request->binary_body = o_malloc(request->binary_body_length);
        if (new_request->binary_body == NULL) {
          y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for new_request->binary_body");
          ulfius_clean_request_full(new_request);
          return NULL;
        }
        memcpy(new_request->binary_body, request->binary_body, request->binary_body_length);
      } else {
        new_request->binary_body_length = 0;
        new_request->binary_body = NULL;
      }
      new_request->binary_body_length = request->binary_body_length;
    } else {
      o_free(new_request);
      new_request = NULL;
    }
  }
  return new_request;
}

#ifndef U_DISABLE_JANSSON
/**
 * ulfius_set_json_body_request
 * Add a json_t j_body to a response
 * return U_OK on success
 */
int ulfius_set_json_body_request(struct _u_request * request, json_t * j_body) {
  if (request != NULL && j_body != NULL && (json_is_array(j_body) || json_is_object(j_body))) {
    // Free all the bodies available
    o_free(request->binary_body);
    request->binary_body = NULL;
    request->binary_body_length = 0;

    request->binary_body = (void*) json_dumps(j_body, JSON_COMPACT);
    if (request->binary_body == NULL) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for request->binary_body");
      return U_ERROR_MEMORY;
    }
    request->binary_body_length = strlen((char*)request->binary_body);
    u_map_put(request->map_header, ULFIUS_HTTP_HEADER_CONTENT, ULFIUS_HTTP_ENCODING_JSON);
    return U_OK;
  } else {
    return U_ERROR_PARAMS;
  }
}

/**
 * ulfius_get_json_body_request
 * Get JSON structure from the request body if the request is valid
 * request: struct _u_request used
 * json_error: structure to store json_error_t if specified
 */  
json_t * ulfius_get_json_body_request(const struct _u_request * request, json_error_t * json_error) {
  if (request != NULL && request->map_header != NULL && NULL != o_strstr(u_map_get_case(request->map_header, ULFIUS_HTTP_HEADER_CONTENT), ULFIUS_HTTP_ENCODING_JSON)) {
    return json_loadb(request->binary_body, request->binary_body_length, JSON_DECODE_ANY, json_error);
  }
  else {
    json_error->line     = 1;
    json_error->position = 1;
    sprintf(json_error->source, "ulfius_get_json_body_request");
    if (NULL == request) {
      json_error->column = 7;
      sprintf(json_error->text, "Request not set.");
    }
    else if (NULL == request->map_header) {
      json_error->column = 26;
      sprintf(json_error->text, "Request header not set.");
    }
    else if (NULL == o_strstr(u_map_get_case(request->map_header, ULFIUS_HTTP_HEADER_CONTENT))) {
      json_error->column = 57;
      sprintf(json_error->text, "HEADER content not valid. Expected '%s' - received '%s'", ULFIUS_HTTP_ENCODING_JSON, u_map_get_case(request->map_header, ULFIUS_HTTP_HEADER_CONTENT));
    }
  }
  return NULL;
}
#endif
