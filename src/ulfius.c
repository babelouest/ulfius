/**
 * 
 * Ulfius Framework
 * 
 * REST framework library
 * 
 * ulfius.c: framework functions definitions
 * 
 * Copyright 2015-2020 Nicolas Mora <mail@babelouest.org>
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

#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <pthread.h>
#include "u_private.h"
#include "ulfius.h"

#ifndef U_DISABLE_CURL
#include <curl/curl.h>
#endif

/** Define mock yder functions when yder is disabled **/
#ifdef U_DISABLE_YDER
int y_init_logs(const char * app, const unsigned long init_mode, const unsigned long init_level, const char * init_log_file, const char * message) {
  (void)(app);
  (void)(init_mode);
  (void)(init_level);
  (void)(init_log_file);
  (void)(message);
  return 1;
}

int y_set_logs_callback(void (* y_callback_log_message) (void * cls, const char * app_name, const time_t date, const unsigned long level, const char * message), void * cls, const char * message) {
  (void)(y_callback_log_message);
  (void)(cls);
  (void)(message);
  return 1;
}

void y_log_message(const unsigned long type, const char * message, ...) {
  (void)(type);
  (void)(message);
}

int y_close_logs() {
  return 1;
}
#endif

/**
 * Fill a map with the key/values specified
 */
