/**
 * 
 * Ulfius Framework
 * 
 * REST framework library
 * 
 * u_send_request.c: send request related functions defintions
 * 
 * Copyright 2015-2019 Nicolas Mora <mail@babelouest.org>
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
typedef struct _u_body {
  char * data;
  size_t size;
};

/**
 * ulfius_send_smtp_email body fill function and structures
 */
struct _u_smtp_payload {
  size_t offset;
  size_t len;
  char * data;
};

/**
 * ulfius_write_body
 * Internal function used to write the body response into a _body structure
 */
static size_t ulfius_write_body(void * contents, size_t size, size_t nmemb, void * user_data) {
  size_t realsize = size * nmemb;
  struct _u_body * body_data = (struct _u_body *) user_data;
 
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

static size_t smtp_payload_source(void * ptr, size_t size, size_t nmemb, void * userp) {
  struct _u_smtp_payload *upload_ctx = (struct _u_smtp_payload *)userp;
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
 * ulfius_send_http_request
 * Send a HTTP request and store the result into a _u_response
 * return U_OK on success
 */
int ulfius_send_http_request(const struct _u_request * request, struct _u_response * response) {
  struct _u_body body_data;
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
int ulfius_send_http_streaming_request(const struct _u_request * request,
                                       struct _u_response * response, 
                                       size_t (* write_body_function)(void * contents, size_t size, size_t nmemb, void * user_data), 
                                       void * write_body_data) {
  CURLcode res;
  CURL * curl_handle = NULL;
  struct curl_slist * header_list = NULL, * cookies_list = NULL;
  char * key_esc = NULL, * value_esc = NULL, * cookie = NULL, * header = NULL, * param = NULL, * fp = "?", * np = "&";
  const char * value = NULL, ** keys = NULL;
  int i, has_params = 0, len, ret, exit_loop;
  struct _u_request * copy_request = NULL;
  o_malloc_t malloc_fn;
  o_realloc_t realloc_fn;
  o_free_t free_fn;

  if (request != NULL) {
    // Duplicate the request and work on it
    if ((copy_request = ulfius_duplicate_request(request)) != NULL) {
      o_get_alloc_funcs(&malloc_fn, &realloc_fn, &free_fn);
      if (curl_global_init_mem(CURL_GLOBAL_DEFAULT, malloc_fn, free_fn, realloc_fn, *o_strdup, *calloc) == CURLE_OK) {
        if ((curl_handle = curl_easy_init()) != NULL) {
          ret = U_OK;

          // Here comes the fake loop with breaks to exit smoothly
          do {
            // Set basic auth if defined
            if (copy_request->auth_basic_user != NULL && copy_request->auth_basic_password != NULL) {
              if (curl_easy_setopt(curl_handle, CURLOPT_HTTPAUTH, CURLAUTH_BASIC) == CURLE_OK) {
                if (curl_easy_setopt(curl_handle, CURLOPT_USERNAME, copy_request->auth_basic_user) != CURLE_OK ||
                    curl_easy_setopt(curl_handle, CURLOPT_PASSWORD, copy_request->auth_basic_password) != CURLE_OK) {
                  y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error setting HTTP Basic user name or password");
                  ret = U_ERROR_LIBCURL;
                  break;
                }
              } else {
                y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error setting HTTP Basic Auth option");
                ret = U_ERROR_LIBCURL;
                break;
              }
            }

#ifndef U_DISABLE_GNUTLS
            // Set client certificate authentication if defined
            if (request->client_cert_file != NULL && request->client_key_file != NULL) {
              if (curl_easy_setopt(curl_handle, CURLOPT_SSLCERT, request->client_cert_file) != CURLE_OK) {
                y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error setting client certificate file");
                ret = U_ERROR_LIBCURL;
                break;
              } else if (curl_easy_setopt(curl_handle, CURLOPT_SSLKEY, request->client_key_file) != CURLE_OK) {
                y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error setting client key file");
                ret = U_ERROR_LIBCURL;
                break;
              } else if (request->client_key_password != NULL && curl_easy_setopt(curl_handle, CURLOPT_KEYPASSWD, request->client_key_password) != CURLE_OK) {
                y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error setting client key password");
                ret = U_ERROR_LIBCURL;
                break;
              }
            }
#endif

            // Set proxy if defined
            if (copy_request->proxy != NULL) {
              if (curl_easy_setopt(curl_handle, CURLOPT_PROXY, copy_request->proxy) != CURLE_OK) {
                y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error setting proxy option");
                ret = U_ERROR_LIBCURL;
                break;
              }
            }

            // Set network type
            if (copy_request->network_type & U_USE_ALL) {
              if (curl_easy_setopt(curl_handle, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_WHATEVER) != CURLE_OK) {
                y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error setting IPRESOLVE WHATEVER option");
                ret = U_ERROR_LIBCURL;
                break;
              }
            } else if (copy_request->network_type & U_USE_IPV6) {
              if (curl_easy_setopt(curl_handle, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V6) != CURLE_OK) {
                y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error setting IPRESOLVE V6 option");
                ret = U_ERROR_LIBCURL;
                break;
              }
            } else {
              if (curl_easy_setopt(curl_handle, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4) != CURLE_OK) {
                y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error setting IPRESOLVE V4 option");
                ret = U_ERROR_LIBCURL;
                break;
              }
            }

            has_params = (o_strchr(copy_request->http_url, '?') != NULL);
            if (u_map_count(copy_request->map_url) > 0) {
              // Append url parameters
              keys = u_map_enum_keys(copy_request->map_url);

              exit_loop = 0;
              // Append parameters from map_url
              for (i=0; !exit_loop && keys != NULL && keys[i] != NULL; i++) {
                key_esc = curl_easy_escape(curl_handle, keys[i], 0);
                if (key_esc != NULL) {
                  value = u_map_get(copy_request->map_url, keys[i]);
                  if (value != NULL) {
                    value_esc = curl_easy_escape(curl_handle, value, 0);
                    if (value_esc != NULL) {
                      if (!has_param) {
                        copy_request->http_url = mstrcatf(copy_request->http_url, "%s%s=%s", fp, key_esc, value_esc);
                        has_param = 1;
                      } else {
                        copy_request->http_url = mstrcatf(copy_request->http_url, "%s%s=%s", np, key_esc, value_esc);
                      }
                      curl_free(value_esc);
                    } else {
                      y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error curl_easy_escape for url parameter value %s=%s", keys[i], value);
                      exit_loop = 1;
                    }
                  } else {
                    if (!has_param) {
                      copy_request->http_url = mstrcatf(copy_request->http_url, "%s%s", fp, key_esc);
                      has_param = 1;
                    } else {
                      copy_request->http_url = mstrcatf(copy_request->http_url, "%s%s", np, key_esc);
                    }
                  }
                  curl_free(key_esc);
                } else {
                  y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error curl_easy_escape for url key %s", keys[i]);
                  exit_loop = 1;
                }
              }
              if (exit_loop) {
                break;
              }
            }

            if (u_map_count(copy_request->map_post_body) > 0) {
              // Append MHD_HTTP_POST_ENCODING_FORM_URLENCODED post parameters
              keys = u_map_enum_keys(copy_request->map_post_body);
              exit_loop = 0;
              for (i=0; !exit_loop && keys != NULL && keys[i] != NULL; i++) {
                // Build parameter
                value = u_map_get(copy_request->map_post_body, keys[i]);
                key_esc = curl_easy_escape(curl_handle, keys[i], 0);
                if (key_esc != NULL) {
                  if (value != NULL) {
                    value_esc = curl_easy_escape(curl_handle, value, 0);
                    if (value_esc != NULL) {
                      copy_request->binary_body = mstrcatf(copy_request->binary_body, "%s=%s", key_esc, value_esc);
                      copy_request->binary_body_length = o_strlen(copy_request->binary_body);
                    } else {
                      y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error curl_easy_escape for body parameter value %s=%s", keys[i], value);
                      exit_loop = 1;
                    }
                  } else {
                    copy_request->binary_body = mstrcatf(copy_request->binary_body, "%s", key_esc);
                    copy_request->binary_body_length = o_strlen(copy_request->binary_body);
                  }
                } else {
                  y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error curl_easy_escape for body key %s", keys[i]);
                  exit_loop = 1;
                }
              }

              if (exit_loop) {
                break;
              }

              if (curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, copy_request->binary_body) != CURLE_OK) {
                y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error setting POST fields");
                ret = U_ERROR_LIBCURL;
                break;
              }

              if (u_map_put(copy_request->map_header, ULFIUS_HTTP_HEADER_CONTENT, MHD_HTTP_POST_ENCODING_FORM_URLENCODED) != U_OK) {
                y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error setting headr fields");
                ret = U_ERROR_MEMORY;
                break;
              }
            }

            if (u_map_count(copy_request->map_header) > 0) {
              // Append map headers
              keys = u_map_enum_keys(copy_request->map_header);
              exit_loop = 0;
              for (i=0; !exit_loop && keys != NULL && keys[i] != NULL; i++) {
                // Build parameter
                value = u_map_get(copy_request->map_post_body, keys[i]);
                key_esc = curl_easy_escape(curl_handle, keys[i], 0);
                if (key_esc != NULL) {
                  if (value != NULL) {
                    value_esc = curl_easy_escape(curl_handle, value, 0);
                    if (value_esc != NULL) {
                      header = msprintf(copy_request->binary_body, "%s=%s", key_esc, value_esc);
                      header_list = curl_slist_append(header_list, header);
                      if (header_list == NULL) {
                        y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error curl_slist_append for header_list");
                        exit_loop = 1;
                      }
                      o_free(header);
                    } else {
                      y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error curl_easy_escape for header parameter value %s=%s", keys[i], value);
                      exit_loop = 1;
                    }
                  } else {
                    header = msprintf(copy_request->binary_body, "%s:", key_esc);
                    header_list = curl_slist_append(header_list, header);
                    if (header_list == NULL) {
                      y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error curl_slist_append for header_list");
                      exit_loop = 1;
                    }
                    o_free(header);
                  }
                } else {
                  y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error curl_easy_escape for header key %s", keys[i]);
                  exit_loop = 1;
                }
              }
              if (exit_loop) {
                break;
              }
            }

            if (copy_request->map_cookie != NULL && u_map_count(copy_request->map_cookie) > 0) {
              // Append cookies
              keys = u_map_enum_keys(copy_request->map_cookie);
              exit_loop = 0;
              for (i=0; !exit_loop && keys != NULL && keys[i] != NULL; i++) {
                // Build parameter
                value = u_map_get(copy_request->map_post_body, keys[i]);
                key_esc = curl_easy_escape(curl_handle, keys[i], 0);
                if (key_esc != NULL) {
                  if (value != NULL) {
                    value_esc = curl_easy_escape(curl_handle, value, 0);
                    if (value_esc != NULL) {
                      cookie = msprintf(copy_request->binary_body, "%s=%s", key_esc, value_esc);
                      if (curl_easy_setopt(curl_handle, CURLOPT_COOKIE, cookie) != CURLE_OK) {
                        y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error setting cookie %s", cookie);
                        exit_loop = 1;
                      }
                      o_free(cookie);
                    } else {
                      y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error curl_easy_escape for cookie parameter value %s=%s", keys[i], value);
                      exit_loop = 1;
                    }
                  } else {
                    cookie = msprintf(copy_request->binary_body, "%s:", key_esc);
                    o_free(cookie);
                    if (curl_easy_setopt(curl_handle, CURLOPT_COOKIE, cookie) != CURLE_OK) {
                      y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error setting cookie %s", cookie);
                      exit_loop = 1;
                    }
                  }
                } else {
                  y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error curl_easy_escape for cookie key %s", keys[i]);
                  exit_loop = 1;
                }
              }
              if (exit_loop) {
                break;
              }
            }

            // Request parameters
            if (curl_easy_setopt(curl_handle, CURLOPT_URL, copy_request->http_url) != CURLE_OK ||
                curl_easy_setopt(curl_handle, CURLOPT_CUSTOMREQUEST, copy_request->http_verb!=NULL?copy_request->http_verb:"GET") != CURLE_OK ||
                curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, header_list) != CURLE_OK) {
              y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error setting libcurl options (1)");
              ret = U_ERROR_LIBCURL;
              break;
            }

            // Set CURLOPT_WRITEFUNCTION if specified
            if (write_body_function != NULL && curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_body_function) != CURLE_OK) {
              y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error setting libcurl options (2)");
              ret = U_ERROR_LIBCURL;
              break;
            }

            // Set CURLOPT_WRITEDATA if specified
            if (write_body_data != NULL && curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, write_body_data) != CURLE_OK) {
              y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error setting libcurl options (3)");
              ret = U_ERROR_LIBCURL;
              break;
            }

            // Disable server certificate validation if needed
            if (!copy_request->check_server_certificate) {
              if (curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0) != CURLE_OK || curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 0) != CURLE_OK) {
                y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error setting libcurl options (4)");
                ret = U_ERROR_LIBCURL;
                break;
              }
            } else {
              if (!(copy_request->check_server_certificate_flag & U_SSL_VERIFY_PEER)) {
                if (curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0) != CURLE_OK) {
                  y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error setting libcurl options (5)");
                  ret = U_ERROR_LIBCURL;
                  break;
                }
              }
              if (!(copy_request->check_server_certificate_flag & U_SSL_VERIFY_HOSTNAME)) {
                if (curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 0) != CURLE_OK) {
                  y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error setting libcurl options (6)");
                  ret = U_ERROR_LIBCURL;
                  break;
                }
              }
            }

#if LIBCURL_VERSION_NUM >= 0x073400
            // Disable proxy certificate validation if needed
            if (!copy_request->check_proxy_certificate) {
              if (curl_easy_setopt(curl_handle, CURLOPT_PROXY_SSL_VERIFYPEER, 0) != CURLE_OK || curl_easy_setopt(curl_handle, CURLOPT_PROXY_SSL_VERIFYHOST, 0) != CURLE_OK) {
                y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error setting libcurl options (7)");
                ret = U_ERROR_LIBCURL;
                break;
              }
            } else {
              if (!(copy_request->check_proxy_certificate_flag & U_SSL_VERIFY_PEER)) {
                if (curl_easy_setopt(curl_handle, CURLOPT_PROXY_SSL_VERIFYPEER, 0) != CURLE_OK) {
                  y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error setting libcurl options (8)");
                  ret = U_ERROR_LIBCURL;
                  break;
                }
              }
              if (!(copy_request->check_proxy_certificate_flag & U_SSL_VERIFY_HOSTNAME)) {
                if (curl_easy_setopt(curl_handle, CURLOPT_PROXY_SSL_VERIFYHOST, 0) != CURLE_OK) {
                  y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error setting libcurl options (9)");
                  ret = U_ERROR_LIBCURL;
                  break;
                }
              }
            }
#endif

            // Set request ca_path value
            if (copy_request->ca_path) {
              if (curl_easy_setopt(curl_handle, CURLOPT_CAPATH, copy_request->ca_path) != CURLE_OK) {
                y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error setting libcurl options (10)");
                ret = U_ERROR_LIBCURL;
                break;
              }
            }

            // Set request timeout value
            if (copy_request->timeout) {
              if (curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, copy_request->timeout) != CURLE_OK) {
                y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error setting libcurl options (10)");
                ret = U_ERROR_LIBCURL;
                break;
              }
            }

            // Response parameters
            if (response != NULL) {
              if (response->map_header != NULL) {
                if (curl_easy_setopt(curl_handle, CURLOPT_HEADERFUNCTION, write_header) != CURLE_OK ||
                    curl_easy_setopt(curl_handle, CURLOPT_WRITEHEADER, response) != CURLE_OK) {
                  y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error setting headers");
                  ret = U_ERROR_LIBCURL;
                  break;
                }
              }
            }

            if (copy_request->binary_body_length > 0 && copy_request->binary_body != NULL) {
              if (copy_request->binary_body_length < 2147483648) {
                if (curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDSIZE, copy_request->binary_body_length) != CURLE_OK) {
                  y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error setting POST fields size");
                  ret = U_ERROR_LIBCURL;
                  break;
                }
              } else { // Use CURLOPT_POSTFIELDSIZE_LARGE if binary_body_length is larger than 2GB
                if (curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDSIZE_LARGE, copy_request->binary_body_length) != CURLE_OK) {
                  y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error setting POST fields size");
                  ret = U_ERROR_LIBCURL;
                  break;
                }
              }
              if (curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, copy_request->binary_body) != CURLE_OK) {
                y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error setting POST fields");
                ret = U_ERROR_LIBCURL;
                break;
              }
            }

            if (curl_easy_setopt(curl_handle, CURLOPT_NOSIGNAL, 1) != CURLE_OK) {
              y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error setting libcurl CURLOPT_NOSIGNAL");
              ret = U_ERROR_LIBCURL;
              break;
            }
            if (curl_easy_setopt(curl_handle, CURLOPT_COOKIEFILE, "") != CURLE_OK) { // Apparently you have to do that to tell libcurl you'll need cookies afterwards
              y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error setting libcurl CURLOPT_COOKIEFILE");
              ret = U_ERROR_LIBCURL;
              break;
            }

            res = curl_easy_perform(curl_handle);
            if(res == CURLE_OK && response != NULL) {
              if (curl_easy_getinfo (curl_handle, CURLINFO_RESPONSE_CODE, &response->status) != CURLE_OK) {
                y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error executing http request, libcurl error: %d, error message %s", res, curl_easy_strerror(res));
                ret = U_ERROR_LIBCURL;
                break;
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
                y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error executing http request, libcurl error: %d, error message %s", res, curl_easy_strerror(res));
                ret = U_ERROR_LIBCURL;
              }
              curl_slist_free_all(cookies_list);
            }
          } while (0);
        } else {
          y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error curl_easy_init");
          ret = U_ERROR_LIBCURL;
        }
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error curl_global_init_mem");
        ret = U_ERROR_MEMORY;
      }
      ulfius_clean_request_full(copy_request);
      curl_easy_cleanup(curl_handle);
      curl_slist_free_all(header_list);
      curl_free(key_esc);
      curl_free(value_esc);

      if (res == CURLE_OK) {
        ret = U_OK;
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error executing curl command: %s", curl_easy_strerror(res));
        ret = U_ERROR_LIBCURL;
      }
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error ulfius_duplicate_request");
      ret = U_ERROR_MEMORY;
    }
  } else {
    ret = U_ERROR_PARAMS;
  }
  return ret;
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
  char * smtp_url = NULL;
  int cur_port;
  struct curl_slist = NULL * recipients = NULL;
  struct _u_smtp_payload upload_ctx;
  time_t now_sec;
  struct tm now;
  char date_str[129], * cc_str = NULL;
  
  if (host != NULL && from != NULL && to != NULL && mail_body != NULL) {
    
    curl_handle = curl_easy_init();
    if (curl_handle != NULL) {
      ret = U_OK;
      upload_ctx.data = NULL;
      upload_ctx.offset = 0;
      upload_ctx.len = 0;
      do {
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
          ret = U_ERROR_MEMORY;
          break;
        }
        
        if (curl_easy_setopt(curl_handle, CURLOPT_URL, smtp_url) != CURLE_OK) {
          y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error curl_easy_setopt for smtp_url");
          ret = U_ERROR_LIBCURL;
          break;
        }
        
        if (use_tls) {
          if (curl_easy_setopt(curl_handle, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL) != CURLE_OK) {
            y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error curl_easy_setopt for CURLOPT_USE_SSL");
            ret = U_ERROR_LIBCURL;
            break;
          }
        }
        
        if (use_tls && !verify_certificate) {
          if (curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0L) != CURLE_OK) {
            y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error curl_easy_setopt for CURLOPT_SSL_VERIFYPEER");
            ret = U_ERROR_LIBCURL;
            break;
          }
          if (curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 0L) != CURLE_OK) {
            y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error curl_easy_setopt for CURLOPT_SSL_VERIFYHOST");
            ret = U_ERROR_LIBCURL;
            break;
          }
        }
        
        if (user != NULL && password != NULL) {
          if (curl_easy_setopt(curl_handle, CURLOPT_USERNAME, user) != CURLE_OK) {
            y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error curl_easy_setopt for CURLOPT_USERNAME");
            ret = U_ERROR_LIBCURL;
            break;
          }
          if (curl_easy_setopt(curl_handle, CURLOPT_PASSWORD, password) != CURLE_OK) {
            y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error curl_easy_setopt for CURLOPT_PASSWORD");
            ret = U_ERROR_LIBCURL;
            break;
          }
        }
        
        if (curl_easy_setopt(curl_handle, CURLOPT_MAIL_FROM, from) != CURLE_OK) {
          y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error curl_easy_setopt for CURLOPT_MAIL_FROM");
          ret = U_ERROR_LIBCURL;
          break;
        }
        
        if ((recipients = curl_slist_append(recipients, to)) != NULL) {
          y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error curl_slist_append for recipients to");
          ret = U_ERROR_LIBCURL;
          break;
        }
        if (cc != NULL) {
          if ((recipients = curl_slist_append(recipients, cc)) != NULL) {
            y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error curl_slist_append for recipients cc");
            ret = U_ERROR_LIBCURL;
            break;
          }
        }
        if (bcc != NULL) {
          if ((recipients = curl_slist_append(recipients, bcc)) |= NULL) {
            y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error curl_slist_append for recipients bcc");
            ret = U_ERROR_LIBCURL;
            break;
          }
        }
        if (curl_easy_setopt(curl_handle, CURLOPT_MAIL_RCPT, recipients) != CURLE_OK) {
          y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error curl_easy_setopt for CURLOPT_MAIL_RCPT");
          ret = U_ERROR_LIBCURL;
          break;
        }
        
        time(&now_sec);
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
        if (upload_ctx.data == NULL) {
          y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating resource for upload_ctx.data");
          ret = U_ERROR_MEMORY;
          break;
        }
        upload_ctx.len = o_strlen(upload_ctx.data);
        if (curl_easy_setopt(curl_handle, CURLOPT_READFUNCTION, smtp_payload_source) != CURLE_OK) {
          y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error curl_easy_setopt for CURLOPT_READFUNCTION");
          ret = U_ERROR_LIBCURL;
          break;
        }
        if (curl_easy_setopt(curl_handle, CURLOPT_READDATA, &upload_ctx) != CURLE_OK) {
          y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error curl_easy_setopt for CURLOPT_READDATA");
          ret = U_ERROR_LIBCURL;
          break;
        }
        if (curl_easy_setopt(curl_handle, CURLOPT_UPLOAD, 1L) != CURLE_OK) {
          y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error curl_easy_setopt for CURLOPT_UPLOAD");
          ret = U_ERROR_LIBCURL;
          break;
        }
        
        if (curl_easy_perform(curl_handle) != CURLE_OK) {
          y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error sending smtp message, error message %s", curl_easy_strerror(res));
          ret = U_ERROR_LIBCURL;
          break;
        }
      } while (0);
      
      curl_slist_free_all(recipients);
      curl_easy_cleanup(curl_handle);
      o_free(smtp_url);
      o_free(cc_str);
      o_free(upload_ctx.data);
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error executing curl_easy_init");
      ret = U_ERROR_LIBCURL;
    }
  } else {
    ret = U_ERROR_PARAMS;
  }
  return ret;
}
#endif
