/**
 * 
 * Ulfius Framework
 * 
 * REST framework library
 * 
 * ulfius.h: public structures and functions declarations
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

#ifndef __ULFIUS_H__
#define __ULFIUS_H__

/** External dependencies **/
#include <pthread.h>
#include <microhttpd.h>

#if defined(_WIN32) && !defined(U_DISABLE_WEBSOCKET)
  #define U_DISABLE_WEBSOCKET
#endif

#if (MHD_VERSION < 0x00095300) && !defined(U_DISABLE_WEBSOCKET)
  #define U_DISABLE_WEBSOCKET
#endif

#ifndef U_DISABLE_WEBSOCKET
  #include <poll.h>
#endif

/** Angharad libraries **/
#include <yder.h>
#include <orcania.h>

#ifndef U_DISABLE_JANSSON
#include <jansson.h>
#endif

#define ULFIUS_STREAM_BLOCK_SIZE_DEFAULT 1024
#define U_STREAM_END MHD_CONTENT_READER_END_OF_STREAM
#define U_STREAM_ERROR MHD_CONTENT_READER_END_WITH_ERROR
#define U_STREAM_SIZE_UNKOWN MHD_SIZE_UNKNOWN

#define U_OK                 0 // No error
#define U_ERROR              1 // Error
#define U_ERROR_MEMORY       2 // Error in memory allocation
#define U_ERROR_PARAMS       3 // Error in input parameters
#define U_ERROR_LIBMHD       4 // Error in libmicrohttpd execution
#define U_ERROR_LIBCURL      5 // Error in libcurl execution
#define U_ERROR_NOT_FOUND    6 // Something was not found

#define ULFIUS_VERSION 2.2.4

#define U_CALLBACK_CONTINUE     0
#define U_CALLBACK_COMPLETE     1
#define U_CALLBACK_UNAUTHORIZED 2
#define U_CALLBACK_ERROR        3

/*************
 * Structures
 *************/

/**
 * struct _u_map
 */
struct _u_map {
  int      nb_values;
  char  ** keys;
  char  ** values;
  size_t * lengths;
};

/**
 * struct _u_cookie
 * the structure containing the response cookie parameters
 */
struct _u_cookie {
  char * key;
  char * value;
  char * expires;
  unsigned int   max_age;
  char * domain;
  char * path;
  int    secure;
  int    http_only;
};

/**
 * 
 * Structure of request parameters
 * 
 * Contains request data
 * http_protocol:             http protocol used (1.0 or 1.1)
 * http_verb:                 http method (GET, POST, PUT, DELETE, etc.), use '*' to match all http methods
 * http_url:                  url used to call this callback function or full url to call when used in a ulfius_send_http_request
 * proxy:                     proxy address to use for outgoing connections, used by ulfius_send_http_request
 * check_server_certificate:  do not check server certificate and hostname if false (default true), used by ulfius_send_http_request
 * timeout                    connection timeout used by ulfius_send_http_request, default is 0
 * client_address:            IP address of the client
 * auth_basic_user:           basic authtication username
 * auth_basic_password:       basic authtication password
 * map_url:                   map containing the url variables, both from the route and the ?key=value variables
 * map_header:                map containing the header variables
 * map_cookie:                map containing the cookie variables
 * map_post_body:             map containing the post body variables (if available)
 * binary_body:               pointer to raw body
 * binary_body_length:        length of raw body
 * 
 */
struct _u_request {
  char *               http_protocol;
  char *               http_verb;
  char *               http_url;
  char *               proxy;
  int                  check_server_certificate;
  long                 timeout;
  struct sockaddr *    client_address;
  char *               auth_basic_user;
  char *               auth_basic_password;
  struct _u_map *      map_url;
  struct _u_map *      map_header;
  struct _u_map *      map_cookie;
  struct _u_map *      map_post_body;
  void *               binary_body;
  size_t               binary_body_length;
};

