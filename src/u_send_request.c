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
  /* received header is nitems * size long in 'buffer' NOT ZERO TERMINATED */
  /* 'userdata' is set with CURLOPT_WRITEDATA */
  
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
    response->protocol = u_strdup(header);
  }
  
  return nitems * size;
}

/**
 * ulfius_send_request
 * Send a HTTP request and return the result into a _u_response
 * return 1 on send success, 0 otherwise
 */
int ulfius_request_http(const struct _u_request * request, struct _u_response * response) {
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

    if (copy_request->map_url != NULL && u_map_count(copy_request->map_url) > 0) {
      keys = u_map_enum_keys(copy_request->map_url);
      for (i=0; keys[i] != NULL; i++) {
        value = u_map_get(copy_request->map_url, keys[i]);
        key_esc = curl_easy_escape(curl_handle, keys[i], 0);
        value_esc = curl_easy_escape(curl_handle, value, 0);
        len = snprintf(NULL, 0, "%s=%s", key_esc, value_esc);
        param = malloc((len + 1)*sizeof(char));
        snprintf(param, (len + 1), "%s=%s", key_esc, value_esc);
        if (i==0) {
          copy_request->http_url = realloc(copy_request->http_url, strlen(copy_request->http_url) + strlen(param) + 2);
          if (strchr(copy_request->http_url, '?') != NULL) {
            strcat(copy_request->http_url, "&");
          } else {
            strcat(copy_request->http_url, "?");
          }
          strcat(copy_request->http_url, param);
        } else {
          copy_request->http_url = realloc(copy_request->http_url, strlen(copy_request->http_url) + strlen(param) + 2);
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
      copy_request->binary_body_length = strlen(copy_request->binary_body);
      u_map_put(copy_request->map_header, ULFIUS_HTTP_HEADER_CONTENT, ULFIUS_HTTP_ENCODING_JSON);
    } else if (u_map_count(copy_request->map_post_body) > 0) {
      free(copy_request->binary_body);
      keys = u_map_enum_keys(copy_request->map_post_body);
      for (i=0; keys[i] != NULL; i++) {
        value = u_map_get(copy_request->map_post_body, keys[i]);
        key_esc = curl_easy_escape(curl_handle, keys[i], 0);
        value_esc = curl_easy_escape(curl_handle, value, 0);
        len = snprintf(NULL, 0, "%s=%s", key_esc, value_esc);
        param = malloc((len + 1)*sizeof(char));
        snprintf(param, (len + 1), "%s=%s", key_esc, value_esc);
        if (i==0) {
          copy_request->http_url = realloc(copy_request->http_url, strlen(copy_request->http_url) + strlen(param) + 1);
          strcat(copy_request->http_url, param);
        } else {
          copy_request->http_url = realloc(copy_request->http_url, strlen(copy_request->http_url) + strlen(param) + 1);
          strcat(copy_request->http_url, param);
        }
        if (i == 0) {
          copy_request->binary_body = malloc(strlen(param) + 1);
        } else {
          copy_request->binary_body = realloc(copy_request->binary_body, strlen(copy_request->binary_body) + strlen(param) + 2);
          strcat(copy_request->binary_body, "&");
        }
        strcat(copy_request->binary_body, param);
        free(value);
        free(param);
        curl_free(key_esc);
        curl_free(value_esc);
      }
      u_map_clean_enum(keys);
      curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, copy_request->binary_body);
      u_map_put(copy_request->map_header, ULFIUS_HTTP_HEADER_CONTENT, MHD_HTTP_POST_ENCODING_FORM_URLENCODED);
    }

    if (copy_request->map_header != NULL && u_map_count(copy_request->map_header) > 0) {
      keys = u_map_enum_keys(copy_request->map_header);
      for (i=0; keys[i] != NULL; i++) {
        value = u_map_get(copy_request->map_header, keys[i]);
        len = snprintf(NULL, 0, "%s:%s", keys[i], value);
        header = malloc((len + 1)*sizeof(char));
        snprintf(header, (len + 1), "%s:%s", keys[i], value);
        header_list = curl_slist_append(header_list, header);
        free(value);
        free(header);
      }
      u_map_clean_enum(keys);
    }

    if (copy_request->map_cookie != NULL && u_map_count(copy_request->map_cookie) > 0) {
      keys = u_map_enum_keys(copy_request->map_cookie);
      for (i=0; keys[i] != NULL; i++) {
        value = u_map_get(copy_request->map_cookie, keys[i]);
        len = snprintf(NULL, 0, "%s=%s", keys[i], value);
        cookie = malloc((len + 1)*sizeof(char));
        snprintf(cookie, (len + 1), "%s=%s", keys[i], value);
        curl_easy_setopt(curl_handle, CURLOPT_COOKIE, cookie);
        free(value);
        free(cookie);
      }
      u_map_clean_enum(keys);
    }
    
    // Request parameters
    curl_easy_setopt(curl_handle, CURLOPT_URL, copy_request->http_url);
    curl_easy_setopt(curl_handle, CURLOPT_CUSTOMREQUEST, copy_request->http_verb);
    curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, header_list);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_body);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &body_data);
    
    // Response parameters
    // TODO: handle Set-Cookie headers
    if (response != NULL) {
      if (response->map_header != NULL) {
        curl_easy_setopt(curl_handle, CURLOPT_HEADERFUNCTION, write_header);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEHEADER, response);
      }
    }

    if (copy_request->binary_body != NULL) {
      curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, copy_request->binary_body);
      if (copy_request->binary_body_length > 0) {
        curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDSIZE, copy_request->binary_body_length);
      }
    }

    curl_easy_setopt(curl_handle, CURLOPT_NOSIGNAL, 1);
    res = curl_easy_perform(curl_handle);
    if(res == CURLE_OK && response != NULL) {
      curl_easy_getinfo (curl_handle, CURLINFO_RESPONSE_CODE, &response->status);
      response->string_body = u_strdup(body_data.data);
      response->binary_body = malloc(body_data.size);
      memcpy(response->binary_body, body_data.data, body_data.size);
      response->binary_body_length = body_data.size;
    }
    free(body_data.data);
    curl_slist_free_all(header_list);
    curl_easy_cleanup(curl_handle);
    ulfius_clean_request_full(copy_request);
    
    return (res == CURLE_OK);
  }
  return 0;
}
