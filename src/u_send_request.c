/**
 * 
 * Ulfius Framework
 * 
 * REST framework library
 * 
 * u_send_request.c: send request related functions defintions
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
#include "u_private.h"
#include "ulfius.h"

#ifndef U_DISABLE_CURL

#include <stdlib.h>
#include <ctype.h>
#include <curl/curl.h>
#include <string.h>

#ifdef _MSC_VER
#define strtok_r strtok_s

struct tm * gmtime_r(const time_t* t, struct tm* r) {
  // gmtime is threadsafe in windows
  struct tm* that = gmtime(t);
  if (that != NULL) {
    *r = *that;
    return r;
  } else {
    return NULL;
  }
}
#endif

/**
 * Internal structure used to store temporarly the response body
 */
typedef struct _body {
  char * data;
  size_t size;
} body;

/**
 * ulfius_write_body
 * Internal function used to write the body response into a _body structure
 */
static size_t ulfius_write_body(void * contents, size_t size, size_t nmemb, void * user_data) {
  size_t realsize = size * nmemb;
  body * body_data = (body *) user_data;
 
  body_data->data = o_realloc(body_data->data, body_data->size + realsize + 1);
  if(body_data->data == NULL) {
    y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for body_data->data");
    return 0;
  }
  
  memcpy(&(body_data->data[body_data->size]), contents, realsize);
  body_data->size += realsize;
  body_data->data[body_data->size] = 0;
 
  return realsize;
}

/**
 * write_header
 * Write the header value into the response map_header structure
 * return the size_t of the header written
 */