#if MHD_VERSION >= 0x00097002
static enum MHD_Result ulfius_fill_map_check_utf8(void * cls, enum MHD_ValueKind kind, const char * key, const char * value) {
#else
static int ulfius_fill_map_check_utf8(void * cls, enum MHD_ValueKind kind, const char * key, const char * value) {
#endif
  char * tmp;
  int res;
  UNUSED(kind);
  
  if (cls == NULL || key == NULL) {
    // Invalid parameters
    y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error invalid parameters for ulfius_fill_map_check_utf8");
    return MHD_NO;
  } else if (utf8_check(key) == NULL && (value == NULL || utf8_check(value) == NULL)) {
    if (u_map_get(((struct _u_map *)cls), key) != NULL) {
      // u_map already has a value with this this key, appending value separated with a comma ',')
      tmp = msprintf("%s,%s", u_map_get(((struct _u_map *)cls), key), (value==NULL?"":value));
      res = u_map_put(((struct _u_map *)cls), key, tmp);
      o_free(tmp);
      if (res == U_OK) {
        return MHD_YES;
      } else {
        return MHD_NO;
      }
    } else if (u_map_put(((struct _u_map *)cls), key, (value==NULL?"":value)) == U_OK) {
      return MHD_YES;
    } else {
      return MHD_NO;
    }
  } else {
    return MHD_YES;
  }
}

/**
 * Fill a map with the key/values specified
 */
#if MHD_VERSION >= 0x00097002
static enum MHD_Result ulfius_fill_map(void * cls, enum MHD_ValueKind kind, const char * key, const char * value) {
#else
static int ulfius_fill_map(void * cls, enum MHD_ValueKind kind, const char * key, const char * value) {
#endif
  char * tmp;
  int res;
  UNUSED(kind);
  
  if (cls == NULL || key == NULL) {
    // Invalid parameters
    y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error invalid parameters for ulfius_fill_map");
    return MHD_NO;
  } else if (u_map_get(((struct _u_map *)cls), key) != NULL) {
    // u_map already has a value with this this key, appending value separated with a comma ',')
    tmp = msprintf("%s,%s", u_map_get(((struct _u_map *)cls), key), (value==NULL?"":value));
    res = u_map_put(((struct _u_map *)cls), key, tmp);
    o_free(tmp);
    if (res == U_OK) {
      return MHD_YES;
    } else {
      return MHD_NO;
    }
  } else if (u_map_put(((struct _u_map *)cls), key, (value==NULL?"":value)) == U_OK) {
    return MHD_YES;
  } else {
    return MHD_NO;
  }
}

/**
 * ulfius_is_valid_endpoint
 * return true if the endpoind has valid parameters
 */
#if MHD_VERSION >= 0x00097002
static enum MHD_Result ulfius_is_valid_endpoint(const struct _u_endpoint * endpoint, int to_delete) {
#else
static int ulfius_is_valid_endpoint(const struct _u_endpoint * endpoint, int to_delete) {
#endif
  if (endpoint != NULL) {
    if (ulfius_equals_endpoints(endpoint, ulfius_empty_endpoint())) {
      // Should be the last endpoint of the list to close it
      return 1;
    } else if (endpoint->http_method == NULL) {
      return 0;
    } else if (!to_delete && endpoint->callback_function == NULL) {
      return 0;
    } else if (endpoint->url_prefix == NULL && endpoint->url_format == NULL) {
      return 0;
    } else {
      return 1;
    }
  } else {
    return 0;
  }
}

/**
 * ulfius_validate_endpoint_list
 * return true if endpoint_list has valid parameters
 */
static int ulfius_validate_endpoint_list(const struct _u_endpoint * endpoint_list, int nb_endpoints) {
  int i;
  if (endpoint_list != NULL) {
    for (i=0; i < nb_endpoints; i++) {
      if (i == 0 && ulfius_equals_endpoints(ulfius_empty_endpoint(), &endpoint_list[i])) {
        // One can not have an empty endpoint in the beginning of the list
        y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error, no empty endpoint allowed in the beginning of the endpoint list");
        return U_ERROR_PARAMS;
      } else if (!ulfius_is_valid_endpoint(&endpoint_list[i], 0)) {
        // One must set at least the parameters http_method, url_format and callback_function
        y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error, endpoint at index %d has invalid parameters", i);
        return U_ERROR_PARAMS;
      }
    }
    return U_OK;
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error, no endpoint list");
    return U_ERROR_PARAMS;
  }
}

/**
 * ulfius_validate_instance
 * return true if u_instance has valid parameters
 */
static int ulfius_validate_instance(const struct _u_instance * u_instance) {
  if (u_instance == NULL ||
      u_instance->port <= 0 ||
      u_instance->port >= 65536 ||
      ulfius_validate_endpoint_list(u_instance->endpoint_list, u_instance->nb_endpoints) != U_OK) {
    y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error, instance or has invalid parameters");
    return U_ERROR_PARAMS;
  }
  return U_OK;
}

/**
 * Internal method used to duplicate the full url before it's manipulated and modified by MHD
 */
void * ulfius_uri_logger (void * cls, const char * uri) {
  struct connection_info_struct * con_info = o_malloc (sizeof (struct connection_info_struct));
  UNUSED(cls);
  
  if (con_info != NULL) {
    con_info->callback_first_iteration = 1;
    con_info->u_instance = NULL;
    u_map_init(&con_info->map_url_initial);
    con_info->request = o_malloc(sizeof(struct _u_request));
    if (con_info->request == NULL) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for con_info->request");
      o_free(con_info);
      return NULL;
    }
    
    if (NULL == con_info->request || ulfius_init_request(con_info->request) != U_OK) {
      ulfius_clean_request_full(con_info->request);
      o_free(con_info);
      return NULL;
    }
    con_info->request->http_url = o_strdup(uri);
    if (o_strchr(uri, '?') != NULL) {
      con_info->request->url_path = o_strndup(uri, o_strchr(uri, '?') - uri);
    } else {
      con_info->request->url_path = o_strdup(uri);
    }
    if (con_info->request->http_url == NULL || con_info->request->url_path == NULL) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for con_info->request->http_url or con_info->request->url_path");
      ulfius_clean_request_full(con_info->request);
      o_free(con_info);
      return NULL;
    }
    con_info->max_post_param_size = 0;
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for con_info");
  }
  return con_info;
}

/**
 * ulfius_get_body_from_response
 * Extract the body data from the response if any
 * Copy it in newly allocated response_buffer and set the size in response_buffer_len
 * return U_OK on success
 */
static int ulfius_get_body_from_response(struct _u_response * response, void ** response_buffer, size_t * response_buffer_len) {
  if (response == NULL || response_buffer == NULL || response_buffer_len == NULL) {
    return U_ERROR_PARAMS;
  } else {
    if (response->binary_body != NULL && response->binary_body_length > 0) {
      // The user sent a binary response
      *response_buffer = o_malloc(response->binary_body_length);
      if (*response_buffer == NULL) {
        y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for response_buffer");
        response->status = MHD_HTTP_INTERNAL_SERVER_ERROR;
        response->binary_body = o_strdup(ULFIUS_HTTP_ERROR_BODY);
        response->binary_body_length = o_strlen(ULFIUS_HTTP_ERROR_BODY);
        if (response->binary_body == NULL) {
          y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for response->binary_body");
          return U_ERROR_MEMORY;
        }
      } else {
        memcpy(*response_buffer, response->binary_body, response->binary_body_length);
        *response_buffer_len = response->binary_body_length;
      }
    } else {
      *response_buffer = NULL;
      *response_buffer_len = 0;
    }
    return U_OK;
  }
}

/**
 * mhd_request_completed
 * function used to clean data allocated after a web call is complete
 */
void mhd_request_completed (void *cls, struct MHD_Connection *connection,
                        void **con_cls, enum MHD_RequestTerminationCode toe) {
  struct connection_info_struct *con_info = *con_cls;
  UNUSED(toe);
  UNUSED(connection);
  UNUSED(cls);
  
  if (NULL == con_info) {
    return;
  }
  if (con_info->has_post_processor && con_info->post_processor != NULL) {
    MHD_destroy_post_processor (con_info->post_processor);
  }
  ulfius_clean_request_full(con_info->request);
  u_map_clean(&con_info->map_url_initial);
  con_info->request = NULL;
  o_free(con_info);
  con_info = NULL;
  *con_cls = NULL;
}

/**
 * mhd_iterate_post_data
 * function used to iterate post parameters
 * if a parameter is larger than max_post_param_size, truncate it
 * return MHD_NO on error
 */
#if MHD_VERSION >= 0x00097002
static enum MHD_Result mhd_iterate_post_data (void * coninfo_cls, enum MHD_ValueKind kind, const char * key,
                                              const char * filename, const char * content_type,
                                              const char * transfer_encoding, const char * data, uint64_t off, size_t size) {
#else
static int mhd_iterate_post_data (void * coninfo_cls, enum MHD_ValueKind kind, const char * key,
                                  const char * filename, const char * content_type,
                                  const char * transfer_encoding, const char * data, uint64_t off, size_t size) {
#endif
  struct connection_info_struct * con_info = coninfo_cls;
  size_t cur_size = size;
  char * data_dup, * filename_param;
  UNUSED(kind);
  
  if (filename != NULL && con_info->u_instance != NULL && con_info->u_instance->file_upload_callback != NULL) {
    if (con_info->u_instance->file_upload_callback(con_info->request, key, filename, content_type, transfer_encoding, data, off, size, con_info->u_instance->file_upload_cls) == U_OK) {
      return MHD_YES;
    } else {
      return MHD_NO;
    }
  } else {
    if (con_info->u_instance) {
      if (con_info->u_instance->check_utf8 && (utf8_check(key) != NULL || data == NULL || utf8_check(data) != NULL || (filename != NULL && utf8_check(filename) != NULL))) {
        return MHD_YES;
      } else {
        data_dup = o_strndup(data, size); // Force value to end with a NULL character
        if (con_info->max_post_param_size > 0) {
          if (off > con_info->max_post_param_size) {
            return MHD_YES;
          } else if (off + size > con_info->max_post_param_size) {
            cur_size = con_info->max_post_param_size - off;
          }
        }
      }
    } else {
      return MHD_NO;
    }
    
    if (filename != NULL) {
      filename_param = msprintf("%s_filename", key);
      if (!u_map_has_key((struct _u_map *)con_info->request->map_post_body, filename_param) && u_map_put((struct _u_map *)con_info->request->map_post_body, filename_param, filename) != U_OK) {
        y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error u_map_put filename value");
      }
      o_free(filename_param);
    }
    
    if (cur_size > 0 && data_dup != NULL && u_map_put_binary((struct _u_map *)con_info->request->map_post_body, key, data_dup, off, cur_size + 1) == U_OK) {
      o_free(data_dup);
      return MHD_YES;
    } else {
      o_free(data_dup);
      return MHD_NO;
    }
  }
}

#if MHD_VERSION >= 0x00096100
  #define MHD_CREATE_RESPONSE_FROM_BUFFER_PIMPED(len, buf, flag) MHD_create_response_from_buffer_with_free_callback((len), (buf), &o_free)
#else
  #define MHD_CREATE_RESPONSE_FROM_BUFFER_PIMPED(len, buf, flag) MHD_create_response_from_buffer((len), (buf), (flag))
#endif

/**
 * ulfius_webservice_dispatcher
 * function executed by libmicrohttpd every time an HTTP call is made
 * return MHD_NO on error
 */
#if MHD_VERSION >= 0x00097002
static enum MHD_Result ulfius_webservice_dispatcher (void * cls,
                                                     struct MHD_Connection * connection,
                                                     const char * url,
                                                     const char * method,
                                                     const char * version,
                                                     const char * upload_data,
                                                     size_t * upload_data_size,
                                                     void ** con_cls) {
#else
static int ulfius_webservice_dispatcher (void * cls,
                                         struct MHD_Connection * connection,
                                         const char * url,
                                         const char * method,
                                         const char * version,
                                         const char * upload_data,
                                         size_t * upload_data_size,
                                         void ** con_cls) {
#endif

  struct _u_endpoint * endpoint_list = ((struct _u_instance *)cls)->endpoint_list, ** current_endpoint_list = NULL, * current_endpoint = NULL;
  struct connection_info_struct * con_info = * con_cls;
  int mhd_ret = MHD_NO, callback_ret = U_OK, i, close_loop = 0, inner_error = U_OK, mhd_response_flag;
#ifndef U_DISABLE_WEBSOCKET
  // Websocket variables
  int upgrade_protocol = 0;
  char * protocol = NULL, * extension = NULL, ** extension_list = NULL;
  size_t extension_len, x, y;
#endif

#ifndef U_DISABLE_GNUTLS
  // Client certificate authentication variables
  const union MHD_ConnectionInfo * ci;
  unsigned int listsize;
  const gnutls_datum_t * pcert;
  gnutls_certificate_status_t client_cert_status = 0;
  int ret_cert;
#endif
  char * content_type, * auth_realm = NULL;
  struct _u_response * response = NULL;
  struct sockaddr * so_client;
  
  void * response_buffer = NULL;
  size_t response_buffer_len = 0;
  
  // Response variables
  struct MHD_Response * mhd_response = NULL;
  
  UNUSED(url);
  
  // Prepare for POST or PUT input data
  // Initialize the input maps
  if (con_info == NULL) {
    y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error con_info is NULL");
    return MHD_NO;
  }
  
  if (con_info->u_instance == NULL) {
    con_info->u_instance = (struct _u_instance *)cls;
  }

  if (con_info->callback_first_iteration) {
#ifndef U_DISABLE_GNUTLS
    ci = MHD_get_connection_info (connection, MHD_CONNECTION_INFO_GNUTLS_SESSION);
    if (ci != NULL && ci->tls_session != NULL) {
      if (((struct _u_instance *)cls)->use_client_cert_auth) {
        if (((ret_cert = gnutls_certificate_verify_peers2(ci->tls_session, &client_cert_status)) != 0 && ret_cert != GNUTLS_E_NO_CERTIFICATE_FOUND) || client_cert_status != 0) {
          y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error gnutls_certificate_verify_peers2");
        } else if (!ret_cert) {
          pcert = gnutls_certificate_get_peers(ci->tls_session, &listsize);
          if ((pcert == NULL) || (listsize == 0)) {
            y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Failed to retrieve client certificate chain");
          } else if (gnutls_x509_crt_init(&(con_info->request->client_cert))) {
            y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Failed to initialize client certificate");
          } else if (gnutls_x509_crt_import(con_info->request->client_cert, &pcert[0], GNUTLS_X509_FMT_DER)) {
            y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Failed to import client certificate");
            gnutls_x509_crt_deinit(con_info->request->client_cert);
          }
        }
      }
    }
#endif
    con_info->callback_first_iteration = 0;
    so_client = MHD_get_connection_info (connection, MHD_CONNECTION_INFO_CLIENT_ADDRESS)->client_addr;
    con_info->has_post_processor = 0;
    con_info->max_post_param_size = ((struct _u_instance *)cls)->max_post_param_size;
    con_info->request->http_protocol = o_strdup(version);
    con_info->request->http_verb = o_strdup(method);
    con_info->request->client_address = o_malloc(sizeof(struct sockaddr));
    if (con_info->request->client_address == NULL || con_info->request->http_verb == NULL) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating client_address or http_verb");
      return MHD_NO;
    }
    memcpy(con_info->request->client_address, so_client, sizeof(struct sockaddr));
    if (con_info->u_instance->check_utf8) {
      MHD_get_connection_values (connection, MHD_HEADER_KIND, ulfius_fill_map_check_utf8, con_info->request->map_header);
      MHD_get_connection_values (connection, MHD_GET_ARGUMENT_KIND, ulfius_fill_map_check_utf8, &con_info->map_url_initial);
      MHD_get_connection_values (connection, MHD_COOKIE_KIND, ulfius_fill_map_check_utf8, con_info->request->map_cookie);
    } else {
      MHD_get_connection_values (connection, MHD_HEADER_KIND, ulfius_fill_map, con_info->request->map_header);
      MHD_get_connection_values (connection, MHD_GET_ARGUMENT_KIND, ulfius_fill_map, &con_info->map_url_initial);
      MHD_get_connection_values (connection, MHD_COOKIE_KIND, ulfius_fill_map, con_info->request->map_cookie);
    }
    content_type = (char*)u_map_get_case(con_info->request->map_header, ULFIUS_HTTP_HEADER_CONTENT);
    
    // Set POST Processor if content-type is properly set
    if (content_type != NULL && (0 == o_strncmp(MHD_HTTP_POST_ENCODING_FORM_URLENCODED, content_type, o_strlen(MHD_HTTP_POST_ENCODING_FORM_URLENCODED)) || 
        0 == o_strncmp(MHD_HTTP_POST_ENCODING_MULTIPART_FORMDATA, content_type, o_strlen(MHD_HTTP_POST_ENCODING_MULTIPART_FORMDATA)))) {
      con_info->has_post_processor = 1;
      con_info->post_processor = MHD_create_post_processor (connection, ULFIUS_POSTBUFFERSIZE, mhd_iterate_post_data, (void *) con_info);
      if (NULL == con_info->post_processor) {
        ulfius_clean_request_full(con_info->request);
        con_info->request = NULL;
        y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating post_processor");
        return MHD_NO;
      }
    }
    return MHD_YES;
  } else if (*upload_data_size != 0) {
    size_t body_len = con_info->request->binary_body_length + *upload_data_size, upload_data_size_current = *upload_data_size;
    
    if (((struct _u_instance *)cls)->max_post_body_size > 0 && con_info->request->binary_body_length + *upload_data_size > ((struct _u_instance *)cls)->max_post_body_size) {
      body_len = ((struct _u_instance *)cls)->max_post_body_size;
      upload_data_size_current = ((struct _u_instance *)cls)->max_post_body_size - con_info->request->binary_body_length;
    }
    
    if (body_len >= con_info->request->binary_body_length) {
      con_info->request->binary_body = o_realloc(con_info->request->binary_body, body_len);
      if (con_info->request->binary_body == NULL) {
        y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for con_info->request->binary_body");
        return MHD_NO;
      } else {
        memcpy((char*)con_info->request->binary_body + con_info->request->binary_body_length, upload_data, upload_data_size_current);
        con_info->request->binary_body_length += upload_data_size_current;
        // Handles request binary_body
        const char * content_type = u_map_get_case(con_info->request->map_header, ULFIUS_HTTP_HEADER_CONTENT);
        if (0 == o_strncmp(MHD_HTTP_POST_ENCODING_FORM_URLENCODED, content_type, o_strlen(MHD_HTTP_POST_ENCODING_FORM_URLENCODED)) || 
            0 == o_strncmp(MHD_HTTP_POST_ENCODING_MULTIPART_FORMDATA, content_type, o_strlen(MHD_HTTP_POST_ENCODING_MULTIPART_FORMDATA))) {
          MHD_post_process (con_info->post_processor, upload_data, *upload_data_size);
        }
        *upload_data_size = 0;
        return MHD_YES;
      }
    } else {
      return MHD_YES;
    }
  } else {
    // Check if the endpoint has one or more matches
    current_endpoint_list = ulfius_endpoint_match(method, con_info->request->url_path, endpoint_list);
    
    // Set to default_endpoint if no match
    if ((current_endpoint_list == NULL || current_endpoint_list[0] == NULL) && ((struct _u_instance *)cls)->default_endpoint != NULL && ((struct _u_instance *)cls)->default_endpoint->callback_function != NULL) {
      current_endpoint_list = o_realloc(current_endpoint_list, 2*sizeof(struct _u_endpoint *));
      if (current_endpoint_list != NULL) {
        if ((current_endpoint_list[0] = o_malloc(sizeof(struct _u_endpoint))) != NULL) {
          if (ulfius_copy_endpoint(current_endpoint_list[0], ((struct _u_instance *)cls)->default_endpoint) != U_OK) {
            y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error ulfius_copy_endpoint for current_endpoint_list[0]");
          }
        } else {
          y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for current_endpoint_list[0] of default endpoint");
        }
        current_endpoint_list[1] = NULL;
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for current_endpoint_list of default endpoint");
      }
    }
    
#if MHD_VERSION >= 0x00096100
    mhd_response_flag = ((struct _u_instance *)cls)->mhd_response_copy_data?MHD_RESPMEM_MUST_COPY:MHD_RESPMEM_MUST_FREE;
#else
    mhd_response_flag = MHD_RESPMEM_MUST_FREE;
#endif
    if (current_endpoint_list[0] != NULL) {
      response = o_malloc(sizeof(struct _u_response));
      if (response == NULL) {
        y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating response");
        mhd_ret = MHD_NO;
      } else if (ulfius_init_response(response) != U_OK) {
        o_free(response);
        y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error ulfius_init_response");
        mhd_ret = MHD_NO;
      } else {
        // Add default headers (if any) to the response header maps
        if (((struct _u_instance *)cls)->default_headers != NULL && u_map_count(((struct _u_instance *)cls)->default_headers) > 0) {
          u_map_clean_full(response->map_header);
          response->map_header = u_map_copy(((struct _u_instance *)cls)->default_headers);
        }
        
        // Initialize auth variables
        con_info->request->auth_basic_user = MHD_basic_auth_get_username_password(connection, &con_info->request->auth_basic_password);
        
        for (i=0; current_endpoint_list[i] != NULL && !close_loop; i++) {
          current_endpoint = current_endpoint_list[i];
          u_map_empty(con_info->request->map_url);
          u_map_copy_into(con_info->request->map_url, &con_info->map_url_initial);
          if (ulfius_parse_url(con_info->request->url_path, current_endpoint, con_info->request->map_url, con_info->u_instance->check_utf8) != U_OK) {
            o_free(response);
            y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error parsing url: ", con_info->request->url_path);
            mhd_ret = MHD_NO;
          }
          // Run callback function with the input parameters filled for the current callback
          callback_ret = current_endpoint->callback_function(con_info->request, response, current_endpoint->user_data);
          if (callback_ret != U_CALLBACK_IGNORE) {
            con_info->request->callback_position++;
          }
          if (response->timeout > 0 && MHD_set_connection_option(connection, MHD_CONNECTION_OPTION_TIMEOUT, response->timeout) !=  MHD_YES) {
            y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error setting connection response timeout value");
          }
          if (response->stream_callback != NULL) {
            // Call the stream_callback function to build the response binary_body
            // A stram_callback is always the last one
            mhd_response = MHD_create_response_from_callback(response->stream_size, response->stream_block_size, response->stream_callback, response->stream_user_data, response->stream_callback_free);
            if (mhd_response == NULL) {
              y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error MHD_create_response_from_callback");
              mhd_ret = MHD_NO;
            } else if (ulfius_set_response_header(mhd_response, response->map_header) == -1 || ulfius_set_response_cookie(mhd_response, response) == -1) {
              y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error setting headers or cookies");
              mhd_ret = MHD_NO;
            }
            close_loop = 1;
#ifndef U_DISABLE_WEBSOCKET
          } else if (((struct _websocket_handle *)response->websocket_handle)->websocket_manager_callback != NULL ||
                     ((struct _websocket_handle *)response->websocket_handle)->websocket_incoming_message_callback != NULL) {
            struct _websocket * websocket = o_malloc(sizeof(struct _websocket));
            int websocket_has_error = 0;
            if (websocket != NULL && ulfius_init_websocket(websocket) == U_OK) {
              // if the session is a valid websocket request,
              // Initiate an UPGRADE session,
              // then run the websocket callback functions with initialized data
              if (NULL != o_strcasestr(u_map_get_case(con_info->request->map_header, "upgrade"), U_WEBSOCKET_UPGRADE_VALUE) &&
                  NULL != u_map_get_case(con_info->request->map_header, "Sec-WebSocket-Key") &&
                  NULL != o_strcasestr(u_map_get_case(con_info->request->map_header, "Connection"), "Upgrade") &&
                  0 == o_strcmp(con_info->request->http_protocol, "HTTP/1.1") &&
                  0 == o_strcmp(u_map_get_case(con_info->request->map_header, "Sec-WebSocket-Version"), "13") &&
                  0 == o_strcmp(con_info->request->http_verb, "GET")) {
                int ret_protocol = U_ERROR, ret_extensions = U_OK;
                // Check websocket_protocol and websocket_extensions to match ours
                if (u_map_has_key(con_info->request->map_header, "Sec-WebSocket-Extensions") && (extension_len = pointer_list_size(((struct _websocket_handle *)response->websocket_handle)->websocket_extension_list))) {
                  if (split_string(u_map_get_case(con_info->request->map_header, "Sec-WebSocket-Extensions"), ",", &extension_list)) {
                    for (x=0; extension_list[x]!=NULL; x++) {
                      for (y=0; y<extension_len; y++) {
                        struct _websocket_extension * ws_ext = (struct _websocket_extension *)pointer_list_get_at(((struct _websocket_handle *)response->websocket_handle)->websocket_extension_list, y);
                        if (ws_ext != NULL && !ws_ext->enabled) {
                          if (ws_ext->websocket_extension_server_match != NULL) {
                            if (ws_ext->websocket_extension_server_match(trimwhitespace(extension_list[x]), &ws_ext->extension_client, ws_ext->websocket_extension_server_match_user_data) == U_OK) {
                              ws_ext->enabled = 1;
                              if (extension != NULL) {
                                extension = mstrcatf(extension, ", %s", ws_ext->extension_client);
                              } else {
                                extension = o_strdup(ws_ext->extension_client);
                              }
                              break;
                            }
                          } else {
                            if (0 == o_strcmp(extension_list[x], ws_ext->extension_server)) {
                              ws_ext->extension_client = o_strdup(extension_list[x]);
                              ws_ext->enabled = 1;
                              if (extension != NULL) {
                                extension = mstrcatf(extension, ", %s", ws_ext->extension_client);
                              } else {
                                extension = o_strdup(ws_ext->extension_client);
                              }
                              break;
                            }
                          }
                        }
                      }
                    }
                  } else {
                    y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error split_string Sec-WebSocket-Extensions");
                  }
                  free_string_array(extension_list);
                } else {
                  ret_extensions = ulfius_check_list_match(u_map_get_case(con_info->request->map_header, "Sec-WebSocket-Extensions"), ((struct _websocket_handle *)response->websocket_handle)->websocket_extensions, ",", &extension);
                }
                if (ret_extensions == U_OK && 
                    (ret_protocol = ulfius_check_first_match(u_map_get_case(con_info->request->map_header, "Sec-WebSocket-Protocol"), ((struct _websocket_handle *)response->websocket_handle)->websocket_protocol, ",", &protocol)) == U_OK) {
                  char websocket_accept[32] = {0};
                  if (ulfius_generate_handshake_answer(u_map_get_case(con_info->request->map_header, "Sec-WebSocket-Key"), websocket_accept)) {
                    websocket->request = ulfius_duplicate_request(con_info->request);
                    if (websocket->request != NULL) {
                      websocket->instance = (struct _u_instance *)cls;
                      websocket->websocket_manager_callback = ((struct _websocket_handle *)response->websocket_handle)->websocket_manager_callback;
                      websocket->websocket_manager_user_data = ((struct _websocket_handle *)response->websocket_handle)->websocket_manager_user_data;
                      websocket->websocket_incoming_message_callback = ((struct _websocket_handle *)response->websocket_handle)->websocket_incoming_message_callback;
                      websocket->websocket_incoming_user_data = ((struct _websocket_handle *)response->websocket_handle)->websocket_incoming_user_data;
                      websocket->websocket_onclose_callback = ((struct _websocket_handle *)response->websocket_handle)->websocket_onclose_callback;
                      websocket->websocket_onclose_user_data = ((struct _websocket_handle *)response->websocket_handle)->websocket_onclose_user_data;
                      websocket->websocket_manager->websocket_extension_list = ((struct _websocket_handle *)response->websocket_handle)->websocket_extension_list;
                      ((struct _websocket_handle *)response->websocket_handle)->websocket_extension_list = NULL;
                      mhd_response = MHD_create_response_for_upgrade(ulfius_start_websocket_cb, websocket);
                      if (mhd_response == NULL) {
                        y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error MHD_create_response_for_upgrade");
                        mhd_ret = MHD_NO;
                      } else {
                        MHD_add_response_header (mhd_response,
                                                 MHD_HTTP_HEADER_UPGRADE,
                                                 U_WEBSOCKET_UPGRADE_VALUE);
                        MHD_add_response_header (mhd_response,
                                                 "Sec-WebSocket-Accept",
                                                 websocket_accept);
                        MHD_add_response_header (mhd_response,
                                                 "Sec-WebSocket-Protocol",
                                                 protocol);
                        MHD_add_response_header (mhd_response,
                                                 "Sec-WebSocket-Extensions",
                                                 extension);
                        if (ulfius_set_response_header(mhd_response, response->map_header) == -1 || ulfius_set_response_cookie(mhd_response, response) == -1) {
                          y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error setting headers or cookies");
                          mhd_ret = MHD_NO;
                          websocket_has_error = 1;
                        } else {
                          ulfius_instance_add_websocket_active((struct _u_instance *)cls, websocket);
                          upgrade_protocol = 1;
                        }
                      }
                    } else {
                      // Error building struct _websocket, sending error 500
                      response->status = MHD_HTTP_INTERNAL_SERVER_ERROR;
                      response_buffer = o_strdup(ULFIUS_HTTP_ERROR_BODY);
                      if (response_buffer == NULL) {
                        y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for websocket->request");
                        mhd_ret = MHD_NO;
                      } else {
                        response_buffer_len = o_strlen(ULFIUS_HTTP_ERROR_BODY);
                        mhd_response = MHD_CREATE_RESPONSE_FROM_BUFFER_PIMPED (response_buffer_len, response_buffer, mhd_response_flag );
                      }
                      websocket_has_error = 1;
                    }
                  } else {
                    // Error building ulfius_generate_handshake_answer, sending error 500
                    response->status = MHD_HTTP_INTERNAL_SERVER_ERROR;
                    response_buffer = o_strdup(ULFIUS_HTTP_ERROR_BODY);
                    if (response_buffer == NULL) {
                      y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for response_buffer");
                      mhd_ret = MHD_NO;
                    } else {
                      response_buffer_len = o_strlen(ULFIUS_HTTP_ERROR_BODY);
                      mhd_response = MHD_CREATE_RESPONSE_FROM_BUFFER_PIMPED (response_buffer_len, response_buffer, mhd_response_flag );
                    }
                    websocket_has_error = 1;
                  }
                } else {
                  response->status = MHD_HTTP_BAD_REQUEST;
                  response_buffer = msprintf("%s%s", (ret_protocol!=U_OK?"Error validating protocol\n":""), (ret_extensions!=U_OK?"Error validating extensions":""));
                  y_log_message(Y_LOG_LEVEL_DEBUG, "Ulfius - Error websocket connection: %s", response_buffer);
                  mhd_response = MHD_CREATE_RESPONSE_FROM_BUFFER_PIMPED (o_strlen(response_buffer), response_buffer, mhd_response_flag );
                  websocket_has_error = 1;
                }
                o_free(protocol);
                o_free(extension);
              } else {
                response_buffer = msprintf("%s%s%s%s%s%s",
                                           o_strcasestr(u_map_get_case(con_info->request->map_header, "upgrade"), U_WEBSOCKET_UPGRADE_VALUE)==NULL?"No Upgrade websocket header\n":"",
                                           o_strcasestr(u_map_get_case(con_info->request->map_header, "Connection"), "Upgrade")==NULL?"No Connection Upgrade header\n":"",
                                           u_map_get_case(con_info->request->map_header, "Sec-WebSocket-Key")==NULL?"No Sec-WebSocket-Key header\n":"",
                                           o_strcmp(con_info->request->http_protocol, "HTTP/1.1")!=0?"Wrong HTTP Protocol":"",
                                           o_strcmp(u_map_get_case(con_info->request->map_header, "Sec-WebSocket-Version"), "13")!=0?"Wrong websocket version\n":"",
                                           o_strcmp(con_info->request->http_verb, "GET")!=0?"Method is not GET":"");
                response->status = MHD_HTTP_BAD_REQUEST;
                y_log_message(Y_LOG_LEVEL_DEBUG, "Ulfius - Error websocket connection: %s", response_buffer);
                mhd_response = MHD_CREATE_RESPONSE_FROM_BUFFER_PIMPED (o_strlen(response_buffer), response_buffer, mhd_response_flag );
                websocket_has_error = 1;
              }
            } else {
              // Error building struct _websocket, sending error 500
              response->status = MHD_HTTP_INTERNAL_SERVER_ERROR;
              response_buffer = o_strdup(ULFIUS_HTTP_ERROR_BODY);
              if (response_buffer == NULL) {
                y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for response_buffer");
                mhd_ret = MHD_NO;
              } else {
                response_buffer_len = o_strlen(ULFIUS_HTTP_ERROR_BODY);
                mhd_response = MHD_CREATE_RESPONSE_FROM_BUFFER_PIMPED (response_buffer_len, response_buffer, mhd_response_flag );
              }
              websocket_has_error = 1;
            }
            close_loop = 1;
            if (websocket_has_error) {
              if (((struct _websocket_handle *)response->websocket_handle)->websocket_onclose_callback != NULL) {
                ((struct _websocket_handle *)response->websocket_handle)->websocket_onclose_callback(con_info->request, websocket->websocket_manager!=NULL?websocket->websocket_manager:NULL, ((struct _websocket_handle *)response->websocket_handle)->websocket_onclose_user_data);
              }
              ulfius_clear_websocket(websocket);
            }
#endif
          } else {
            if ((callback_ret == U_CALLBACK_CONTINUE || callback_ret == U_CALLBACK_IGNORE) && current_endpoint_list[i+1] == NULL) {
              // If callback_ret is U_CALLBACK_CONTINUE or U_CALLBACK_IGNORE but callback function is the last one on the list
              callback_ret = U_CALLBACK_COMPLETE;
            }
            // Test callback_ret to know what to do
            switch (callback_ret) {
              case U_CALLBACK_CONTINUE:
              case U_CALLBACK_IGNORE:
                break;
              case U_CALLBACK_COMPLETE:
                close_loop = 1;
                if (ulfius_get_body_from_response(response, &response_buffer, &response_buffer_len) == U_OK) {
                  // Build the response binary_body
                  mhd_response = MHD_CREATE_RESPONSE_FROM_BUFFER_PIMPED (response_buffer_len, response_buffer, mhd_response_flag );
                  if (mhd_response == NULL) {
                    y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error MHD_create_response_from_buffer");
                    mhd_ret = MHD_NO;
                  } else if (ulfius_set_response_header(mhd_response, response->map_header) == -1 || ulfius_set_response_cookie(mhd_response, response) == -1) {
                    y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error setting headers or cookies");
                    mhd_ret = MHD_NO;
                  }
                } else {
                  // Error building response, sending error 500
                  response->status = MHD_HTTP_INTERNAL_SERVER_ERROR;
                  response_buffer = o_strdup(ULFIUS_HTTP_ERROR_BODY);
                  if (response_buffer == NULL) {
                    y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for response_buffer");
                    mhd_ret = MHD_NO;
                  } else {
                    response_buffer_len = o_strlen(ULFIUS_HTTP_ERROR_BODY);
                    mhd_response = MHD_CREATE_RESPONSE_FROM_BUFFER_PIMPED (response_buffer_len, response_buffer, mhd_response_flag );
                  }
                }
                break;
              case U_CALLBACK_UNAUTHORIZED:
                close_loop = 1;
                // Wrong credentials, send status 401 and realm value if set
                if (ulfius_get_body_from_response(response, &response_buffer, &response_buffer_len) == U_OK) {
                  mhd_response = MHD_CREATE_RESPONSE_FROM_BUFFER_PIMPED (response_buffer_len, response_buffer, mhd_response_flag );
                  if (ulfius_set_response_header(mhd_response, response->map_header) == -1 || ulfius_set_response_cookie(mhd_response, response) == -1) {
                    inner_error = U_ERROR_PARAMS;
                    y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error setting headers or cookies");
                    response->status = MHD_HTTP_INTERNAL_SERVER_ERROR;
                    response->binary_body = o_strdup(ULFIUS_HTTP_ERROR_BODY);
                    response->binary_body_length = o_strlen(ULFIUS_HTTP_ERROR_BODY);
                    if (response->binary_body == NULL) {
                      inner_error = U_ERROR_MEMORY;
                      y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for response->binary_body");
                      mhd_ret = MHD_NO;
                    }
                  } else {
                    inner_error = U_CALLBACK_UNAUTHORIZED;
                  }
                } else {
                  // Error building response, sending error 500
                  response_buffer = o_strdup(ULFIUS_HTTP_ERROR_BODY);
                  if (response_buffer == NULL) {
                    inner_error = U_ERROR_MEMORY;
                    y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for response_buffer");
                    mhd_ret = MHD_NO;
                  } else {
                    response_buffer_len = o_strlen(ULFIUS_HTTP_ERROR_BODY);
                    mhd_response = MHD_CREATE_RESPONSE_FROM_BUFFER_PIMPED (response_buffer_len, response_buffer, mhd_response_flag );
                    inner_error = U_CALLBACK_UNAUTHORIZED;
                  }
                }
                if (response->auth_realm != NULL) {
                  auth_realm = response->auth_realm;
                } else if (((struct _u_instance *)cls)->default_auth_realm != NULL) {
                  auth_realm = ((struct _u_instance *)cls)->default_auth_realm;
                }
                break;
              case U_CALLBACK_ERROR:
              default:
                close_loop = 1;
                response->status = MHD_HTTP_INTERNAL_SERVER_ERROR;
                response_buffer = o_strdup(ULFIUS_HTTP_ERROR_BODY);
                if (response_buffer == NULL) {
                  y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for response_buffer");
                  mhd_ret = MHD_NO;
                } else {
                  response_buffer_len = o_strlen(ULFIUS_HTTP_ERROR_BODY);
                  mhd_response = MHD_CREATE_RESPONSE_FROM_BUFFER_PIMPED (response_buffer_len, response_buffer, mhd_response_flag );
                }
                break;
            }
          }
        }
        
        if (!con_info->request->callback_position && ((struct _u_instance *)cls)->default_endpoint != NULL && ((struct _u_instance *)cls)->default_endpoint->callback_function != NULL) {
          callback_ret = ((struct _u_instance *)cls)->default_endpoint->callback_function(con_info->request, response, ((struct _u_instance *)cls)->default_endpoint->user_data);
          // Test callback_ret to know what to do
          switch (callback_ret) {
            case U_CALLBACK_UNAUTHORIZED:
              // Wrong credentials, send status 401 and realm value if set
              if (ulfius_get_body_from_response(response, &response_buffer, &response_buffer_len) == U_OK) {
                mhd_response = MHD_CREATE_RESPONSE_FROM_BUFFER_PIMPED (response_buffer_len, response_buffer, mhd_response_flag );
                if (ulfius_set_response_header(mhd_response, response->map_header) == -1 || ulfius_set_response_cookie(mhd_response, response) == -1) {
                  inner_error = U_ERROR_PARAMS;
                  y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error setting headers or cookies");
                  response->status = MHD_HTTP_INTERNAL_SERVER_ERROR;
                  response->binary_body = o_strdup(ULFIUS_HTTP_ERROR_BODY);
                  response->binary_body_length = o_strlen(ULFIUS_HTTP_ERROR_BODY);
                  if (response->binary_body == NULL) {
                    inner_error = U_ERROR_MEMORY;
                    y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for response->binary_body");
                    mhd_ret = MHD_NO;
                  }
                } else {
                  inner_error = U_CALLBACK_UNAUTHORIZED;
                }
              } else {
                // Error building response, sending error 500
                response_buffer = o_strdup(ULFIUS_HTTP_ERROR_BODY);
                if (response_buffer == NULL) {
                  inner_error = U_ERROR_MEMORY;
                  y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for response_buffer");
                  mhd_ret = MHD_NO;
                } else {
                  response_buffer_len = o_strlen(ULFIUS_HTTP_ERROR_BODY);
                  mhd_response = MHD_CREATE_RESPONSE_FROM_BUFFER_PIMPED (response_buffer_len, response_buffer, mhd_response_flag );
                  inner_error = U_CALLBACK_UNAUTHORIZED;
                }
              }
              if (response->auth_realm != NULL) {
                auth_realm = response->auth_realm;
              } else if (((struct _u_instance *)cls)->default_auth_realm != NULL) {
                auth_realm = ((struct _u_instance *)cls)->default_auth_realm;
              }
              break;
            case U_CALLBACK_ERROR:
              close_loop = 1;
              response->status = MHD_HTTP_INTERNAL_SERVER_ERROR;
              response_buffer = o_strdup(ULFIUS_HTTP_ERROR_BODY);
              if (response_buffer == NULL) {
                y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for response_buffer");
                mhd_ret = MHD_NO;
              } else {
                response_buffer_len = o_strlen(ULFIUS_HTTP_ERROR_BODY);
                mhd_response = MHD_CREATE_RESPONSE_FROM_BUFFER_PIMPED (response_buffer_len, response_buffer, mhd_response_flag );
              }
              break;
            case U_CALLBACK_CONTINUE:
            case U_CALLBACK_IGNORE:
            case U_CALLBACK_COMPLETE:
              if (ulfius_get_body_from_response(response, &response_buffer, &response_buffer_len) == U_OK) {
                // Build the response binary_body
                mhd_response = MHD_CREATE_RESPONSE_FROM_BUFFER_PIMPED (response_buffer_len, response_buffer, mhd_response_flag );
                if (mhd_response == NULL) {
                  y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error MHD_create_response_from_buffer");
                  mhd_ret = MHD_NO;
                } else if (ulfius_set_response_header(mhd_response, response->map_header) == -1 || ulfius_set_response_cookie(mhd_response, response) == -1) {
                  y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error setting headers or cookies");
                  mhd_ret = MHD_NO;
                }
              } else {
                // Error building response, sending error 500
                response->status = MHD_HTTP_INTERNAL_SERVER_ERROR;
                response_buffer = o_strdup(ULFIUS_HTTP_ERROR_BODY);
                if (response_buffer == NULL) {
                  y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for response_buffer");
                  mhd_ret = MHD_NO;
                } else {
                  response_buffer_len = o_strlen(ULFIUS_HTTP_ERROR_BODY);
                  mhd_response = MHD_CREATE_RESPONSE_FROM_BUFFER_PIMPED (response_buffer_len, response_buffer, mhd_response_flag );
                }
              }
              break;
            default:
              break;
          }
        }
        
        if (mhd_response != NULL) {
          if (auth_realm != NULL && inner_error == U_CALLBACK_UNAUTHORIZED) {
            mhd_ret = MHD_queue_basic_auth_fail_response (connection, auth_realm, mhd_response);
          } else if (inner_error == U_CALLBACK_UNAUTHORIZED) {
            mhd_ret = MHD_queue_response (connection, MHD_HTTP_UNAUTHORIZED, mhd_response);
#ifndef U_DISABLE_WEBSOCKET
          } else if (upgrade_protocol) {
            mhd_ret = MHD_queue_response (connection,
                                          MHD_HTTP_SWITCHING_PROTOCOLS,
                                          mhd_response);
#endif
          } else {
            mhd_ret = MHD_queue_response (connection, response->status, mhd_response);
          }
          MHD_destroy_response (mhd_response);
          // Free Response parameters
          ulfius_clean_response_full(response);
          response = NULL;
        }
      }
    } else {
      response_buffer = o_strdup(ULFIUS_HTTP_NOT_FOUND_BODY);
      if (response_buffer == NULL) {
        y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for response_buffer");
        mhd_ret = MHD_NO;
      } else {
        response_buffer_len = o_strlen(ULFIUS_HTTP_NOT_FOUND_BODY);
        mhd_response = MHD_CREATE_RESPONSE_FROM_BUFFER_PIMPED (response_buffer_len, response_buffer, mhd_response_flag );
        mhd_ret = MHD_queue_response (connection, MHD_HTTP_NOT_FOUND, mhd_response);
        MHD_destroy_response (mhd_response);
      }
    }
#if MHD_VERSION < 0x00096100
    if (mhd_response_flag == MHD_RESPMEM_MUST_COPY) {
      o_free(response_buffer);
    }
#else
    (void)mhd_response_flag;
#endif
    for (i=0; current_endpoint_list[i] != NULL; i++) {
      ulfius_clean_endpoint(current_endpoint_list[i]);
      o_free(current_endpoint_list[i]);
    }
    o_free(current_endpoint_list);
    return mhd_ret;
  }
}

/**
 * ulfius_run_mhd_daemon
 * Starts a mhd daemon for the specified instance
 * return a pointer to the mhd_daemon on success, NULL on error
 * 
 */
static struct MHD_Daemon * ulfius_run_mhd_daemon(struct _u_instance * u_instance, const char * key_pem, const char * cert_pem, const char * root_ca_perm) {
  unsigned int mhd_flags = MHD_USE_THREAD_PER_CONNECTION;
  int index;

#ifdef DEBUG
  mhd_flags |= MHD_USE_DEBUG;
#endif
#if MHD_VERSION >= 0x00095300
  mhd_flags |= MHD_USE_INTERNAL_POLLING_THREAD;
#endif
#ifndef U_DISABLE_WEBSOCKET
  mhd_flags |= MHD_ALLOW_UPGRADE;
#endif
  
  if (u_instance->mhd_daemon == NULL) {
    struct MHD_OptionItem mhd_ops[8];
    
    // Default options
    mhd_ops[0].option = MHD_OPTION_NOTIFY_COMPLETED;
    mhd_ops[0].value = (intptr_t)mhd_request_completed;
    mhd_ops[0].ptr_value = NULL;
    
#if MHD_VERSION >= 0x00095208
    // If bind_address6 is specified, listen only to IPV6 addresses
    if (u_instance->bind_address6 != NULL) {
      mhd_ops[1].option = MHD_OPTION_SOCK_ADDR;
      mhd_ops[1].value = 0;
      mhd_ops[1].ptr_value = (void *)u_instance->bind_address6;
      mhd_flags |= MHD_USE_IPv6;
    } else {
      mhd_ops[1].option = MHD_OPTION_SOCK_ADDR;
      mhd_ops[1].value = 0;
      mhd_ops[1].ptr_value = (void *)u_instance->bind_address;
      // Default network stack is listening to IPV4 only
      if ((u_instance->network_type & U_USE_IPV4) && (u_instance->network_type & U_USE_IPV6)) {
        // If u_instance->network_type & U_USE_ALL, listen to IPV4 and IPV6 addresses
        mhd_flags |= MHD_USE_DUAL_STACK;
      } else if (u_instance->network_type & U_USE_IPV6) {
        // If u_instance->network_type & U_USE_IPV6, listen to IPV6 addresses only
        mhd_flags |= MHD_USE_IPv6;
      }
    }
#else
    mhd_ops[1].option = MHD_OPTION_SOCK_ADDR;
    mhd_ops[1].value = 0;
    mhd_ops[1].ptr_value = (void *)u_instance->bind_address;
#endif
    
    mhd_ops[2].option = MHD_OPTION_URI_LOG_CALLBACK;
    mhd_ops[2].value = (intptr_t)ulfius_uri_logger;
    mhd_ops[2].ptr_value = NULL;
    
    index = 3;

    if (key_pem != NULL && cert_pem != NULL) {
      // HTTPS parameters
      mhd_flags |= MHD_USE_SSL;
      mhd_ops[index].option = MHD_OPTION_HTTPS_MEM_KEY;
      mhd_ops[index].value = 0;
      mhd_ops[index].ptr_value = (void*)key_pem;
     
      mhd_ops[index + 1].option = MHD_OPTION_HTTPS_MEM_CERT;
      mhd_ops[index + 1].value = 0;
      mhd_ops[index + 1].ptr_value = (void*)cert_pem;
      
      index += 2;

      if (root_ca_perm != NULL) {
        mhd_ops[index].option = MHD_OPTION_HTTPS_MEM_TRUST;
        mhd_ops[index].value = 0;
        mhd_ops[index].ptr_value = (void *)root_ca_perm;

        index++;
      }
    }
    if (u_instance->timeout > 0) {
      mhd_ops[index].option = MHD_OPTION_CONNECTION_TIMEOUT;
      mhd_ops[index].value = u_instance->timeout;
      mhd_ops[index].ptr_value = NULL;
      
      index++;
    }

    mhd_ops[index].option = MHD_OPTION_END;
    mhd_ops[index].value = 0;
    mhd_ops[index].ptr_value = NULL;

    return MHD_start_daemon (
      mhd_flags, u_instance->port, NULL, NULL, &ulfius_webservice_dispatcher, (void *)u_instance, 
      MHD_OPTION_ARRAY, mhd_ops,
      MHD_OPTION_END
    );
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error, instance already started");
    return NULL;
  }
}

/**
 * ulfius_start_framework
 * Initializes the framework and run the webservice based on the parameters given
 * return true if no error
 * 
 * u_instance:    pointer to a struct _u_instance that describe its port and bind address
 * return U_OK on success
 */
int ulfius_start_framework(struct _u_instance * u_instance) {
#ifndef U_DISABLE_GNUTLS
  return ulfius_start_secure_ca_trust_framework(u_instance, NULL, NULL, NULL);
#else
  return ulfius_start_secure_framework(u_instance, NULL, NULL);
#endif
}

/**
 * ulfius_start_secure_framework
 * Initializes the framework and run the webservice based on the parameters given using an HTTPS connection
 * 
 * u_instance:    pointer to a struct _u_instance that describe its port and bind address
 * key_pem:       private key for the server
 * cert_pem:      server certificate
 * return U_OK on success
 */
int ulfius_start_secure_framework(struct _u_instance * u_instance, const char * key_pem, const char * cert_pem) {
#ifndef U_DISABLE_GNUTLS
  return ulfius_start_secure_ca_trust_framework(u_instance, key_pem, cert_pem, NULL);
#else
#ifndef U_DISABLE_JANSSON
  o_malloc_t malloc_fn;
  o_free_t free_fn;
  
  o_get_alloc_funcs(&malloc_fn, NULL, &free_fn);
  json_set_alloc_funcs((json_malloc_t)malloc_fn, (json_free_t)free_fn);
#endif
  // Check parameters and validate u_instance and endpoint_list that there is no mistake
  if (u_instance == NULL) {
    y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - ulfius_start_secure_framework - Error, u_instance is NULL");
    return U_ERROR_PARAMS;
  } else if ((key_pem == NULL && cert_pem != NULL) || (key_pem != NULL && cert_pem == NULL)) {
    y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - ulfius_start_secure_framework - Error, you must specify key_pem and cert_pem");
    return U_ERROR_PARAMS;
  }
  if (ulfius_validate_instance(u_instance) == U_OK) {
    u_instance->mhd_daemon = ulfius_run_mhd_daemon(u_instance, key_pem, cert_pem, NULL);
    
    if (u_instance->mhd_daemon == NULL) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error MHD_start_daemon, aborting");
      u_instance->status = U_STATUS_ERROR;
      return U_ERROR_LIBMHD;
    } else {
      u_instance->status = U_STATUS_RUNNING;
      return U_OK;
    }
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - ulfius_start_secure_framework - error input parameters");
    return U_ERROR_PARAMS;
  }
#endif
}

#ifndef U_DISABLE_GNUTLS
/**
 * ulfius_start_secure_ca_trust_framework
 * Initializes the framework and run the webservice based on the parameters given using an HTTPS connection
 * And using a root server to authenticate client connections
 * 
 * u_instance:    pointer to a struct _u_instance that describe its port and bind address
 * key_pem:       private key for the server
 * cert_pem:      server certificate
 * root_ca_pem:   client root CA you're willing to trust for this instance
 * return U_OK on success
 */
int ulfius_start_secure_ca_trust_framework(struct _u_instance * u_instance, const char * key_pem, const char * cert_pem, const char * root_ca_pem) {
#ifndef U_DISABLE_JANSSON
  o_malloc_t malloc_fn;
  o_free_t free_fn;
  
  o_get_alloc_funcs(&malloc_fn, NULL, &free_fn);
  json_set_alloc_funcs((json_malloc_t)malloc_fn, (json_free_t)free_fn);
#endif
  // Check parameters and validate u_instance and endpoint_list that there is no mistake
  if (u_instance == NULL) {
    y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - ulfius_start_secure_ca_trust_framework - Error, u_instance is NULL");
    return U_ERROR_PARAMS;
  } else if ((key_pem == NULL && cert_pem != NULL) || (key_pem != NULL && cert_pem == NULL)) {
    y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - ulfius_start_secure_ca_trust_framework - Error, you must specify key_pem and cert_pem");
    return U_ERROR_PARAMS;
  } else if (root_ca_pem != NULL && (key_pem == NULL || cert_pem == NULL)) {
    y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - ulfius_start_secure_ca_trust_framework - Error, you must specify key_pem and cert_pem in addition to root_ca_pem");
    return U_ERROR_PARAMS;
  }
  if (root_ca_pem != NULL) {
    u_instance->use_client_cert_auth = 1;
  } else {
    u_instance->use_client_cert_auth = 0;
  }
  if (ulfius_validate_instance(u_instance) == U_OK) {
    u_instance->mhd_daemon = ulfius_run_mhd_daemon(u_instance, key_pem, cert_pem, root_ca_pem);
    
    if (u_instance->mhd_daemon == NULL) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error MHD_start_daemon, aborting");
      u_instance->status = U_STATUS_ERROR;
      return U_ERROR_LIBMHD;
    } else {
      u_instance->status = U_STATUS_RUNNING;
      return U_OK;
    }
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - ulfius_start_secure_ca_trust_framework - error input parameters");
    return U_ERROR_PARAMS;
  }
}
#endif

