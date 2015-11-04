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
	u_map_put(((struct _u_map *)cls), key, value);
	return MHD_YES;
}

/**
 * validate_instance
 * return true if u_instance has valid parameters
 */
int validate_instance(const struct _u_instance * u_instance){
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

/**
 * Initialize the framework environment and start the instance
 */
int ulfius_init_framework(struct _u_instance * u_instance, struct _u_endpoint * endpoint_list) {
	
	// Validate u_instance and endpoint_list that there is no mistake
	
	if (validate_instance(u_instance) && validate_endpoint_list(endpoint_list)) {
		u_instance->mhd_daemon = MHD_start_daemon (MHD_USE_THREAD_PER_CONNECTION, 
					u_instance->port, NULL, NULL, &ulfius_webservice_dispatcher, (void *)endpoint_list, 
					MHD_OPTION_NOTIFY_COMPLETED, request_completed, NULL,
					MHD_OPTION_SOCK_ADDR, u_instance->bind_address,
					MHD_OPTION_END);
		
		return (u_instance->mhd_daemon != NULL);
	} else {
		return 0;
	}
}

/**
 * Redirect the http call to the proper function
 * This function is called multiple times (at least 2)
 * so it must run the callback function one time, after everything is set up
 */
int ulfius_webservice_dispatcher (void * cls, struct MHD_Connection * connection,
				const char * url, const char * method,
				const char * version, const char * upload_data,
				size_t * upload_data_size, void ** con_cls) {
	struct _u_endpoint * endpoint_list = (struct _u_endpoint *)cls, * current_endpoint;
	struct connection_info_struct * con_info_post = NULL;
  struct connection_info_struct * con_info = * con_cls;
  int mhd_ret = MHD_NO, callback_ret = ULFIUS_CALLBACK_RESPONSE_ERROR;
  char * content_type;
  struct _u_response* response = NULL;
  
  // Response variables
  struct MHD_Response * mhd_response = NULL;
	
	// Prepare for POST or PUT input data
	// Initialize the input maps
	if (NULL == *con_cls) {
		con_info_post = malloc (sizeof (struct connection_info_struct));
		if (NULL == con_info_post) {
			return MHD_NO;
		}
		con_info_post->has_post_processor = 0;
		con_info_post->request = malloc(sizeof(struct _u_request));
		
		if (NULL == con_info_post->request) {
			return MHD_NO;
		}
		
		con_info_post->request->http_verb  = (char*)method;
		con_info_post->request->http_url   = (char*)url;
		con_info_post->request->map_url    = malloc(sizeof(struct _u_map));
		con_info_post->request->map_header = malloc(sizeof(struct _u_map));
		con_info_post->request->map_cookie = malloc(sizeof(struct _u_map));
		con_info_post->request->map_post_body = NULL;
		con_info_post->request->json_body = NULL;
		if (NULL == con_info_post->request->map_url || NULL == con_info_post->request->map_header || NULL == con_info_post->request->map_cookie) {
			free(con_info_post->request->map_url);
			con_info_post->request->map_url = NULL;
			free(con_info_post->request->map_header);
			con_info_post->request->map_header = NULL;
			free(con_info_post->request->map_cookie);
			con_info_post->request->map_cookie = NULL;
			free(con_info_post->request);
			con_info_post->request = NULL;
			free(con_info_post);
			con_info_post = NULL;
			return MHD_NO;
		}
		u_map_init(con_info_post->request->map_url);
		u_map_init(con_info_post->request->map_header);
		u_map_init(con_info_post->request->map_cookie);
		MHD_get_connection_values (connection, MHD_HEADER_KIND, ulfius_fill_map, con_info_post->request->map_header);
		MHD_get_connection_values (connection, MHD_GET_ARGUMENT_KIND, ulfius_fill_map, con_info_post->request->map_url);
		MHD_get_connection_values (connection, MHD_COOKIE_KIND, ulfius_fill_map, con_info_post->request->map_cookie);
		content_type = u_map_get_case(con_info_post->request->map_header, "content-type");
		// Set POST Processor if content-type is properly set
		if (content_type != NULL && (0 == strncmp(MHD_HTTP_POST_ENCODING_FORM_URLENCODED, content_type, strlen(MHD_HTTP_POST_ENCODING_FORM_URLENCODED)) || 
				0 == strncmp(MHD_HTTP_POST_ENCODING_MULTIPART_FORMDATA, content_type, strlen(MHD_HTTP_POST_ENCODING_MULTIPART_FORMDATA)))) {
			con_info_post->has_post_processor = 1;
			con_info_post->post_processor = MHD_create_post_processor (connection, ULFIUS_POSTBUFFERSIZE, iterate_post_data, (void *) con_info_post);
			if (NULL == con_info_post->post_processor) {
				free(con_info_post->request->map_url);
				con_info_post->request->map_url = NULL;
				free(con_info_post->request->map_header);
				con_info_post->request->map_header = NULL;
				free(con_info_post->request->map_cookie);
				con_info_post->request->map_cookie = NULL;
				free(con_info_post->request);
				con_info_post->request = NULL;
				free(con_info_post);
				con_info_post = NULL;
				return MHD_NO;
			}
		}
		free(content_type);
		content_type = NULL;
		*con_cls = (void *) con_info_post;
		return MHD_YES;
	}
	if (*upload_data_size != 0) {
		// Handles request body
		char * content_type = u_map_get(con_info->request->map_header, "Content-Type");
		if (0 == strncmp(MHD_HTTP_POST_ENCODING_FORM_URLENCODED, content_type, strlen(MHD_HTTP_POST_ENCODING_FORM_URLENCODED)) || 
				0 == strncmp(MHD_HTTP_POST_ENCODING_MULTIPART_FORMDATA, content_type, strlen(MHD_HTTP_POST_ENCODING_MULTIPART_FORMDATA))) {
			con_info->request->map_post_body = malloc(sizeof(struct _u_map));
			u_map_init(con_info->request->map_post_body);
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
		*upload_data_size = 0;
		free(content_type);
		content_type = NULL;
		return MHD_YES;
	} else {
		// Check if the endpoint has a match
		current_endpoint = endpoint_match(method, url, endpoint_list);
		
		response = malloc(sizeof(struct _u_response));
		response->status = 0;
		response->map_header = malloc(sizeof(struct _u_map));
		u_map_init(response->map_header);
		response->map_cookie = u_map_copy(con_info->request->map_cookie);
		response->string_body = NULL;
		response->json_body = NULL;
		response->binary_body = NULL;
		response->binary_body_length = 0;
		
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
				if (response->string_body != NULL) {
					free(response->string_body);
				}
				response->string_body = json_dumps(response->json_body, JSON_COMPACT);
				mhd_response = MHD_create_response_from_buffer (strlen (response->string_body), (void *) response->string_body, MHD_RESPMEM_MUST_FREE );
				MHD_add_response_header (mhd_response, ULFIUS_HTTP_HEADER_CONTENT, ULFIUS_HTTP_ENCODING_JSON);
				json_decref(response->json_body);
			} else if (!callback_ret && response->binary_body != NULL && response->binary_body_length > 0) {
				// The user sent a binary response
				mhd_response = MHD_create_response_from_buffer (response->binary_body_length, response->binary_body, MHD_RESPMEM_MUST_FREE );
				response->binary_body = NULL;
			} else if (!callback_ret) {
				// The user sent a string response
				if (response->string_body == NULL) {
					response->string_body = strdup("");
				}
				mhd_response = MHD_create_response_from_buffer (strlen (response->string_body), (void *) response->string_body, MHD_RESPMEM_MUST_FREE );
			} else if (response->string_body == NULL && response->json_body == NULL && response->binary_body == NULL) {
				// No valid response parameters, sending error 500
				response->status = MHD_HTTP_INTERNAL_SERVER_ERROR;
				response->string_body = strdup(ULFIUS_HTTP_ERROR_BODY);
			} else {
				// callback return value different than 0, sending error 500
				if (response->string_body != NULL) {
					free(response->string_body);
				}
				response->string_body = strdup(ULFIUS_HTTP_ERROR_BODY);
				response->status = MHD_HTTP_INTERNAL_SERVER_ERROR;
				mhd_response = MHD_create_response_from_buffer (strlen (response->string_body), (void *) response->string_body, MHD_RESPMEM_MUST_FREE );
			}
			response->json_body = NULL;
			
			if (!callback_ret) {
				set_response_header(mhd_response, response->map_header);
				set_response_cookie(mhd_response, response->map_cookie);
			}
			
			mhd_ret = MHD_queue_response (connection, response->status, mhd_response);
			MHD_destroy_response (mhd_response);
		} else {
			(response->string_body) = strdup(ULFIUS_HTTP_NOT_FOUND_BODY);
			response->status = MHD_HTTP_NOT_FOUND;
			mhd_response = MHD_create_response_from_buffer (strlen (response->string_body), (void *) response->string_body, MHD_RESPMEM_MUST_FREE );
			mhd_ret = MHD_queue_response (connection, response->status, mhd_response);
			MHD_destroy_response (mhd_response);
		}
		
		// Free Response parameters
		if (response->binary_body != NULL) {
			free(response->binary_body);
		}
		u_map_clean(response->map_header);
		response->map_header = NULL;
		u_map_clean(response->map_cookie);
		response->map_cookie = NULL;
		free(response->json_body);
		response->json_body = NULL;
		response->string_body = NULL;
		free(response);
		response = NULL;

		return mhd_ret;
	}
}