static size_t write_header(void * buffer, size_t size, size_t nitems, void * user_data) {
  
  struct _u_response * response = (struct _u_response *) user_data;
  char * header = (char *)buffer, * key, * value, * saveptr, * tmp;
  
  if (o_strchr(header, ':') != NULL) {
    if (response->map_header != NULL) {
      // Expecting a header (key: value)
      key = trimwhitespace(strtok_r(header, ":", &saveptr));
      value = trimwhitespace(strtok_r(NULL, "", &saveptr));
      
      if (!u_map_has_key_case(response->map_header, key)) {
        u_map_put(response->map_header, key, value);
      } else {
        tmp = msprintf("%s, %s", u_map_get_case(response->map_header, key), value);
        if (u_map_remove_from_key_case(response->map_header, key) != U_OK || u_map_put(response->map_header, key, tmp)) {
          y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error setting header value for name %s", key);
        }
        o_free(tmp);
      }
    }
  } else if (o_strlen(trimwhitespace(header)) > 0) {
    // Expecting the HTTP/x.x header
    if (response->protocol != NULL) {
      o_free(response->protocol);
    }
    response->protocol = o_strdup(header);
    if (response->protocol == NULL) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for response->protocol");
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
  body body_data;
  body_data.size = 0;
  body_data.data = NULL;
  int res;
  
  res = ulfius_send_http_streaming_request(request, response, ulfius_write_body, (void *)&body_data);
  
  if (res == U_OK && response != NULL) {
    if (body_data.data != NULL && body_data.size > 0) {
      response->binary_body = o_malloc(body_data.size);
      if (response->binary_body == NULL) {
        y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for response->binary_body");
        o_free(body_data.data);
        return U_ERROR_MEMORY;
      }
      memcpy(response->binary_body, body_data.data, body_data.size);
      response->binary_body_length = body_data.size;
    }
    o_free(body_data.data);
    return U_OK;
  } else {
    o_free(body_data.data);
    return res;
  }
}

/**
 * ulfius_send_http_streaming_request
 * Send a HTTP request and store the result into a _u_response
 * Except for the body which will be available using write_body_function in the write_body_data
 * return U_OK on success
 */
int ulfius_send_http_streaming_request(const struct _u_request * request, struct _u_response * response, size_t (* write_body_function)(void * contents, size_t size, size_t nmemb, void * user_data), void * write_body_data) {
  CURLcode res;
  CURL * curl_handle = NULL;
  struct curl_slist * header_list = NULL, * cookies_list = NULL;
  char * key_esc, * value_esc, * cookie, * header, * param, * fp = "?", * np = "&";
  const char * value, ** keys;
  int i, has_params = 0, len;
  struct _u_request * copy_request = NULL;
  o_malloc_t malloc_fn;
  o_realloc_t realloc_fn;
  o_free_t free_fn;

  if (request != NULL) {
    // Duplicate the request and work on it
    copy_request = ulfius_duplicate_request(request);
    if (copy_request == NULL) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error ulfius_duplicate_request");
      return U_ERROR_MEMORY;
    } else {
    
      o_get_alloc_funcs(&malloc_fn, &realloc_fn, &free_fn);
      if (curl_global_init_mem(CURL_GLOBAL_DEFAULT, malloc_fn, free_fn, realloc_fn, *o_strdup, *calloc) != CURLE_OK) {
        y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error curl_global_init_mem");
        return U_ERROR_MEMORY;
      }
      curl_handle = curl_easy_init();

      // Append header values
      if (copy_request->map_header == NULL) {
        copy_request->map_header = o_malloc(sizeof(struct _u_map));
        if (copy_request->map_header == NULL) {
          y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for copy_request->map_header");
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
      
      // Set basic auth if defined
      if (copy_request->auth_basic_user != NULL && copy_request->auth_basic_password != NULL) {
        if (curl_easy_setopt(curl_handle, CURLOPT_HTTPAUTH, CURLAUTH_BASIC) == CURLE_OK) {
          if (curl_easy_setopt(curl_handle, CURLOPT_USERNAME, copy_request->auth_basic_user) != CURLE_OK ||
              curl_easy_setopt(curl_handle, CURLOPT_PASSWORD, copy_request->auth_basic_password) != CURLE_OK) {
            y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error setting HTTP Basic user name or password");
            ulfius_clean_request_full(copy_request);
            curl_easy_cleanup(curl_handle);
            return U_ERROR_LIBCURL;
          }
        } else {
          y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error setting HTTP Basic Auth option");
          ulfius_clean_request_full(copy_request);
          curl_easy_cleanup(curl_handle);
          return U_ERROR_LIBCURL;
        }
      }

#ifndef U_DISABLE_GNUTLS
      // Set client certificate authentication if defined
      if (request->client_cert_file != NULL && request->client_key_file != NULL) {
        if (curl_easy_setopt(curl_handle, CURLOPT_SSLCERT, request->client_cert_file) != CURLE_OK) {
          y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error setting client certificate file");
          ulfius_clean_request_full(copy_request);
          curl_easy_cleanup(curl_handle);
          return U_ERROR_LIBCURL;
        } else if (curl_easy_setopt(curl_handle, CURLOPT_SSLKEY, request->client_key_file) != CURLE_OK) {
          y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error setting client key file");
          ulfius_clean_request_full(copy_request);
          curl_easy_cleanup(curl_handle);
          return U_ERROR_LIBCURL;
        } else if (request->client_key_password != NULL && curl_easy_setopt(curl_handle, CURLOPT_KEYPASSWD, request->client_key_password) != CURLE_OK) {
          y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error setting client key password");
          ulfius_clean_request_full(copy_request);
          curl_easy_cleanup(curl_handle);
          return U_ERROR_LIBCURL;
        }
      }
#endif
      
      // Set proxy if defined
      if (copy_request->proxy != NULL) {
        if (curl_easy_setopt(curl_handle, CURLOPT_PROXY, copy_request->proxy) != CURLE_OK) {
          y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error setting proxy option");
          ulfius_clean_request_full(copy_request);
          curl_easy_cleanup(curl_handle);
          return U_ERROR_LIBCURL;
        }
      }

      // Set network type
      if (copy_request->network_type & U_USE_ALL) {
        if (curl_easy_setopt(curl_handle, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_WHATEVER) != CURLE_OK) {
          y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error setting IPRESOLVE WHATEVER option");
          ulfius_clean_request_full(copy_request);
          curl_easy_cleanup(curl_handle);
          return U_ERROR_LIBCURL;
        }
      } else if (copy_request->network_type & U_USE_IPV6) {
        if (curl_easy_setopt(curl_handle, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V6) != CURLE_OK) {
          y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error setting IPRESOLVE V6 option");
          ulfius_clean_request_full(copy_request);
          curl_easy_cleanup(curl_handle);
          return U_ERROR_LIBCURL;
        }
      } else {
        if (curl_easy_setopt(curl_handle, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4) != CURLE_OK) {
          y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error setting IPRESOLVE V4 option");
          ulfius_clean_request_full(copy_request);
          curl_easy_cleanup(curl_handle);
          return U_ERROR_LIBCURL;
        }
      }
      
      has_params = (o_strchr(copy_request->http_url, '?') != NULL);
      if (copy_request->map_url != NULL && u_map_count(copy_request->map_url) > 0) {
        // Append url parameters
        keys = u_map_enum_keys(copy_request->map_url);
        
        // Append parameters from map_url
        for (i=0; keys != NULL && keys[i] != NULL; i++) {
          value = u_map_get(copy_request->map_url, keys[i]);
          key_esc = curl_easy_escape(curl_handle, keys[i], 0);
          value_esc = curl_easy_escape(curl_handle, value, 0);
          if (key_esc == NULL || value_esc == NULL) {
            curl_free(key_esc);
            curl_free(value_esc);
            ulfius_clean_request_full(copy_request);
            curl_slist_free_all(header_list);
            curl_easy_cleanup(curl_handle);
            return U_ERROR_MEMORY;
          }
          param = msprintf("%s=%s", key_esc, value_esc);
          if (param == NULL) {
            y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for param");
            curl_free(key_esc);
            curl_free(value_esc);
            ulfius_clean_request_full(copy_request);
            curl_slist_free_all(header_list);
            curl_easy_cleanup(curl_handle);
            return U_ERROR_MEMORY;
          }
          if (has_params == 0) {
            copy_request->http_url = o_realloc(copy_request->http_url, o_strlen(copy_request->http_url) + o_strlen(param) + 2);
            if (copy_request->http_url == NULL) {
              y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for copy_request->http_url");
              o_free(param);
              curl_free(key_esc);
              curl_free(value_esc);
              ulfius_clean_request_full(copy_request);
              curl_slist_free_all(header_list);
              curl_easy_cleanup(curl_handle);
              return U_ERROR_MEMORY;
            }
            strcat(copy_request->http_url, fp);
            strcat(copy_request->http_url, param);
            has_params = 1;
          } else {
            copy_request->http_url = o_realloc(copy_request->http_url, o_strlen(copy_request->http_url) + o_strlen(param) + 2);
            if (copy_request->http_url == NULL) {
              y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for copy_request->http_url");
              o_free(param);
              curl_free(key_esc);
              curl_free(value_esc);
              ulfius_clean_request_full(copy_request);
              curl_slist_free_all(header_list);
              curl_easy_cleanup(curl_handle);
              return U_ERROR_MEMORY;
            }
            strcat(copy_request->http_url, np);
            strcat(copy_request->http_url, param);
          }
          o_free(param);
          curl_free(key_esc);
          curl_free(value_esc);
        }
      }

      if (u_map_count(copy_request->map_post_body) > 0) {
        // Append MHD_HTTP_POST_ENCODING_FORM_URLENCODED post parameters
        keys = u_map_enum_keys(copy_request->map_post_body);
        for (i=0; keys != NULL && keys[i] != NULL; i++) {
          // Build parameter
          value = u_map_get(copy_request->map_post_body, keys[i]);
          if (value == NULL) {
            ulfius_clean_request_full(copy_request);
            curl_slist_free_all(header_list);
            curl_easy_cleanup(curl_handle);
            return U_ERROR_MEMORY;
          }
          key_esc = curl_easy_escape(curl_handle, keys[i], 0);
          value_esc = curl_easy_escape(curl_handle, value, 0);
          if (key_esc == NULL || value_esc == NULL) {
            curl_free(key_esc);
            curl_free(value_esc);
            ulfius_clean_request_full(copy_request);
            curl_slist_free_all(header_list);
            curl_easy_cleanup(curl_handle);
            return U_ERROR_MEMORY;
          }
          param = msprintf("%s=%s", key_esc, value_esc);
          if (param == NULL) {
            y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for param");
            curl_free(key_esc);
            curl_free(value_esc);
            ulfius_clean_request_full(copy_request);
            curl_slist_free_all(header_list);
            curl_easy_cleanup(curl_handle);
            return U_ERROR_MEMORY;
          }
          
          // Append parameter to body
          if (copy_request->binary_body_length == 0) {
            len = o_strlen(param) + sizeof(char);
            copy_request->binary_body = o_malloc(len);
            if (copy_request->binary_body == NULL) {
              y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for copy_request->binary_body");
              o_free(param);
              curl_free(key_esc);
              curl_free(value_esc);
              ulfius_clean_request_full(copy_request);
              curl_slist_free_all(header_list);
              curl_easy_cleanup(curl_handle);
              return U_ERROR_MEMORY;
            }
            o_strcpy(copy_request->binary_body, "");
            strcat(copy_request->binary_body, param);
            copy_request->binary_body_length = len;
          } else {
            len = (copy_request->binary_body_length + o_strlen(param) + sizeof(char));
            copy_request->binary_body = o_realloc(copy_request->binary_body, len);
            if (copy_request->binary_body == NULL) {
              y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for copy_request->binary_body");
              o_free(param);
              curl_free(key_esc);
              curl_free(value_esc);
              ulfius_clean_request_full(copy_request);
              curl_slist_free_all(header_list);
              curl_easy_cleanup(curl_handle);
              return U_ERROR_MEMORY;
            }
            memcpy((char*)copy_request->binary_body + copy_request->binary_body_length, np, 1);
            memcpy((char*)copy_request->binary_body + copy_request->binary_body_length + 1, param, o_strlen(param));
            copy_request->binary_body_length = len;
          }
          
          o_free(param);
          curl_free(key_esc);
          curl_free(value_esc);
        }
        
        if (curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, copy_request->binary_body) != CURLE_OK) {
          ulfius_clean_request_full(copy_request);
          curl_slist_free_all(header_list);
          curl_easy_cleanup(curl_handle);
          y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error setting POST fields");
          return U_ERROR_LIBCURL;
        }
        
        if (u_map_put(copy_request->map_header, ULFIUS_HTTP_HEADER_CONTENT, MHD_HTTP_POST_ENCODING_FORM_URLENCODED) != U_OK) {
          ulfius_clean_request_full(copy_request);
          curl_slist_free_all(header_list);
          curl_easy_cleanup(curl_handle);
          return U_ERROR_MEMORY;
        }
      }

      if (u_map_count(copy_request->map_header) > 0) {
        // Append map headers
        keys = u_map_enum_keys(copy_request->map_header);
        for (i=0; keys != NULL && keys[i] != NULL; i++) {
          value = u_map_get(copy_request->map_header, keys[i]);
          if (value == NULL) {
            ulfius_clean_request_full(copy_request);
            curl_slist_free_all(header_list);
            curl_easy_cleanup(curl_handle);
            return U_ERROR_MEMORY;
          }
          header = msprintf("%s:%s", keys[i], value);
          if (header == NULL) {
            y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for header");
            ulfius_clean_request_full(copy_request);
            curl_slist_free_all(header_list);
            curl_easy_cleanup(curl_handle);
            return U_ERROR_MEMORY;
          }
          header_list = curl_slist_append(header_list, header);
          if (header_list == NULL) {
            o_free(header);
            ulfius_clean_request_full(copy_request);
            curl_slist_free_all(header_list);
            curl_easy_cleanup(curl_handle);
            return U_ERROR_MEMORY;
          }
          o_free(header);
        }
      }

      if (copy_request->map_cookie != NULL && u_map_count(copy_request->map_cookie) > 0) {
        // Append cookies
        keys = u_map_enum_keys(copy_request->map_cookie);
        for (i=0; keys != NULL && keys[i] != NULL; i++) {
          value = u_map_get(copy_request->map_cookie, keys[i]);
          if (value == NULL) {
            ulfius_clean_request_full(copy_request);
            curl_slist_free_all(header_list);
            curl_easy_cleanup(curl_handle);
            return U_ERROR_MEMORY;
          }
          cookie = msprintf("%s=%s", keys[i], value);
          if (cookie == NULL) {
            y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for cookie");
            ulfius_clean_request_full(copy_request);
            curl_slist_free_all(header_list);
            curl_easy_cleanup(curl_handle);
            return U_ERROR_MEMORY;
          }
          if (curl_easy_setopt(curl_handle, CURLOPT_COOKIE, cookie) != CURLE_OK) {
            o_free(cookie);
            ulfius_clean_request_full(copy_request);
            curl_slist_free_all(header_list);
            curl_easy_cleanup(curl_handle);
            return U_ERROR_MEMORY;
          }
          o_free(cookie);
        }
      }
      
      // Request parameters
      if (curl_easy_setopt(curl_handle, CURLOPT_URL, copy_request->http_url) != CURLE_OK ||
          curl_easy_setopt(curl_handle, CURLOPT_CUSTOMREQUEST, copy_request->http_verb!=NULL?copy_request->http_verb:"GET") != CURLE_OK ||
          curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, header_list) != CURLE_OK) {
        ulfius_clean_request_full(copy_request);
        curl_slist_free_all(header_list);
        curl_easy_cleanup(curl_handle);
        y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error setting libcurl options (1)");
        return U_ERROR_LIBCURL;
      }
      
      // Set CURLOPT_WRITEFUNCTION if specified
      if (write_body_function != NULL && curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_body_function) != CURLE_OK) {
        ulfius_clean_request_full(copy_request);
        curl_slist_free_all(header_list);
        curl_easy_cleanup(curl_handle);
        y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error setting libcurl options (2)");
        return U_ERROR_LIBCURL;
      }
      
      // Set CURLOPT_WRITEDATA if specified
      if (write_body_data != NULL && curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, write_body_data) != CURLE_OK) {
        ulfius_clean_request_full(copy_request);
        curl_slist_free_all(header_list);
        curl_easy_cleanup(curl_handle);
        y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error setting libcurl options (3)");
        return U_ERROR_LIBCURL;
      }
      
      // Disable server certificate validation if needed
      if (!copy_request->check_server_certificate) {
        if (curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0) != CURLE_OK || curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 0) != CURLE_OK) {
          ulfius_clean_request_full(copy_request);
          curl_slist_free_all(header_list);
          curl_easy_cleanup(curl_handle);
          y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error setting libcurl options (4)");
          return U_ERROR_LIBCURL;
        }
      } else {
        if (!(copy_request->check_server_certificate_flag & U_SSL_VERIFY_PEER)) {
          if (curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0) != CURLE_OK) {
            ulfius_clean_request_full(copy_request);
            curl_slist_free_all(header_list);
            curl_easy_cleanup(curl_handle);
            y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error setting libcurl options (5)");
            return U_ERROR_LIBCURL;
          }
        }
        if (!(copy_request->check_server_certificate_flag & U_SSL_VERIFY_HOSTNAME)) {
          if (curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 0) != CURLE_OK) {
            ulfius_clean_request_full(copy_request);
            curl_slist_free_all(header_list);
            curl_easy_cleanup(curl_handle);
            y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error setting libcurl options (6)");
            return U_ERROR_LIBCURL;
          }
        }
      }
      
