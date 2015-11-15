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
  if (u_map_put(((struct _u_map *)cls), key, value)) {
    return MHD_YES;
  } else {
    return MHD_NO;
  }
}

/**
 * validate_instance
 * return true if u_instance has valid parameters
 */
int validate_instance(const struct _u_instance * u_instance) {
  return (u_instance != NULL && u_instance->port > 0 && u_instance->port < 65536);
}

/**
 * validate_endpoint_list
 * return true if endpoint_list has valid parameters
 */
int validate_endpoint_list(const struct _u_endpoint * endpoint_list) {
  int i;
  if (endpoint_list != NULL) {
    for (i=0; endpoint_list[i].http_method != NULL; i++) {
      if (endpoint_list[i].http_method == NULL && endpoint_list[i].url_format == NULL && endpoint_list[i].user_data == NULL && endpoint_list[i].callback_function == NULL && i == 0) {
        // One can not have an empty endpoint in the beginning of the list
        return 0;
      } else if (!(endpoint_list[i].http_method != NULL && endpoint_list[i].url_format != NULL && endpoint_list[i].callback_function != NULL)) {
        // One must set at least the parameters http_method, url_format and callback_function
        return 0;
      }
    }
    return 1;
  } else {
    return 0;
  }
}

void * ulfius_uri_logger (void * cls, const char * uri) {
  struct connection_info_struct * con_info = malloc (sizeof (struct connection_info_struct));
  if (con_info != NULL) {
    con_info->callback_first_iteration = 1;
    con_info->request = malloc(sizeof(struct _u_request));
    if (con_info->request == NULL) {
      free(con_info);
      return NULL;
    }
    
    if (NULL == con_info->request || !ulfius_init_request(con_info->request)) {
      free(con_info);
      con_info = NULL;
      return NULL;
    }
    con_info->request->http_url = u_strdup(uri);
  }
  return con_info;
}

/**
 * ulfius_init_framework
 * Initializes the framework and run the webservice based on the parameters given
 * return truze if no error
 * 
 * u_instance:    pointer to a struct _u_instance that describe its port and bind address
 * endpoint_list: array of struct _u_endpoint that will describe endpoints used for the application
 *                the array MUST have an empty struct _u_endpoint at the end of it
 *                {NULL, NULL, NULL, NULL}
 * return true on success, false otherwise
 */