/**
 * ulfius_start_framework_with_mhd_options
 * Initializes the framework and run the webservice based on the specified MHD options table given in parameter
 * Read https://www.gnu.org/software/libmicrohttpd/tutorial.html for more information
 * This is for user who know what they do, Ulfius' options used in other `ulfius_start_framework_*`
 * are good for most use cases where you need a multi-threaded HTTP webservice
 * Some struct MHD_OptionItem may cause unexpected problems with Ulfius API
 * If you find an unresolved issue with this function you can open an issue in GitHub
 * But some issues may not be solvable if fixing them would break Ulfius API or philosophy
 * i.e.: you're on your own
 * @param u_instance pointer to a struct _u_instance that describe its port and bind address
 * @param mhd_flags OR-ed combination of MHD_FLAG values
 * @param mhd_ops struct MHD_OptionItem * options table
 * - MUST contain an option with the fllowing value: {.option = MHD_OPTION_NOTIFY_COMPLETED; .value = (intptr_t)mhd_request_completed; .ptr_value = NULL;}
 * - MUST contain an option with the fllowing value: {.option = MHD_OPTION_URI_LOG_CALLBACK; .value = (intptr_t)ulfius_uri_logger; .ptr_value = NULL;}
 * - MUST end with a terminal struct MHD_OptionItem: {.option = MHD_OPTION_END; .value = 0; .ptr_value = NULL;}
 * @return U_OK on success
 */
