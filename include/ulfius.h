/**
 * 
 * @file ulfius.h
 * @brief Ulfius framework
 * 
 * REST framework library
 * 
 * ulfius.h: public structures and functions declarations
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

#ifndef __ULFIUS_H__
#define __ULFIUS_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "ulfius-cfg.h"

/** External dependencies **/

#ifndef U_DISABLE_GNUTLS
  #ifndef _GNU_SOURCE
    #define _GNU_SOURCE
  #endif
  #include <gnutls/gnutls.h>
  #include <gnutls/x509.h>
#endif

#ifndef U_DISABLE_WEBSOCKET
  #include <poll.h>
  #ifndef POLLRDHUP
    #define POLLRDHUP 0x2000
  #endif
#endif

#include <pthread.h>
#include <microhttpd.h>

#if defined(_WIN32) && !defined(U_DISABLE_WEBSOCKET)
  #define U_DISABLE_WEBSOCKET
#endif

#if (MHD_VERSION < 0x00095300) && !defined(U_DISABLE_WEBSOCKET)
  #define U_DISABLE_WEBSOCKET
#endif

/** Angharad libraries **/
#include <orcania.h>

/** To disable all yder log messages, this flag must be enabled **/
#ifndef U_DISABLE_YDER
  #include <yder.h>
#else

#define Y_LOG_MODE_NONE     0
#define Y_LOG_MODE_CONSOLE  0
#define Y_LOG_MODE_SYSLOG   0
#define Y_LOG_MODE_FILE     0
#define Y_LOG_MODE_JOURNALD 0
#define Y_LOG_MODE_CALLBACK 0
#define Y_LOG_MODE_CURRENT  0

#define Y_LOG_LEVEL_NONE    0
#define Y_LOG_LEVEL_DEBUG   0
#define Y_LOG_LEVEL_INFO    0
#define Y_LOG_LEVEL_WARNING 0
#define Y_LOG_LEVEL_ERROR   0
#define Y_LOG_LEVEL_CURRENT 0

int y_init_logs(const char * app, const unsigned long init_mode, const unsigned long init_level, const char * init_log_file, const char * message);
int y_set_logs_callback(void (* y_callback_log_message) (void * cls, const char * app_name, const time_t date, const unsigned long level, const char * message), void * cls, const char * message);
void y_log_message(const unsigned long type, const char * message, ...);
int y_close_logs();
#endif

#ifndef U_DISABLE_JANSSON
#include <jansson.h>
#endif

/**
 * @defgroup const Constants
 * @{
 */

#define ULFIUS_STREAM_BLOCK_SIZE_DEFAULT 1024
#define U_STREAM_END MHD_CONTENT_READER_END_OF_STREAM
#define U_STREAM_ERROR MHD_CONTENT_READER_END_WITH_ERROR
#define U_STREAM_SIZE_UNKOWN MHD_SIZE_UNKNOWN

#define U_OK                 0 ///< No error
#define U_ERROR              1 ///< Error
#define U_ERROR_MEMORY       2 ///< Error in memory allocation
#define U_ERROR_PARAMS       3 ///< Error in input parameters
#define U_ERROR_LIBMHD       4 ///< Error in libmicrohttpd execution
#define U_ERROR_LIBCURL      5 ///< Error in libcurl execution
#define U_ERROR_NOT_FOUND    6 ///< Something was not found
#define U_ERROR_DISCONNECTED 7 ///< Connection closed

#define U_CALLBACK_CONTINUE     0 ///< Callback exited with success, continue to next callback
#define U_CALLBACK_COMPLETE     1 ///< Callback exited with success, exit callback list
#define U_CALLBACK_UNAUTHORIZED 2 ///< Request is unauthorized, exit callback list and return status 401
#define U_CALLBACK_ERROR        3 ///< Error during request process, exit callback list and return status 500

#define U_COOKIE_SAME_SITE_NONE   0 ///< Set same_site cookie property to 0
#define U_COOKIE_SAME_SITE_STRICT 1 ///< Set same_site cookie property to strict
#define U_COOKIE_SAME_SITE_LAX    2 ///< Set same_site cookie property to lax

#define U_USE_IPV4 0x0001 ///< Use instance in IPV4 mode only
#define U_USE_IPV6 0x0010 ///< Use instance in IPV6 mode only
#define U_USE_ALL (U_USE_IPV4|U_USE_IPV6) ///< Use instance in both IPV4 and IPV6 mode

#define U_SSL_VERIFY_PEER     0x0001 ///< Verify TLS session with peers
#define U_SSL_VERIFY_HOSTNAME 0x0010 ///< Verify TLS session with hostname

/**
 * Options available to set or get properties using
 * ulfius_set_request_properties or ulfius_set_request_properties
 */
typedef enum {
  U_OPT_NONE                          = 0, ///< Empty option to complete a ulfius_set_request_properties or ulfius_set_request_properties
  U_OPT_HTTP_VERB                     = 1, ///< http method (GET, POST, PUT, DELETE, etc.), expected option value type: const char *
  U_OPT_HTTP_URL                      = 2, ///< full url used to call this callback function or full url to call when used in a ulfius_send_http_request, expected option value type: const char *
  U_OPT_HTTP_PROXY                    = 3, ///< proxy address to use for outgoing connections, used by ulfius_send_http_request, expected option value type: const char *
#if MHD_VERSION >= 0x00095208
  U_OPT_NETWORK_TYPE                  = 4, ///< Force connect to ipv4, ipv6 addresses or both, values available are U_USE_ALL, U_USE_IPV4 or U_USE_IPV6, expected option value type: unsigned short
#endif
  U_OPT_CHECK_SERVER_CERTIFICATE      = 5, ///< check server certificate and hostname, default true, used by ulfius_send_http_request, expected option value type: int
  U_OPT_CHECK_SERVER_CERTIFICATE_FLAG = 6, ///< check certificate peer and or server hostname if check_server_certificate is enabled, values available are U_SSL_VERIFY_PEER, U_SSL_VERIFY_HOSTNAME or both, default value is both (U_SSL_VERIFY_PEER|U_SSL_VERIFY_HOSTNAME), used by ulfius_send_http_request, expected option value type: int
  U_OPT_CHECK_PROXY_CERTIFICATE       = 7, ///< check proxy certificate and hostname, default true, used by ulfius_send_http_request, requires libcurl >= 7.52, expected option value type: int
  U_OPT_CHECK_PROXY_CERTIFICATE_FLAG  = 8, ///< check certificate peer and or proxy hostname if check_proxy_certificate is enabled, values available are U_SSL_VERIFY_PEER, U_SSL_VERIFY_HOSTNAME or both, default value is both (U_SSL_VERIFY_PEER|U_SSL_VERIFY_HOSTNAME), used by ulfius_send_http_request, requires libcurl >= 7.52, expected option value type: int
  U_OPT_FOLLOW_REDIRECT               = 9, ///< follow url redirections, used by ulfius_send_http_request, expected option value type: int
  U_OPT_CA_PATH                       = 10, ///< specify a path to CA certificates instead of system path, used by ulfius_send_http_request, expected option value type: const char *
  U_OPT_TIMEOUT                       = 11, ///< connection timeout used by ulfius_send_http_request, default is 0 _or_ Timeout in seconds to close the connection because of inactivity between the client and the server, expected option value type: unsigned long
  U_OPT_AUTH_BASIC_USER               = 12, ///< basic authentication username, expected option value type: const char *
  U_OPT_AUTH_BASIC_PASSWORD           = 13, ///< basic authentication password, expected option value type: const char *
  U_OPT_URL_PARAMETER                 = 14, ///< Add to the map containing the url variables, both from the route and the ?key=value variables, expected option value type: const char *, const char *
  U_OPT_HEADER_PARAMETER              = 15, ///< Add to the map containing the header variables, expected option value type: const char *, const char *
  U_OPT_COOKIE_PARAMETER              = 16, ///< Add to the map containing the cookie variables, expected option value type: const char *, const char *
  U_OPT_POST_BODY_PARAMETER           = 17, ///< Add to the map containing the post body variables (if available), expected option value type: const char *, const char *
  U_OPT_URL_PARAMETER_REMOVE          = 18, ///< Remove from the map containing the url variables, both from the route and the ?key=value variables, expected option value type: const char *
  U_OPT_HEADER_PARAMETER_REMOVE       = 19, ///< Remove from map containing the header variables, expected option value type: const char *
  U_OPT_COOKIE_PARAMETER_REMOVE       = 20, ///< Remove from map containing the cookie variables, expected option value type: const char *
  U_OPT_POST_BODY_PARAMETER_REMOVE    = 21, ///< Remove from map containing the post body variables (if available), expected option value type: const char *
  U_OPT_BINARY_BODY                   = 22, ///< Set a raw body to the request or the reponse, expected option value type: const char *, size_t
  U_OPT_STRING_BODY                   = 23, ///< Set a char * body to the request or the reponse, expected option value type: const char *
#ifndef U_DISABLE_JANSSON
  U_OPT_JSON_BODY                     = 24, ///< Set a stringified json_t * body to the request or the reponse, expected option value type: json_t *
#endif
#ifndef U_DISABLE_GNUTLS
  U_OPT_CLIENT_CERT_FILE              = 25, ///< path to client certificate file for sending http requests with certificate authentication, available only if GnuTLS support is enabled, expected option value type: const char *
  U_OPT_CLIENT_KEY_FILE               = 26, ///< path to client key file for sending http requests with certificate authentication, available only if GnuTLS support is enabled, expected option value type: const char *
  U_OPT_CLIENT_KEY_PASSWORD           = 27, ///< password to unlock client key file, available only if GnuTLS support is enabled, expected option value type: const char *
#endif
  U_OPT_STATUS                        = 28, ///< HTTP response status code (200, 404, 500, etc), expected option value type: long
  U_OPT_AUTH_REALM                    = 29, ///< realm to send to the client response on authenticationb failed, expected option value type: const char *
  U_OPT_SHARED_DATA                   = 30  ///< any data shared between callback functions, must be allocated and freed by the callback functions, expected option value type: void *
} u_option;