#if LIBCURL_VERSION_NUM >= 0x073400
      // Disable proxy certificate validation if needed
      if (!copy_request->check_proxy_certificate) {
        if (curl_easy_setopt(curl_handle, CURLOPT_PROXY_SSL_VERIFYPEER, 0) != CURLE_OK || curl_easy_setopt(curl_handle, CURLOPT_PROXY_SSL_VERIFYHOST, 0) != CURLE_OK) {
          ulfius_clean_request_full(copy_request);
          curl_slist_free_all(header_list);
          curl_easy_cleanup(curl_handle);
          y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error setting libcurl options (7)");
          return U_ERROR_LIBCURL;
        }
      } else {
        if (!(copy_request->check_proxy_certificate_flag & U_SSL_VERIFY_PEER)) {
          if (curl_easy_setopt(curl_handle, CURLOPT_PROXY_SSL_VERIFYPEER, 0) != CURLE_OK) {
            ulfius_clean_request_full(copy_request);
            curl_slist_free_all(header_list);
            curl_easy_cleanup(curl_handle);
            y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error setting libcurl options (8)");
            return U_ERROR_LIBCURL;
          }
        }
        if (!(copy_request->check_proxy_certificate_flag & U_SSL_VERIFY_HOSTNAME)) {
          if (curl_easy_setopt(curl_handle, CURLOPT_PROXY_SSL_VERIFYHOST, 0) != CURLE_OK) {
            ulfius_clean_request_full(copy_request);
            curl_slist_free_all(header_list);
            curl_easy_cleanup(curl_handle);
            y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error setting libcurl options (9)");
            return U_ERROR_LIBCURL;
          }
        }
      }
