/**
 * 
 * Ulfius Framework
 * 
 * REST framework library
 * 
 * ulfius.c: framework functions definitions
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
 * Fill a map with the key/values specified
 */
static int ulfius_fill_map(void *cls, enum MHD_ValueKind kind, const char *key, const char *value) {
  if (u_map_put(((struct _u_map *)cls), key, value) == U_OK) {
    return MHD_YES;
  } else {
    return MHD_NO;
  }
}

/**
 * ulfius_validate_instance
 * return true if u_instance has valid parameters
 */
int ulfius_validate_instance(const struct _u_instance * u_instance) {
  if (u_instance == NULL || u_instance->port <= 0 || u_instance->port >= 65536 || !ulfius_validate_endpoint_list(u_instance->endpoint_list, u_instance->nb_endpoints)) {
    y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error, instance or has invalid parameters");
    return 0;
  }
  return 1;
}

/**
 * ulfius_validate_endpoint_list
 * return true if endpoint_list has valid parameters
 */
int ulfius_validate_endpoint_list(const struct _u_endpoint * endpoint_list, int nb_endpoints) {
  int i;
  if (endpoint_list != NULL) {
    for (i=0; i < nb_endpoints; i++) {
      if (i == 0 && ulfius_equals_endpoints(ulfius_empty_endpoint(), &endpoint_list[i])) {
        // One can not have an empty endpoint in the beginning of the list
        y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error, no empty endpoint allowed in the beginning of the endpoint list");
        return 0;
      } else if (!ulfius_is_valid_endpoint(&endpoint_list[i])) {
        // One must set at least the parameters http_method, url_format and callback_function
        y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error, endpoint at index %d has invalid parameters", i);
        return 0;
      }
    }
    return 1;
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error, no endpoint list");
    return 0;
  }
}

/**
 * Internal method used to duplicate the full url before it's manipulated and modified by MHD
 */
void * ulfius_uri_logger (void * cls, const char * uri) {
  struct connection_info_struct * con_info = malloc (sizeof (struct connection_info_struct));
  if (con_info != NULL) {
    con_info->callback_first_iteration = 1;
    con_info->request = malloc(sizeof(struct _u_request));
    if (con_info->request == NULL) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for con_info->request");
      free(con_info);
      return NULL;
    }
    
    if (NULL == con_info->request || ulfius_init_request(con_info->request) != U_OK) {
      ulfius_clean_request_full(con_info->request);
      free(con_info);
      return NULL;
    }
    con_info->request->http_url = nstrdup(uri);
    if (con_info->request->http_url == NULL) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for con_info->request->http_url");
      ulfius_clean_request_full(con_info->request);
      free(con_info);
      return NULL;
    }
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for con_info");
  }
  return con_info;
}

/**
 * ulfius_run_mhd_daemon
 * Starts a mhd daemon for the specified instance
 * return a pointer to the mhd_daemon on success, NULL on error
 * 
 */
struct MHD_Daemon * ulfius_run_mhd_daemon(struct _u_instance * u_instance, const char * key_pem, const char * cert_pem) {
  uint mhd_flags = MHD_USE_THREAD_PER_CONNECTION;
#ifdef DEBUG
  mhd_flags |= MHD_USE_DEBUG;
#endif
  