int ulfius_start_framework_with_mhd_options(struct _u_instance * u_instance, unsigned int mhd_flags, struct MHD_OptionItem * mhd_ops) {
  if (u_instance == NULL) {
    y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - ulfius_start_framework_with_mhd_options - Error, u_instance is NULL");
    return U_ERROR_PARAMS;
  } else if (mhd_ops == NULL) {
    y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - ulfius_start_framework_with_mhd_options - Error, mhd_ops is NULL");
    return U_ERROR_PARAMS;
  } else {
    u_instance->mhd_daemon = MHD_start_daemon (mhd_flags, u_instance->port, NULL, NULL, &ulfius_webservice_dispatcher, (void *)u_instance, MHD_OPTION_ARRAY, mhd_ops, MHD_OPTION_END);
    if (u_instance->mhd_daemon != NULL) {
      u_instance->status = U_STATUS_RUNNING;
      return U_OK;
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - ulfius_start_framework_with_mhd_options - Error MHD_start_daemon, aborting");
      u_instance->status = U_STATUS_ERROR;
      return U_ERROR_LIBMHD;
    }
  }
}

/**
 * ulfius_stop_framework
 * 
 * Stop the webservice
 * u_instance:    pointer to a struct _u_instance that describe its port and bind address
 * return U_OK on success
 */