#endif
      
      // Set request ca_path value
      if (copy_request->ca_path) {
        if (curl_easy_setopt(curl_handle, CURLOPT_CAPATH, copy_request->ca_path) != CURLE_OK) {
          ulfius_clean_request_full(copy_request);
          curl_slist_free_all(header_list);
          curl_easy_cleanup(curl_handle);
          y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error setting libcurl options (10)");
          return U_ERROR_LIBCURL;
        }
      }
      
      // Set request timeout value
      if (copy_request->timeout) {
        if (curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, copy_request->timeout) != CURLE_OK) {
          ulfius_clean_request_full(copy_request);
          curl_slist_free_all(header_list);
          curl_easy_cleanup(curl_handle);
          y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error setting libcurl options (10)");
          return U_ERROR_LIBCURL;
        }
      }
      
      // Response parameters
      if (response != NULL) {
        if (response->map_header != NULL) {
          if (curl_easy_setopt(curl_handle, CURLOPT_HEADERFUNCTION, write_header) != CURLE_OK ||
              curl_easy_setopt(curl_handle, CURLOPT_WRITEHEADER, response) != CURLE_OK) {
            ulfius_clean_request_full(copy_request);
            curl_slist_free_all(header_list);
            curl_easy_cleanup(curl_handle);
            y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error setting headers");
            return U_ERROR_LIBCURL;
          }
        }
      }

      if (copy_request->binary_body_length > 0 && copy_request->binary_body != NULL) {
        if (copy_request->binary_body_length < 2147483648) {
          if (curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDSIZE, copy_request->binary_body_length) != CURLE_OK) {
            ulfius_clean_request_full(copy_request);
            curl_slist_free_all(header_list);
            curl_easy_cleanup(curl_handle);
            y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error setting POST fields size");
            return U_ERROR_LIBCURL;
          }
        } else { // Use CURLOPT_POSTFIELDSIZE_LARGE if binary_body_length is larger than 2GB
          if (curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDSIZE_LARGE, copy_request->binary_body_length) != CURLE_OK) {
            ulfius_clean_request_full(copy_request);
            curl_slist_free_all(header_list);
            curl_easy_cleanup(curl_handle);
            y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error setting POST fields size");
            return U_ERROR_LIBCURL;
          }
        }
        if (curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, copy_request->binary_body) != CURLE_OK) {
          ulfius_clean_request_full(copy_request);
          curl_slist_free_all(header_list);
          curl_easy_cleanup(curl_handle);
          y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error setting POST fields");
          return U_ERROR_LIBCURL;
        }
      }

      if (curl_easy_setopt(curl_handle, CURLOPT_NOSIGNAL, 1) != CURLE_OK) {
        curl_slist_free_all(header_list);
        curl_easy_cleanup(curl_handle);
        ulfius_clean_request_full(copy_request);
        y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error setting libcurl CURLOPT_NOSIGNAL");
        return U_ERROR_LIBCURL;
      }
      if (curl_easy_setopt(curl_handle, CURLOPT_COOKIEFILE, "") != CURLE_OK) { // Apparently you have to do that to tell libcurl you'll need cookies afterwards
        curl_slist_free_all(header_list);
        curl_easy_cleanup(curl_handle);
        ulfius_clean_request_full(copy_request);
        y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error setting libcurl CURLOPT_COOKIEFILE");
        return U_ERROR_LIBCURL;
      }
      
      res = curl_easy_perform(curl_handle);
      if(res == CURLE_OK && response != NULL) {
        if (curl_easy_getinfo (curl_handle, CURLINFO_RESPONSE_CODE, &response->status) != CURLE_OK) {
          curl_slist_free_all(header_list);
          curl_easy_cleanup(curl_handle);
          ulfius_clean_request_full(copy_request);
          y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error executing http request, libcurl error: %d, error message %s", res, curl_easy_strerror(res));
          return U_ERROR_LIBCURL;
        }
        
        if (curl_easy_getinfo(curl_handle, CURLINFO_COOKIELIST, &cookies_list) == CURLE_OK) {
          struct curl_slist * nc = cookies_list;
          char * key = NULL, * value = NULL, * expires = NULL, * domain = NULL, * path = NULL;
          int secure = 0, http_only = 0;
          
          while (nc != NULL) {
            char * nc_dup = o_strdup(nc->data), * saveptr, * elt;
            int counter = 0;
            
            if (nc_dup != NULL) {
              elt = strtok_r(nc_dup, "\t", &saveptr);
              while (elt != NULL) {
                // libcurl cookie format is domain\tsecure\tpath\thttp_only\texpires\tkey\tvalue
                switch (counter) {
                  case 0:
                    domain = o_strdup(elt);
                    break;
                  case 1:
                    secure = (0==o_strcmp(elt, "TRUE"));
                    break;
                  case 2:
                    path = o_strdup(elt);
                    break;
                  case 3:
                    http_only = (0==o_strcmp(elt, "TRUE"));
                    break;
                  case 4:
                    expires = o_strdup(elt);
                    break;
                  case 5:
                    key = o_strdup(elt);
                    break;
                  case 6:
                    value = o_strdup(elt);
                    break;
                }
                elt = strtok_r(NULL, "\t", &saveptr);
                counter++;
              }
              if (ulfius_add_cookie_to_response(response, key, value, expires, 0, domain, path, secure, http_only) != U_OK) {
                y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error adding cookie %s/%s to response", key, value);
              }
              o_free(key);
              o_free(value);
              o_free(domain);
              o_free(path);
              o_free(expires);
            }
            o_free(nc_dup);
            nc = nc->next;
          }
        } else {
          curl_slist_free_all(header_list);
          curl_easy_cleanup(curl_handle);
          ulfius_clean_request_full(copy_request);
          y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error executing http request, libcurl error: %d, error message %s", res, curl_easy_strerror(res));
          return U_ERROR_LIBCURL;
        }
        curl_slist_free_all(cookies_list);
      }
      curl_slist_free_all(header_list);
      curl_easy_cleanup(curl_handle);
      ulfius_clean_request_full(copy_request);
      
      if (res == CURLE_OK) {
        return U_OK;
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error executing curl command: %s", curl_easy_strerror(res));
        return U_ERROR_LIBCURL;
      }
    }
  }
  return U_ERROR_PARAMS;
}