/**
 * @}
 */

/*************
 * Structures
 *************/

/**
 * @defgroup struct Structures
 * structures definitions
 * @{
 */

/**
 * struct _u_map
 */
struct _u_map {
  int      nb_values; /* !< Values count */
  char  ** keys; /* !< Array of keys */
  char  ** values; /* !< Array of values */
  size_t * lengths; /* !< Lengths of each values */
};

/**
 * struct _u_cookie
 * the structure containing the response cookie parameters
 */
struct _u_cookie {
  char * key; /* !< key if the cookie */
  char * value; /* !< value of the cookie */
  char * expires; /* !< expiration date of the cookie */
  unsigned int   max_age; /* !< duration of the cookie in seconds */
  char * domain; /* !< domain for the cookie */
  char * path; /* !< url path for the cookie */
  int    secure; /* !< flag to set cookie secure or not */
  int    http_only; /* !< flag to set cookie for HTTP connections only or not */
  int    same_site; /* !< flag to set same_site option to the cookie */
};

/**
 * 
 * @struct _u_request request parameters
 * @brief definition of the parameters available in a struct _u_request
 *                                 
 */
struct _u_request {
  char *               http_protocol; /* !< http protocol used (1.0 or 1.1) */
  char *               http_verb; /* !< http method (GET, POST, PUT, DELETE, etc.) */
  char *               http_url; /* !< full url used to call this callback function or full url to call when used in a ulfius_send_http_request */
  char *               url_path; /* !< url path only used to call this callback function (ex, if http_url is /path/?param=1, url_path is /path/) */
  char *               proxy; /* !<proxy address to use for outgoing connections, used by ulfius_send_http_request  */
#if MHD_VERSION >= 0x00095208
  unsigned short       network_type; /* !< Force connect to ipv4, ipv6 addresses or both, values available are U_USE_ALL, U_USE_IPV4 or U_USE_IPV6 */
#endif
  int                  check_server_certificate; /* !< check server certificate and hostname, default true, used by ulfius_send_http_request */
  int                  check_server_certificate_flag; /* !< check certificate peer and or server hostname if check_server_certificate is enabled, values available are U_SSL_VERIFY_PEER, U_SSL_VERIFY_HOSTNAME or both, default value is both (U_SSL_VERIFY_PEER|U_SSL_VERIFY_HOSTNAME), used by ulfius_send_http_request */
  int                  check_proxy_certificate; /* !< check proxy certificate and hostname, default true, used by ulfius_send_http_request, requires libcurl >= 7.52 */
  int                  check_proxy_certificate_flag; /* !< check certificate peer and or proxy hostname if check_proxy_certificate is enabled, values available are U_SSL_VERIFY_PEER, U_SSL_VERIFY_HOSTNAME or both, default value is both (U_SSL_VERIFY_PEER|U_SSL_VERIFY_HOSTNAME), used by ulfius_send_http_request, requires libcurl >= 7.52 */
  int                  follow_redirect; /* !< follow url redirections, used by ulfius_send_http_request */
  char *               ca_path; /* !< specify a path to CA certificates instead of system path, used by ulfius_send_http_request */
  unsigned long        timeout; /* !< connection timeout used by ulfius_send_http_request, default is 0 */
  struct sockaddr *    client_address; /* !< IP address of the client */
  char *               auth_basic_user; /* !< basic authentication username */
  char *               auth_basic_password; /* !< basic authentication password */
  struct _u_map *      map_url; /* !< map containing the url variables, both from the route and the ?key=value variables */
  struct _u_map *      map_header; /* !< map containing the header variables */
  struct _u_map *      map_cookie; /* !< map containing the cookie variables */
  struct _u_map *      map_post_body; /* !< map containing the post body variables (if available) */
  void *               binary_body; /* !< raw body */
  size_t               binary_body_length; /* !< length of raw body */
  unsigned int         callback_position; /* !< position of the current callback function in the callback list, starts at 0 */
#ifndef U_DISABLE_GNUTLS
  gnutls_x509_crt_t    client_cert; /* !< x509 certificate of the client if the instance uses client certificate authentication and the client is authenticated, available only if GnuTLS support is enabled */
  char *               client_cert_file; /* !< path to client certificate file for sending http requests with certificate authentication, available only if GnuTLS support is enabled */
  char *               client_key_file; /* !< path to client key file for sending http requests with certificate authentication, available only if GnuTLS support is enabled */
  char *               client_key_password; /* !< password to unlock client key file, available only if GnuTLS support is enabled */
#endif
};

/**
 * 
 * @struct _u_response response parameters
 * @brief definition of the parameters available in a struct _u_response
 * 
 */
struct _u_response {
  long               status; /* !< HTTP status code (200, 404, 500, etc) */
  char             * protocol; /* !< HTTP Protocol sent */
  struct _u_map    * map_header; /* !< map containing the header variables */
  unsigned int       nb_cookies; /* !< number of cookies sent */
  struct _u_cookie * map_cookie; /* !< array of cookies sent */
  char             * auth_realm; /* !< realm to send to the client on authenticationb failed */
  void             * binary_body; /* !< raw binary content */
  size_t             binary_body_length; /* !< length of the binary_body */
  ssize_t         (* stream_callback) (void * stream_user_data, uint64_t offset, char * out_buf, size_t max); /* !< callback function to stream data in response body */
  void            (* stream_callback_free) (void * stream_user_data); /* !< callback function to free data allocated for streaming */
  uint64_t           stream_size; /* !< size of the streamed data (U_STREAM_SIZE_UNKOWN if unknown) */
  size_t             stream_block_size; /* !< size of each block to be streamed, set according to your system */
  void             * stream_user_data; /* !< user defined data that will be available in your callback stream functions */
  void             * websocket_handle; /* !< handle for websocket extension */
  void *             shared_data; /* !< any data shared between callback functions, must be allocated and freed by the callback functions */
  unsigned int       timeout; /* !< Timeout in seconds to close the connection because of inactivity between the client and the server */
};

/**
 * 
 * @struct _u_endpoint endpoint definition
 * @brief Contains all informations needed for an endpoint
 * 
 */
struct _u_endpoint {
  char       * http_method; /* !< http verb (GET, POST, PUT, etc.) in upper case */
  char       * url_prefix; /* !< prefix for the url (optional) */
  char       * url_format; /* !< string used to define the endpoint format, separate words with / to define a variable in the url, prefix it with @ or :, example: /test/resource/:name/elements, on an url_format that ends with '*', the rest of the url will not be tested */
  unsigned int priority; /* !< endpoint priority in descending order (0 is the higher priority) */
  int       (* callback_function)(const struct _u_request * request, /* !< pointer to a function that will be executed each time the endpoint is called, you must declare the function as described. */
                                  struct _u_response * response,
                                  void * user_data);
  void       * user_data; /* !< pointer to a data or a structure that will be available in callback_function */
};

/**
 * 
 * @struct _u_instance Ulfius instance definition
 * @brief Contains the needed data for an ulfius instance to work
 * 
 */