int ulfius_stop_framework(struct _u_instance * u_instance) {
  if (u_instance != NULL && u_instance->mhd_daemon != NULL) {
#ifndef U_DISABLE_WEBSOCKET
    int i;
    // Loop in all active websockets and send close signal
    for (i=((struct _websocket_handler *)u_instance->websocket_handler)->nb_websocket_active-1; i>=0; i--) {
      ((struct _websocket_handler *)u_instance->websocket_handler)->websocket_active[i]->websocket_manager->close_flag = 1;
    }
    pthread_mutex_lock(&((struct _websocket_handler *)u_instance->websocket_handler)->websocket_close_lock);
    while (((struct _websocket_handler *)u_instance->websocket_handler)->nb_websocket_active > 0) {
      pthread_cond_wait(&((struct _websocket_handler *)u_instance->websocket_handler)->websocket_close_cond, &((struct _websocket_handler *)u_instance->websocket_handler)->websocket_close_lock);
    }
    pthread_mutex_unlock(&((struct _websocket_handler *)u_instance->websocket_handler)->websocket_close_lock);
#endif 
    MHD_stop_daemon (u_instance->mhd_daemon);
    u_instance->mhd_daemon = NULL;
    u_instance->status = U_STATUS_STOP;
    return U_OK;
  } else if (u_instance != NULL) {
    u_instance->status = U_STATUS_ERROR;
    return U_ERROR_PARAMS;
  } else {
    return U_ERROR;
  }
}