/**
 * 
 * Structure of response parameters
 * 
 * Contains response data that must be set by the user
 * status:                              HTTP status code (200, 404, 500, etc)
 * protocol:                            HTTP Protocol sent
 * map_header:                          map containing the header variables
 * nb_cookies:                          number of cookies sent
 * map_cookie:                          array of cookies sent
 * auth_realm:                          realm to send to the client on authenticationb failed
 * binary_body:                         a void * containing a raw binary content
 * binary_body_length:                  the length of the binary_body
 * stream_callback:                     callback function to stream data in response body
 * stream_callback_free:                callback function to free data allocated for streaming
 * stream_size:                         size of the streamed data (U_STREAM_SIZE_UNKOWN if unknown)
 * stream_block_size:                   size of each block to be streamed, set according to your system
 * stream_user_data:                    user defined data that will be available in your callback stream functions
 * websocket_handle:                    handle for websocket extension
 * shared_data:                         any data shared between callback functions, must be allocated and freed by the callback functions
 * 
 */
struct _u_response {
  long               status;
  char             * protocol;
  struct _u_map    * map_header;
  unsigned int       nb_cookies;
  struct _u_cookie * map_cookie;
  char             * auth_realm;
  void             * binary_body;
  size_t             binary_body_length;
  ssize_t         (* stream_callback) (void * stream_user_data, uint64_t offset, char * out_buf, size_t max);
  void            (* stream_callback_free) (void * stream_user_data);
  uint64_t           stream_size;
  size_t             stream_block_size;
  void             * stream_user_data;
  void             * websocket_handle;
  void *             shared_data;
};

/**
 * 
 * Structure of an endpoint
 * 
 * Contains all informations needed for an endpoint
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
 * 
 */
struct _u_endpoint {
  char * http_method;
  char * url_prefix;
  char * url_format;
  unsigned int   priority;
  int (* callback_function)(const struct _u_request * request, // Input parameters (set by the framework)
                            struct _u_response * response,     // Output parameters (set by the user)
                            void * user_data);
  void * user_data;
};

/**
 * 
 * Structure of an instance
 * 
 * Contains the needed data for an ulfius instance to work
 * 
 * mhd_daemon:            pointer to the libmicrohttpd daemon
 * status:                status of the current instance, status are U_STATUS_STOP, U_STATUS_RUNNING or U_STATUS_ERROR
 * port:                  port number to listen to
 * bind_address:          ip address to listen to (optional)
 * nb_endpoints:          Number of available endpoints
 * default_auth_realm:    Default realm on authentication error
 * endpoint_list:         List of available endpoints
 * default_endpoint:      Default endpoint if no other endpoint match the current url
 * default_headers:       Default headers that will be added to all response->map_header
 * max_post_param_size:   maximum size for a post parameter, 0 means no limit, default 0
 * max_post_body_size:    maximum size for the entire post body, 0 means no limit, default 0
 * 
 */
struct _u_instance {
  struct MHD_Daemon          *  mhd_daemon;
  int                           status;
  unsigned int                          port;
  struct sockaddr_in          * bind_address;
  int                           nb_endpoints;
  char                        * default_auth_realm;
  struct _u_endpoint          * endpoint_list;
  struct _u_endpoint          * default_endpoint;
  struct _u_map               * default_headers;
  size_t                        max_post_param_size;
  size_t                        max_post_body_size;
  void                        * websocket_handler;
  int                        (* file_upload_callback) (const struct _u_request * request, 
                                                       const char * key, 
                                                       const char * filename, 
                                                       const char * content_type, 
                                                       const char * transfer_encoding, 
                                                       const char * data, 
                                                       uint64_t off, 
                                                       size_t size, 
                                                       void * cls);
  void                        * file_upload_cls;
};

/**
 * Structures used to facilitate data manipulations (internal)
 */
struct connection_info_struct {
  struct _u_instance       * u_instance;
  struct MHD_PostProcessor * post_processor;
  int                        has_post_processor;
  int                        callback_first_iteration;
  struct _u_request        * request;
  size_t                     max_post_param_size;
  struct _u_map              map_url_initial;
};

/**********************************
 * Instance functions declarations
 **********************************/