struct _u_instance {
  struct MHD_Daemon          *  mhd_daemon; /* !< pointer to the libmicrohttpd daemon */
  int                           status; /* !< status of the current instance, status are U_STATUS_STOP, U_STATUS_RUNNING or U_STATUS_ERROR */
  unsigned int                  port; /* !< port number to listen to */
#if MHD_VERSION >= 0x00095208
  unsigned short                network_type; /* !< Listen to ipv4 and or ipv6 connections, values available are U_USE_ALL, U_USE_IPV4 or U_USE_IPV6 */
#endif
  struct sockaddr_in          * bind_address; /* !< ipv4 address to listen to (optional) */
  struct sockaddr_in6         * bind_address6; /* !< ipv6 address to listen to (optional) */
  unsigned int                  timeout; /* !< Timeout to close the connection because of inactivity between the client and the server */
  int                           nb_endpoints; /* !< Number of available endpoints */
  char                        * default_auth_realm; /* !< Default realm on authentication error */
  struct _u_endpoint          * endpoint_list; /* !< List of available endpoints */
  struct _u_endpoint          * default_endpoint; /* !< Default endpoint if no other endpoint match the current url */
  struct _u_map               * default_headers; /* !< Default headers that will be added to all response->map_header */
  size_t                        max_post_param_size; /* !< maximum size for a post parameter, 0 means no limit, default 0 */
  size_t                        max_post_body_size; /* !< maximum size for the entire post body, 0 means no limit, default 0 */
  void                        * websocket_handler; /* !< handler for the websocket structure */
  int                        (* file_upload_callback) (const struct _u_request * request,  /* !< callback function to manage file upload by blocks */
                                                       const char * key, 
                                                       const char * filename, 
                                                       const char * content_type, 
                                                       const char * transfer_encoding, 
                                                       const char * data, 
                                                       uint64_t off, 
                                                       size_t size, 
                                                       void * cls);
  void                        * file_upload_cls; /* !< any pointer to pass to the file_upload_callback function */
  int                           mhd_response_copy_data; /* !< to choose between MHD_RESPMEM_MUST_COPY and MHD_RESPMEM_MUST_FREE, only if you use MHD < 0.9.61, otherwise this option is skipped because it's useless */
  int                           check_utf8; /* !< check that all parameters values in the request (url, header and post_body), are valid utf8 strings, if a parameter value has non utf8 character, the value, will be ignored, default 1 */
#ifndef U_DISABLE_GNUTLS
  int                           use_client_cert_auth; /* !< Internal variable use to indicate if the instance uses client certificate authentication, Do not change this value, available only if websocket support is enabled */
#endif
};