/**
 * Parse the POST data
 */
int iterate_post_data (void *coninfo_cls, enum MHD_ValueKind kind, const char *key,
	      const char *filename, const char *content_type,
	      const char *transfer_encoding, const char *data, uint64_t off,
	      size_t size) {
	struct connection_info_struct *con_info = coninfo_cls;
	u_map_put((struct _u_map *)con_info->request->map_post_body, key, data);
  return MHD_YES;
}

/**
 * Mark the request completed so ulfius can keep going
 */
void request_completed (void *cls, struct MHD_Connection *connection,
		   void **con_cls, enum MHD_RequestTerminationCode toe) {
  struct connection_info_struct *con_info = *con_cls;
  if (NULL == con_info) {
    return;
  }
  // Request parameters
	if (con_info->request->json_body != NULL) {
		json_decref(con_info->request->json_body);
  }
  if (NULL != con_info && con_info->has_post_processor) {
    MHD_destroy_post_processor (con_info->post_processor);
  }
  u_map_clean(con_info->request->map_url);
  con_info->request->map_url = NULL;
  u_map_clean(con_info->request->map_header);
  con_info->request->map_header = NULL;
  u_map_clean(con_info->request->map_cookie);
  con_info->request->map_cookie = NULL;
	u_map_clean(con_info->request->map_post_body);
	con_info->request->map_post_body = NULL;
  free(con_info->request);
  con_info->request = NULL;
  free(con_info);
  con_info = NULL;
  *con_cls = NULL;
}