  if (u_instance->mhd_daemon == NULL) {
    struct MHD_OptionItem mhd_ops[6];
    
    // Default options
    mhd_ops[0].option = MHD_OPTION_NOTIFY_COMPLETED;
    mhd_ops[0].value = (intptr_t)mhd_request_completed;
    mhd_ops[0].ptr_value = NULL;
    
    mhd_ops[1].option = MHD_OPTION_SOCK_ADDR;
    mhd_ops[1].value = (intptr_t)u_instance->bind_address;
    mhd_ops[1].ptr_value = NULL;
    
    mhd_ops[2].option = MHD_OPTION_URI_LOG_CALLBACK;
    mhd_ops[2].value = (intptr_t)ulfius_uri_logger;
    mhd_ops[2].ptr_value = NULL;
    
    mhd_ops[3].option = MHD_OPTION_END;
    mhd_ops[3].value = 0;
    mhd_ops[3].ptr_value = NULL;
    
    if (key_pem != NULL && cert_pem != NULL) {
      // HTTPS parameters
      mhd_flags |= MHD_USE_SSL;
      mhd_ops[3].option = MHD_OPTION_HTTPS_MEM_KEY;
      mhd_ops[3].value = 0;
      mhd_ops[3].ptr_value = (void*)key_pem;
      
      mhd_ops[4].option = MHD_OPTION_HTTPS_MEM_CERT;
      mhd_ops[4].value = 0;
      mhd_ops[4].ptr_value = (void*)cert_pem;
      
      mhd_ops[5].option = MHD_OPTION_END;
      mhd_ops[5].value = 0;
      mhd_ops[5].ptr_value = NULL;
    }
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
  // Validate u_instance and endpoint_list that there is no mistake
  if (ulfius_validate_instance(u_instance)) {
    u_instance->mhd_daemon = ulfius_run_mhd_daemon(u_instance, NULL, NULL);
    
    if (u_instance->mhd_daemon == NULL) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error MHD_start_daemon, aborting");
      u_instance->status = U_STATUS_ERROR;
      return U_ERROR_LIBMHD;
    } else {
      u_instance->status = U_STATUS_RUNNING;
      return U_OK;
    }
  } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "ulfius_start_framework - error input parameters");
    return U_ERROR_PARAMS;
  }
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
  // Validate u_instance and endpoint_list that there is no mistake
  if (ulfius_validate_instance(u_instance) && key_pem != NULL && cert_pem != NULL) {
    u_instance->mhd_daemon = ulfius_run_mhd_daemon(u_instance, key_pem, cert_pem);
    
    if (u_instance->mhd_daemon == NULL) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error MHD_start_daemon, aborting");
      u_instance->status = U_STATUS_ERROR;
      return U_ERROR_LIBMHD;
    } else {
      u_instance->status = U_STATUS_RUNNING;
      return U_OK;
    }
  } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "ulfius_start_secure_framework - error input parameters");
    return U_ERROR_PARAMS;
  }
}

int ulfius_get_body_from_response(struct _u_response * response, void ** response_buffer, size_t * response_buffer_len) {
  if (response == NULL || response_buffer == NULL || response_buffer_len == NULL) {
    return U_ERROR_PARAMS;
  } else {
    if (response->json_body != NULL) {
      // The user sent a json response
      if (u_map_put(response->map_header, ULFIUS_HTTP_HEADER_CONTENT, ULFIUS_HTTP_ENCODING_JSON) == U_OK) {
        *response_buffer = (void*) json_dumps(response->json_body, JSON_COMPACT);
        if (*response_buffer == NULL) {
          y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error parsing json body");
          response->status = MHD_HTTP_INTERNAL_SERVER_ERROR;
          response->string_body = nstrdup(ULFIUS_HTTP_ERROR_BODY);
          if (response->string_body == NULL) {
            y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for response->string_body");
            return U_ERROR_MEMORY;
          }
          return U_ERROR_PARAMS;
        } else {
          *response_buffer_len = strlen (*response_buffer);
        }
      } else {
        response->status = MHD_HTTP_INTERNAL_SERVER_ERROR;
        *response_buffer = nstrdup(ULFIUS_HTTP_ERROR_BODY);
        if (*response_buffer == NULL) {
          y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for response_buffer");
          return U_ERROR_MEMORY;
        }
        *response_buffer_len = strlen (ULFIUS_HTTP_ERROR_BODY);
      }
    } else if (response->binary_body != NULL && response->binary_body_length > 0) {
      // The user sent a binary response
      *response_buffer = malloc(response->binary_body_length);
      if (*response_buffer == NULL) {
        y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for response_buffer");
        response->status = MHD_HTTP_INTERNAL_SERVER_ERROR;
        response->string_body = nstrdup(ULFIUS_HTTP_ERROR_BODY);
        if (response->string_body == NULL) {
          y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for response->string_body");
          return U_ERROR_MEMORY;
        }
      } else {
        memcpy(*response_buffer, response->binary_body, response->binary_body_length);
        *response_buffer_len = response->binary_body_length;
      }
    } else {
      // The user sent a string response
      if (response->string_body == NULL) {
        *response_buffer = nstrdup("");
        *response_buffer_len = 0;
      } else {
        *response_buffer = nstrdup(response->string_body);
        *response_buffer_len = strlen(*response_buffer);
      }
    }
    return U_OK;
  }
}

