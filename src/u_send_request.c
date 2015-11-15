/**
 * 
 * Ulfius Framework
 * 
 * REST framework library
 * 
 * u_send_request.c: send request related functions defintions
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
 * Internal structure used to store temporarly the response body
 */
typedef struct _body {
  char * data;
  size_t size;
} body;

/**
 * write_body
 * Internal function used to write the body response into a _body structure
 */
size_t write_body(void * contents, size_t size, size_t nmemb, void * user_data) {
  size_t realsize = size * nmemb;
  body * body_data = (body *) user_data;
 
  body_data->data = realloc(body_data->data, body_data->size + realsize + 1);
  if(body_data->data == NULL) {
    return 0;
  }
 
  memcpy(&(body_data->data[body_data->size]), contents, realsize);
  body_data->size += realsize;
  body_data->data[body_data->size] = 0;
 
  return realsize;
}

/**
 * trim_whitespace
 * Return the string without its beginning and ending whitespaces
 */
char * trim_whitespace(char *str) {
  char *end;

  // Trim leading space
  while(isspace(*str)) str++;

  if(*str == 0)  // All spaces?
    return str;

  // Trim trailing space
  end = str + strlen(str) - 1;
  while(end > str && isspace(*end)) end--;

  // Write new null terminator
  *(end+1) = 0;

  return str;
}

/**
 * write_header
 * Write the header value into the response map_header structure
 * return the size_t of the header written
 */
static size_t write_header(void * buffer, size_t size, size_t nitems, void * user_data) {
  
  struct _u_response * response = (struct _u_response *) user_data;
  char * header = (char *)buffer, * key, * value, * saveptr;
  
  if (strchr(header, ':') != NULL) {
    if (response->map_header != NULL) {
      // Expecting a header (key: value)
      key = trim_whitespace(strtok_r(header, ":", &saveptr));
      value = trim_whitespace(strtok_r(NULL, ":", &saveptr));
      
      u_map_put(response->map_header, key, value);
    }
  } else if (strlen(trim_whitespace(header)) > 0) {
    // Expecting the HTTP/x.x header
    if (response->protocol != NULL) {
      free(response->protocol);
    }
    response->protocol = u_strdup(header);
    if (response->protocol == NULL) {
      return 0;
    }
  }
  
  return nitems * size;
}

/**
 * ulfius_send_http_request
 * Send a HTTP request and store the result into a _u_response
 * return U_OK on success
 */