/**
 * ulfius_copy_endpoint
 * return a copy of an endpoint with duplicate values
 */
int ulfius_copy_endpoint(struct _u_endpoint * dest, const struct _u_endpoint * source) {
  if (source != NULL && dest != NULL) {
    dest->http_method = o_strdup(source->http_method);
    dest->url_prefix = o_strdup(source->url_prefix);
    dest->url_format = o_strdup(source->url_format);
    dest->callback_function = source->callback_function;
    dest->user_data = source->user_data;
    dest->priority = source->priority;
    if (ulfius_is_valid_endpoint(dest, 0)) {
      return U_OK;
    } else {
      return U_ERROR_MEMORY;
    }
  }
  return U_ERROR_PARAMS;
}

/**
 * duplicate_endpoint_list
 * return a copy of an endpoint list with duplicate values
 * returned value must be free'd after use
 */
struct _u_endpoint * ulfius_duplicate_endpoint_list(const struct _u_endpoint * endpoint_list) {
  struct _u_endpoint * to_return = NULL;
  int i;
  
  if (endpoint_list != NULL) {
    for (i=0; endpoint_list[i].http_method != NULL; i++) {
      if ((to_return = o_realloc(to_return, (i+1)*sizeof(struct _u_endpoint *))) == NULL) {
        y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for duplicate_endpoint_list.to_return");
        return NULL;
      } else {
        ulfius_copy_endpoint(&to_return[i], &endpoint_list[i]);
      }
    }
  }
  return to_return;
}

/**
 * clean_endpoint
 * free allocated memory by an endpoint
 */
void ulfius_clean_endpoint(struct _u_endpoint * endpoint) {
  if (endpoint != NULL) {
    o_free(endpoint->http_method);
    o_free(endpoint->url_prefix);
    o_free(endpoint->url_format);
    endpoint->http_method = NULL;
    endpoint->url_prefix = NULL;
    endpoint->url_format = NULL;
  }
}

/**
 * ulfius_clean_endpoint_list
 * free allocated memory by an endpoint list
 */
void ulfius_clean_endpoint_list(struct _u_endpoint * endpoint_list) {
  int i;
  
  if (endpoint_list != NULL) {
    for (i=0; endpoint_list[i].http_method != NULL; i++) {
      ulfius_clean_endpoint(&endpoint_list[i]);
    }
    o_free(endpoint_list);
  }
}

/**
 * Add a struct _u_endpoint * to the specified u_instance
 * Can be done during the execution of the webservice for injection
 * u_instance: pointer to a struct _u_instance that describe its port and bind address
 * u_endpoint: pointer to a struct _u_endpoint that will be copied in the u_instance endpoint_list
 * return U_OK on success
 */
int ulfius_add_endpoint(struct _u_instance * u_instance, const struct _u_endpoint * u_endpoint) {
  int res;
  
  if (u_instance != NULL && u_endpoint != NULL) {
    if (ulfius_is_valid_endpoint(u_endpoint, 0)) {
      if (u_instance->endpoint_list == NULL) {
        // No endpoint, create a list with 2 endpoints so the last one is an empty one
        u_instance->endpoint_list = o_malloc(2 * sizeof(struct _u_endpoint));
        if (u_instance->endpoint_list == NULL) {
          y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - ulfius_add_endpoint, Error allocating memory for u_instance->endpoint_list");
          return U_ERROR_MEMORY;
        }
        u_instance->nb_endpoints = 1;
      } else {
        u_instance->nb_endpoints++;
        u_instance->endpoint_list = o_realloc(u_instance->endpoint_list, (u_instance->nb_endpoints + 1) * sizeof(struct _u_endpoint));
        if (u_instance->endpoint_list == NULL) {
          y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - ulfius_add_endpoint, Error reallocating memory for u_instance->endpoint_list");
          return U_ERROR_MEMORY;
        }
      }
      res = ulfius_copy_endpoint(&u_instance->endpoint_list[u_instance->nb_endpoints - 1], u_endpoint);
      if (res != U_OK) {
        return res;
      } else {
        // Add empty endpoint at the end of the endpoint list
        ulfius_copy_endpoint(&u_instance->endpoint_list[u_instance->nb_endpoints], ulfius_empty_endpoint());
      }
      return U_OK;
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - ulfius_add_endpoint, invalid struct _u_endpoint");
      return U_ERROR_PARAMS;
    }
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - ulfius_add_endpoint, invalid parameters");
    return U_ERROR_PARAMS;
  }
  return U_ERROR;
}

/**
 * Add a struct _u_endpoint * list to the specified u_instance
 * Can be done during the execution of the webservice for injection
 * u_instance: pointer to a struct _u_instance that describe its port and bind address
 * u_endpoint_list: pointer to a struct _u_endpoint that will be copied in the u_instance endpoint_list
 * return U_OK on success
 */
int ulfius_add_endpoint_list(struct _u_instance * u_instance, const struct _u_endpoint ** u_endpoint_list) {
  int i, res;
  if (u_instance != NULL && u_endpoint_list != NULL) {
    for (i=0; !ulfius_equals_endpoints(u_endpoint_list[i], ulfius_empty_endpoint()); i++) {
      res = ulfius_add_endpoint(u_instance, u_endpoint_list[i]);
      if (res != U_OK) {
        return res;
      }
    }
    return U_OK;
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - ulfius_add_endpoint_list, invalid parameters");
    return U_ERROR_PARAMS;
  }
  return U_ERROR;
}

/**
 * Remove a struct _u_endpoint * from the specified u_instance
 * Can be done during the execution of the webservice for injection
 * u_instance: pointer to a struct _u_instance that describe its port and bind address
 * u_endpoint: pointer to a struct _u_endpoint that will be removed in the u_instance endpoint_list
 * The parameters _u_endpoint.http_method, _u_endpoint.url_prefix and _u_endpoint.url_format are strictly compared for the match
 * If no endpoint is found, return U_ERROR_NOT_FOUND
 * return U_OK on success
 */