/**
 * ulfius_webservice_dispatcher
 * function executed by libmicrohttpd every time an HTTP call is made
 * return MHD_NO on error
 */
int ulfius_webservice_dispatcher (void * cls, struct MHD_Connection * connection,
                                  const char * url, const char * method,
                                  const char * version, const char * upload_data,
                                  size_t * upload_data_size, void ** con_cls) {
  struct _u_endpoint * endpoint_list = ((struct _u_instance *)cls)->endpoint_list, * current_endpoint;
  struct connection_info_struct * con_info = * con_cls;
  int mhd_ret = MHD_NO, callback_ret = U_OK, auth_ret = U_OK;
  char * content_type;
  struct _u_response * response = NULL;
  struct sockaddr * so_client;
  
  void * response_buffer = NULL;
  size_t response_buffer_len = 0;
  
  // Response variables
  struct MHD_Response * mhd_response = NULL;
  
  // Prepare for POST or PUT input data
  // Initialize the input maps
  if (con_info == NULL) {
    y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error con_info is NULL");
    return MHD_NO;
  }
  
  if (con_info->callback_first_iteration) {
    con_info->callback_first_iteration = 0;
    so_client = MHD_get_connection_info (connection, MHD_CONNECTION_INFO_CLIENT_ADDRESS)->client_addr;
    con_info->has_post_processor = 0;
    
    con_info->request->http_verb = nstrdup(method);
    con_info->request->client_address = malloc(sizeof(struct sockaddr));
    if (con_info->request->client_address == NULL || con_info->request->http_verb == NULL) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating client_address or http_verb");
      return MHD_NO;
    }
    memcpy(con_info->request->client_address, so_client, sizeof(struct sockaddr));
    MHD_get_connection_values (connection, MHD_HEADER_KIND, ulfius_fill_map, con_info->request->map_header);
    MHD_get_connection_values (connection, MHD_GET_ARGUMENT_KIND, ulfius_fill_map, con_info->request->map_url);
    MHD_get_connection_values (connection, MHD_COOKIE_KIND, ulfius_fill_map, con_info->request->map_cookie);
    content_type = (char*)u_map_get_case(con_info->request->map_header, "content-type");
    
    // Set POST Processor if content-type is properly set
    if (content_type != NULL && (0 == strncmp(MHD_HTTP_POST_ENCODING_FORM_URLENCODED, content_type, strlen(MHD_HTTP_POST_ENCODING_FORM_URLENCODED)) || 
        0 == strncmp(MHD_HTTP_POST_ENCODING_MULTIPART_FORMDATA, content_type, strlen(MHD_HTTP_POST_ENCODING_MULTIPART_FORMDATA)))) {
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
  }
  
  if (*upload_data_size != 0) {
    // Handles request body
    const char * content_type = u_map_get(con_info->request->map_header, "Content-Type");
    if (0 == strncmp(MHD_HTTP_POST_ENCODING_FORM_URLENCODED, content_type, strlen(MHD_HTTP_POST_ENCODING_FORM_URLENCODED)) || 
        0 == strncmp(MHD_HTTP_POST_ENCODING_MULTIPART_FORMDATA, content_type, strlen(MHD_HTTP_POST_ENCODING_MULTIPART_FORMDATA))) {
      MHD_post_process (con_info->post_processor, upload_data, *upload_data_size);
    } else if (0 == strncmp(ULFIUS_HTTP_ENCODING_JSON, content_type, strlen(ULFIUS_HTTP_ENCODING_JSON))) {
      json_error_t json_error;
      con_info->request->json_body = json_loadb(upload_data, *upload_data_size, JSON_DECODE_ANY, &json_error);
      if (!con_info->request->json_body) {
        con_info->request->json_has_error = 1;
        con_info->request->json_error = &json_error;
      } else {
        con_info->request->json_has_error = 0;
      }
    }
    con_info->request->binary_body = malloc(*upload_data_size);
    if (con_info->request->binary_body == NULL) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for con_info->request->binary_body");
      return MHD_NO;
    } else {
      memcpy(con_info->request->binary_body, upload_data, *upload_data_size);
      con_info->request->binary_body_length = *upload_data_size;
      *upload_data_size = 0;
      return MHD_YES;
    }
  } else {
    // Check if the endpoint has a match
    current_endpoint = ulfius_endpoint_match(method, url, endpoint_list);
    
    // Set to default_endpoint if no match
    if (current_endpoint == NULL && ((struct _u_instance *)cls)->default_endpoint != NULL && ((struct _u_instance *)cls)->default_endpoint->callback_function != NULL) {
      current_endpoint = ((struct _u_instance *)cls)->default_endpoint;
    }
    
    if (current_endpoint != NULL) {
    
      response = malloc(sizeof(struct _u_response));
      if (response == NULL) {
        y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating response");
        return MHD_NO;
      }
      if (ulfius_init_response(response) != U_OK) {
        free(response);
        y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error ulfius_init_response");
        return MHD_NO;
      }
      if (ulfius_parse_url(url, current_endpoint, con_info->request->map_url) != U_OK) {
        free(response);
        y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error parsing url: ", url);
        return MHD_NO;
      }
      
      // Add default headers (if any to the response header maps
      if (((struct _u_instance *)cls)->default_headers != NULL && u_map_count(((struct _u_instance *)cls)->default_headers) > 0) {
          u_map_clean_full(response->map_header);
          response->map_header = u_map_copy(((struct _u_instance *)cls)->default_headers);
      }
      
      // Initialize auth variables
      con_info->request->auth_basic_user = MHD_basic_auth_get_username_password(connection, &con_info->request->auth_basic_password);
      
      // Call auth_function if set
      if (current_endpoint->auth_function != NULL) {
        auth_ret = current_endpoint->auth_function(con_info->request, response, current_endpoint->auth_data);
      }
      
      if (auth_ret == U_ERROR_UNAUTHORIZED) {
        // Wrong credentials, send status 401 and realm value
        if (current_endpoint->auth_realm == NULL) {
          // If no realm is set, send an erro 500 and log the error
          y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - No realm value set, abort authentication error");
          response->status = MHD_HTTP_INTERNAL_SERVER_ERROR;
          response->string_body = nstrdup(ULFIUS_HTTP_ERROR_BODY);
          if (response->string_body == NULL) {
            y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for response->string_body");
            return MHD_NO;
          }
          mhd_response = MHD_create_response_from_buffer (response_buffer_len, response_buffer, MHD_RESPMEM_MUST_FREE );
        } else if (ulfius_get_body_from_response(response, &response_buffer, &response_buffer_len) == U_OK) {
          mhd_response = MHD_create_response_from_buffer (response_buffer_len, response_buffer, MHD_RESPMEM_MUST_FREE );
          if (ulfius_set_response_header(mhd_response, response->map_header) == -1 || ulfius_set_response_cookie(mhd_response, response) == -1) {
            response->status = MHD_HTTP_INTERNAL_SERVER_ERROR;
            response->string_body = nstrdup(ULFIUS_HTTP_ERROR_BODY);
            if (response->string_body == NULL) {
              y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for response->string_body");
              return MHD_NO;
            }
          }
        } else {
          // Error building response, sending error 500
          response_buffer = nstrdup(ULFIUS_HTTP_ERROR_BODY);
          if (response_buffer == NULL) {
            y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for response_buffer");
            return MHD_NO;
          }
          response_buffer_len = strlen(ULFIUS_HTTP_ERROR_BODY);
          mhd_response = MHD_create_response_from_buffer (response_buffer_len, response_buffer, MHD_RESPMEM_MUST_FREE );
        }
        mhd_ret = MHD_queue_basic_auth_fail_response (connection, current_endpoint->auth_realm, mhd_response);
        MHD_destroy_response (mhd_response);
      
        // Free Response parameters
        ulfius_clean_response_full(response);
        response = NULL;
      } else if (auth_ret == U_OK) {
        // Reset response structure
        ulfius_clean_response(response);
        ulfius_init_response(response);
        // Endpoint found, run callback function with the input parameters filled
        callback_ret = current_endpoint->callback_function(con_info->request, response, current_endpoint->user_data);
        
        if (callback_ret == U_OK) {
          if (ulfius_get_body_from_response(response, &response_buffer, &response_buffer_len) == U_OK) {
            mhd_response = MHD_create_response_from_buffer (response_buffer_len, response_buffer, MHD_RESPMEM_MUST_FREE );
            if (ulfius_set_response_header(mhd_response, response->map_header) == -1 || ulfius_set_response_cookie(mhd_response, response) == -1) {
              response->status = MHD_HTTP_INTERNAL_SERVER_ERROR;
              response->string_body = nstrdup(ULFIUS_HTTP_ERROR_BODY);
              if (response->string_body == NULL) {
                y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for response->string_body");
                return MHD_NO;
              }
            }
          } else {
            // Error building response, sending error 500
            response->status = MHD_HTTP_INTERNAL_SERVER_ERROR;
            response_buffer = nstrdup(ULFIUS_HTTP_ERROR_BODY);
            if (response_buffer == NULL) {
              y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for response_buffer");
              return MHD_NO;
            }
            response_buffer_len = strlen(ULFIUS_HTTP_ERROR_BODY);
            mhd_response = MHD_create_response_from_buffer (response_buffer_len, response_buffer, MHD_RESPMEM_MUST_FREE );
          }
        } else {
          // Error building response, sending error 500
          response->status = MHD_HTTP_INTERNAL_SERVER_ERROR;
          response_buffer = nstrdup(ULFIUS_HTTP_ERROR_BODY);
          if (response_buffer == NULL) {
            y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for response_buffer");
            return MHD_NO;
          }
          response_buffer_len = strlen(ULFIUS_HTTP_ERROR_BODY);
          mhd_response = MHD_create_response_from_buffer (response_buffer_len, response_buffer, MHD_RESPMEM_MUST_FREE );
        }

        mhd_ret = MHD_queue_response (connection, response->status, mhd_response);
        MHD_destroy_response (mhd_response);
      
        // Free Response parameters
        ulfius_clean_response_full(response);
        response = NULL;
      } else {
        // Error building response, sending error 500
        response_buffer = nstrdup(ULFIUS_HTTP_ERROR_BODY);
        if (response_buffer == NULL) {
          y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for response_buffer");
          return MHD_NO;
        }
        response_buffer_len = strlen(ULFIUS_HTTP_ERROR_BODY);
        
        mhd_ret = MHD_queue_response (connection, MHD_HTTP_INTERNAL_SERVER_ERROR, mhd_response);
        MHD_destroy_response (mhd_response);
      
        // Free Response parameters
        ulfius_clean_response_full(response);
        response = NULL;
      }
      
    } else {
      response_buffer = nstrdup(ULFIUS_HTTP_NOT_FOUND_BODY);
      if (response_buffer == NULL) {
        y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for response_buffer");
        return MHD_NO;
      }
      response_buffer_len = strlen(ULFIUS_HTTP_NOT_FOUND_BODY);
      mhd_response = MHD_create_response_from_buffer (response_buffer_len, response_buffer, MHD_RESPMEM_MUST_FREE );
      mhd_ret = MHD_queue_response (connection, MHD_HTTP_NOT_FOUND, mhd_response);
      MHD_destroy_response (mhd_response);
    }
    
    return mhd_ret;
  }
}

