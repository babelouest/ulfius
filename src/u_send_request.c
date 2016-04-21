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
    y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for body_data->data");
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
    response->protocol = nstrdup(header);
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
  CURLcode res;
  CURL * curl_handle = NULL;
  struct curl_slist * header_list = NULL;
  char * key_esc, * value_esc, * cookie, * header, * param, * fp = "?", * np = "&";
  const char * value, ** keys, * content_type;
  int i, has_params = 0, len;
  struct _u_request * copy_request = NULL;

  if (request != NULL) {
    // Duplicate the request and work on it
    copy_request = ulfius_duplicate_request(request);
    if (copy_request == NULL) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error ulfius_duplicate_request");
      return U_ERROR_MEMORY;
    }
    curl_handle = curl_easy_init();
    body body_data;
    body_data.size = 0;
    body_data.data = NULL;

    if (copy_request != NULL) {
      // Append header values
      if (copy_request->map_header == NULL) {
        copy_request->map_header = malloc(sizeof(struct _u_map));
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
      
      has_params = (strchr(copy_request->http_url, '?') != NULL);
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
            copy_request->http_url = realloc(copy_request->http_url, strlen(copy_request->http_url) + strlen(param) + 2);
            if (copy_request->http_url == NULL) {
              y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for copy_request->http_url");
              free(param);
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
            copy_request->http_url = realloc(copy_request->http_url, strlen(copy_request->http_url) + strlen(param) + 2);
            if (copy_request->http_url == NULL) {
              y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for copy_request->http_url");
              free(param);
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
          free(param);
          curl_free(key_esc);
          curl_free(value_esc);
        }
      }

      if (copy_request->json_body != NULL) {
        // Append json body parameter
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
            len = strlen(param) + sizeof(char);
            copy_request->binary_body = malloc(len);
            if (copy_request->binary_body == NULL) {
              y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for copy_request->binary_body");
              free(param);
              curl_free(key_esc);
              curl_free(value_esc);
              ulfius_clean_request_full(copy_request);
              curl_slist_free_all(header_list);
              curl_easy_cleanup(curl_handle);
              return U_ERROR_MEMORY;
            }
            strcpy(copy_request->binary_body, "");
            strcat(copy_request->binary_body, param);
            copy_request->binary_body_length = len;
          } else {
            len = (copy_request->binary_body_length + strlen(param) + sizeof(char));
            copy_request->binary_body = realloc(copy_request->binary_body, len);
            if (copy_request->binary_body == NULL) {
              y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for copy_request->binary_body");
              free(param);
              curl_free(key_esc);
              curl_free(value_esc);
              ulfius_clean_request_full(copy_request);
              curl_slist_free_all(header_list);
              curl_easy_cleanup(curl_handle);
              return U_ERROR_MEMORY;
            }
            memcpy(copy_request->binary_body + copy_request->binary_body_length, np, 1);
            memcpy(copy_request->binary_body + copy_request->binary_body_length + 1, param, strlen(param));
            copy_request->binary_body_length = len;
          }
          
          free(param);
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

      if (copy_request->map_header != NULL && u_map_count(copy_request->map_header) > 0) {
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
            free(header);
            ulfius_clean_request_full(copy_request);
            curl_slist_free_all(header_list);
            curl_easy_cleanup(curl_handle);
            return U_ERROR_MEMORY;
          }
          free(header);
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
            free(cookie);
            ulfius_clean_request_full(copy_request);
            curl_slist_free_all(header_list);
            curl_easy_cleanup(curl_handle);
            return U_ERROR_MEMORY;
          }
          free(cookie);
        }
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
            y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error setting headers");
            return U_ERROR_LIBCURL;
          }
        }
      }

      if (copy_request->binary_body != NULL) {
        if (curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, copy_request->binary_body) != CURLE_OK) {
          ulfius_clean_request_full(copy_request);
          curl_slist_free_all(header_list);
          curl_easy_cleanup(curl_handle);
          y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error setting POST fields");
          return U_ERROR_LIBCURL;
        }
        if (copy_request->binary_body_length > 0) {
          if (curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDSIZE, copy_request->binary_body_length) != CURLE_OK) {
            ulfius_clean_request_full(copy_request);
            curl_slist_free_all(header_list);
            curl_easy_cleanup(curl_handle);
            y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error setting POST fields");
            return U_ERROR_LIBCURL;
          }
        }
      }

      if (curl_easy_setopt(curl_handle, CURLOPT_NOSIGNAL, 1) != CURLE_OK) {
        curl_slist_free_all(header_list);
        curl_easy_cleanup(curl_handle);
        ulfius_clean_request_full(copy_request);
        y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error setting libcurl CURLOPT_NOSIGNAL");
        return U_ERROR_LIBCURL;
      }
      res = curl_easy_perform(curl_handle);
      if(res == CURLE_OK && response != NULL) {
        if (curl_easy_getinfo (curl_handle, CURLINFO_RESPONSE_CODE, &response->status) != CURLE_OK) {
          free(body_data.data);
          curl_slist_free_all(header_list);
          curl_easy_cleanup(curl_handle);
          ulfius_clean_request_full(copy_request);
          y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error executing http request, libcurl error: %d, error message %s", res, curl_easy_strerror(res));
          return U_ERROR_LIBCURL;
        }
        if (body_data.data != NULL && body_data.size > 0) {
          response->string_body = nstrdup(body_data.data);
          response->binary_body = malloc(body_data.size);
          if (response->binary_body == NULL || response->string_body == NULL) {
            y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for response->binary_body");
            free(body_data.data);
            free(response->binary_body);
            free(response->string_body);
            curl_slist_free_all(header_list);
            curl_easy_cleanup(curl_handle);
            ulfius_clean_request_full(copy_request);
            return U_ERROR_MEMORY;
          }
          memcpy(response->binary_body, body_data.data, body_data.size);
          response->binary_body_length = body_data.size;
        }
        
        content_type = u_map_get_case(response->map_header, ULFIUS_HTTP_HEADER_CONTENT);
        if (content_type != NULL && 0 == nstrcmp(content_type, ULFIUS_HTTP_ENCODING_JSON)) {
          // Parsing json content
          response->json_body = json_loads(body_data.data, JSON_DECODE_ANY, NULL);
        }
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

/**
 * ulfius_send_smtp_email body fill function and structures
 */
#define MAIL_DATE    0
#define MAIL_TO      1
#define MAIL_FROM    2
#define MAIL_CC      3
#define MAIL_SUBJECT 4
#define MAIL_DATA    5
#define MAIL_END     6

struct upload_status {
  int lines_read;
  char * to;
  char * from;
  char * cc;
  char * subject;
  char * data;
};
 
static size_t smtp_payload_source(void * ptr, size_t size, size_t nmemb, void * userp) {
  struct upload_status *upload_ctx = (struct upload_status *)userp;
  char * data = NULL;
  size_t len = 0;

  if ((size == 0) || (nmemb == 0) || ((size*nmemb) < 1)) {
    return 0;
  }

  if (upload_ctx->lines_read == MAIL_DATE) {
    time_t now;
    time(&now);
    data = malloc(128*sizeof(char));
    if (data == NULL) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for MAIL_DATE\n");
      return 0;
    } else {
      strftime(data, 128, "Date: %a, %d %b %Y %T %z\r\n", gmtime(&now));
      len = strlen(data);
    }
  } else if (upload_ctx->lines_read == MAIL_TO) {
    data = msprintf("To: %s\r\n", upload_ctx->to);
    if (data == NULL) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for MAIL_TO\n");
      return 0;
    }
    len = strlen(data);
  } else if (upload_ctx->lines_read == MAIL_FROM) {
    data = msprintf("From: %s\r\n", upload_ctx->from);
    if (data == NULL) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for MAIL_FROM\n");
      return 0;
    }
    len = strlen(data);
  } else if (upload_ctx->lines_read == MAIL_CC && upload_ctx->cc) {
    data = msprintf("Cc: %s\r\n", upload_ctx->cc);
    if (data == NULL) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for MAIL_CC\n");
      return 0;
    }
    len = strlen(data);
  } else if (upload_ctx->lines_read == MAIL_SUBJECT) {
    data = msprintf("Subject: %s\r\n", upload_ctx->subject);
    if (data == NULL) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for MAIL_SUBJECT\n");
      return 0;
    }
    len = strlen(data);
  } else if (upload_ctx->lines_read == MAIL_DATA) {
    data = msprintf("Content-Type: text/plain; charset=utf-8\r\n\r\n%s\r\n", upload_ctx->data);
    if (data == NULL) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for MAIL_DATA\n");
      return 0;
    }
    len = strlen(data);
  }

  if (upload_ctx->lines_read != MAIL_END) {
    memcpy(ptr, data, (len+1));
    upload_ctx->lines_read++;
    
    // Skip next if it's cc and there is no cc
    if (upload_ctx->lines_read == MAIL_CC && !upload_ctx->cc) {
      upload_ctx->lines_read++;
    }
    free(data);
 
    return len;
  } else if (upload_ctx->lines_read == MAIL_END) {
    return 0;
  }

  y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error setting mail payload, len is %d, lines_read is %d", len, upload_ctx->lines_read);
  return 0;
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
      
      upload_ctx.lines_read = 0;
      upload_ctx.to = (char*)to;
      upload_ctx.from = (char*)from;
      upload_ctx.cc = (char*)cc;
      upload_ctx.subject = (char*)subject;
      upload_ctx.data = (char*)mail_body;
      curl_easy_setopt(curl_handle, CURLOPT_READFUNCTION, smtp_payload_source);
      curl_easy_setopt(curl_handle, CURLOPT_READDATA, &upload_ctx);
      curl_easy_setopt(curl_handle, CURLOPT_UPLOAD, 1L);
      
      res = curl_easy_perform(curl_handle);
      curl_slist_free_all(recipients);
      curl_easy_cleanup(curl_handle);
      free(smtp_url);
      
      if (res != CURLE_OK) {
        y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error sending smtp message, libcurl error: %d, error message %s", res, curl_easy_strerror(res));
        return U_ERROR_LIBCURL;
      } else {
        return U_OK;
      }
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error executing curl_easy_init");
      return U_ERROR_LIBCURL;
    }
  } else {
    return U_ERROR_PARAMS;
  }
}