/**
 * free data allocated by ulfius functions
 */
void u_free(void * data);

/**
 * ulfius_init_instance
 * 
 * Initialize a struct _u_instance * with default values
 * return U_OK on success
 */
int ulfius_init_instance(struct _u_instance * u_instance, unsigned int port, struct sockaddr_in * bind_address, const char * default_auth_realm);

/**
 * ulfius_clean_instance
 * 
 * Clean memory allocated by a struct _u_instance *
 */
void ulfius_clean_instance(struct _u_instance * u_instance);

/**
 * ulfius_start_framework
 * Initializes the framework and run the webservice based on the parameters given
 * 
 * u_instance:    pointer to a struct _u_instance that describe its port and bind address
 * return U_OK on success
 */
int ulfius_start_framework(struct _u_instance * u_instance);

/**
 * ulfius_start_secure_framework
 * Initializes the framework and run the webservice based on the parameters given using an HTTPS connection
 * 
 * u_instance:    pointer to a struct _u_instance that describe its port and bind address
 * key_pem:       private key for the server
 * cert_pem:      server certificate
 * return U_OK on success
 */
int ulfius_start_secure_framework(struct _u_instance * u_instance, const char * key_pem, const char * cert_pem);

/**
 * ulfius_stop_framework
 * 
 * Stop the webservice
 * u_instance:    pointer to a struct _u_instance that describe its port and bind address
 * return U_OK on success
 */
int ulfius_stop_framework(struct _u_instance * u_instance);

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
                                             void * cls);

/***********************************
 * Endpoints functions declarations
 ***********************************/

/**
 * Add a struct _u_endpoint * to the specified u_instance
 * Can be done during the execution of the webservice for injection
 * u_instance: pointer to a struct _u_instance that describe its port and bind address
 * u_endpoint: pointer to a struct _u_endpoint that will be copied in the u_instance endpoint_list
 * return U_OK on success
 */
int ulfius_add_endpoint(struct _u_instance * u_instance, const struct _u_endpoint * u_endpoint);

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
                               void * user_data);

/**
 * Add a struct _u_endpoint * list to the specified u_instance
 * Can be done during the execution of the webservice for injection
 * u_instance: pointer to a struct _u_instance that describe its port and bind address
 * u_endpoint_list: pointer to an array of struct _u_endpoint ending with a ulfius_empty_endpoint() that will be copied in the u_instance endpoint_list
 * return U_OK on success
 */
int ulfius_add_endpoint_list(struct _u_instance * u_instance, const struct _u_endpoint ** u_endpoint_list);

/**
 * Remove a struct _u_endpoint * from the specified u_instance
 * Can be done during the execution of the webservice for injection
 * u_instance: pointer to a struct _u_instance that describe its port and bind address
 * u_endpoint: pointer to a struct _u_endpoint that will be removed in the u_instance endpoint_list
 * The parameters _u_endpoint.http_method, _u_endpoint.url_prefix and _u_endpoint.url_format are strictly compared for the match
 * If no endpoint is found, return U_ERROR_NOT_FOUND
 * return U_OK on success
 */
int ulfius_remove_endpoint(struct _u_instance * u_instance, const struct _u_endpoint * u_endpoint);

/**
 * ulfius_set_default_endpoint
 * Set the default endpoint
 * This endpoint will be called if no endpoint match the url called
 * u_instance: pointer to a struct _u_instance that describe its port and bind address
 * auth_function:     a pointer to a function that will be executed prior to the callback for authentication
 *                    you must declare the function as described.
 * auth_data:         a pointer to a data or a structure that will be available in auth_function
 * auth_realm:        realm value for authentication
 * callback_function: a pointer to a function that will be executed each time the endpoint is called
 *                    you must declare the function as described.
 * user_data:         a pointer to a data or a structure that will be available in callback_function
 * to remove a default endpoint, call ulfius_set_default_endpoint with NULL parameter for callback_function
 * return U_OK on success
 */