/**
 * mhd_iterate_post_data
 * function used to iterate post parameters
 * return MHD_NO on error
 */
int mhd_iterate_post_data (void *coninfo_cls, enum MHD_ValueKind kind, const char *key,
                      const char *filename, const char *content_type,
                      const char *transfer_encoding, const char *data, uint64_t off,
                      size_t size) {
  struct connection_info_struct *con_info = coninfo_cls;
  if (u_map_put((struct _u_map *)con_info->request->map_post_body, key, data) == U_OK) {
    return MHD_YES;
  } else {
    return MHD_NO;
  }
}

/**
 * mhd_request_completed
 * function used to clean data allocated after a web call is complete
 */
void mhd_request_completed (void *cls, struct MHD_Connection *connection,
                        void **con_cls, enum MHD_RequestTerminationCode toe) {
  struct connection_info_struct *con_info = *con_cls;
  if (NULL == con_info) {
    return;
  }
  if (NULL != con_info && con_info->has_post_processor) {
    MHD_destroy_post_processor (con_info->post_processor);
  }
  ulfius_clean_request_full(con_info->request);
  con_info->request = NULL;
  free(con_info);
  con_info = NULL;
  *con_cls = NULL;
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
    MHD_stop_daemon (u_instance->mhd_daemon);
    u_instance->mhd_daemon = NULL;
    u_instance->status = U_STATUS_STOP;
    return U_OK;
  } else {
    u_instance->status = U_STATUS_ERROR;
    return U_ERROR_PARAMS;
  }
}