int ulfius_send_http_request(const struct _u_request * request, struct _u_response * response) {
  CURLcode res;
  CURL * curl_handle = NULL;
  struct curl_slist * header_list = NULL;
  char ** keys = NULL, * value, * key_esc, * value_esc, * cookie, * header, * param;
  int i, len;
  struct _u_request * copy_request = NULL;

  if (request != NULL) {
    copy_request = ulfius_duplicate_request(request);
    curl_handle = curl_easy_init();
    body body_data;
    body_data.size = 0;
    body_data.data = NULL;

    if (copy_request != NULL) {
      if (copy_request->map_header == NULL) {
        copy_request->map_header = malloc(sizeof(struct _u_map));
        if (copy_request->map_header == NULL) {
          ulfius_clean_request_full(copy_request);
          curl_slist_free_all(header_list);
          curl_easy_cleanup(curl_handle);
          return U_ERROR_MEMORY;
        }
        if (u_map_init(copy_request->map_header) != U_OK) {
          ulfius_clean_request_full(copy_request);
          curl_slist_free_all(header_list);
          curl_easy_cleanup(curl_handle);
          return U_ERROR_MEMORY;
        }
      }
      if (copy_request->map_url != NULL && u_map_count(copy_request->map_url) > 0) {
        keys = u_map_enum_keys(copy_request->map_url);
        for (i=0; keys != NULL && keys[i] != NULL; i++) {
          value = u_map_get(copy_request->map_url, keys[i]);
          if (value == NULL) {
            u_map_clean_enum(keys);
            ulfius_clean_request_full(copy_request);
            curl_slist_free_all(header_list);
            curl_easy_cleanup(curl_handle);
            return U_ERROR_MEMORY;
          }
          key_esc = curl_easy_escape(curl_handle, keys[i], 0);
          value_esc = curl_easy_escape(curl_handle, value, 0);
          if (key_esc == NULL || value_esc == NULL) {
            free(value);
            u_map_clean_enum(keys);
            curl_free(key_esc);
            curl_free(value_esc);
            ulfius_clean_request_full(copy_request);
            curl_slist_free_all(header_list);
            curl_easy_cleanup(curl_handle);
            return U_ERROR_MEMORY;
          }
          len = snprintf(NULL, 0, "%s=%s", key_esc, value_esc);
          param = malloc((len + 1)*sizeof(char));
          if (param == NULL) {
            free(value);
            curl_free(key_esc);
            curl_free(value_esc);
            u_map_clean_enum(keys);
            ulfius_clean_request_full(copy_request);
            curl_slist_free_all(header_list);
            curl_easy_cleanup(curl_handle);
            return U_ERROR_MEMORY;
          }
          snprintf(param, (len + 1), "%s=%s", key_esc, value_esc);
          if (i==0) {
            copy_request->http_url = realloc(copy_request->http_url, strlen(copy_request->http_url) + strlen(param) + 2);
            if (copy_request->http_url == NULL) {
              free(value);
              free(param);
              curl_free(key_esc);
              curl_free(value_esc);
              u_map_clean_enum(keys);
              ulfius_clean_request_full(copy_request);
              curl_slist_free_all(header_list);
              curl_easy_cleanup(curl_handle);
              return U_ERROR_MEMORY;
            }
            if (strchr(copy_request->http_url, '?') != NULL) {
              strcat(copy_request->http_url, "&");
            } else {
              strcat(copy_request->http_url, "?");
            }
            strcat(copy_request->http_url, param);
          } else {
            copy_request->http_url = realloc(copy_request->http_url, strlen(copy_request->http_url) + strlen(param) + 2);
            if (copy_request->http_url == NULL) {
              free(value);
              free(param);
              curl_free(key_esc);
              curl_free(value_esc);
              u_map_clean_enum(keys);
              ulfius_clean_request_full(copy_request);
              curl_slist_free_all(header_list);
              curl_easy_cleanup(curl_handle);
              return U_ERROR_MEMORY;
            }
            strcat(copy_request->http_url, "&");
            strcat(copy_request->http_url, param);
          }
          free(value);
          free(param);
          curl_free(key_esc);
          curl_free(value_esc);
        }
        u_map_clean_enum(keys);
      }

      if (copy_request->json_body != NULL) {
        free(copy_request->binary_body);
        copy_request->binary_body = json_dumps(copy_request->json_body, JSON_COMPACT);
        if (copy_request->binary_body == NULL) {
          ulfius_clean_request_full(copy_request);
          curl_slist_free_all(header_list);
          curl_easy_cleanup(curl_handle);
          return U_ERROR_MEMORY;
        } else {
          copy_request->binary_body_length = strlen(copy_request->binary_body);
          if (u_map_put(copy_request->map_header, ULFIUS_HTTP_HEADER_CONTENT, ULFIUS_HTTP_ENCODING_JSON) != U_OK) {
            ulfius_clean_request_full(copy_request);
            curl_slist_free_all(header_list);
            curl_easy_cleanup(curl_handle);
            return U_ERROR_MEMORY;
          }
        }
      } else if (u_map_count(copy_request->map_post_body) > 0) {
        free(copy_request->binary_body);
        keys = u_map_enum_keys(copy_request->map_post_body);
        for (i=0; keys != NULL && keys[i] != NULL; i++) {
          value = u_map_get(copy_request->map_post_body, keys[i]);
          if (value == NULL) {
            u_map_clean_enum(keys);
            ulfius_clean_request_full(copy_request);
            curl_slist_free_all(header_list);
            curl_easy_cleanup(curl_handle);
            return U_ERROR_MEMORY;
          }
          key_esc = curl_easy_escape(curl_handle, keys[i], 0);
          value_esc = curl_easy_escape(curl_handle, value, 0);
          if (key_esc == NULL || value_esc == NULL) {
            free(value);
            curl_free(key_esc);
            curl_free(value_esc);
            u_map_clean_enum(keys);
            ulfius_clean_request_full(copy_request);
            curl_slist_free_all(header_list);
            curl_easy_cleanup(curl_handle);
            return U_ERROR_MEMORY;
          }
          len = snprintf(NULL, 0, "%s=%s", key_esc, value_esc);
          param = malloc((len + 1)*sizeof(char));
          if (param == NULL) {
            free(value);
            curl_free(key_esc);
            curl_free(value_esc);
            u_map_clean_enum(keys);
            ulfius_clean_request_full(copy_request);
            curl_slist_free_all(header_list);
            curl_easy_cleanup(curl_handle);
            return U_ERROR_MEMORY;
          }
          snprintf(param, (len + 1), "%s=%s", key_esc, value_esc);
          if (i==0) {
            copy_request->http_url = realloc(copy_request->http_url, strlen(copy_request->http_url) + strlen(param) + 1);
            if (copy_request->http_url == NULL) {
              free(value);
              free(param);
              curl_free(key_esc);
              curl_free(value_esc);
              u_map_clean_enum(keys);
              ulfius_clean_request_full(copy_request);
              curl_slist_free_all(header_list);
              curl_easy_cleanup(curl_handle);
              return U_ERROR_MEMORY;
            }
            strcat(copy_request->http_url, param);
          } else {
            copy_request->http_url = realloc(copy_request->http_url, strlen(copy_request->http_url) + strlen(param) + 1);
            if (copy_request->http_url == NULL) {
              free(value);
              free(param);
              curl_free(key_esc);
              curl_free(value_esc);
              u_map_clean_enum(keys);
              ulfius_clean_request_full(copy_request);
              curl_slist_free_all(header_list);
              curl_easy_cleanup(curl_handle);
              return U_ERROR_MEMORY;
            }
            strcat(copy_request->http_url, param);
          }
          if (i == 0) {
            copy_request->binary_body = malloc(strlen(param) + 1);
            if (copy_request->binary_body == NULL) {
              free(value);
              free(param);
              curl_free(key_esc);
              curl_free(value_esc);
              u_map_clean_enum(keys);
              ulfius_clean_request_full(copy_request);
              curl_slist_free_all(header_list);
              curl_easy_cleanup(curl_handle);
              return U_ERROR_MEMORY;
            }
          } else {
            copy_request->binary_body = realloc(copy_request->binary_body, strlen(copy_request->binary_body) + strlen(param) + 2);
            if (copy_request->binary_body == NULL) {
              free(value);
              free(param);
              curl_free(key_esc);
              curl_free(value_esc);
              u_map_clean_enum(keys);
              ulfius_clean_request_full(copy_request);
              curl_slist_free_all(header_list);
              curl_easy_cleanup(curl_handle);
              return U_ERROR_MEMORY;
            }
            strcat(copy_request->binary_body, "&");
          }
          strcat(copy_request->binary_body, param);
          free(value);
          free(param);
          curl_free(key_esc);
          curl_free(value_esc);
        }
        u_map_clean_enum(keys);
        if (curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, copy_request->binary_body) != CURLE_OK) {
          ulfius_clean_request_full(copy_request);
          curl_slist_free_all(header_list);
          curl_easy_cleanup(curl_handle);
          return U_ERROR_LIBCURL;
        }
        if (u_map_put(copy_request->map_header, ULFIUS_HTTP_HEADER_CONTENT, MHD_HTTP_POST_ENCODING_FORM_URLENCODED) != U_OK) {
          ulfius_clean_request_full(copy_request);
          curl_slist_free_all(header_list);
          curl_easy_cleanup(curl_handle);
          return U_ERROR_MEMORY;
        }
      }

      if (copy_request->map_header != NULL && u_map_count(copy_request->map_header) > 0) {
        keys = u_map_enum_keys(copy_request->map_header);
        for (i=0; keys != NULL && keys[i] != NULL; i++) {
          value = u_map_get(copy_request->map_header, keys[i]);
          if (value == NULL) {
            u_map_clean_enum(keys);
            ulfius_clean_request_full(copy_request);
            curl_slist_free_all(header_list);
            curl_easy_cleanup(curl_handle);
            return U_ERROR_MEMORY;
          }
          len = snprintf(NULL, 0, "%s:%s", keys[i], value);
          header = malloc((len + 1)*sizeof(char));
          if (header == NULL) {
            free(value);
            u_map_clean_enum(keys);
            ulfius_clean_request_full(copy_request);
            curl_slist_free_all(header_list);
            curl_easy_cleanup(curl_handle);
            return U_ERROR_MEMORY;
          }
          snprintf(header, (len + 1), "%s:%s", keys[i], value);
          header_list = curl_slist_append(header_list, header);
          if (header_list == NULL) {
            free(value);
            free(header);
            u_map_clean_enum(keys);
            ulfius_clean_request_full(copy_request);
            curl_slist_free_all(header_list);
            curl_easy_cleanup(curl_handle);
            return U_ERROR_MEMORY;
          }
          free(value);
          free(header);
        }
        u_map_clean_enum(keys);
      }

      if (copy_request->map_cookie != NULL && u_map_count(copy_request->map_cookie) > 0) {
        keys = u_map_enum_keys(copy_request->map_cookie);
        for (i=0; keys != NULL && keys[i] != NULL; i++) {
          value = u_map_get(copy_request->map_cookie, keys[i]);
          if (value == NULL) {
            u_map_clean_enum(keys);
            ulfius_clean_request_full(copy_request);
            curl_slist_free_all(header_list);
            curl_easy_cleanup(curl_handle);
            return U_ERROR_MEMORY;
          }
          len = snprintf(NULL, 0, "%s=%s", keys[i], value);
          cookie = malloc((len + 1)*sizeof(char));
          if (cookie == NULL) {
            free(value);
            u_map_clean_enum(keys);
            ulfius_clean_request_full(copy_request);
            curl_slist_free_all(header_list);
            curl_easy_cleanup(curl_handle);
            return U_ERROR_MEMORY;
          }
          snprintf(cookie, (len + 1), "%s=%s", keys[i], value);
          if (curl_easy_setopt(curl_handle, CURLOPT_COOKIE, cookie) != CURLE_OK) {
            free(value);
            free(cookie);
            u_map_clean_enum(keys);
            ulfius_clean_request_full(copy_request);
            curl_slist_free_all(header_list);
            curl_easy_cleanup(curl_handle);
            return U_ERROR_MEMORY;
          }
          free(value);
          free(cookie);
        }
        u_map_clean_enum(keys);
      }
      
      // Request parameters
      if (curl_easy_setopt(curl_handle, CURLOPT_URL, copy_request->http_url) != CURLE_OK ||
          curl_easy_setopt(curl_handle, CURLOPT_CUSTOMREQUEST, copy_request->http_verb) != CURLE_OK ||
          curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, header_list) != CURLE_OK ||
          curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_body) != CURLE_OK ||
          curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &body_data) != CURLE_OK) {
        ulfius_clean_request_full(copy_request);
        curl_slist_free_all(header_list);
        curl_easy_cleanup(curl_handle);
        return U_ERROR_MEMORY;
      }
      
      // Response parameters
      if (response != NULL) {
        if (response->map_header != NULL) {
          if (curl_easy_setopt(curl_handle, CURLOPT_HEADERFUNCTION, write_header) != CURLE_OK ||
              curl_easy_setopt(curl_handle, CURLOPT_WRITEHEADER, response) != CURLE_OK) {
            ulfius_clean_request_full(copy_request);
            curl_slist_free_all(header_list);
            curl_easy_cleanup(curl_handle);
            return U_ERROR_LIBCURL;
          }
        }
      }

      if (copy_request->binary_body != NULL) {
        if (curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, copy_request->binary_body) != CURLE_OK) {
          ulfius_clean_request_full(copy_request);
          curl_slist_free_all(header_list);
          curl_easy_cleanup(curl_handle);
          return U_ERROR_LIBCURL;
        }
        if (copy_request->binary_body_length > 0) {
          if (curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDSIZE, copy_request->binary_body_length) != CURLE_OK) {
            ulfius_clean_request_full(copy_request);
            curl_slist_free_all(header_list);
            curl_easy_cleanup(curl_handle);
            return U_ERROR_LIBCURL;
          }
        }
      }

      if (curl_easy_setopt(curl_handle, CURLOPT_NOSIGNAL, 1) != CURLE_OK) {
        curl_slist_free_all(header_list);
        curl_easy_cleanup(curl_handle);
        ulfius_clean_request_full(copy_request);
        return U_ERROR_LIBCURL;
      }
      res = curl_easy_perform(curl_handle);
      if(res == CURLE_OK && response != NULL) {
        if (curl_easy_getinfo (curl_handle, CURLINFO_RESPONSE_CODE, &response->status) != CURLE_OK) {
          ulfius_clean_request_full(copy_request);
          free(body_data.data);
          curl_slist_free_all(header_list);
          curl_easy_cleanup(curl_handle);
          ulfius_clean_request_full(copy_request);
          return U_ERROR_LIBCURL;
        }
        response->string_body = u_strdup(body_data.data);
        response->binary_body = malloc(body_data.size);
        if (response->binary_body == NULL) {
          free(body_data.data);
          curl_slist_free_all(header_list);
          curl_easy_cleanup(curl_handle);
          ulfius_clean_request_full(copy_request);
          return U_ERROR_MEMORY;
        }
        memcpy(response->binary_body, body_data.data, body_data.size);
        response->binary_body_length = body_data.size;
      }
      free(body_data.data);
      curl_slist_free_all(header_list);
      curl_easy_cleanup(curl_handle);
      ulfius_clean_request_full(copy_request);
      
      if (res == CURLE_OK) {
        return U_OK;
      } else {
        return U_ERROR_LIBCURL;
      }
    }
  }
  return U_ERROR_PARAMS;
}