int ulfius_remove_endpoint(struct _u_instance * u_instance, const struct _u_endpoint * u_endpoint) {
  int i, j, found = 0, ret = U_OK;
  char * trim_prefix = NULL, * trim_prefix_save = NULL, * trim_format = NULL, * trim_format_save = NULL,
       * trim_cur_prefix = NULL, * trim_cur_prefix_save = NULL, * trim_cur_format = NULL, * trim_cur_format_save = NULL;
  if (u_instance != NULL && u_endpoint != NULL && !ulfius_equals_endpoints(u_endpoint, ulfius_empty_endpoint()) && ulfius_is_valid_endpoint(u_endpoint, 1)) {
    trim_prefix_save = o_strdup(u_endpoint->url_prefix);
    trim_prefix = trimcharacter(trim_prefix_save, '/');
    trim_format_save = o_strdup(u_endpoint->url_format);
    trim_format = trimcharacter(trim_format_save, '/');
    for (i=u_instance->nb_endpoints-1; i>=0 && ret == U_OK; i--) {
      trim_cur_prefix_save = o_strdup(u_instance->endpoint_list[i].url_prefix);
      trim_cur_prefix = trimcharacter(trim_cur_prefix_save, '/');
      trim_cur_format_save = o_strdup(u_instance->endpoint_list[i].url_format);
      trim_cur_format = trimcharacter(trim_cur_format_save, '/');
      
      // Compare u_endpoint with u_instance->endpoint_list[i]
      if (0 == o_strcmp(u_instance->endpoint_list[i].http_method, u_endpoint->http_method) &&
          0 == o_strcmp(trim_cur_prefix, trim_prefix) &&
          0 == o_strcmp(trim_cur_format, trim_format)) {
        // It's a match!
        // Remove current endpoint and move the next ones to their previous index, then reduce the endpoint_list by 1
        found = 1;
        o_free(u_instance->endpoint_list[i].http_method);
        o_free(u_instance->endpoint_list[i].url_prefix);
        o_free(u_instance->endpoint_list[i].url_format);
        for (j=i; j<u_instance->nb_endpoints; j++) {
          u_instance->endpoint_list[j] = u_instance->endpoint_list[j+1];
        }
        u_instance->nb_endpoints--;
        u_instance->endpoint_list = o_realloc(u_instance->endpoint_list, (u_instance->nb_endpoints + 1)*sizeof(struct _u_endpoint));
        if (u_instance->endpoint_list == NULL) {
          y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - ulfius_add_endpoint, Error reallocating memory for u_instance->endpoint_list");
          ret = U_ERROR_MEMORY;
        }
      }
      o_free(trim_cur_prefix_save);
      o_free(trim_cur_format_save);
      trim_cur_prefix_save = NULL;
      trim_cur_format_save = NULL;
    }
    if (!found) {
      ret = U_ERROR_NOT_FOUND;
    }
    o_free(trim_prefix_save);
    o_free(trim_format_save);
    trim_prefix_save = NULL;
    trim_format_save = NULL;
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - ulfius_remove_endpoint, invalid parameters");
    ret = U_ERROR_PARAMS;
  }
  return ret;
}

/**
 * ulfius_empty_endpoint
 * return an empty endpoint that goes at the end of an endpoint list
 */
const struct _u_endpoint * ulfius_empty_endpoint() {
  static struct _u_endpoint empty_endpoint;
  
  empty_endpoint.http_method = NULL;
  empty_endpoint.url_prefix = NULL;
  empty_endpoint.url_format = NULL;
  empty_endpoint.callback_function = NULL;
  empty_endpoint.user_data = NULL;
  return &empty_endpoint;
}

/**
 * ulfius_equals_endpoints
 * Compare 2 endpoints and return true if their method, prefix and format are the same or if both are NULL
 */
int ulfius_equals_endpoints(const struct _u_endpoint * endpoint1, const struct _u_endpoint * endpoint2) {
  if (endpoint1 != NULL && endpoint2 != NULL) {
    if (endpoint1 == endpoint2) {
      return 1;
    } else if (o_strcmp(endpoint2->http_method, endpoint1->http_method) != 0) {
        return 0;
    } else if (o_strcmp(endpoint2->url_prefix, endpoint1->url_prefix) != 0) {
        return 0;
    } else if (o_strcmp(endpoint2->url_format, endpoint1->url_format) != 0) {
        return 0;
    } else {
      return 1;
    }
  } else {
    return 1;
  }
}

/**
 * Add a struct _u_endpoint * to the specified u_instance with its values specified
 * Can be done during the execution of the webservice for injection
 * u_instance: pointer to a struct _u_instance that describe its port and bind address
 * http_method:       http verb (GET, POST, PUT, etc.) in upper case
 * url_prefix:        prefix for the url (optional)
 * url_format:        string used to define the endpoint format
 *                    separate words with /
 *                    to define a variable in the url, prefix it with @ or :
 *                    example: /test/resource/:name/elements
 *                    on an url_format that ends with '*', the rest of the url will not be tested
 * priority:          endpoint priority in descending order (0 is the higher priority)
 * callback_function: a pointer to a function that will be executed each time the endpoint is called
 *                    you must declare the function as described.
 * user_data:         a pointer to a data or a structure that will be available in callback_function
 * return U_OK on success
 */
int ulfius_add_endpoint_by_val(struct _u_instance * u_instance,
                               const char * http_method,
                               const char * url_prefix,
                               const char * url_format,
                               unsigned int priority,
                               int (* callback_function)(const struct _u_request * request, // Input parameters (set by the framework)
                                                         struct _u_response * response,     // Output parameters (set by the user)
                                                         void * user_data),
                               void * user_data) {
  struct _u_endpoint endpoint;
  if (u_instance != NULL) {
    endpoint.http_method = (char *)http_method;
    endpoint.url_prefix = (char *)url_prefix;
    endpoint.url_format = (char *)url_format;
    endpoint.priority = priority;
    endpoint.callback_function = callback_function;
    endpoint.user_data = user_data;
    return ulfius_add_endpoint(u_instance, &endpoint);
  } else {
    return U_ERROR_PARAMS;
  }
}

/**
 * Remove a struct _u_endpoint * from the specified u_instance
 * using the specified values used to identify an endpoint
 * Can be done during the execution of the webservice for injection
 * u_instance: pointer to a struct _u_instance that describe its port and bind address
 * http_method: http_method used by the endpoint
 * url_prefix: url_prefix used by the endpoint
 * url_format: url_format used by the endpoint
 * The parameters _u_endpoint.http_method, _u_endpoint.url_prefix and _u_endpoint.url_format are strictly compared for the match
 * If no endpoint is found, return U_ERROR_NOT_FOUND
 * return U_OK on success
 */
int ulfius_remove_endpoint_by_val(struct _u_instance * u_instance, const char * http_method, const char * url_prefix, const char * url_format) {
  struct _u_endpoint endpoint;
  if (u_instance != NULL) {
    endpoint.http_method = (char *)http_method;
    endpoint.url_prefix = (char *)url_prefix;
    endpoint.url_format = (char *)url_format;
    endpoint.callback_function = NULL;
    return ulfius_remove_endpoint(u_instance, &endpoint);
  } else {
    return U_ERROR_PARAMS;
  }
}

/**
 * ulfius_set_default_endpoint
 * Set the default endpoint
 * This endpoint will be called if no endpoint match the url called
 * callback_function: a pointer to a function that will be executed each time the endpoint is called
 *                    you must declare the function as described.
 * user_data:         a pointer to a data or a structure that will be available in callback_function
 * to remove a default endpoint function, call ulfius_set_default_endpoint with NULL parameter for callback_function
 * return U_OK on success
 */
int ulfius_set_default_endpoint(struct _u_instance * u_instance,
                                         int (* callback_function)(const struct _u_request * request, struct _u_response * response, void * user_data),
                                         void * user_data) {
  if (u_instance != NULL && callback_function != NULL) {
    if (u_instance->default_endpoint == NULL) {
      u_instance->default_endpoint = o_malloc(sizeof(struct _u_endpoint));
      if (u_instance->default_endpoint == NULL) {
        y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for u_instance->default_endpoint");
        return U_ERROR_MEMORY;
      }
    }
    u_instance->default_endpoint->http_method = NULL;
    u_instance->default_endpoint->url_prefix = NULL;
    u_instance->default_endpoint->url_format = NULL;
    u_instance->default_endpoint->callback_function = callback_function;
    u_instance->default_endpoint->user_data = user_data;
    u_instance->default_endpoint->priority = 0;
    return U_OK;
  } else {
    return U_ERROR_PARAMS;
  }
}

/**
 * ulfius_set_upload_file_callback_function
 * 
 * Set the callback function to handle file upload
 * Used to facilitate large files upload management
 * The callback function file_upload_callback will be called
 * multiple times, with the uploaded file in striped in parts
 * 
 * Warning: If this function is used, all the uploaded files
 * for the instance will be managed via this function, and they
 * will no longer be available in the struct _u_request in the
 * ulfius callback function afterwards.
 * 
 * Thanks to Thad Phetteplace for the help on this feature
 * 
 * u_instance:    pointer to a struct _u_instance that describe its port and bind address
 * file_upload_callback: Pointer to a callback function that will handle all file uploads
 * cls: a pointer that will be passed to file_upload_callback each tim it's called
 */
int ulfius_set_upload_file_callback_function(struct _u_instance * u_instance,
                                             int (* file_upload_callback) (const struct _u_request * request, 
                                                                           const char * key, 
                                                                           const char * filename, 
                                                                           const char * content_type, 
                                                                           const char * transfer_encoding, 
                                                                           const char * data, 
                                                                           uint64_t off, 
                                                                           size_t size, 
                                                                           void * cls),
                                             void * cls) {
  if (u_instance != NULL && file_upload_callback != NULL) {
    u_instance->file_upload_callback = file_upload_callback;
    u_instance->file_upload_cls = cls;
    return U_OK;
  } else {
    return U_ERROR_PARAMS;
  }
}

/**
 * ulfius_clean_instance
 * 
 * Clean memory allocated by a struct _u_instance *
 */
void ulfius_clean_instance(struct _u_instance * u_instance) {
  if (u_instance != NULL) {
    ulfius_clean_endpoint_list(u_instance->endpoint_list);
    u_map_clean_full(u_instance->default_headers);
    o_free(u_instance->default_auth_realm);
    o_free(u_instance->default_endpoint);
    u_instance->endpoint_list = NULL;
    u_instance->default_headers = NULL;
    u_instance->default_auth_realm = NULL;
    u_instance->bind_address = NULL;
    u_instance->default_endpoint = NULL;
#ifndef U_DISABLE_WEBSOCKET
    /* ulfius_clean_instance might be called without websocket_handler being initialized */
    if ((struct _websocket_handler *)u_instance->websocket_handler) {
        if (((struct _websocket_handler *)u_instance->websocket_handler)->pthread_init && 
            (pthread_mutex_destroy(&((struct _websocket_handler *)u_instance->websocket_handler)->websocket_close_lock) ||
            pthread_cond_destroy(&((struct _websocket_handler *)u_instance->websocket_handler)->websocket_close_cond))) {
          y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error destroying websocket_close_lock or websocket_close_cond");
        }
        o_free(u_instance->websocket_handler);
        u_instance->websocket_handler = NULL;
    }
#endif
  }
}

/**
 * internal_ulfius_init_instance
 * 
 * Initialize a struct _u_instance * with default values
 * internal function used by both ulfius_init_instance and ulfius_init_instance_ipv6
 * port:               tcp port to bind to, must be between 1 and 65535
 * bind_address4:      IPv4 address to listen to, optional, the reference is borrowed, the structure isn't copied
 * bind_address6:      IPv6 address to listen to, optional, the reference is borrowed, the structure isn't copied
 * network_type:       Type of network to listen to, values available are U_USE_IPV4, U_USE_IPV6 or U_USE_ALL
 * default_auth_realm: default realm to send to the client on authentication error
 * return U_OK on success
 */