/**
 * ulfius_is_valid_endpoint
 * return true if the endpoind has valid parameters
 */
int ulfius_is_valid_endpoint(const struct _u_endpoint * endpoint) {
  if (endpoint != NULL) {
    if (ulfius_equals_endpoints(endpoint, ulfius_empty_endpoint())) {
      // Should be the last endpoint of the list to close it
      return 1;
    } else if (endpoint->http_method == NULL) {
      return 0;
    } else if (endpoint->callback_function == NULL) {
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
 * ulfius_copy_endpoint
 * return a copy of an endpoint with duplicate values
 * returned value must be free'd after use
 */
int ulfius_copy_endpoint(struct _u_endpoint * dest, const struct _u_endpoint * source) {
  if (source != NULL && dest != NULL) {
    dest->http_method = nstrdup(source->http_method);
    dest->url_prefix = nstrdup(source->url_prefix);
    dest->url_format = nstrdup(source->url_format);
    dest->auth_function = source->auth_function;
    dest->auth_data = source->auth_data;
    dest->auth_realm = nstrdup(source->auth_realm);
    dest->callback_function = source->callback_function;
    dest->user_data = source->user_data;
    if (ulfius_is_valid_endpoint(dest)) {
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
      if ((to_return = realloc(to_return, (i+1)*sizeof(struct _u_endpoint *))) == NULL) {
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
    free(endpoint->http_method);
    free(endpoint->url_prefix);
    free(endpoint->url_format);
    free(endpoint->auth_realm);
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
    free(endpoint_list);
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
  int i, res;
  
  if (u_instance != NULL && u_endpoint != NULL) {
    if (ulfius_is_valid_endpoint(u_endpoint)) {
      if (u_instance->endpoint_list == NULL) {
        // No endpoint, create a list with 2 endpoints so the last one is an empty one
        u_instance->endpoint_list = malloc(2 * sizeof(struct _u_endpoint));
        if (u_instance->endpoint_list == NULL) {
          y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - ulfius_add_endpoint, Error allocating memory for u_instance->endpoint_list");
          return U_ERROR_MEMORY;
        }
        u_instance->nb_endpoints = 1;
      } else {
        // List has endpoints, append this one if it doesn't exist yet
        for (i=0; i <= u_instance->nb_endpoints; i++) {
          if (ulfius_equals_endpoints(u_endpoint, &(u_instance->endpoint_list[i]))) {
            y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - ulfius_add_endpoint, Error endpoint already exist");
            return U_ERROR_PARAMS;
          }
        }
        u_instance->nb_endpoints++;
        u_instance->endpoint_list = realloc(u_instance->endpoint_list, (u_instance->nb_endpoints + 1) * sizeof(struct _u_endpoint));
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
  int i, j;
  if (u_instance != NULL && u_endpoint != NULL && !ulfius_equals_endpoints(u_endpoint, ulfius_empty_endpoint()) && ulfius_is_valid_endpoint(u_endpoint)) {
    for (i=0; i<u_instance->nb_endpoints; i++) {
      // Compare u_endpoint with u_instance->endpoint_list[i]
      if ((u_endpoint->http_method != NULL && 0 == strcmp(u_instance->endpoint_list[i].http_method, u_endpoint->http_method)) &&
          (u_endpoint->url_prefix != NULL && 0 == strcmp(u_instance->endpoint_list[i].url_prefix, u_endpoint->url_prefix)) &&
          (u_endpoint->url_format != NULL && 0 == strcmp(u_instance->endpoint_list[i].url_format, u_endpoint->url_format))) {
        // It's a match!
        // Remove current endpoint and move the next ones to their previous index, then reduce the endpoint_list by 1
        free(u_instance->endpoint_list[i].http_method);
        free(u_instance->endpoint_list[i].url_prefix);
        free(u_instance->endpoint_list[i].url_format);
        for (j=i; j<u_instance->nb_endpoints; j++) {
          u_instance->endpoint_list[j] = u_instance->endpoint_list[j+1];
        }
        u_instance->nb_endpoints--;
        u_instance->endpoint_list = realloc(u_instance->endpoint_list, (u_instance->nb_endpoints + 1)*sizeof(struct _u_endpoint));
        if (u_instance->endpoint_list == NULL) {
          y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - ulfius_add_endpoint, Error reallocating memory for u_instance->endpoint_list");
          return U_ERROR_MEMORY;
        }
      }
    }
    return U_OK;
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - ulfius_remove_endpoint, invalid parameters");
    return U_ERROR_PARAMS;
  }
  return U_ERROR;
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
    } else if (nstrcmp(endpoint2->http_method, endpoint1->http_method) != 0) {
        return 0;
    } else if (nstrcmp(endpoint2->url_prefix, endpoint1->url_prefix) != 0) {
        return 0;
    } else if (nstrcmp(endpoint2->url_format, endpoint1->url_format) != 0) {
        return 0;
    } else {
      return 1;
    }
  } else {
    return 1;
  }
}

/**
 * ulfius_init_instance
 * 
 * Initialize a struct _u_instance * with default values
 * return U_OK on success
 */
int ulfius_init_instance(struct _u_instance * u_instance, int port, struct sockaddr_in * bind_address) {
  if (u_instance != NULL) {
    u_instance->mhd_daemon = NULL;
    u_instance->status = U_STATUS_STOP;
    u_instance->port = port;
    u_instance->bind_address = bind_address;
    u_instance->nb_endpoints = 0;
    u_instance->endpoint_list = NULL;
    u_instance->default_headers = malloc(sizeof(struct _u_map));
    if (u_instance->default_headers == NULL) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for u_instance->default_headers");
      return U_ERROR_MEMORY;
    }
    u_map_init(u_instance->default_headers);
    u_instance->default_endpoint = NULL;
    return U_OK;
  } else {
    return U_ERROR_PARAMS;
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
 * auth_function:     a pointer to a function that will be executed prior to the callback for authentication
 *                    you must declare the function as described.
 * auth_data:         a pointer to a data or a structure that will be available in auth_function
 * auth_realm:        realm value for authentication
 * callback_function: a pointer to a function that will be executed each time the endpoint is called
 *                    you must declare the function as described.
 * user_data:         a pointer to a data or a structure that will be available in callback_function
 * return U_OK on success
 */
int ulfius_add_endpoint_by_val(struct _u_instance * u_instance,
                               const char * http_method,
                               const char * url_prefix,
                               const char * url_format,
                               int (* auth_function)(const struct _u_request * request, // Input parameters (set by the framework)
                                                         struct _u_response * response,     // Output parameters (set by the user)
                                                         void * user_data),
                               void * auth_data,
                               char * auth_realm,
                               int (* callback_function)(const struct _u_request * request, // Input parameters (set by the framework)
                                                         struct _u_response * response,     // Output parameters (set by the user)
                                                         void * user_data),
                               void * user_data) {
  struct _u_endpoint endpoint;
  if (u_instance != NULL && ((auth_function != NULL && auth_realm != NULL) || (auth_function == NULL && auth_realm == NULL))) {
    endpoint.http_method = (char*)http_method;
    endpoint.url_prefix = (char*)url_prefix;
    endpoint.url_format = (char*)url_format;
    endpoint.auth_function = auth_function;
    endpoint.auth_data = auth_data;
    endpoint.auth_realm = auth_realm;
    endpoint.callback_function = callback_function;
    endpoint.user_data = user_data;
    return ulfius_add_endpoint(u_instance, &endpoint);
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
    free(u_instance->default_endpoint);
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
    return ulfius_remove_endpoint(u_instance, &endpoint);
  } else {
    return U_ERROR_PARAMS;
  }
}

/**
 * ulfius_set_default_endpoint
 * Set the default endpoint
 * This endpoint will be called if no endpoint match the url called
 * auth_function:     a pointer to a function that will be executed prior to the callback for authentication
 *                    you must declare the function as described.
 * auth_data:         a pointer to a data or a structure that will be available in auth_function
 * callback_function: a pointer to a function that will be executed each time the endpoint is called
 *                    you must declare the function as described.
 * user_data:         a pointer to a data or a structure that will be available in callback_function
 * to remove a default endpoint function, call ulfius_set_default_endpoint with NULL parameter for callback_function
 * return U_OK on success
 */
int ulfius_set_default_endpoint(struct _u_instance * u_instance,
                                         int (* auth_function)(const struct _u_request * request, struct _u_response * response, void * auth_data),
                                         void * auth_data,
                                         char * auth_realm,
                                         int (* callback_function)(const struct _u_request * request, struct _u_response * response, void * user_data),
                                         void * user_data) {
  if (u_instance != NULL && ((auth_function != NULL && auth_realm != NULL) || (auth_function == NULL && auth_realm == NULL))) {
    if (u_instance->default_endpoint == NULL) {
      u_instance->default_endpoint = malloc(sizeof(struct _u_endpoint));
      if (u_instance->default_endpoint == NULL) {
        y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for u_instance->default_endpoint");
        return U_ERROR_MEMORY;
      }
    }
    u_instance->default_endpoint->http_method = NULL;
    u_instance->default_endpoint->url_prefix = NULL;
    u_instance->default_endpoint->url_format = NULL;
    u_instance->default_endpoint->auth_function = auth_function;
    u_instance->default_endpoint->auth_data = auth_data;
    u_instance->default_endpoint->auth_realm = auth_realm;
    u_instance->default_endpoint->callback_function = callback_function;
    u_instance->default_endpoint->user_data = user_data;
    return U_OK;
  } else {
    return U_ERROR_PARAMS;
  }
}