int ulfius_init_framework(struct _u_instance * u_instance, struct _u_endpoint * endpoint_list) {
  
  // Validate u_instance and endpoint_list that there is no mistake
  
  if (validate_instance(u_instance) && validate_endpoint_list(endpoint_list)) {
    u_instance->mhd_daemon = MHD_start_daemon (MHD_USE_THREAD_PER_CONNECTION, 
          u_instance->port, NULL, NULL, &ulfius_webservice_dispatcher, (void *)endpoint_list, 
          MHD_OPTION_NOTIFY_COMPLETED, request_completed, NULL,
          MHD_OPTION_SOCK_ADDR, u_instance->bind_address,
          MHD_OPTION_URI_LOG_CALLBACK, ulfius_uri_logger, NULL,
          MHD_OPTION_END);
    
    return (u_instance->mhd_daemon != NULL);
  } else {
    return 0;
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
  struct _u_endpoint * endpoint_list = (struct _u_endpoint *)cls, * current_endpoint;
  struct connection_info_struct * con_info = * con_cls;
  int mhd_ret = MHD_NO, callback_ret = ULFIUS_CALLBACK_RESPONSE_ERROR;
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
    return MHD_NO;
  }
  
  if (con_info->callback_first_iteration) {
    con_info->callback_first_iteration = 0;
    so_client = MHD_get_connection_info (connection, MHD_CONNECTION_INFO_CLIENT_ADDRESS)->client_addr;
    con_info->has_post_processor = 0;
    
    con_info->request->http_verb = u_strdup(method);
    con_info->request->client_address = malloc(sizeof(struct sockaddr));
    if (con_info->request->client_address == NULL) {
      return MHD_NO;
    }
    memcpy(con_info->request->client_address, so_client, sizeof(struct sockaddr));
    MHD_get_connection_values (connection, MHD_HEADER_KIND, ulfius_fill_map, con_info->request->map_header);
    MHD_get_connection_values (connection, MHD_GET_ARGUMENT_KIND, ulfius_fill_map, con_info->request->map_url);
    MHD_get_connection_values (connection, MHD_COOKIE_KIND, ulfius_fill_map, con_info->request->map_cookie);
    content_type = u_map_get_case(con_info->request->map_header, "content-type");
    
    // Set POST Processor if content-type is properly set
    if (content_type != NULL && (0 == strncmp(MHD_HTTP_POST_ENCODING_FORM_URLENCODED, content_type, strlen(MHD_HTTP_POST_ENCODING_FORM_URLENCODED)) || 
        0 == strncmp(MHD_HTTP_POST_ENCODING_MULTIPART_FORMDATA, content_type, strlen(MHD_HTTP_POST_ENCODING_MULTIPART_FORMDATA)))) {
      con_info->has_post_processor = 1;
      con_info->post_processor = MHD_create_post_processor (connection, ULFIUS_POSTBUFFERSIZE, iterate_post_data, (void *) con_info);
      if (NULL == con_info->post_processor) {
        ulfius_clean_request_full(con_info->request);
        con_info->request = NULL;
        return MHD_NO;
      }
    }
    free(content_type);
    content_type = NULL;
    return MHD_YES;
  }
  
  if (*upload_data_size != 0) {
    // Handles request body
    char * content_type = u_map_get(con_info->request->map_header, "Content-Type");
    if (0 == strncmp(MHD_HTTP_POST_ENCODING_FORM_URLENCODED, content_type, strlen(MHD_HTTP_POST_ENCODING_FORM_URLENCODED)) || 
        0 == strncmp(MHD_HTTP_POST_ENCODING_MULTIPART_FORMDATA, content_type, strlen(MHD_HTTP_POST_ENCODING_MULTIPART_FORMDATA))) {
      MHD_post_process (con_info->post_processor, upload_data, *upload_data_size);
    } else if (0 == strncmp(ULFIUS_HTTP_ENCODING_JSON, content_type, strlen(ULFIUS_HTTP_ENCODING_JSON))) {
      json_error_t j_error;
      con_info->request->json_body = json_loads(upload_data, JSON_DECODE_ANY, &j_error);
      if (!con_info->request->json_body) {
        con_info->request->json_error = 1;
      } else {
        con_info->request->json_error = 0;
      }
    }
    con_info->request->binary_body = u_strdup(upload_data);
    con_info->request->binary_body_length = *upload_data_size;
    *upload_data_size = 0;
    free(content_type);
    content_type = NULL;
    return MHD_YES;
  } else {
    // Check if the endpoint has a match
    current_endpoint = endpoint_match(method, url, endpoint_list);
    
    response = malloc(sizeof(struct _u_response));
    if (response == NULL) {
      return MHD_NO;
    }
    if (!ulfius_init_response(response)) {
      free(response);
      return MHD_NO;
    }
    
    if (current_endpoint != NULL) {
      // Endpoint found, run callback function with the input parameters filled
      parse_url(url, current_endpoint, con_info->request->map_url);
      
      callback_ret = current_endpoint->callback_function(con_info->request, response, current_endpoint->user_data);

      // Set the response code to 200 OK if the user did not (or forgot to) set one
      if (response->status == 0) {
        response->status = 200;
      }
      if (!callback_ret && response->json_body != NULL) {
        // The user sent a json response
        if (u_map_put(response->map_header, ULFIUS_HTTP_HEADER_CONTENT, ULFIUS_HTTP_ENCODING_JSON)) {
          response_buffer = (void*) json_dumps(response->json_body, JSON_COMPACT);
          if (response_buffer == NULL) {
            response->status = MHD_HTTP_INTERNAL_SERVER_ERROR;
            response->string_body = u_strdup(ULFIUS_HTTP_ERROR_BODY);
          } else {
            response_buffer_len = strlen ((char*)response_buffer);
          }
        } else {
          response->status = MHD_HTTP_INTERNAL_SERVER_ERROR;
          response->string_body = u_strdup(ULFIUS_HTTP_ERROR_BODY);
        }
      } else if (!callback_ret && response->binary_body != NULL && response->binary_body_length > 0) {
        // The user sent a binary response
        response_buffer = malloc(response->binary_body_length);
        if (response_buffer == NULL) {
          response->status = MHD_HTTP_INTERNAL_SERVER_ERROR;
          response->string_body = u_strdup(ULFIUS_HTTP_ERROR_BODY);
        } else {
          memcpy(response_buffer, response->binary_body, response->binary_body_length);
          response_buffer_len = response->binary_body_length;
        }
      } else if (!callback_ret) {
        // The user sent a string response
        if (response->string_body == NULL) {
          response_buffer = u_strdup("");
        } else {
          response_buffer = u_strdup(response->string_body);
          response_buffer_len = strlen(response_buffer);
        }
      } else if (response->string_body == NULL && response->json_body == NULL && response->binary_body == NULL) {
        // No valid response parameters, sending error 500
        response->status = MHD_HTTP_INTERNAL_SERVER_ERROR;
        response->string_body = u_strdup(ULFIUS_HTTP_ERROR_BODY);
      } else {
        // callback return value different than 0, sending error 500
        response_buffer = u_strdup(ULFIUS_HTTP_ERROR_BODY);
        response_buffer_len = strlen(ULFIUS_HTTP_ERROR_BODY);
        response->status = MHD_HTTP_INTERNAL_SERVER_ERROR;
      }
      mhd_response = MHD_create_response_from_buffer (response_buffer_len, response_buffer, MHD_RESPMEM_MUST_FREE );
      
      if (!callback_ret) {
        if (set_response_header(mhd_response, response->map_header) == -1 || set_response_cookie(mhd_response, response) == -1) {
          response->status = MHD_HTTP_INTERNAL_SERVER_ERROR;
          response->string_body = u_strdup(ULFIUS_HTTP_ERROR_BODY);
        }
      }
      
      mhd_ret = MHD_queue_response (connection, response->status, mhd_response);
      MHD_destroy_response (mhd_response);
    } else {
      response_buffer = u_strdup(ULFIUS_HTTP_NOT_FOUND_BODY);
      response_buffer_len = strlen(ULFIUS_HTTP_NOT_FOUND_BODY);
      response->status = MHD_HTTP_NOT_FOUND;
      mhd_response = MHD_create_response_from_buffer (response_buffer_len, response_buffer, MHD_RESPMEM_MUST_FREE );
      mhd_ret = MHD_queue_response (connection, response->status, mhd_response);
      MHD_destroy_response (mhd_response);
    }
    
    // Free Response parameters
    ulfius_clean_response_full(response);
    response = NULL;

    return mhd_ret;
  }
}

/**
 * iterate_post_data
 * function used to iterate post parameters
 * return MHD_NO on error
 */
int iterate_post_data (void *coninfo_cls, enum MHD_ValueKind kind, const char *key,
                      const char *filename, const char *content_type,
                      const char *transfer_encoding, const char *data, uint64_t off,
                      size_t size) {
  struct connection_info_struct *con_info = coninfo_cls;
  if (u_map_put((struct _u_map *)con_info->request->map_post_body, key, data)) {
    return MHD_YES;
  } else {
    return MHD_NO;
  }
}

/**
 * request_completed
 * function used to clean data allocated after a web call is complete
 */
void request_completed (void *cls, struct MHD_Connection *connection,
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
 * return true on success, false otherwise
 */
int ulfius_stop_framework(struct _u_instance * u_instance) {
  if (u_instance != NULL && u_instance->mhd_daemon != NULL) {
    MHD_stop_daemon (u_instance->mhd_daemon);
    return 1;
  } else {
    return 0;
  }
}

/**
 * u_strdup
 * a modified strdup function that don't crash when source is NULL, instead return NULL
 * Returned value must be free'd after use
 */
char * u_strdup(const char * source) {
  return (source==NULL?NULL:strdup(source));
}
