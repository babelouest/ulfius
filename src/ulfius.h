/**
 * 
 * Ulfius Framework
 * 
 * REST framework library
 * 
 * ulfius.h: structures and functions declarations
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

#ifndef __ULFIUS_H__
#define __ULFIUS_H__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <microhttpd.h>
#include <jansson.h>
#include <curl/curl.h>

#define ULFIUS_URL_SEPARATOR       "/"
#define ULFIUS_HTTP_ENCODING_JSON  "application/json"
#define ULFIUS_HTTP_HEADER_CONTENT "Content-Type"
#define ULFIUS_HTTP_NOT_FOUND_BODY "Resource not found"
#define ULFIUS_HTTP_ERROR_BODY     "Server Error"

#define ULFIUS_CALLBACK_RESPONSE_OK    0
#define ULFIUS_CALLBACK_RESPONSE_ERROR 1

#define ULFIUS_COOKIE_ATTRIBUTE_EXPIRES  "Expires"
#define ULFIUS_COOKIE_ATTRIBUTE_MAX_AGE  "Max-Age"
#define ULFIUS_COOKIE_ATTRIBUTE_DOMAIN   "Domain"
#define ULFIUS_COOKIE_ATTRIBUTE_PATH     "Path"
#define ULFIUS_COOKIE_ATTRIBUTE_SECURE   "Secure"
#define ULFIUS_COOKIE_ATTRIBUTE_HTTPONLY "HttpOnly"

#define ULFIUS_POSTBUFFERSIZE 1024

#define ULFIUS_VERSION 0.9.1

/*************
 * Structures
 *************/

/**
 * struct _u_map_value
 * a simple pair of key/value
 */
struct _u_map_value {
  char * key;
  char * value;
};

/**
 * struct _u_map
 * the structure containing the umap
 */
struct _u_map {
  int nb_values;
  struct _u_map_value * value_list;
};

/**
 * struct _u_cookie
 * the structure containing the response cookie parameters
 */
struct _u_cookie {
  char * key;
  char * value;
  char * expires;
  uint max_age;
  char * domain;
  char * path;
  int secure;
  int http_only;
};

/**
 * 
 * Structure of an instance
 * 
 * Contains the needed data for an ulfius instance to work
 * 
 * mhd_daemon:   pointer to the libmicrohttpd daemon
 * port:         port number to listen to
 * bind_address: ip address to listen to (if needed)
 * 
 */
struct _u_instance {
  struct MHD_Daemon * mhd_daemon;
  int port;
  struct sockaddr_in * bind_address;
};

/**
 * 
 * Structure of request parameters
 * 
 * Contains request data
 * http_verb:      http method (GET, POST, PUT, DELETE, etc.)
 * http_url:       url used to call this callback function
 * full_uri:       full uri used by the client to call the webservice
 * client_address: IP address of the client
 * map_url:        map containing the url variables, both from the route and the ?key=value variables
 * map_header:     map containing the header variables
 * map_cookie:     map containing the cookie variables
 * map_post_body:  map containing the post body variables (if available)
 * json_body:      json_t * object containing the json body (if available)
 * json_error:     true if the json body was not parsed by jansson (if available)
 * 
 */
struct _u_request {
  char *               http_verb;
  char *               http_url;
  char *               full_uri;
  struct sockaddr *    client_address;
  struct _u_map *      map_url;
  struct _u_map *      map_header;
  struct _u_map *      map_cookie;
  struct _u_map *      map_post_body;
  json_t *             json_body;
  int                  json_error;
  void *               binary_body;
  int                  binary_body_length;
};

/**
 * 
 * Structure of response parameters
 * 
 * Contains response data that must be set by the user
 * status:             HTTP status code (200, 404, 500, etc)
 * protocol:           HTTP Protocol sent
 * map_header:         map containing the header variables
 * nb_cookies:         number of cookies sent
 * map_cookie:         array of cookies sent
 * string_body:        a char * containing the raw body response
 * json_body:          a json_t * object containing the json response
 * binary_body:        a void * containing a binary content
 * binary_body_length: the length of the binary_body
 * 
 */
struct _u_response {
  long               status;
  char             * protocol;
  struct _u_map    * map_header;
  unsigned int       nb_cookies;
  struct _u_cookie * map_cookie;
  char             * string_body;
  json_t           * json_body;
  void             * binary_body;
  unsigned int       binary_body_length;
};

/**
 * 
 * Structure of an endpoint
 * 
 * Contains all informations needed for an endpoint
 * http_method:       http verb (GET, POST, PUT, etc.) in upper case
 * url_format:        string used to define the endpoint format
 *                    separate words with /
 *                    to define a variable in the url, prefix it with @ or :
 *                    example: /test/resource/:name/elements
 *                    on an url_format that ends with '*', the rest of the url will not be tested
 * callback_function: a pointer to a function that will be executed each time the endpoint is called
 *                    you must declare the function as described.
 * user_data:         a pointer to a data or a structure that will be available in the callback function
 * 
 */