/**
 * ulfius_send_smtp_email body fill function and structures
 */
struct upload_status {
  size_t offset;
  size_t len;
  char * data;
};
 
static size_t smtp_payload_source(void * ptr, size_t size, size_t nmemb, void * userp) {
  struct upload_status *upload_ctx = (struct upload_status *)userp;
  size_t len;
  
  if ((size * nmemb) < (upload_ctx->len - upload_ctx->offset)) {
    len = size*nmemb;
  } else {
    len = upload_ctx->len - upload_ctx->offset;
  }
  memcpy(ptr, upload_ctx->data+upload_ctx->offset, len);
  upload_ctx->offset += len;
  return len;
}

/**
 * Send an email using libcurl
 * email is plain/text and UTF8 charset
 * host: smtp server host name
 * port: tcp port number (optional, 0 for default)
 * use_tls: true if the connection is tls secured
 * verify_certificate: true if you want to disable the certificate verification on a tls server
 * user: connection user name (optional, NULL: no user name)
 * password: connection password (optional, NULL: no password)
 * from: from address (mandatory)
 * to: to recipient address (mandatory)
 * cc: cc recipient address (optional, NULL: no cc)
 * bcc: bcc recipient address (optional, NULL: no bcc)
 * subject: email subject (mandatory)
 * mail_body: email body (mandatory)
 * return U_OK on success
 */