int ulfius_set_default_endpoint(struct _u_instance * u_instance,
                                         int (* callback_function)(const struct _u_request * request, struct _u_response * response, void * user_data),
                                         void * user_data);

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
int ulfius_remove_endpoint_by_val(struct _u_instance * u_instance, const char * http_method, const char * url_prefix, const char * url_format);

/**
 * ulfius_empty_endpoint
 * return an empty endpoint that goes at the end of an endpoint list
 */
const struct _u_endpoint * ulfius_empty_endpoint();

/**
 * ulfius_copy_endpoint
 * return a copy of an endpoint with duplicate values
 */
int ulfius_copy_endpoint(struct _u_endpoint * dest, const struct _u_endpoint * source);

/**
 * u_copy_endpoint_list
 * return a copy of an endpoint list with duplicate values
 * returned value must be free'd after use
 */
struct _u_endpoint * ulfius_duplicate_endpoint_list(const struct _u_endpoint * endpoint_list);

/**
 * ulfius_clean_endpoint
 * free allocated memory by an endpoint
 */
void ulfius_clean_endpoint(struct _u_endpoint * endpoint);

/**
 * ulfius_clean_endpoint_list
 * free allocated memory by an endpoint list
 */
void ulfius_clean_endpoint_list(struct _u_endpoint * endpoint_list);

/**
 * ulfius_equals_endpoints
 * Compare 2 endpoints and return true if their method, prefix and format are the same or if both are NULL
 */
int ulfius_equals_endpoints(const struct _u_endpoint * endpoint1, const struct _u_endpoint * endpoint2);

#ifndef U_DISABLE_CURL
/********************************************
 * Requests/Responses functions declarations
 ********************************************/

/**
 * ulfius_send_http_request
 * Send a HTTP request and store the result into a _u_response
 * return U_OK on success
 */
int ulfius_send_http_request(const struct _u_request * request, struct _u_response * response);

/**
 * ulfius_send_http_streaming_request
 * Send a HTTP request and store the result into a _u_response
 * Except for the body which will be available using write_body_function in the write_body_data
 * return U_OK on success
 */
int ulfius_send_http_streaming_request(const struct _u_request * request, struct _u_response * response, size_t (* write_body_function)(void * contents, size_t size, size_t nmemb, void * user_data), void * write_body_data);

/**
 * ulfius_send_smtp_email
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
                            const char * mail_body);
#endif

/**
 * ulfius_add_cookie_to_header
 * add a cookie to the cookie map
 * return U_OK on success
 */
int ulfius_add_cookie_to_response(struct _u_response * response, const char * key, const char * value, const char * expires, const unsigned int max_age, 
                      const char * domain, const char * path, const int secure, const int http_only);

/**
 * ulfius_add_header_to_response
 * add a header to the response
 * return U_OK on success
 */
int ulfius_add_header_to_response(struct _u_response * response, const char * key, const char * value);

/**
 * ulfius_set_string_body_response
 * Add a string body to a response
 * body must end with a '\0' character
 * return U_OK on success
 */
int ulfius_set_string_body_response(struct _u_response * response, const unsigned int status, const char * body);

/**
 * ulfius_set_binary_body_response
 * Add a binary body to a response
 * return U_OK on success
 */
int ulfius_set_binary_body_response(struct _u_response * response, const unsigned int status, const char * body, const size_t length);

/**
 * ulfius_set_empty_body_response
 * Set an empty response with only a status
 * return U_OK on success
 */
int ulfius_set_empty_body_response(struct _u_response * response, const unsigned int status);

/**
 * ulfius_set_stream_response
 * Set an stream response with a status
 * return U_OK on success
 */
int ulfius_set_stream_response(struct _u_response * response, 
                                const unsigned int status,
                                ssize_t (* stream_callback) (void * stream_user_data, uint64_t offset, char * out_buf, size_t max),
                                void (* stream_callback_free) (void * stream_user_data),
                                uint64_t stream_size,
                                size_t stream_block_size,
                                void * stream_user_data);

/**
 * ulfius_init_request
 * Initialize a request structure by allocating inner elements
 * return U_OK on success
 */