struct _u_endpoint {
  char * http_method;
  char * url_format;
  int (* callback_function)(const struct _u_request * request, // Input parameters (set by the framework)
                            struct _u_response * response,     // Output parameters (set by the user)
                            void * user_data);
  void * user_data;
};

/**
 * Structures used to facilitate data manipulations (internal)
 */
struct connection_info_struct {
  struct MHD_PostProcessor * post_processor;
  int                        has_post_processor;
  int                        callback_first_iteration;
  struct _u_request *        request;
};

/********************************
 * Public functions declarations
 ********************************/

/**
 * ulfius_init_framework
 * Initializes the framework and run the webservice based on the parameters given
 * return truze if no error
 * 
 * u_instance:    pointer to a struct _u_instance that describe its port and bind address
 * endpoint_list: array of struct _u_endpoint that will describe endpoints used for the application
 *                the array MUST have an empty struct _u_endpoint at the end of it
 *                {NULL, NULL, NULL, NULL}
 */
int ulfius_init_framework(struct _u_instance * u_instance, struct _u_endpoint * endpoint_list);

/**
 * ulfius_stop_framework
 * 
 * Stop the webservice
 * u_instance:    pointer to a struct _u_instance that describe its port and bind address
 */
int ulfius_stop_framework(struct _u_instance * u_instance);

/**
 * ulfius_add_cookie_to_header
 * add a cookie to the cookie map
 */
int ulfius_add_cookie_to_response(struct _u_response * response, const char * key, const char * value, const char * expires, const uint max_age, 
                      const char * domain, const char * path, const int secure, const int http_only);

/**
 * ulfius_send_request
 * Send a HTTP request and store the result into a _u_response
 * return true if everything went fine, false otherwise
 */
int ulfius_request_http(const struct _u_request * request, struct _u_response * response);

/**
 * ulfius_init_request
 * Initialize a request structure by allocating inner elements
 * return true if everything went fine, false otherwise
 */
int ulfius_init_request(struct _u_request * request);

/**
 * ulfius_init_response
 * Initialize a response structure by allocating inner elements
 * return true if everything went fine, false otherwise
 */
int ulfius_init_response(struct _u_response * response);

/**
 * ulfius_clean_request
 * clean the specified request's inner elements
 * user must free the parent pointer if needed after clean
 * or use ulfius_clean_request_full
 * return true if no error
 */
int ulfius_clean_request(struct _u_request * request);

/**
 * ulfius_clean_request_full
 * clean the specified request and all its elements
 * return true if no error
 */
int ulfius_clean_request_full(struct _u_request * request);

/**
 * ulfius_clean_response
 * clean the specified response's inner elements
 * user must free the parent pointer if needed after clean
 * or use ulfius_clean_response_full
 * return true if no error
 */
int ulfius_clean_response(struct _u_response * response);

/**
 * ulfius_clean_response_full
 * clean the specified response and all its elements
 * return true if no error
 */
int ulfius_clean_response_full(struct _u_response * response);

/**
 * umap declarations
 * umap is a simple map structure that handles sets of key/value maps
 * 
 * Be careful, umap is VERY memory unfriendly, every pointer returned by the functions must be freed after use
 * 
 */

/**
 * initialize a struct _u_map
 * this function MUST be called after a declaration or allocation
 */
void u_map_init(struct _u_map * map);

/**
 * free the struct _u_map and its components
 * return true if no error
 */
int u_map_clean_full(struct _u_map * u_map);

/**
 * free the struct _u_map's inner components
 * return true if no error
 */
int u_map_clean(struct _u_map * u_map);

/**
 * free an enum return by functions u_map_enum_keys or u_map_enum_values
 * return true if no error
 */
int u_map_clean_enum(char ** array);

/**
 * returns an array containing all the keys in the struct _u_map
 * return an array of char * ending with a NULL element
 * use u_map_clean_enum(char ** array) to clean a returned array
 */
char ** u_map_enum_keys(const struct _u_map * u_map);

/**
 * returns an array containing all the values in the struct _u_map
 * return an array of char * ending with a NULL element
 * use u_map_clean_enum(char ** array) to clean a returned array
 */
char ** u_map_enum_values(const struct _u_map * u_map);

/**
 * return true if the sprcified u_map contains the specified key
 * false otherwise
 * search is case sensitive
 */
int u_map_has_key(const struct _u_map * u_map, const char * key);

/**
 * return true if the sprcified u_map contains the specified value
 * false otherwise
 * search is case sensitive
 */
int u_map_has_value(const struct _u_map * u_map, const char * value);