static int internal_ulfius_init_instance(struct _u_instance * u_instance, unsigned int port, struct sockaddr_in * bind_address4, struct sockaddr_in6 * bind_address6, unsigned short network_type, const char * default_auth_realm) {
#if MHD_VERSION >= 0x00095208
  if (u_instance != NULL && port > 0 && port < 65536 && (bind_address4 == NULL || bind_address6 == NULL) && (network_type & U_USE_ALL)) {
#else
UNUSED(network_type);
  if (u_instance != NULL && port > 0 && port < 65536) {
#endif
    u_instance->mhd_daemon = NULL;
    u_instance->status = U_STATUS_STOP;
    u_instance->port = port;
    u_instance->bind_address = bind_address4;
    u_instance->bind_address6 = bind_address6;
#if MHD_VERSION >= 0x00095208
    u_instance->network_type = network_type;
#endif
    u_instance->timeout = 0;
    u_instance->default_auth_realm = o_strdup(default_auth_realm);
    u_instance->nb_endpoints = 0;
    u_instance->endpoint_list = NULL;
    u_instance->default_headers = o_malloc(sizeof(struct _u_map));
    u_instance->mhd_response_copy_data = 0;
    u_instance->check_utf8 = 1;
    if (u_instance->default_headers == NULL) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for u_instance->default_headers");
      ulfius_clean_instance(u_instance);
      return U_ERROR_MEMORY;
    }
    u_map_init(u_instance->default_headers);
    u_instance->default_endpoint = NULL;
    u_instance->max_post_param_size = 0;
    u_instance->max_post_body_size = 0;
    u_instance->file_upload_callback = NULL;
    u_instance->file_upload_cls = NULL;
#ifndef U_DISABLE_GNUTLS
    u_instance->use_client_cert_auth = 0;
#endif
#ifndef U_DISABLE_WEBSOCKET
    u_instance->websocket_handler = o_malloc(sizeof(struct _websocket_handler));
    if (u_instance->websocket_handler == NULL) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for u_instance->websocket_handler");
      ulfius_clean_instance(u_instance);
      return U_ERROR_MEMORY;
    }
    ((struct _websocket_handler *)u_instance->websocket_handler)->pthread_init = 0;
    ((struct _websocket_handler *)u_instance->websocket_handler)->nb_websocket_active = 0;
    ((struct _websocket_handler *)u_instance->websocket_handler)->websocket_active = NULL;
    if (pthread_mutex_init(&((struct _websocket_handler *)u_instance->websocket_handler)->websocket_close_lock, NULL) || 
        pthread_cond_init(&((struct _websocket_handler *)u_instance->websocket_handler)->websocket_close_cond, NULL)) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error initializing websocket_close_lock or websocket_close_cond");
      ulfius_clean_instance(u_instance);
      return U_ERROR_MEMORY;
    }
    ((struct _websocket_handler *)u_instance->websocket_handler)->pthread_init = 1;
#else
    u_instance->websocket_handler = NULL;
#endif
    return U_OK;
  } else {
    return U_ERROR_PARAMS;
  }
}

/**
 * ulfius_init_instance
 * 
 * Initialize a struct _u_instance * with default values
 * Binds to IPV4 addresses only
 * port:               tcp port to bind to, must be between 1 and 65535
 * bind_address:       IPv4 address to listen to, optional, the reference is borrowed, the structure isn't copied
 * default_auth_realm: default realm to send to the client on authentication error
 * return U_OK on success
 */
int ulfius_init_instance(struct _u_instance * u_instance, unsigned int port, struct sockaddr_in * bind_address, const char * default_auth_realm) {
  return internal_ulfius_init_instance(u_instance, port, bind_address, NULL, U_USE_IPV4, default_auth_realm);
}

#if MHD_VERSION >= 0x00095208
/**
 * ulfius_init_instance_ipv6
 * 
 * Initialize a struct _u_instance * with default values
 * Binds to IPV6 and IPV4 or IPV6 addresses only
 * port:               tcp port to bind to, must be between 1 and 65535
 * bind_address:       IPv6 address to listen to, optional, the reference is borrowed, the structure isn't copied
 * network_type:       Type of network to listen to, values available are U_USE_IPV6 or U_USE_ALL
 * default_auth_realm: default realm to send to the client on authentication error
 * return U_OK on success
 */
int ulfius_init_instance_ipv6(struct _u_instance * u_instance, unsigned int port, struct sockaddr_in6 * bind_address, unsigned short network_type, const char * default_auth_realm) {
  if (network_type & U_USE_IPV6) {
    return internal_ulfius_init_instance(u_instance, port, NULL, bind_address, bind_address!=NULL?U_USE_IPV6:network_type, default_auth_realm);
  } else {
    return U_ERROR_PARAMS;
  }
}
#endif

/**
 * free data allocated by ulfius functions
 */
void u_free(void * data) {
  o_free(data);
}

/**
 * The utf8_check() function scans the '\0'-terminated string starting
 * at s. It returns a pointer to the first byte of the first malformed
 * or overlong UTF-8 sequence found, or NULL if the string contains
 * only correct UTF-8. It also spots UTF-8 sequences that could cause
 * trouble if converted to UTF-16, namely surrogate characters
 * (U+D800..U+DFFF) and non-Unicode positions (U+FFFE..U+FFFF). This
 * routine is very likely to find a malformed sequence if the input
 * uses any other encoding than UTF-8. It therefore can be used as a
 * very effective heuristic for distinguishing between UTF-8 and other
 * encodings.
 *
 * I wrote this code mainly as a specification of functionality; there
 * are no doubt performance optimizations possible for certain CPUs.
 *
 * Markus Kuhn <http://www.cl.cam.ac.uk/~mgk25/> -- 2005-03-30
 * License: http://www.cl.cam.ac.uk/~mgk25/short-license.html
 */
const unsigned char * utf8_check(const char * s_orig) {
  const unsigned char * s = (unsigned char *)s_orig;
  while (*s) {
    if (*s < 0x80) {
      /* 0xxxxxxx */
      s++;
    } else if ((s[0] & 0xe0) == 0xc0) {
      /* 110XXXXx 10xxxxxx */
      if ((s[1] & 0xc0) != 0x80 ||
          (s[0] & 0xfe) == 0xc0) {                  /* overlong? */
        return s;
      } else {
        s += 2;
      }
    } else if ((s[0] & 0xf0) == 0xe0) {
      /* 1110XXXX 10Xxxxxx 10xxxxxx */
      if ((s[1] & 0xc0) != 0x80 ||
	  (s[2] & 0xc0) != 0x80 ||
	  (s[0] == 0xe0 && (s[1] & 0xe0) == 0x80) ||      /* overlong? */
	  (s[0] == 0xed && (s[1] & 0xe0) == 0xa0) ||      /* surrogate? */
	  (s[0] == 0xef && s[1] == 0xbf &&
	   (s[2] & 0xfe) == 0xbe)) {                      /* U+FFFE or U+FFFF? */
        return s;
      } else {
        s += 3;
      }
    } else if ((s[0] & 0xf8) == 0xf0) {
      /* 11110XXX 10XXxxxx 10xxxxxx 10xxxxxx */
      if ((s[1] & 0xc0) != 0x80 ||
	  (s[2] & 0xc0) != 0x80 ||
	  (s[3] & 0xc0) != 0x80 ||
	  (s[0] == 0xf0 && (s[1] & 0xf0) == 0x80) ||      /* overlong? */
	  (s[0] == 0xf4 && s[1] > 0x8f) || s[0] > 0xf4) { /* > U+10FFFF? */
        return s;
      } else {
        s += 4;
      }
    } else {
      return s;
    }
  }

  return NULL;
}

/**
 * Converts a hex character to its integer value
 */
static char from_hex(char ch) {
  return isdigit(ch) ? ch - '0' : tolower(ch) - 'a' + 10;
}

/**
 * Converts an integer value to its hex character
 */
static char to_hex(char code) {
  static char hex[] = "0123456789ABCDEF";
  return hex[code & 15];
}

/**
 * Returns a url-encoded version of str
 * returned value must be cleaned after use
 * Thanks Geek Hideout!
 * http://www.geekhideout.com/urlcode.shtml
 */
char * ulfius_url_encode(const char * str) {
  char * pstr = (char*)str, * buf = NULL, * pbuf = NULL;
  if (str != NULL) {
    buf = malloc(strlen(str) * 3 + 1);
    if (buf != NULL) {
      pbuf = buf;
      while (* pstr) {
        // "$-_.+!*'(),"
        if (isalnum(* pstr) || * pstr == '$' || * pstr == '-' || * pstr == '_' ||
            * pstr == '.' || * pstr == '!' || * pstr == '*' ||
            * pstr == '\'' || * pstr == '(' || * pstr == ')' || * pstr == ',') 
          * pbuf++ = * pstr;
        else if (* pstr == ' ') 
          * pbuf++ = '+';
        else 
          * pbuf++ = '%', * pbuf++ = to_hex(* pstr >> 4), * pbuf++ = to_hex(* pstr & 15);
        pstr++;
      }
      * pbuf = '\0';
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating resources for buf (ulfius_url_encode)");
    }
    return buf;
  } else {
    return NULL;
  }
}

/**
 * Returns a url-decoded version of str
 * returned value must be cleaned after use
 * Thanks Geek Hideout!
 * http://www.geekhideout.com/urlcode.shtml
 */
char * ulfius_url_decode(const char * str) {
  char * pstr = (char*)str, * buf = NULL, * pbuf = NULL;
  if (str != NULL) {
    buf = malloc(strlen(str) + 1);
    if (buf != NULL) {
      pbuf = buf;
      while (* pstr) {
        if (* pstr == '%') {
          if (pstr[1] && pstr[2]) {
            * pbuf++ = from_hex(pstr[1]) << 4 | from_hex(pstr[2]);
            pstr += 2;
          }
        } else if (* pstr == '+') { 
          * pbuf++ = ' ';
        } else {
          * pbuf++ = * pstr;
        }
        pstr++;
      }
      * pbuf = '\0';
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating resources for buf (ulfius_url_decode)");
    }
    return buf;
  } else {
    return NULL;
  }
}

int ulfius_global_init() {
  int ret = U_OK;
  o_malloc_t malloc_fn;
  o_realloc_t realloc_fn;
  o_free_t free_fn;
  
  o_get_alloc_funcs(&malloc_fn, &realloc_fn, &free_fn);
#ifndef U_DISABLE_CURL
  if (curl_global_init(CURL_GLOBAL_ALL) != CURLE_OK) {
    y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error curl_global_init");
    ret = U_ERROR;
  } else {
    if (curl_global_init_mem(CURL_GLOBAL_DEFAULT, malloc_fn, free_fn, realloc_fn, *o_strdup, *calloc) != CURLE_OK) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error curl_global_init_mem");
      ret = U_ERROR_MEMORY;
    }
  }
#endif
#ifndef U_DISABLE_JANSSON
  json_set_alloc_funcs((json_malloc_t)malloc_fn, (json_free_t)free_fn);
#endif
  return ret;
}

void ulfius_global_close() {
#ifndef U_DISABLE_CURL
  curl_global_cleanup();
#endif
}