int ulfius_init_request(struct _u_request * request);

/**
 * ulfius_clean_request
 * clean the specified request's inner elements
 * user must free the parent pointer if needed after clean
 * or use ulfius_clean_request_full
 * return U_OK on success
 */
int ulfius_clean_request(struct _u_request * request);

/**
 * ulfius_clean_request_full
 * clean the specified request and all its elements
 * return U_OK on success
 */
int ulfius_clean_request_full(struct _u_request * request);

/**
 * ulfius_init_response
 * Initialize a response structure by allocating inner elements
 * return U_OK on success
 */
int ulfius_init_response(struct _u_response * response);

/**
 * ulfius_clean_response
 * clean the specified response's inner elements
 * user must free the parent pointer if needed after clean
 * or use ulfius_clean_response_full
 * return U_OK on success
 */
int ulfius_clean_response(struct _u_response * response);

/**
 * ulfius_clean_response_full
 * clean the specified response and all its elements
 * return U_OK on success
 */
int ulfius_clean_response_full(struct _u_response * response);

/**
 * ulfius_copy_response
 * Copy the source response elements into the des response
 * return U_OK on success
 */
int ulfius_copy_response(struct _u_response * dest, const struct _u_response * source);

/**
 * ulfius_clean_cookie
 * clean the cookie's elements
 * return U_OK on success
 */
int ulfius_clean_cookie(struct _u_cookie * cookie);

/**
 * Copy the cookie source elements into dest elements
 * return U_OK on success
 */
int ulfius_copy_cookie(struct _u_cookie * dest, const struct _u_cookie * source);

/**
 * create a new request based on the source elements
 * returned value must be cleaned after use
 */
struct _u_request * ulfius_duplicate_request(const struct _u_request * request);

/**
 * create a new response based on the source elements
 * return value must be cleaned after use
 */
struct _u_response * ulfius_duplicate_response(const struct _u_response * response);

#ifndef U_DISABLE_JANSSON
/**
 * ulfius_get_json_body_request
 * Get JSON structure from the request body if the request is valid
 */
json_t * ulfius_get_json_body_request(const struct _u_request * request, json_error_t * json_error);

/**
 * ulfius_set_json_body_request
 * Add a json_t j_body to a request
 * return U_OK on success
 */
int ulfius_set_json_body_request(struct _u_request * request, json_t * j_body);

/**
 * ulfius_set_json_body_response
 * Add a json_t j_body to a response
 * return U_OK on success
 */
int ulfius_set_json_body_response(struct _u_response * response, const unsigned int status, const json_t * j_body);

/**
 * ulfius_get_json_body_response
 * Get JSON structure from the response body if the request is valid
 */
json_t * ulfius_get_json_body_response(struct _u_response * response, json_error_t * json_error);
#endif

/************************************************************************
 * _u_map declarations                                                  *  
 * _u_map is a simple map structure that handles sets of key/value maps *
 ************************************************************************/

/**
 * initialize a struct _u_map
 * this function MUST be called after a declaration or allocation
 * return U_OK on success
 */
int u_map_init(struct _u_map * map);

/**
 * free the struct _u_map's inner components
 * return U_OK on success
 */
int u_map_clean(struct _u_map * u_map);

/**
 * free the struct _u_map and its components
 * return U_OK on success
 */
int u_map_clean_full(struct _u_map * u_map);

/**
 * free an enum return by functions u_map_enum_keys or u_map_enum_values
 * return U_OK on success
 */
int u_map_clean_enum(char ** array);

/**
 * returns an array containing all the keys in the struct _u_map
 * return an array of char * ending with a NULL element
 */
const char ** u_map_enum_keys(const struct _u_map * u_map);

/**
 * returns an array containing all the values in the struct _u_map
 * return an array of char * ending with a NULL element
 */
const char ** u_map_enum_values(const struct _u_map * u_map);

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
 * return true if the sprcified u_map contains the specified value up until the specified length
 * false otherwise
 * search is case sensitive
 */