/**
 * add the specified key/value pair into the specified u_map
 * if the u_map already contains a pair with the same key, replace the value
 * return true if no error
 */
int u_map_put(struct _u_map * u_map, const char * key, const char * value);

/**
 * get the value corresponding to the specified key in the u_map
 * return NULL if no match found
 * search is case sensitive
 * returned value must be freed after use
 */
char * u_map_get(const struct _u_map * u_map, const const char * key);

/**
 * return true if the sprcified u_map contains the specified key
 * false otherwise
 * search is case insensitive
 */
int u_map_has_key_case(const struct _u_map * u_map, const char * key);

/**
 * return true if the sprcified u_map contains the specified value
 * false otherwise
 * search is case insensitive
 */
int u_map_has_value_case(const struct _u_map * u_map, const char * value);

/**
 * get the value corresponding to the specified key in the u_map
 * return NULL if no match found
 * search is case insensitive
 * returned value must be freed after use
 */
char * u_map_get_case(const struct _u_map * u_map, const char * key);

/**
 * Create an exact copy of the specified struct _u_map
 * return a reference to the copy, NULL otherwise
 * returned value must be freed after use
 */
struct _u_map * u_map_copy(const struct _u_map * source);

/**
 * Return the number of key/values pair in the specified struct _u_map
 * Return -1 on error
 */
int u_map_count(const struct _u_map * source);

/**********************************
 * Internal functions declarations
 **********************************/

/**
 * validate_instance
 * return true if u_instance has valid parameters
 */
int validate_instance(const struct _u_instance * u_instance);

/**
 * validate_endpoint_list
 * return true if endpoint_list has valid parameters
 */
int validate_endpoint_list(const struct _u_endpoint * endpoint_list);

/**
 * ulfius_webservice_dispatcher
 * function executed by libmicrohttpd every time an HTTP call is made
 */
int ulfius_webservice_dispatcher (void *cls, struct MHD_Connection *connection,
                              const char *url, const char *method,
                              const char *version, const char *upload_data,
                              size_t *upload_data_size, void **con_cls);
/**
 * iterate_post_data
 * function used to iterate post parameters
 */
int iterate_post_data (void *coninfo_cls, enum MHD_ValueKind kind, const char *key,
              const char *filename, const char *content_type,
              const char *transfer_encoding, const char *data, uint64_t off,
              size_t size);
/**
 * request_completed
 * function used to clean data allocated after a web call is complete
 */
void request_completed (void *cls, struct MHD_Connection *connection,
                   void **con_cls, enum MHD_RequestTerminationCode toe);
/**
 * split_url
 * return an array of char based on the url words
 */
char ** split_url(const char * url);

/**
 * endpoint_match
 * return the endpoint matching the url called with the proper http method
 * return NULL if no endpoint is found
 */
struct _u_endpoint * endpoint_match(const char * method, const char * url, struct _u_endpoint * endpoint_list);

/**
 * url_format_match
 * return true if splitted_url matches splitted_url_format
 * false otherwise
 */
int url_format_match(const char ** splitted_url, const char ** splitted_url_format);

/**
 * parse_url
 * fills map with the keys/values defined in the url that are described in the endpoint format url
 * return true if no error
 */
int parse_url(const char * url, const struct _u_endpoint * endpoint, struct _u_map * map);

/**
 * set_response_header
 * adds headers defined in the response_map_header to the response
 * return true if no error
 */
int set_response_header(struct MHD_Response * response, const struct _u_map * response_map_header);

/**
 * set_response_cookie
 * adds cookies defined in the response_map_cookie
 * return true if no error
 */
int set_response_cookie(struct MHD_Response * mhd_response, const struct _u_response * response);

/**
 * Add a cookie in the cookie map as defined in the RFC 6265
 */
char * get_cookie_header(const struct _u_cookie * cookie);

/**
 * ulfius_clean_cookie
 * clean the cookie's elements
 */
int ulfius_clean_cookie(struct _u_cookie * cookie);

/**
 * Copy the cookie source elements into dest elements
 */
int ulfius_copy_cookie(struct _u_cookie * dest, const struct _u_cookie * source);

/**
 * ulfius_copy_response
 * Copy the source response elements into the des response
 */
int ulfius_copy_response(struct _u_response * dest, const struct _u_response * source);

/**
 * create a new request based on the source elements
 * return value must be free'd
 */
struct _u_request * ulfius_duplicate_request(const struct _u_request * request);

/**
 * create a new response based on the source elements
 * return value must be free'd
 */
struct _u_response * ulfius_duplicate_response(const struct _u_response * response);

/**
 * u_strdup
 * a modified strdup function that don't crash when source is NULL, instead return NULL
 */
char * u_strdup(const char * source);

#endif // __ULFIUS_H__
