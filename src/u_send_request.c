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
  size_t len;

  if ((size == 0) || (nmemb == 0) || ((size*nmemb) < 1)) {
    return 0;
  }

  if (upload_ctx->lines_read == MAIL_DATE) {
    time_t now;
    time(&now);
    data = malloc(128*sizeof(char));
    if (data == NULL) {
      return 0;
    } else {
      strftime(data, 128, "Date: %a, %d %b %Y %T %z\r\n", gmtime(&now));
      len = strlen(data);
    }
  } else if (upload_ctx->lines_read == MAIL_TO) {
    len = snprintf(NULL, 0, "To: %s\r\n", upload_ctx->to);
    data = malloc((len+1)*sizeof(char));
    if (data == NULL) {
      return 0;
    } else {
      snprintf(data, (len+1), "To: %s\r\n", upload_ctx->to);
    }
  } else if (upload_ctx->lines_read == MAIL_FROM) {
    len = snprintf(NULL, 0, "From: %s\r\n", upload_ctx->from);
    data = malloc((len+1)*sizeof(char));
    if (data == NULL) {
      return 0;
    } else {
      snprintf(data, (len+1), "From: %s\r\n", upload_ctx->from);
    }
  } else if (upload_ctx->lines_read == MAIL_CC && upload_ctx->cc) {
    len = snprintf(NULL, 0, "Cc: %s\r\n", upload_ctx->cc);
    data = malloc((len+1)*sizeof(char));
    if (data == NULL) {
      return 0;
    } else {
      snprintf(data, (len+1), "Cc: %s\r\n", upload_ctx->cc);
    }
  } else if (upload_ctx->lines_read == MAIL_SUBJECT) {
    len = snprintf(NULL, 0, "Subject: %s\r\n", upload_ctx->subject);
    data = malloc((len+1)*sizeof(char));
    if (data == NULL) {
      return 0;
    } else {
      snprintf(data, (len+1), "Subject: %s\r\n\r\n", upload_ctx->subject);
    }
  } else if (upload_ctx->lines_read == MAIL_DATA) {
    len = snprintf(NULL, 0, "%s\r\n", upload_ctx->data);
    data = malloc((len+1)*sizeof(char));
    if (data == NULL) {
      return 0;
    } else {
      snprintf(data, (len+1), "%s\r\n", upload_ctx->data);
    }
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
  }
 
  return 0;
}

/**
 * Send an email using the specified parameters
 * return U_OK on success
 */
int ulfius_send_smtp_email(const char * host, const int port, const int use_tls, const int verify_certificate, const char * user, const char * password, const char * from, const char * to, const char * cc, const char * bcc, const char * subject, const char * mail_body) {
  CURL * curl_handle;
  CURLcode res = CURLE_OK;
  char * smtp_url;
  int len, cur_port;
  struct curl_slist * recipients = NULL;
  struct upload_status upload_ctx;
 
  
  if (host != NULL && from != NULL && to != NULL && mail_body != NULL) {
    
    curl_handle = curl_easy_init();
    if (curl_handle != NULL) {
      if (port == 0 && !use_tls) {
        cur_port = 25;
      } else if (port == 0 && use_tls) {
        cur_port = 587;
      }
      len = snprintf(NULL, 0, "smtp%s://%s:%d", use_tls?"s":"", host, cur_port);
      smtp_url = malloc((len+1)*sizeof(char));
      if (smtp_url == NULL) {
        return U_ERROR_MEMORY;
      }
      snprintf(smtp_url, (len+1), "smtp%s://%s:%d", use_tls?"s":"", host, cur_port);
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
        return U_ERROR_LIBCURL;
      } else {
        return U_OK;
      }
    } else {
      return U_ERROR_LIBCURL;
    }
  } else {
    return U_ERROR_PARAMS;
  }
}