int u_map_has_value_binary(const struct _u_map * u_map, const char * value, size_t length);

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
 * add the specified key/value pair into the specified u_map
 * if the u_map already contains a pair with the same key, replace the value
 * return U_OK on success
 */
int u_map_put(struct _u_map * u_map, const char * key, const char * value);

/**
 * add the specified key/binary value pair into the specified u_map
 * if the u_map already contains a pair with the same key,
 * replace the value at the specified offset with the specified length
 * return U_OK on success
 */
int u_map_put_binary(struct _u_map * u_map, const char * key, const char * value, uint64_t offset, size_t length);

/**
 * get the value length corresponding to the specified key in the u_map
 * return -1 if no match found
 * search is case sensitive
 */
ssize_t u_map_get_length(const struct _u_map * u_map, const char * key);

/**
 * get the value length corresponding to the specified key in the u_map
 * return -1 if no match found
 * search is case insensitive
 */
ssize_t u_map_get_case_length(const struct _u_map * u_map, const char * key);

/**
 * get the value corresponding to the specified key in the u_map
 * return NULL if no match found
 * search is case sensitive
 */
const char * u_map_get(const struct _u_map * u_map, const char * key);

/**
 * get the value corresponding to the specified key in the u_map
 * return NULL if no match found
 * search is case insensitive
 */
const char * u_map_get_case(const struct _u_map * u_map, const char * key);

/**
 * remove an pair key/value that has the specified key
 * return U_OK on success, U_NOT_FOUND if key was not found, error otherwise
 */
int u_map_remove_from_key(struct _u_map * u_map, const char * key);

/**
 * remove all pairs key/value that has the specified key (case insensitive search)
 * return U_OK on success, U_NOT_FOUND if key was not found, error otherwise
 */
int u_map_remove_from_key_case(struct _u_map * u_map, const char * key);

/**
 * remove all pairs key/value that has the specified value
 * return U_OK on success, U_NOT_FOUND if key was not found, error otherwise
 */
int u_map_remove_from_value(struct _u_map * u_map, const char * value);

/**
 * remove all pairs key/value that has the specified value (case insensitive search)
 * return U_OK on success, U_NOT_FOUND if key was not found, error otherwise
 */
int u_map_remove_from_value_case(struct _u_map * u_map, const char * value);

/**
 * remove all pairs key/value that has the specified value up until the specified length
 * return U_OK on success, U_NOT_FOUND if key was not found, error otherwise
 */
int u_map_remove_from_value_binary(struct _u_map * u_map, const char * key, size_t length);

/**
 * remove the pair key/value at the specified index
 * return U_OK on success, U_NOT_FOUND if index is out of bound, error otherwise
 */
int u_map_remove_at(struct _u_map * u_map, const int index);

/**
 * Create an exact copy of the specified struct _u_map
 * return a reference to the copy, NULL otherwise
 * returned value must be free'd after use
 */
struct _u_map * u_map_copy(const struct _u_map * source);

/**
 * Copy all key/values pairs of source into dest
 * If key is already present in dest, it's overwritten
 * return U_OK on success, error otherwise
 */
int u_map_copy_into(struct _u_map * dest, const struct _u_map * source);

/**
 * Return the number of key/values pair in the specified struct _u_map
 * Return -1 on error
 */
int u_map_count(const struct _u_map * source);

/**
 * Empty a struct u_map of all its elements
 * return U_OK on success, error otherwise
 */
int u_map_empty(struct _u_map * u_map);

#ifndef U_DISABLE_WEBSOCKET
/**********************************
 * Websocket functions declarations
 **********************************/

#define U_WEBSOCKET_MAGIC_STRING     "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"
#define U_WEBSOCKET_UPGRADE_VALUE    "websocket"
#define U_WEBSOCKET_BAD_REQUEST_BODY "Error in websocket handshake, wrong parameters"
#define U_WEBSOCKET_USEC_WAIT        50
#define WEBSOCKET_MAX_CLOSE_TRY      10