int ulfius_send_smtp_email(const char * host, 
                            const int port, 
                            const int use_tls, 
                            const int verify_certificate, 
                            const char * user, 
                            const char * password, 
                            const char * from, 
                            const char * to, 
                            const char * cc, 
                            const char * bcc, 
                            const char * subject, 
                            const char * mail_body) {
  CURL * curl_handle;
  CURLcode res = CURLE_OK;
  char * smtp_url;
  int cur_port;
  struct curl_slist * recipients = NULL;
  struct upload_status upload_ctx;
  time_t now_sec;
  struct tm now;
  char date_str[129], * cc_str;
  
  if (host != NULL && from != NULL && to != NULL && mail_body != NULL) {
    
    curl_handle = curl_easy_init();
    if (curl_handle != NULL) {
      if (port == 0 && !use_tls) {
        cur_port = 25;
      } else if (port == 0 && use_tls) {
        cur_port = 587;
      } else {
        cur_port = port;
      }
      smtp_url = msprintf("smtp%s://%s:%d", use_tls?"s":"", host, cur_port);
      if (smtp_url == NULL) {
        y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for smtp_url");
        return U_ERROR_MEMORY;
      }
      curl_easy_setopt(curl_handle, CURLOPT_URL, smtp_url);
      
      if (use_tls) {
        curl_easy_setopt(curl_handle, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);
      }
      
      if (use_tls && !verify_certificate) {
        curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 0L);
      }
      
      if (user != NULL && password != NULL) {
        curl_easy_setopt(curl_handle, CURLOPT_USERNAME, user);
        curl_easy_setopt(curl_handle, CURLOPT_PASSWORD, password);
      }
      
      curl_easy_setopt(curl_handle, CURLOPT_MAIL_FROM, from);
      
      recipients = curl_slist_append(recipients, to);
      if (cc != NULL) {
        recipients = curl_slist_append(recipients, cc);
      }
      if (bcc != NULL) {
        recipients = curl_slist_append(recipients, bcc);
      }
      curl_easy_setopt(curl_handle, CURLOPT_MAIL_RCPT, recipients);
      
      time(&now_sec);
      upload_ctx.offset = 0;
      gmtime_r(&now_sec, &now);
#ifdef _WIN32
      strftime(date_str, 128, "Date: %a, %d %b %Y %H:%M:%S %z", &now);
#else
      strftime(date_str, 128, "Date: %a, %d %b %Y %T %z", &now);
#endif
      if (cc != NULL) {
        cc_str = msprintf("Cc: %s\r\n", cc);
      } else {
        cc_str = o_strdup("");
      }
      upload_ctx.data = msprintf("%s\r\n" // date_str
                                 "To: %s\r\n"
                                 "From: %s\r\n"
                                 "%s"
                                 "Subject: %s\r\n"
                                 "Content-Type: text/plain; charset=utf-8\r\n\r\n%s\r\n",
                                 date_str,
                                 to,
                                 from,
                                 cc_str,
                                 subject!=NULL?subject:"",
                                 mail_body);
      if (upload_ctx.data != NULL) {
        upload_ctx.len = o_strlen(upload_ctx.data);
        curl_easy_setopt(curl_handle, CURLOPT_READFUNCTION, smtp_payload_source);
        curl_easy_setopt(curl_handle, CURLOPT_READDATA, &upload_ctx);
        curl_easy_setopt(curl_handle, CURLOPT_UPLOAD, 1L);
        
        res = curl_easy_perform(curl_handle);
        curl_slist_free_all(recipients);
        curl_easy_cleanup(curl_handle);
        o_free(smtp_url);
        o_free(cc_str);
        o_free(upload_ctx.data);
        
        if (res != CURLE_OK) {
          y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error sending smtp message, libcurl error: %d, error message %s", res, curl_easy_strerror(res));
          return U_ERROR_LIBCURL;
        } else {
          return U_OK;
        }
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating resource for upload_ctx.data");
        return U_ERROR_MEMORY;
      }
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error executing curl_easy_init");
      return U_ERROR_LIBCURL;
    }
  } else {
    return U_ERROR_PARAMS;
  }
}
#endif