/**
 * @}
 */

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
 * @defgroup memory
 * memory management function
 * @{
 */

/**
 * free data allocated by ulfius functions
 * @param data data to free
 */
void u_free(void * data);

/**
 * @}
 */

/**
 * @defgroup instance struct _u_instance
 * struct _u_instance management functions
 * @{
 */

/**
 * ulfius_init_instance
 * 
 * Initialize a struct _u_instance * with default values
 * Binds to IPV4 addresses only
 * @param port tcp port to bind to, must be between 1 and 65535
 * @param bind_address IPv4 address to listen to, optional, the reference is borrowed, the structure isn't copied
 * @param default_auth_realm default realm to send to the client on authentication error
 * @return U_OK on success
 */
int ulfius_init_instance(struct _u_instance * u_instance, unsigned int port, struct sockaddr_in * bind_address, const char * default_auth_realm);

#if MHD_VERSION >= 0x00095208
/**
 * ulfius_init_instance_ipv6
 * 
 * Initialize a struct _u_instance * with default values
 * Binds to IPV6 and IPV4 addresses or IPV6 addresses only
 * @param port tcp port to bind to, must be between 1 and 65535
 * @param bind_address IPv6 address to listen to, optional, the reference is borrowed, the structure isn't copied
 * @param network_type Type of network to listen to, values available are U_USE_IPV6 or U_USE_ALL
 * @param default_auth_realm default realm to send to the client on authentication error
 * @return U_OK on success
 */
int ulfius_init_instance_ipv6(struct _u_instance * u_instance, unsigned int port, struct sockaddr_in6 * bind_address, unsigned short network_type, const char * default_auth_realm);
#endif

/**
 * ulfius_clean_instance
 * 
 * Clean memory allocated by a struct _u_instance *
 * @param u_instance an Ulfius instance
 */
void ulfius_clean_instance(struct _u_instance * u_instance);

/**
 * ulfius_start_framework
 * Initializes the framework and run the webservice based on the parameters given
 * 
 * @param u_instance pointer to a struct _u_instance that describe its port and bind address
 * @return U_OK on success
 */
int ulfius_start_framework(struct _u_instance * u_instance);

/**
 * ulfius_start_secure_framework
 * Initializes the framework and run the webservice based on the parameters given using an HTTPS connection
 * 
 * @param u_instance pointer to a struct _u_instance that describe its port and bind address
 * @param key_pem private key for the server
 * @param cert_pem server certificate
 * @return U_OK on success
 */
int ulfius_start_secure_framework(struct _u_instance * u_instance, const char * key_pem, const char * cert_pem);

#ifndef U_DISABLE_GNUTLS
/**
 * ulfius_start_secure_ca_trust_framework
 * Initializes the framework and run the webservice based on the parameters given using an HTTPS connection
 * And using a root server to authenticate client connections
 * 
 * @param u_instance pointer to a struct _u_instance that describe its port and bind address
 * @param key_pem private key for the server
 * @param cert_pem server certificate
 * @param root_ca_pem client root CA you're willing to trust for this instance
 * @return U_OK on success
 */
int ulfius_start_secure_ca_trust_framework(struct _u_instance * u_instance, const char * key_pem, const char * cert_pem, const char * root_ca_pem);
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
 * @param mhd_ops struct MHD_OptionItem * options table, 
 * - MUST contain an option with the fllowing value: {.option = MHD_OPTION_NOTIFY_COMPLETED; .value = (intptr_t)mhd_request_completed; .ptr_value = NULL;}
 * - MUST contain an option with the fllowing value: {.option = MHD_OPTION_URI_LOG_CALLBACK; .value = (intptr_t)ulfius_uri_logger; .ptr_value = NULL;}
 * - MUST end with a terminal struct MHD_OptionItem: {.option = MHD_OPTION_END; .value = 0; .ptr_value = NULL;}
 * @return U_OK on success
 */
int ulfius_start_framework_with_mhd_options(struct _u_instance * u_instance, unsigned int mhd_flags, struct MHD_OptionItem * options);

/**
 * Internal functions externalized to use ulfius_start_framework_with_mhd_options
 */
void mhd_request_completed (void *cls, struct MHD_Connection *connection, void **con_cls, enum MHD_RequestTerminationCode toe);
void * ulfius_uri_logger (void * cls, const char * uri);

/**
 * ulfius_stop_framework
 * 
 * Stop the webservice
 * @param u_instance pointer to a struct _u_instance that describe its port and bind address
 * @return U_OK on success
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
 * @param u_instance pointer to a struct _u_instance that describe its port and bind address
 * @param file_upload_callback Pointer to a callback function that will handle all file uploads
 * @param cls a pointer that will be passed to file_upload_callback each tim it's called
 * @return U_OK on success
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

/**
 * @}
 */

/**
 * @defgroup endpoints struct _u_endpoint
 * struct _u_endpoint management functions
 * @{
 */

/***********************************
 * Endpoints functions declarations
 ***********************************/

/**
 * Add a struct _u_endpoint * to the specified u_instance
 * Can be done during the execution of the webservice for injection
 * @param u_instance pointer to a struct _u_instance that describe its port and bind address
 * @param u_endpoint pointer to a struct _u_endpoint that will be copied in the u_instance endpoint_list
 * @param return U_OK on success
 */
int ulfius_add_endpoint(struct _u_instance * u_instance, const struct _u_endpoint * u_endpoint);

/**
 * Add a struct _u_endpoint * to the specified u_instance with its values specified
 * Can be done during the execution of the webservice for injection
 * @param u_instance pointer to a struct _u_instance that describe its port and bind address
 * @param http_method http verb (GET, POST, PUT, etc.) in upper case
 * @param url_prefix prefix for the url (optional)
 * @param url_format string used to define the endpoint format
 *                    separate words with /
 *                    to define a variable in the url, prefix it with @ or :
 *                    example: /test/resource/:name/elements
 *                    on an url_format that ends with '*', the rest of the url will not be tested
 * @param priority endpoint priority in descending order (0 is the higher priority)
 * @param callback_function a pointer to a function that will be executed each time the endpoint is called
 * you must declare the function as described.
 * @param user_data a pointer to a data or a structure that will be available in callback_function
 * @return U_OK on success
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
 * @param u_instance pointer to a struct _u_instance that describe its port and bind address
 * @param u_endpoint_list pointer to an array of struct _u_endpoint ending with a ulfius_empty_endpoint() that will be copied in the u_instance endpoint_list
 * @return U_OK on success
 */
int ulfius_add_endpoint_list(struct _u_instance * u_instance, const struct _u_endpoint ** u_endpoint_list);

/**
 * Remove a struct _u_endpoint * from the specified u_instance
 * Can be done during the execution of the webservice for injection
 * @param u_instance pointer to a struct _u_instance that describe its port and bind address
 * @param u_endpoint pointer to a struct _u_endpoint that will be removed in the u_instance endpoint_list
 * The parameters _u_endpoint.http_method, _u_endpoint.url_prefix and _u_endpoint.url_format are strictly compared for the match
 * If no endpoint is found, return U_ERROR_NOT_FOUND
 * @return U_OK on success
 */
int ulfius_remove_endpoint(struct _u_instance * u_instance, const struct _u_endpoint * u_endpoint);

/**
 * ulfius_set_default_endpoint
 * Set the default endpoint
 * This endpoint will be called if no endpoint match the url called
 * @param u_instance pointer to a struct _u_instance that describe its port and bind address
 * @param auth_function     a pointer to a function that will be executed prior to the callback for authentication
 *                    you must declare the function as described.
 * @param auth_data a pointer to a data or a structure that will be available in auth_function
 * @param auth_realm realm value for authentication
 * callback_function a pointer to a function that will be executed each time the endpoint is called
 *                    you must declare the function as described.
 * @param user_data a pointer to a data or a structure that will be available in callback_function
 * to remove a default endpoint, call ulfius_set_default_endpoint with NULL parameter for callback_function
 * @return U_OK on success
 */
int ulfius_set_default_endpoint(struct _u_instance * u_instance,
                                         int (* callback_function)(const struct _u_request * request, struct _u_response * response, void * user_data),
                                         void * user_data);

/**
 * Remove a struct _u_endpoint * from the specified u_instance
 * using the specified values used to identify an endpoint
 * Can be done during the execution of the webservice for injection
 * The parameters _u_endpoint.http_method, _u_endpoint.url_prefix and _u_endpoint.url_format are strictly compared for the match
 * If no endpoint is found, return U_ERROR_NOT_FOUND
 * @param u_instance pointer to a struct _u_instance that describe its port and bind address
 * @param http_method http_method used by the endpoint
 * @param url_prefix url_prefix used by the endpoint
 * @param url_format url_format used by the endpoint
 * @return U_OK on success
 */
int ulfius_remove_endpoint_by_val(struct _u_instance * u_instance, const char * http_method, const char * url_prefix, const char * url_format);

/**
 * ulfius_empty_endpoint
 * @return empty endpoint that goes at the end of an endpoint list
 */
const struct _u_endpoint * ulfius_empty_endpoint();

/**
 * ulfius_copy_endpoint
 * makes a copy of an endpoint with duplicate values
 * @param dest the endpoint destination
 * @param source the endpoint source
 * @return U_OK on success
 */
int ulfius_copy_endpoint(struct _u_endpoint * dest, const struct _u_endpoint * source);

/**
 * u_copy_endpoint_list
 * makes a copy of an endpoint list with duplicate values
 * @param endpoint_list an array of struct _u_endpoint * finishing with a ulfius_empty_endpoint()
 * @return a list with duplicate values
 * returned value must be free'd after use
 */
struct _u_endpoint * ulfius_duplicate_endpoint_list(const struct _u_endpoint * endpoint_list);

/**
 * ulfius_clean_endpoint
 * free allocated memory by an endpoint
 * @param endpoint the endpoint to cleanup
 */
void ulfius_clean_endpoint(struct _u_endpoint * endpoint);

/**
 * ulfius_clean_endpoint_list
 * free allocated memory by an endpoint list
 * @param endpoint_list the list of endpoints to cleanup, finishing with a ulfius_empty_endpoint()
 */
void ulfius_clean_endpoint_list(struct _u_endpoint * endpoint_list);

/**
 * ulfius_equals_endpoints
 * Compare 2 endpoints
 * @param endpoint1 the first endpoint to compare
 * @param endpoint2 the second endpoint to compare
 * @return true if their method, prefix and format are the same or if both are NULL, false otherwise
 */
int ulfius_equals_endpoints(const struct _u_endpoint * endpoint1, const struct _u_endpoint * endpoint2);

/**
 * @}
 */

/**
 * @defgroup http_smtp_client Client HTTP and SMTP
 * client HTTP and SMTP requests management functions
 * @{
 */

#ifndef U_DISABLE_CURL
/********************************************
 * Requests/Responses functions declarations
 ********************************************/

/**
 * ulfius_send_http_request
 * Send a HTTP request and store the result into a _u_response
 * @param request the struct _u_request that contains all the input parameters to perform the HTTP request
 * @param response the struct _u_response that will be filled with all response parameter values, optional, may be NULL
 * @return U_OK on success
 */
int ulfius_send_http_request(const struct _u_request * request, struct _u_response * response);

/**
 * ulfius_send_http_streaming_request
 * Send a HTTP request and store the result into a _u_response
 * Except for the body which will be available using write_body_function in the write_body_data
 * @param request the struct _u_request that contains all the input parameters to perform the HTTP request
 * @param response the struct _u_response that will be filled with all response parameter values, optional, may be NULL
 * @param write_body_function a pointer to a function that will be used to receive response body in chunks
 * @param write_body_data a user-defined poitner that will be passed in parameter to write_body_function
 * @return U_OK on success
 */
int ulfius_send_http_streaming_request(const struct _u_request * request, struct _u_response * response, size_t (* write_body_function)(void * contents, size_t size, size_t nmemb, void * user_data), void * write_body_data);

/**
 * ulfius_send_smtp_email
 * Send an email using libcurl
 * email is plain/text and UTF8 charset
 * @param host smtp server host name
 * @param port tcp port number (optional, 0 for default)
 * @param use_tls true if the connection is tls secured
 * @param verify_certificate true if you want to disable the certificate verification on a tls server
 * @param user connection user name (optional, NULL: no user name)
 * @param password connection password (optional, NULL: no password)
 * @param from from address (mandatory)
 * @param to to recipient address (mandatory)
 * @param cc cc recipient address (optional, NULL: no cc)
 * @param bcc bcc recipient address (optional, NULL: no bcc)
 * @param subject email subject (mandatory)
 * @param mail_body email body (mandatory)
 * @return U_OK on success
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

/**
 * Send an email using libcurl
 * email has the content-type specified in parameter
 * @param host smtp server host name
 * @param port tcp port number (optional, 0 for default)
 * @param use_tls true if the connection is tls secured
 * @param verify_certificate true if you want to disable the certificate verification on a tls server
 * @param user connection user name (optional, NULL: no user name)
 * @param password connection password (optional, NULL: no password)
 * @param from from address (mandatory)
 * @param to to recipient address (mandatory)
 * @param cc cc recipient address (optional, NULL: no cc)
 * @param bcc bcc recipient address (optional, NULL: no bcc)
 * @param content_type: content-type to add to the e-mail body
 * @param subject email subject (mandatory)
 * @param mail_body email body (mandatory)
 * @return U_OK on success
 */

int ulfius_send_smtp_rich_email(const char * host, 
                                const int port, 
                                const int use_tls, 
                                const int verify_certificate, 
                                const char * user, 
                                const char * password, 
                                const char * from, 
                                const char * to, 
                                const char * cc, 
                                const char * bcc, 
                                const char * content_type,
                                const char * subject, 
                                const char * mail_body);
#endif

/**
 * @}
 */

/**
 * @defgroup cookie Cookies
 * Cookies management functions
 * @{
 */

/**
 * ulfius_add_cookie_to_response
 * add a cookie to the cookie map
 * @param response the response to add the cookie to
 * @param key the cookie key
 * @param value the cookie value
 * @param expires the expiration date of the ccokie in ISO format (optional)
 * @param max_age the maximum age of the cookie in seconds (optional)
 * @param domain the domain of the cookie (optional)
 * @param pat the path of the cookie (optional)
 * @param secure wether the cookie must be secure or not (optional)
 * @param http_only wether the cookie must be used only for http requests or not (optional)
 * @return U_OK on success
 */
int ulfius_add_cookie_to_response(struct _u_response * response, const char * key, const char * value, const char * expires, const unsigned int max_age, 
                                  const char * domain, const char * path, const int secure, const int http_only);

/**
 * ulfius_add_same_site_cookie_to_response
 * add a cookie to the cookie map with a SameSite attribute
 * @param response the response to add the cookie to
 * @param key the cookie key
 * @param value the cookie value
 * @param expires the expiration date of the ccokie in ISO format (optional)
 * @param max_age the maximum age of the cookie in seconds (optional)
 * @param domain the domain of the cookie (optional)
 * @param pat the path of the cookie (optional)
 * @param secure wether the cookie must be secure or not (optional)
 * @param http_only wether the cookie must be used only for http requests or not (optional)
 * @param same_site parameter must have one of the following values:
 * - U_COOKIE_SAME_SITE_NONE   - No SameSite attribute
 * - U_COOKIE_SAME_SITE_STRICT - SameSite attribute set to 'Strict'
 * - U_COOKIE_SAME_SITE_LAX    - SameSite attribute set to 'Lax'
 * @return U_OK on success
 */
int ulfius_add_same_site_cookie_to_response(struct _u_response * response, const char * key, const char * value, const char * expires, const unsigned int max_age, 
                                            const char * domain, const char * path, const int secure, const int http_only, const int same_site);

/**
 * @}
 */

/**
 * @defgroup parameters URL, POST and Header parameters
 * URL, POST and Header management functions
 * @{
 */

/**
 * ulfius_add_header_to_response
 * add a header to the response
 * @param response the response to be updated
 * @param key the key of the header
 * @param value the value of the header
 * @return U_OK on success
 */
int ulfius_add_header_to_response(struct _u_response * response, const char * key, const char * value);

/**
 * ulfius_set_string_body_request
 * Set a string string_body to a request, replace any existing body in the request
 * @param request the request to be updated
 * @param string_body string to set to the body response must end with a '\0' character
 * @return U_OK on success
 */
int ulfius_set_string_body_request(struct _u_request * request, const char * string_body);

/**
 * ulfius_set_binary_body_request
 * Set a binary binary_body to a request, replace any existing body in the request
 * @param request the request to be updated
 * @param binary_body an array of char to set to the body response
 * @param length the length of binary_body to set to the request body
 * return U_OK on success
 */
int ulfius_set_binary_body_request(struct _u_request * request, const char * binary_body, const size_t length);

/**
 * ulfius_set_empty_body_request
 * Set an empty request body
 * @param request the request to be updated
 * @return U_OK on success
 */
int ulfius_set_empty_body_request(struct _u_request * request);

/**
 * ulfius_set_string_body_response
 * Add a string body to a response, replace any existing body in the response
 * @param response the response to be updated
 * @param status the http status code to set to the response
 * @param body the string body to set, must end with a '\0' character
 * @return U_OK on success
 */
int ulfius_set_string_body_response(struct _u_response * response, const unsigned int status, const char * body);

/**
 * ulfius_set_binary_body_response
 * Add a binary body to a response, replace any existing body in the response
 * @param response the response to be updated
 * @param status the http status code to set to the response
 * @param body the array of char to set
 * @param length the length of body to set to the request body
 * @return U_OK on success
 */
int ulfius_set_binary_body_response(struct _u_response * response, const unsigned int status, const char * body, const size_t length);

/**
 * ulfius_set_empty_body_response
 * Set an empty response with only a status
 * @param response the response to be updated
 * @param status the http status code to set to the response
 * @return U_OK on success
 */
int ulfius_set_empty_body_response(struct _u_response * response, const unsigned int status);

/**
 * @}
 */

/**
 * @defgroup stream Response streaming
 * Response streaming function
 * @{
 */

/**
 * ulfius_set_stream_response
 * Set an stream response with a status
 * @param response the response to be updated
 * @param status the http status code to set to the response
 * @param stream_callback a pointer to a function that will handle the response stream
 * @param stream_callback_free a pointer to a function that will free its allocated resoures during stream_callback
 * @param stream_size size of the streamed data (U_STREAM_SIZE_UNKOWN if unknown)
 * @param stream_block_size preferred size of each stream chunk, may be overwritten by the system if necessary
 * @param stream_user_data a user-defined pointer that will be available in stream_callback and stream_callback_free
 * @return U_OK on success
 */
int ulfius_set_stream_response(struct _u_response * response, 
                                const unsigned int status,
                                ssize_t (* stream_callback) (void * stream_user_data, uint64_t offset, char * out_buf, size_t max),
                                void (* stream_callback_free) (void * stream_user_data),
                                uint64_t stream_size,
                                size_t stream_block_size,
                                void * stream_user_data);

/**
 * @}
 */

/**
 * @defgroup request_response_cookie struct _u_request, struct _u_response and struct _u_cookie
 * struct _u_request, struct _u_response and struct _u_cookie management functions
 * @{
 */

/**
 * ulfius_init_request
 * Initialize a request structure by allocating inner elements
 * @param request the request to initialize
 * @return U_OK on success
 */
int ulfius_init_request(struct _u_request * request);

/**
 * ulfius_clean_request
 * clean the specified request's inner elements
 * user must free the parent pointer if needed after clean
 * or use ulfius_clean_request_full
 * @param request the request to cleanup
 * @return U_OK on success
 */
int ulfius_clean_request(struct _u_request * request);

/**
 * ulfius_clean_request_full
 * clean the specified request and all its elements
 * @param request the request to cleanup
 * @return U_OK on success
 */
int ulfius_clean_request_full(struct _u_request * request);

/**
 * ulfius_copy_request
 * Copy the source request elements into the dest request
 * @param dest the request to receive the copied data
 * @param source the source request to copy
 * @return U_OK on success
 */
int ulfius_copy_request(struct _u_request * dest, const struct _u_request * source);

/**
 * ulfius_set_request_properties
 * Set a list of properties to a request
 * return U_OK on success
 */
int ulfius_set_request_properties(struct _u_request * request, ...);

/**
 * ulfius_init_response
 * Initialize a response structure by allocating inner elements
 * @param response the response to initialize
 * @return U_OK on success
 */
int ulfius_init_response(struct _u_response * response);

/**
 * ulfius_clean_response
 * clean the specified response's inner elements
 * user must free the parent pointer if needed after clean
 * or use ulfius_clean_response_full
 * @param response the response to cleanup
 * @return U_OK on success
 */
int ulfius_clean_response(struct _u_response * response);

/**
 * ulfius_clean_response_full
 * clean the specified response and all its elements
 * @param response the response to cleanup
 * @return U_OK on success
 */
int ulfius_clean_response_full(struct _u_response * response);

/**
 * ulfius_copy_response
 * Copy the source response elements into the dest response
 * @param dest the response to receive the copied data
 * @param source the source response to copy
 * @return U_OK on success
 */
int ulfius_copy_response(struct _u_response * dest, const struct _u_response * source);

/**
 * ulfius_clean_cookie
 * clean the cookie's elements
 * @param cookie the cookie structure to cleanup
 * @return U_OK on success
 */
int ulfius_clean_cookie(struct _u_cookie * cookie);

/**
 * Copy the cookie source elements into dest elements
 * @param dest the cookie to receive the copied data
 * @param source the cookie response to copy
 * @return U_OK on success
 */
int ulfius_copy_cookie(struct _u_cookie * dest, const struct _u_cookie * source);

/**
 * create a new request based on the source elements
 * returned value must be cleaned after use
 * @param request the request to duplicate
 * @return a heap-allocated request
 */
struct _u_request * ulfius_duplicate_request(const struct _u_request * request);

/**
 * create a new response based on the source elements
 * return value must be cleaned after use
 * @param response the response to duplicate
 * @return a heap-allocated response
 */
struct _u_response * ulfius_duplicate_response(const struct _u_response * response);

/**
 * ulfius_set_response_properties
 * Set a list of properties to a response
 * return U_OK on success
 */
int ulfius_set_response_properties(struct _u_response * response, ...);

/**
 * @}
 */

/**
 * @defgroup url_encode URL Encode
 * URL Encode functions
 * @{
 */

/**
 * Returns a url-decoded version of str
 * returned value must be cleaned after use
 * Thanks Geek Hideout!
 * http://www.geekhideout.com/urlcode.shtml
 * @param str the string to decode
 * @return a heap-allocated string
 */
char * ulfius_url_decode(const char * str);

/**
 * Returns a url-encoded version of str
 * returned value must be cleaned after use
 * Thanks Geek Hideout!
 * http://www.geekhideout.com/urlcode.shtml
 * @param str the string to encode
 * @return a heap-allocated string
 */
char * ulfius_url_encode(const char * str);

/**
 * @}
 */

/**
 * @defgroup request_response_cookie struct _u_request, struct _u_response and struct _u_cookie
 * struct _u_request, struct _u_response and struct _u_cookie management functions
 * @{
 */

#ifndef U_DISABLE_JANSSON
/**
 * ulfius_get_json_body_request
 * Get JSON structure from the request body if the request is valid
 * In case of an error in getting or parsing JSON data in the request,
 * the structure json_error_t * json_error will be filled with an error
 * message if json_error is not NULL
 * @param request the request to retrieve the JSON data
 * @param json_error a json_error_t reference that will contain decoding errors if any, may be NULL
 * @return a json_t * containing the JSON decoded, NULL on error
 */
json_t * ulfius_get_json_body_request(const struct _u_request * request, json_error_t * json_error);

/**
 * ulfius_set_json_body_request
 * Add a json_t j_body to a request
 * @param request the request to retrieve the JSON data
 * @param j_body a json_t to stringify in the body
 * @return U_OK on success
 */
int ulfius_set_json_body_request(struct _u_request * request, json_t * j_body);

/**
 * ulfius_get_json_body_response
 * Get JSON structure from the response body if the response is valid
 * In case of an error in getting or parsing JSON data in the response,
 * the structure json_error_t * json_error will be filled with an error
 * message if json_error is not NULL
 * @param response the response to retrieve the JSON data
 * @param json_error a json_error_t reference that will contain decoding errors if any, may be NULL
 * @return a json_t * containing the JSON decoded, NULL on error
 */
json_t * ulfius_get_json_body_response(struct _u_response * response, json_error_t * json_error);

/**
 * ulfius_set_json_body_response
 * Add a json_t j_body to a response
 * @param response the response to retrieve the JSON data
 * @param j_body a json_t to stringify in the body
 * @return U_OK on success
 */
int ulfius_set_json_body_response(struct _u_response * response, const unsigned int status, const json_t * j_body);
#endif

/**
 * @}
 */

/**
 * @defgroup u_map struct _u_map
 * struct _u_map management functions
 * @{
 */

/************************************************************************
 * _u_map declarations                                                  *  
 * _u_map is a simple map structure that handles sets of key/value maps *
 ************************************************************************/

/**
 * initialize a struct _u_map
 * this function MUST be called after a declaration or allocation
 * @param u_map the _u_map to initialize
 * @return U_OK on success
 */
int u_map_init(struct _u_map * u_map);

/**
 * free the struct _u_map's inner components
 * @param u_map the _u_map to cleanup
 * @return U_OK on success
 */
int u_map_clean(struct _u_map * u_map);

/**
 * free the struct _u_map and its components
 * @param u_map the _u_map to cleanup
 * @return U_OK on success
 */
int u_map_clean_full(struct _u_map * u_map);

/**
 * free an enum return by functions u_map_enum_keys or u_map_enum_values
 * @param array the string array to cleanup
 * @return U_OK on success
 */
int u_map_clean_enum(char ** array);

/**
 * returns an array containing all the keys in the struct _u_map
 * @param u_map the _u_map to retreive the keys from
 * @return an array of char * ending with a NULL element
 */
const char ** u_map_enum_keys(const struct _u_map * u_map);

/**
 * returns an array containing all the values in the struct _u_map
 * @param u_map the _u_map to retreive the values from
 * @return an array of char * ending with a NULL element
 */
const char ** u_map_enum_values(const struct _u_map * u_map);

/**
 * Detects if the key exists in the _u_map
 * search is case sensitive
 * @param u_map the _u_map to analyze
 * @param key the key to look for
 * @return true if the sprcified u_map contains the specified key
 * false otherwise
 */
int u_map_has_key(const struct _u_map * u_map, const char * key);

/**
 * Detects if the value exists in the _u_map, value must be a char * string
 * search is case sensitive
 * @param u_map the _u_map to analyze
 * @param value the value to look for
 * @return true if the sprcified u_map contains the specified value
 * false otherwise
 */
int u_map_has_value(const struct _u_map * u_map, const char * value);

/**
 * Detects if the value exists in the _u_map, value may be any byte array
 * search is case sensitive
 * @param u_map the _u_map to analyze
 * @param value the value to look for
 * @param length the length of the value to look for
 * @return true if the sprcified u_map contains the specified value up until the specified length
 * false otherwise
 */
int u_map_has_value_binary(const struct _u_map * u_map, const char * value, size_t length);

/**
 * Detects if the key exists in the _u_map
 * search is case insensitive
 * @param u_map the _u_map to analyze
 * @param key the key to look for
 * @return true if the sprcified u_map contains the specified key
 * false otherwise
 */
int u_map_has_key_case(const struct _u_map * u_map, const char * key);

/**
 * Detects if the key exists in the _u_map
 * search is case insensitive
 * @param u_map the _u_map to analyze
 * @param value the value to look for
 * @return true if the sprcified u_map contains the specified value
 * false otherwise
 */
int u_map_has_value_case(const struct _u_map * u_map, const char * value);

/**
 * add the specified key/value pair into the specified u_map
 * if the u_map already contains a pair with the same key, replace the value
 * @param u_map the _u_map to update
 * @param key the key string
 * @param value the value string
 * @return U_OK on success
 */
int u_map_put(struct _u_map * u_map, const char * key, const char * value);

/**
 * add the specified key/binary value pair into the specified u_map
 * if the u_map already contains a pair with the same key,
 * replace the value at the specified offset with the specified length
 * @param u_map the _u_map to update
 * @param key the key string
 * @param value the value binary
 * @param offset the start offset to set value in u_map value
 * @param length the length of value to set
 * @return U_OK on success
 */
int u_map_put_binary(struct _u_map * u_map, const char * key, const char * value, uint64_t offset, size_t length);

/**
 * get the value length corresponding to the specified key in the u_map
 * search is case sensitive
 * @param u_map the _u_map to analyze
 * @param key the key look for
 * @return the value length if found, -1 if no match found
 */
ssize_t u_map_get_length(const struct _u_map * u_map, const char * key);

/**
 * get the value length corresponding to the specified key in the u_map
 * search is case insensitive
 * @param u_map the _u_map to analyze
 * @param key the key look for
 * @return the value length if found, -1 if no match found
 */
ssize_t u_map_get_case_length(const struct _u_map * u_map, const char * key);

/**
 * get the value corresponding to the specified key in the u_map
 * search is case sensitive
 * @param u_map the _u_map to analyze
 * @param key the key to look for
 * @return the value if key exists NULL if no match found
 */
const char * u_map_get(const struct _u_map * u_map, const char * key);

/**
 * get the value corresponding to the specified key in the u_map
 * search is case insensitive
 * @param u_map the _u_map to analyze
 * @param key the key to look for
 * @return the value if key exists NULL if no match found
 */
const char * u_map_get_case(const struct _u_map * u_map, const char * key);

/**
 * remove an pair key/value that has the specified key
 * search is case sensitive
 * @param u_map the _u_map to analyze
 * @param key the key to look for
 * @return U_OK on success, U_NOT_FOUND if key was not found, error otherwise
 */
int u_map_remove_from_key(struct _u_map * u_map, const char * key);

/**
 * remove all pairs key/value that has the specified key (case insensitive search)
 * search is case insensitive
 * @param u_map the _u_map to analyze
 * @param key the key to look for
 * @return U_OK on success, U_NOT_FOUND if key was not found, error otherwise
 */
int u_map_remove_from_key_case(struct _u_map * u_map, const char * key);

/**
 * remove all pairs key/value that has the specified value
 * search is case sensitive
 * @param u_map the _u_map to analyze
 * @param value the value to look for
 * @return U_OK on success, U_NOT_FOUND if key was not found, error otherwise
 */
int u_map_remove_from_value(struct _u_map * u_map, const char * value);

/**
 * remove all pairs key/value that has the specified value
 * search is case insensitive
 * @param u_map the _u_map to analyze
 * @param value the value to look for
 * @return U_OK on success, U_NOT_FOUND if key was not found, error otherwise
 */
int u_map_remove_from_value_case(struct _u_map * u_map, const char * value);

/**
 * remove all pairs key/value that has the specified value up until the specified length
 * @param u_map the _u_map to analyze
 * @param key the key to look for
 * @param length the length of key
 * @return U_OK on success, U_NOT_FOUND if key was not found, error otherwise
 */
int u_map_remove_from_value_binary(struct _u_map * u_map, const char * key, size_t length);

/**
 * remove the pair key/value at the specified index
 * @param u_map the _u_map to analyze
 * @param index the position of the tuple to remove
 * @return U_OK on success, U_NOT_FOUND if index is out of bound, error otherwise
 */
int u_map_remove_at(struct _u_map * u_map, const int index);

/**
 * Create an exact copy of the specified struct _u_map
 * @param source the _u_map to copy
 * @return a reference to the copy, NULL otherwise
 * returned value must be free'd after use
 */
struct _u_map * u_map_copy(const struct _u_map * source);

/**
 * Copy all key/values pairs of source into dest
 * If a key is already present in dest, value is overwritten
 * @param dest the _u_map to update
 * @param source the _u_map to copy
 * @return U_OK on success, error otherwise
 */
int u_map_copy_into(struct _u_map * dest, const struct _u_map * source);

/**
 * Count the number of elements in the _u_map
 * @param source the _u_map to analyze
 * @return the number of key/values pair in the specified struct _u_map
 * Return -1 on error
 */
int u_map_count(const struct _u_map * source);

/**
 * Empty a struct u_map of all its elements
 * @param u_map the _u_map to empty
 * @return U_OK on success, error otherwise
 */
int u_map_empty(struct _u_map * u_map);

/**
 * @}
 */

/**
 * @defgroup websocket
 * Websocket management functions
 * @{
 */

#ifndef U_DISABLE_WEBSOCKET

/**********************************
 * Websocket functions declarations
 **********************************/

#define U_WEBSOCKET_USER_AGENT "Ulfius Websocket Client Framework"

#define U_WEBSOCKET_MAGIC_STRING     "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"
#define U_WEBSOCKET_UPGRADE_VALUE    "websocket"
#define U_WEBSOCKET_BAD_REQUEST_BODY "Error in websocket handshake, wrong parameters"
#define U_WEBSOCKET_USEC_WAIT        50
#define WEBSOCKET_MAX_CLOSE_TRY      10

#define U_WEBSOCKET_BIT_FIN         0x80
#define U_WEBSOCKET_MASK            0x80
#define U_WEBSOCKET_LEN_MASK        0x7F
#define U_WEBSOCKET_OPCODE_CONTINUE 0x00
#define U_WEBSOCKET_OPCODE_TEXT     0x01
#define U_WEBSOCKET_OPCODE_BINARY   0x02
#define U_WEBSOCKET_OPCODE_CLOSE    0x08
#define U_WEBSOCKET_OPCODE_PING     0x09
#define U_WEBSOCKET_OPCODE_PONG     0x0A
#define U_WEBSOCKET_OPCODE_CLOSED   0xFD
#define U_WEBSOCKET_OPCODE_ERROR    0xFE
#define U_WEBSOCKET_OPCODE_NONE     0xFF

#define U_WEBSOCKET_NONE   0
#define U_WEBSOCKET_SERVER 1
#define U_WEBSOCKET_CLIENT 2

#define U_WEBSOCKET_STATUS_OPEN  0
#define U_WEBSOCKET_STATUS_CLOSE 1
#define U_WEBSOCKET_STATUS_ERROR 2

#define WEBSOCKET_RESPONSE_HTTP       0x0001
#define WEBSOCKET_RESPONSE_UPGRADE    0x0002
#define WEBSOCKET_RESPONSE_CONNECTION 0x0004
#define WEBSOCKET_RESPONSE_ACCEPT     0x0008
#define WEBSOCKET_RESPONSE_PROTCOL    0x0010
#define WEBSOCKET_RESPONSE_EXTENSION  0x0020

/**
 * @struct _websocket_manager Websocket manager structure
 * contains among other things the socket
 * the status (open, closed), and the list of incoming and outcoming messages
 * Used on public callback functions
 */
struct _websocket_manager {
  struct _websocket_message_list * message_list_incoming; /* !< list of incoming messages */
  struct _websocket_message_list * message_list_outcoming; /* !< list of outcoming messages */
  int                              connected; /* !< flag to know if the websocket is connected or not */
  int                              close_flag; /* !< flag to set before closing a websocket */
  MHD_socket                       mhd_sock; /* !< reference to libmicrohttpd's socket for websocket server */
  int                              tcp_sock; /* !< tcp socket for websocket client */
  int                              tls; /* !< set to 1 if the websocket is in a TLS socket */
  gnutls_session_t                 gnutls_session; /* !< GnuTLS session for websocket client */
  gnutls_certificate_credentials_t xcred; /* !< certificate credential used by GnuTLS */
  char                           * protocol; /* !< websocket protocol */
  char                           * extensions; /* !< websocket extension */
  pthread_mutex_t                  read_lock; /* !< mutex to read data in the socket */
  pthread_mutex_t                  write_lock; /* !< mutex to write data in the socket */
  pthread_mutex_t                  status_lock; /* !< mutex to broadcast new status */
  pthread_cond_t                   status_cond; /* !< condition to broadcast new status */
  struct pollfd                    fds;
  int                              type;
};

/**
 * @struct _websocket_message websocket message structure
 * contains all the data of a websocket message
 * and the timestamp of when it was sent of received
 */
struct _websocket_message {
  time_t  datestamp; /* !< date stamp of the message */
  uint8_t opcode; /* !< opcode for the message (string or binary) */
  uint8_t has_mask; /* !< does the message contain a mask? */
  uint8_t mask[4]; /* !< mask used if any */
  size_t  data_len; /* !< length of the data */
  char *  data; /* !< message data */
};

/**
 * @struct _websocket_message_list List of websocket messages
 */
struct _websocket_message_list {
  struct _websocket_message ** list; /* !< messages list */
  size_t len; /* !< message list length */
};

/**
 * @struct _websocket websocket structure
 * contains all the data of the websocket
 */
struct _websocket {
  struct _u_instance               * instance; /* !< reference to the ulfius instance if any */
  struct _u_request                * request; /* !< refrence to the ulfius request of any */
  void                             (* websocket_manager_callback) (const struct _u_request * request, /* !< reference to a function called after the websocket handshake */
                                                                   struct _websocket_manager * websocket_manager,
                                                                   void * websocket_manager_user_data);
  void                             * websocket_manager_user_data; /* !< a user-defined reference that will be available in websocket_manager_callback */
  void                             (* websocket_incoming_message_callback) (const struct _u_request * request, /* !< reference to a function called each time a message arrives */
                                                                            struct _websocket_manager * websocket_manager,
                                                                            const struct _websocket_message * message,
                                                                            void * websocket_incoming_user_data);
  void                             * websocket_incoming_user_data; /* !< a user-defined reference that will be available in websocket_incoming_message_callback */
  void                             (* websocket_onclose_callback) (const struct _u_request * request, /* !< reference to a function called after the websocket connection ends */
                                                                   struct _websocket_manager * websocket_manager,
                                                                   void * websocket_onclose_user_data);
  void                             * websocket_onclose_user_data; /* !< a user-defined reference that will be available in websocket_onclose_callback */
  struct _websocket_manager        * websocket_manager; /* !< refrence to the websocket manager if any */
  struct MHD_UpgradeResponseHandle * urh; /* !< reference used by libmicrohttpd to upgrade the connection */
};

/**
 * @struct _websocket_client_handler Handler for the websocket client, to allow the program to know the status of a websocket client
 */
struct _websocket_client_handler {
  struct _websocket * websocket; /* !< the websocket to use */
  struct _u_response * response; /* !< the response attached to the websocket */
};

/********************************/
/** Common websocket functions **/
/********************************/

/**
 * Sends a message in the websocket
 * @param websocket_manager the websocket manager to use for sending the message
 * @param opcode the opcode to use
 * values available are U_WEBSOCKET_OPCODE_TEXT, U_WEBSOCKET_OPCODE_BINARY, U_WEBSOCKET_OPCODE_PING, U_WEBSOCKET_OPCODE_PONG, U_WEBSOCKET_OPCODE_CLOSE
 * @param data_len the length of the data to send
 * @param data the data to send
 * @return U_OK on success
 */
int ulfius_websocket_send_message(struct _websocket_manager * websocket_manager,
                                  const uint8_t opcode,
                                  const uint64_t data_len,
                                  const char * data);

/**
 * Send a fragmented message in the websocket
 * each fragment size will be at most fragment_len
 * @param websocket_manager the websocket manager to use for sending the message
 * @param opcode the opcode to use
 * values available are U_WEBSOCKET_OPCODE_TEXT, U_WEBSOCKET_OPCODE_BINARY, U_WEBSOCKET_OPCODE_PING, U_WEBSOCKET_OPCODE_PONG, U_WEBSOCKET_OPCODE_CLOSE
 * @param data_len the length of the data to send
 * @param data the data to send
 * @param fragment_len the maximum length of each fragment
 * @return U_OK on success
 */
int ulfius_websocket_send_fragmented_message(struct _websocket_manager * websocket_manager,
                                             const uint8_t opcode,
                                             const uint64_t data_len,
                                             const char * data,
                                             const uint64_t fragment_len);

/**
 * Return the first message of the message list
 * Return NULL if message_list has no message
 * Use it with struct _websocket_manager->message_list_incoming
 * or struct _websocket_manager->message_list_outcoming
 * @param message_list the list to pop the first message
 * @return a _websocket_message reference
 * Returned value must be cleared after use
 */
struct _websocket_message * ulfius_websocket_pop_first_message(struct _websocket_message_list * message_list);

/**
 * Clear data of a websocket message
 * @param message the message to cleanup
 */
void ulfius_clear_websocket_message(struct _websocket_message * message);

/********************************/
/** Server websocket functions **/
/********************************/

/**
 * Set a websocket in the response
 * You must set at least websocket_manager_callback or websocket_incoming_message_callback
 * @param response struct _u_response to send back the websocket initialization, mandatory
 * @param websocket_protocol list of protocols, separated by a comma, or NULL if all protocols are accepted
 * @param websocket_extensions list of extensions, separated by a comma, or NULL if all extensions are accepted
 * @param websocket_manager_callback callback function called right after the handshake acceptance, optional
 * @param websocket_manager_user_data any data that will be given to the websocket_manager_callback, optional
 * @param websocket_incoming_message_callback callback function called on each incoming complete message, optional
 * @param websocket_incoming_user_data any data that will be given to the websocket_incoming_message_callback, optional
 * @param websocket_onclose_callback callback function called right before closing the websocket, must be complete for the websocket to close
 * @param websocket_onclose_user_data any data that will be given to the websocket_onclose_callback, optional
 * @return U_OK on success
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
 * Sets the websocket in closing mode
 * The websocket will not necessarily be closed at the return of this function,
 * it will process through the end of the `websocket_manager_callback`
 * and the `websocket_onclose_callback` calls first.
 * This function has no effect if the websocket isn't connected
 * @param websocket_manager the _websocket_manager to send close signal to
 * @return U_OK on success, U_ERROR on error
 */
int ulfius_websocket_send_close_signal(struct _websocket_manager * websocket_manager);

/**
 * Get the websocket status
 * @param websocket_manager the _websocket_manager to analyze
 * @return the status of the websocket connection
 * Returned values can be U_WEBSOCKET_STATUS_OPEN or U_WEBSOCKET_STATUS_CLOSE
 * wether the websocket is open or closed, or U_WEBSOCKET_STATUS_ERROR on error
 */
int ulfius_websocket_status(struct _websocket_manager * websocket_manager);

/**
 * Wait until the websocket connection is closed or the timeout in milliseconds is reached
 * if timeout is 0, no timeout is set
 * @param websocket_manager the _websocket_manager to analyze
 * @param timeout timeout in milliseconds
 * @return U_WEBSOCKET_STATUS_OPEN or U_WEBSOCKET_STATUS_CLOSE
 * wether the websocket is open or closed, or U_WEBSOCKET_STATUS_ERROR on error
 */
int ulfius_websocket_wait_close(struct _websocket_manager * websocket_manager, unsigned int timeout);

/********************************/
/** Client websocket functions **/
/********************************/

/**
 * Open a websocket client connection
 * @param request the request to use to open the websocket connection
 * @param websocket_manager_callback a reference to a function called after the handshake, may be NULL
 * @param websocket_manager_user_data a user-defined pointer passed to websocket_manager_callback
 * @param websocket_incoming_message_callback a reference to a function called each time a message arrives in the websocket, may be NULL
 * @param websocket_incoming_user_data a user-defined pointer passed to websocket_incoming_message_callback
 * @param websocket_onclose_callback a reference to a function called after the websocket connection is closed, may be NULL
 * @param websocket_onclose_user_data a user-defined pointer passed to websocket_onclose_callback
 * @param websocket_client_handler the handler of the websocket
 * @param response the response attached with the websocket
 * @return U_OK on success
 */
int ulfius_open_websocket_client_connection(struct _u_request * request,
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
                                            void * websocket_onclose_user_data,
                                            struct _websocket_client_handler * websocket_client_handler,
                                            struct _u_response * response);
/**
 * Send a close signal to the websocket
 * @param websocket_client_handler the handler to the websocket connection
 * @return U_OK when the signal is sent, U_ERROR on error
 */
int ulfius_websocket_client_connection_send_close_signal(struct _websocket_client_handler * websocket_client_handler);

/**
 * Closes a websocket client connection
 * @param websocket_client_handler the handler to the websocket connection
 * @return U_OK when the websocket is closed, U_ERROR on error
 */
int ulfius_websocket_client_connection_close(struct _websocket_client_handler * websocket_client_handler);

/**
 * Returns the status of the websocket client connection
 * @param websocket_client_handler the handler to the websocket connection
 * @return U_WEBSOCKET_STATUS_OPEN or U_WEBSOCKET_STATUS_CLOSE
 * wether the websocket is open or closed, or U_WEBSOCKET_STATUS_ERROR on error
 */
int ulfius_websocket_client_connection_status(struct _websocket_client_handler * websocket_client_handler);

/**
 * Wait until the websocket client connection is closed or the timeout in milliseconds is reached
 * if timeout is 0, no timeout is set
 * @param websocket_client_handler the handler to the websocket connection
 * @param timeout timeout in milliseconds
 * @return U_WEBSOCKET_STATUS_OPEN or U_WEBSOCKET_STATUS_CLOSE
 * wether the websocket is open or closed, or U_WEBSOCKET_STATUS_ERROR on error
 */
int ulfius_websocket_client_connection_wait_close(struct _websocket_client_handler * websocket_client_handler, unsigned int timeout);

/**
 * Set values for a struct _u_request to open a websocket
 * request must be previously initialized
 * @param request the request to use to open the websocket
 * @param url the url of the websocket service
 * @param websocket_protocol the protocol for the websocket, may be NULL
 * @param websocket_extensions the extension for the websocket, may be NULL
 * @return U_OK on success
 */
int ulfius_set_websocket_request(struct _u_request * request,
                                 const char * url,
                                 const char * websocket_protocol,
                                 const char * websocket_extensions);

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

#define ULFIUS_POSTBUFFERSIZE 65536

#define U_STATUS_STOP     0
#define U_STATUS_RUNNING  1
#define U_STATUS_ERROR    2

#ifndef U_DISABLE_WEBSOCKET

/**
 * @struct _websocket_handle handle for a websocket
 */
struct _websocket_handle {
  char             * websocket_protocol; /* !< protocol for the websocket */
  char             * websocket_extensions; /* !< extensions for the websocket */
  void            (* websocket_manager_callback) (const struct _u_request * request, /* !< callback function for working with the websocket */
                                                  struct _websocket_manager * websocket_manager,
                                                  void * websocket_manager_user_data);
  void             * websocket_manager_user_data; /* !< user-defined data that will be handled to websocket_manager_callback */
  void            (* websocket_incoming_message_callback) (const struct _u_request * request, /* !< callback function that will be called every time a message arrives from the client in the websocket */
                                                           struct _websocket_manager * websocket_manager,
                                                           const struct _websocket_message * message,
                                                           void * websocket_incoming_user_data);
  void             * websocket_incoming_user_data; /* !< user-defined data that will be handled to websocket_incoming_message_callback */
  void            (* websocket_onclose_callback) (const struct _u_request * request, /* !< callback function that will be called if the websocket is open while the program calls ulfius_stop_framework */
                                                  struct _websocket_manager * websocket_manager,
                                                  void * websocket_onclose_user_data);
  void             * websocket_onclose_user_data; /* !< user-defined data that will be handled to websocket_onclose_callback */
};

/**
 * @struct _websocket_handler handler for the websockets list
 */
struct _websocket_handler {
  size_t                        nb_websocket_active; /* !< number of active websocket */
  struct _websocket          ** websocket_active; /* !< array of active websocket */
  pthread_mutex_t               websocket_close_lock; /* !< mutex to broadcast close signal */
  pthread_cond_t                websocket_close_cond; /* !< condition to broadcast close signal */
  int                           pthread_init;
};

#endif // U_DISABLE_WEBSOCKET

/**
 * @}
 */

/**
 * @defgroup cert TLS client certificate
 * TLS client certificate management functions
 * @{
 */

#ifndef U_DISABLE_GNUTLS
/*
 * ulfius_export_client_certificate_pem
 * Exports the client certificate using PEM format
 * @param request struct _u_request used
 * @return the certificate in PEM format
 * returned value must be u_free'd after use
 */
char * ulfius_export_client_certificate_pem(const struct _u_request * request);

/*
 * ulfius_import_client_certificate_pem
 * Imports the client certificate using PEM format
 * @param request struct _u_request used
 * @param str_cert client certificate in PEM format
 * @return U_OK on success
 */
int ulfius_import_client_certificate_pem(struct _u_request * request, const char * str_cert);

#endif // U_DISABLE_GNUTLS

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif // __ULFIUS_H__