#define U_WEBSOCKET_BIT_FIN         0x80
#define U_WEBSOCKET_HAS_MASK        0x80
#define U_WEBSOCKET_LEN_MASK        0x7F
#define U_WEBSOCKET_OPCODE_CONTINUE 0x00
#define U_WEBSOCKET_OPCODE_TEXT     0x01
#define U_WEBSOCKET_OPCODE_BINARY   0x01
#define U_WEBSOCKET_OPCODE_CLOSE    0x08
#define U_WEBSOCKET_OPCODE_PING     0x09
#define U_WEBSOCKET_OPCODE_PONG     0x0A
#define U_WEBSOCKET_OPCODE_CLOSED   0xFD
#define U_WEBSOCKET_OPCODE_ERROR    0xFE
#define U_WEBSOCKET_OPCODE_NONE     0xFF

/**
 * Websocket manager structure
 * contains among other things the socket
 * the status (open, closed), and the list of incoming and outcoming messages
 * Used on public callback functions
 */
struct _websocket_manager {
  struct _websocket_message_list * message_list_incoming;
  struct _websocket_message_list * message_list_outcoming;
  int connected;
  int closing;
  int manager_closed;
  MHD_socket sock;
  pthread_mutex_t read_lock;
  pthread_mutex_t write_lock;
  struct pollfd fds;
};

/**
 * websocket message structure
 * contains all the data of a websocket message
 * and the timestamp of when it was sent of received
 */
struct _websocket_message {
  time_t datestamp;
  uint8_t opcode;
  uint8_t has_mask;
  uint8_t mask[4];
  uint64_t data_len;
  char * data;
};

struct _websocket_message_list {
  struct _websocket_message ** list;
  size_t len;
};

/**
 * websocket structure
 * contains all the data of the websocket
 */
struct _websocket {
  struct _u_instance               * instance;
  struct _u_request                * request;
  char                             * websocket_protocol;
  char                             * websocket_extensions;
  void                             (* websocket_manager_callback) (const struct _u_request * request,
                                                                  struct _websocket_manager * websocket_manager,
                                                                  void * websocket_manager_user_data);
  void                             * websocket_manager_user_data;
  void                             (* websocket_incoming_message_callback) (const struct _u_request * request,
                                                                           struct _websocket_manager * websocket_manager,
                                                                           const struct _websocket_message * message,
                                                                           void * websocket_incoming_user_data);
  void                             * websocket_incoming_user_data;
  void                             (* websocket_onclose_callback) (const struct _u_request * request,
                                                                  struct _websocket_manager * websocket_manager,
                                                                  void * websocket_onclose_user_data);
  void                             * websocket_onclose_user_data;
  int                                tls;
  struct _websocket_manager        * websocket_manager;
  struct MHD_UpgradeResponseHandle * urh;
};

/**
 * Set a websocket in the response
 * You must set at least websocket_manager_callback or websocket_incoming_message_callback
 * @Parameters
 * response: struct _u_response to send back the websocket initialization, mandatory
 * websocket_protocol: list of protocols, separated by a comma, or NULL if all protocols are accepted
 * websocket_extensions: list of extensions, separated by a comma, or NULL if all extensions are accepted
 * websocket_manager_callback: callback function called right after the handshake acceptance, optional
 * websocket_manager_user_data: any data that will be given to the websocket_manager_callback, optional
 * websocket_incoming_message_callback: callback function called on each incoming complete message, optional
 * websocket_incoming_user_data: any data that will be given to the websocket_incoming_message_callback, optional
 * websocket_onclose_callback: callback function called right before closing the websocket, must be complete for the websocket to close
 * websocket_onclose_user_data: any data that will be given to the websocket_onclose_callback, optional
 * @Return value: U_OK on success
 */
int ulfius_set_websocket_response(struct _u_response * response,
                                   const char * websocket_protocol,
                                   const char * websocket_extensions, 
                                   void (* websocket_manager_callback) (const struct _u_request * request,
                                                                        struct _websocket_manager * websocket_manager,
                                                                        void * websocket_manager_user_data),
                                   void * websocket_manager_user_data,
                                   void (* websocket_incoming_message_callback) (const struct _u_request * request,
                                                                                 struct _websocket_manager * websocket_manager,
                                                                                 const struct _websocket_message * message,
                                                                                 void * websocket_incoming_user_data),
                                   void * websocket_incoming_user_data,
                                   void (* websocket_onclose_callback) (const struct _u_request * request,
                                                                        struct _websocket_manager * websocket_manager,
                                                                        void * websocket_onclose_user_data),
                                   void * websocket_onclose_user_data);

/**
 * Send a message in the websocket
 * Return U_OK on success
 */
int ulfius_websocket_send_message(struct _websocket_manager * websocket_manager,
                                  const uint8_t opcode,
                                  const uint64_t data_len,
                                  const char * data);

/**
 * Return the first message of the message list
 * Return NULL if message_list has no message
 * Returned value must be cleared after use
 */
struct _websocket_message * ulfius_websocket_pop_first_message(struct _websocket_message_list * message_list);

/**
 * Clear data of a websocket message
 */
void ulfius_clear_websocket_message(struct _websocket_message * message);

#endif

/** Macro values **/
#define ULFIUS_URL_SEPARATOR       "/"
#define ULFIUS_HTTP_ENCODING_JSON  "application/json"
#define ULFIUS_HTTP_HEADER_CONTENT "Content-Type"
#define ULFIUS_HTTP_NOT_FOUND_BODY "Resource not found"
#define ULFIUS_HTTP_ERROR_BODY     "Server Error"

#define ULFIUS_COOKIE_ATTRIBUTE_EXPIRES  "Expires"
#define ULFIUS_COOKIE_ATTRIBUTE_MAX_AGE  "Max-Age"
#define ULFIUS_COOKIE_ATTRIBUTE_DOMAIN   "Domain"
#define ULFIUS_COOKIE_ATTRIBUTE_PATH     "Path"
#define ULFIUS_COOKIE_ATTRIBUTE_SECURE   "Secure"
#define ULFIUS_COOKIE_ATTRIBUTE_HTTPONLY "HttpOnly"

#define ULFIUS_POSTBUFFERSIZE 1024

#define U_STATUS_STOP     0
#define U_STATUS_RUNNING  1
#define U_STATUS_ERROR    2

#ifndef U_DISABLE_WEBSOCKET

/**
 * websocket_manager_callback:          callback function for working with the websocket
 * websocket_manager_user_data:         user-defined data that will be handled to websocket_manager_callback
 * websocket_incoming_message_callback: callback function that will be called every time a message arrives from the client in the websocket
 * websocket_incoming_user_data:        user-defined data that will be handled to websocket_incoming_message_callback
 * websocket_onclose_callback:          callback function that will be called if the websocket is open while the program calls ulfius_stop_framework
 * websocket_onclose_user_data:         user-defined data that will be handled to websocket_onclose_callback
 */
struct _websocket_handle {
  char             * websocket_protocol;
  char             * websocket_extensions;
  void            (* websocket_manager_callback) (const struct _u_request * request,
                                                  struct _websocket_manager * websocket_manager,
                                                  void * websocket_manager_user_data);
  void             * websocket_manager_user_data;
  void            (* websocket_incoming_message_callback) (const struct _u_request * request,
                                                           struct _websocket_manager * websocket_manager,
                                                           const struct _websocket_message * message,
                                                           void * websocket_incoming_user_data);
  void             * websocket_incoming_user_data;
  void            (* websocket_onclose_callback) (const struct _u_request * request,
                                                  struct _websocket_manager * websocket_manager,
                                                  void * websocket_onclose_user_data);
  void             * websocket_onclose_user_data;
};

struct _websocket_handler {
  size_t                        nb_websocket_active;
  struct _websocket          ** websocket_active;
  pthread_mutex_t               websocket_close_lock;
  pthread_cond_t                websocket_close_cond;
  int                           pthread_init;
};

#endif // U_DISABLE_WEBSOCKET

#endif // __ULFIUS_H__
