# Ulfius API Documentation

- [Header file](#header-file)
- [Return values](#return-values)
- [Memory management](#memory-management)
- [Webservice initialization](#webservice-initialization)
  - [Instance structure](#instance-structure)
  - [Endpoint structure](#endpoint-structure)
  - [Multiple callback functions](#multiple-callback-functions)
  - [Multiple urls with similar pattern](#multiple-urls-with-similar-pattern)
- [Start and stop webservice](#start-and-stop-webservice)
- [Callback functions management](#callback-functions-management)
  - [Request structure](#request-structure)
  - [Response structure](#response-structure)
  - [Callback functions return value](#callback-functions-return-value)
  - [Use JSON in request and response body](#use-json-in-request-and-response-body)
  - [Additional functions](#additional-functions)
  - [Memory management](#memory-management)
  - [Character encoding](#character-encoding)
  - [Cookie management](#cookie-management)
  - [File upload](#file-upload)
  - [Streaming data](#streaming-data)
  - [Websockets communication](#websockets-communication)
    - [Websocket management](#websocket-management)
    - [Messages manipulation](#messages-manipulation)
    - [Server-side websocket](#server-side-websocket)
      - [Starting a websocket communication](#starting-a-websocket-communication)
      - [Websocket status](#websocket-status)
    - [Client-side websocket](#client-side-websocket)
      - [Prepare the request](#prepare-the-request)
      - [Open the websocket](#open-the-websocket)
      - [Websocket status](#websocket-status-1)
- [Outgoing request functions](#outgoing-request-functions)
  - [Send HTTP request API](#send-http-request-api)
  - [Send SMTP request API](#send-http-request-api)
- [struct _u_map API](#struct-_u_map-api)
- [What's new in Ulfius 2.2?](#)
- [What's new in Ulfius 2.1?](#)
- [What's new in Ulfius 2.0?](#)
  - [Multiple callback functions](#)
  - [Keep only binary_body in struct _u_request and struct _u_response](#)
  - [Websocket service](#)
  - [Remove libjansson and libcurl hard dependency](#)
  - [Ready-to-use callback functions](#)
- [Update existing programs from Ulfius 2.0 to 2.1](#update-existing-programs-from-ulfius-20-to-21)
- [Update existing programs from Ulfius 1.x to 2.0](#update-existing-programs-from-ulfius-1x-to-20)

## Header file

Include file `ulfius.h` in your source file:

```C
#include <ulfius.h>
```

On your linker command, add ulfius as a dependency library, e.g. `-lulfius` for gcc.

### Return values

When specified, some functions return `U_OK` on success, and other values otherwise. `U_OK` is 0, other values are non-0 values. The defined return value list is the following:
```C
#define U_OK                 0 // No error
#define U_ERROR              1 // Error
#define U_ERROR_MEMORY       2 // Error in memory allocation
#define U_ERROR_PARAMS       3 // Error in input parameters
#define U_ERROR_LIBMHD       4 // Error in libmicrohttpd execution
#define U_ERROR_LIBCURL      5 // Error in libcurl execution
#define U_ERROR_NOT_FOUND    6 // Something was not found
```

### Memory management

Ulfius uses the memory allocation functions `malloc/realloc/calloc/free` by default, but you can overwrite this and use any other memory allocation functions of your choice. Use Orcania's functions `o_set_alloc_funcs` and `o_get_alloc_funcs` to set and get memory allocation functions.

```C
void o_set_alloc_funcs(o_malloc_t malloc_fn, o_realloc_t realloc_fn, o_free_t free_fn);
void o_get_alloc_funcs(o_malloc_t * malloc_fn, o_realloc_t * realloc_fn, o_free_t * free_fn);
```

Data structures allocated have their specific cleanup functions. To free pointer allocated, you should use the function `u_free` that is intended to use your memory management functions.

```c
/**
 * ulfius_clean_instance
 * 
 * Clean memory allocated by a struct _u_instance *
 */
void ulfius_clean_instance(struct _u_instance * u_instance);

/**
 * ulfius_clean_request
 * clean the specified request's inner elements
 * user must free the parent pointer if needed after clean
 * or use ulfius_clean_request_full
 * return U_OK on success
 */
int ulfius_clean_request(struct _u_request * request);

/**
 * ulfius_clean_response
 * clean the specified response's inner elements
 * user must free the parent pointer if needed after clean
 * or use ulfius_clean_response_full
 * return U_OK on success
 */
int ulfius_clean_response(struct _u_response * response);

/**
 * free the struct _u_map's inner components
 * return U_OK on success
 */
int u_map_clean(struct _u_map * u_map);

/**
 * free data allocated by ulfius functions
 */
void u_free(void * data);
```

### Webservice initialization

Ulfius framework runs as an async task in the background. When initialized, a thread is executed in the background. This thread will listen to the specified port and dispatch the calls to the specified callback functions. Ulfius allows adding and removing new endpoints during the instance execution.

To run a webservice, you must initialize a `struct _u_instance` and add your endpoints.

#### Instance structure

The `struct _u_instance` is defined as:

```C
/**
 * 
 * Structure of an instance
 * 
 * Contains the needed data for an ulfius instance to work
 * 
 * mhd_daemon:             pointer to the libmicrohttpd daemon
 * status:                 status of the current instance, status are U_STATUS_STOP, U_STATUS_RUNNING or U_STATUS_ERROR
 * port:                   port number to listen to
 * bind_address:           ip address to listen to (optional)
 * timeout:                Timeout to close the connection because of inactivity between the client and the server
 * nb_endpoints:           Number of available endpoints
 * default_auth_realm:     Default realm on authentication error
 * endpoint_list:          List of available endpoints
 * default_endpoint:       Default endpoint if no other endpoint match the current url
 * default_headers:        Default headers that will be added to all response->map_header
 * max_post_param_size:    maximum size for a post parameter, 0 means no limit, default 0
 * max_post_body_size:     maximum size for the entire post body, 0 means no limit, default 0
 * websocket_handler:      handler for the websocket structure
 * file_upload_callback:   callback function to manage file upload by blocks
 * file_upload_cls:        any pointer to pass to the file_upload_callback function
 * mhd_response_copy_data: to choose between MHD_RESPMEM_MUST_COPY and MHD_RESPMEM_MUST_FREE
 * 
 */
struct _u_instance {
  struct MHD_Daemon          *  mhd_daemon;
  int                           status;
  unsigned int                  port;
  struct sockaddr_in          * bind_address;
  unsigned int                  timeout;
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
  int                           mhd_response_copy_data;
};
```

In the `struct _u_instance` structure, the element `port` must be set to the port number you want to listen to, the element `bind_address` is used if you want to listen only to a specific IP address. The element `mhd_daemon` is used by the framework, don't modify it.

You can use the functions `ulfius_init_instance` and `ulfius_clean_instance` to facilitate the manipulation of the structure:

```C
/**
 * ulfius_init_instance
 * 
 * Initialize a struct _u_instance * with default values
 * return U_OK on success
 */
int ulfius_init_instance(struct _u_instance * u_instance, int port, struct sockaddr_in * bind_address, const char * default_auth_realm);

/**
 * ulfius_clean_instance
 * 
 * Clean memory allocated by a struct _u_instance *
 */
void ulfius_clean_instance(struct _u_instance * u_instance);
```

#### Endpoint structure

The `struct _u_endpoint` is defined as:

```C
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
  uint   priority;
  int (* callback_function)(const struct _u_request * request, // Input parameters (set by the framework)
                            struct _u_response * response,     // Output parameters (set by the user)
                            void * user_data);
  void * user_data;
};
```

Some functions help you facilitate endpoints manipulation:

```C
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
                               uint priority,
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
 * ulfius_set_default_callback_function
 * Set the default callback function
 * This callback will be called if no endpoint match the url called
 * callback_function: a pointer to a function that will be executed each time the endpoint is called
 *                    you must declare the function as described.
 * user_data:         a pointer to a data or a structure that will be available in the callback function
 * to remove a default callback function, call ulfius_set_default_callback_function with NULL parameter for callback_function
 * return U_OK on success
 */
int ulfius_set_default_callback_function(struct _u_instance * u_instance,
                                         int (* callback_function)(const struct _u_request * request, struct _u_response * response, void * user_data),
                                         void * user_data);
```

HTTP Method can be an existing or not existing method, or `*` for any method. You must specify a url_prefix, a url_format or both, callback_function is mandatory, user_data is optional.

If you fill your array of endoints manually, your `struct _u_endpoint` array **MUST** end with an empty `struct _u_endpoint`.

You can manually declare an endpoint or use the dedicated functions as `int ulfius_add_endpoint` or `int ulfius_add_endpoint_by_val`. It's recommended to use the dedicated functions to fill this array though.

If you manipulate the attribute `u_instance.endpoint_list`, you must end the list with an empty endpoint (see `const struct _u_endpoint * ulfius_empty_endpoint()`), and you must set the attribute `u_instance.nb_endpoints` accordingly. Also, you must use dynamically allocated values (`malloc`) for attributes `http_method`, `url_prefix` and `url_format`.

#### Multiple callback functions

Ulfius allows multiple callbacks for the same endpoint. This is helpful when you need to execute several actions in sequence, for example check authentication, get resource, set cookie, then gzip response body. That's also why a priority must be set for each callback.

The priority is in descending order, which means that it starts with 0 (highest priority) and priority decreases when priority number increases. There is no more signification to the priority number, which means you can use any incrementation of your choice.

`Warning`: Having 2 callback functions with the same priority number will result in an undefined execution order result.

To help passing parameters between callback functions of the same request, the value `struct _u_response.shared_data` can bse used. But it will not be allocated or freed by the framework, the program using this variable must free by itself.

#### Multiple urls with similar pattern

If you need to differentiate multiple urls with similar pattern, you can use priorities among multiple callback function.

For example, if you have 2 endpoints with the following patterns:

1- `/example/:id`
2- `/example/findByStatus`

You'll probably need the callback referred in 2- to be called and the callback referred in 1- not when the url called is the exact pattern as in 2-. Nevertheless, you'll need callback referred in 1- in all the other cases.

In that case, you'll have to set a higher priority to the endpoint with the url 2- and return its callback function with the value `U_CALLBACK_COMPLETE`. Remember, if the first callback returns `U_CALLBACK_CONTINUE`, the second callback will be called afterwards.

```C
int callback_example_find_by_status(const struct _u_request * request, struct _u_response * response, void * user_data) {
  /* do something here... */
  return U_CALLBACK_COMPLETE;
}

int callback_example_by_id(const struct _u_request * request, struct _u_response * response, void * user_data) {
  /* do something else there... */
  return U_CALLBACK_CONTINUE;
}

int main() {
  /* initialize program and instance */
  ulfius_add_endpoint_by_val(instance, "GET", NULL, "/example/:id", 1, &callback_example_by_id, my_user_data);
  ulfius_add_endpoint_by_val(instance, "GET", NULL, "/example/findByStatus", 0, &callback_example_find_by_status, my_user_data);
  /* start instance and run program */
}
```

### Start and stop webservice

#### Start webservice

The starting point function are `ulfius_start_framework` or `ulfius_start_secure_framework`:

```C
/**
 * ulfius_start_framework
 * Initializes the framework and run the webservice based on the parameters given
 * return truze if no error
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
```

In your program, where you want to start the web server, execute the function `ulfius_start_framework(struct _u_instance * u_instance)` for a non-secure http connection or `ulfius_start_secure_framework(struct _u_instance * u_instance, const char * key_pem, const char * cert_pem)` for a secure https connection, using a valid private key and a valid corresponding server certificate, see openssl documentation for certificate generation. Those function accept the previously declared `instance` as first parameter. You can reuse the same callback function as much as you want for different endpoints. On success, these functions returns `U_CALLBACK_CONTINUE` or `U_CALLBACK_COMPLETE`, otherwise an error code.

Note: for security concerns, after running `ulfius_start_secure_framework`, you can free the parameters `key_pem` and `cert_pem` if you want to.

#### Stop webservice

To stop the webservice, call the following function:

```C
/**
 * ulfius_stop_framework
 * 
 * Stop the webservice
 * u_instance:    pointer to a struct _u_instance that describe its port and bind address
 * return U_OK on success
 */
int ulfius_stop_framework(struct _u_instance * u_instance);
```

### Callback functions management

The callback function is the function executed when a user calls an endpoint managed by your webservice (as defined in your `struct _u_endpoint` list).

The callback function has the following signature:

```C
int (* callback_function)(const struct _u_request * request, // Input parameters (set by the framework)
                          struct _u_response * response,     // Output parameters (set by the user)
                          void * user_data);
```

In the callback function definition, the variables `request` and `response` will be initialized by the framework, and the `user_data` variable will be assigned to the user_data defined in your endpoint list definition.

#### Request structure

The request variable is defined as:

```C
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
```

#### Response structure

The response variable is defined as:

```C
/**
 * 
 * Structure of response parameters
 * 
 * Contains response data that must be set by the user
 * status:               HTTP status code (200, 404, 500, etc)
 * protocol:             HTTP Protocol sent
 * map_header:           map containing the header variables
 * nb_cookies:           number of cookies sent
 * map_cookie:           array of cookies sent
 * auth_realm:           realm to send to the client on authenticationb failed
 * binary_body:          a void * containing a raw binary content
 * binary_body_length:   the length of the binary_body
 * stream_callback:      callback function to stream data in response body
 * stream_callback_free: callback function to free data allocated for streaming
 * stream_size:          size of the streamed data (U_STREAM_SIZE_UNKOWN if unknown)
 * stream_block_size:    size of each block to be streamed, set according to your system
 * stream_user_data:     user defined data that will be available in your callback stream functions
 * websocket_handle:     handle for websocket extension
 * shared_data:          any data shared between callback functions, must be allocated and freed by the callback functions
 * timeout:              Timeout in seconds to close the connection because of inactivity between the client and the server
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
  unsigned int       timeout;
};
```

In the response variable set by the framework to the callback function, the structure is initialized with no data.

The user can set the `binary_body` before the return statement, or no response body at all if no need. If a `binary_body` is set, its size must be set to `binary_body_length`. `binary_body` is free'd by the framework when the response has been sent to the client, so you must use dynamically allocated values. If no status is set, status 200 will be sent to the client.

Some functions are dedicated to handle the response:

```C
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
int ulfius_set_string_body_response(struct _u_response * response, const uint status, const char * body);

/**
 * ulfius_set_binary_response
 * Add a binary body to a response
 * return U_OK on success
 */
int ulfius_set_binary_response(struct _u_response * response, const uint status, const char * body, const size_t length);

/**
 * ulfius_set_empty_body_response
 * Set an empty response with only a status
 * return U_OK on success
 */
int ulfius_set_empty_body_response(struct _u_response * response, const uint status);

/**
 * ulfius_set_stream_response
 * Set an stream response with a status
 * return U_OK on success
 */
int ulfius_set_stream_response(struct _u_response * response, 
                                const uint status,
                                ssize_t (* stream_callback) (void * stream_user_data, uint64_t offset, char * out_buf, size_t max);
                                void (* stream_callback_free) (void * stream_user_data),
                                uint64_t stream_size,
                                size_t stream_block_size,
                                void * stream_user_data);

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
```

#### Callback functions return value

The callback returned value can have the following values:

- `U_CALLBACK_CONTINUE`: The framework can transfer the request and the response to the next callback function in priority order if there is one, or complete the transaction and send back the response to the client.
- `U_CALLBACK_COMPLETE`: The framework must complete the transaction and send the response to the client without calling any further callback function.
- `U_CALLBACK_UNAUTHORIZED`: The framework must complete the transaction without calling any further callback function and send an unauthorized response to the client with the status 401, the body specified and the `auth_realm` value if specified.
- `U_CALLBACK_ERROR`: An error occured during execution, the framework must complete the transaction without calling any further callback function and send an error 500 to the client.

#### Use JSON in request and response body

In Ulfius 2.0, hard dependency with `libjansson` has been removed, the jansson library is now optional but enabled by default.

If you want to remove JSON dependency, build Ulfius library using Makefile with the flag `JANSSONFLAG=-DU_DISABLE_JANSSON` or with CMake with th option `-DWITH_WEBSOCKET=off`.

```
$ make JANSSONFLAG=-DU_DISABLE_JANSSON # Makefile
$ cmake -DWITH_WEBSOCKET=off # CMake
```

if JSON is enabled, the following functions are available in Ulfius:

```C
/**
 * ulfius_get_json_body_request
 * Get JSON structure from the request body if the request is valid
 * In case of an error in getting or parsing JSON data in the request,
 * the structure json_error_t * json_error will be filled with an error
 * message if json_error is not NULL
 */
json_t * ulfius_get_json_body_request(const struct _u_request * request, json_error_t * json_error);

/**
 * ulfius_set_json_body_request
 * Add a json_t body to a request
 * return U_OK on success
 */
int ulfius_set_json_body_request(struct _u_request * request, json_t * body);

/**
 * ulfius_get_json_body_response
 * Get JSON structure from the response body if the request is valid
 * In case of an error in getting or parsing JSON data in the request,
 * the structure json_error_t * json_error will be filled with an error
 * message if json_error is not NULL
 */
json_t * ulfius_get_json_body_response(struct _u_response * response, json_error_t * json_error);

/**
 * ulfius_set_json_body_response
 * Add a json_t body to a response
 * return U_OK on success
 */
int ulfius_set_json_body_response(struct _u_response * response, const uint status, const json_t * body);
```

The `jansson` api documentation is available at the following address: [Jansson documentation](https://jansson.readthedocs.org/).

Note: According to the [JSON RFC section 6](https://tools.ietf.org/html/rfc4627#section-6), the MIME media type for JSON text is `application/json`. Thus, if there is no HTTP header specifying JSON content-type, the functions `ulfius_get_json_body_request` and `ulfius_get_json_body_response` will return NULL.

#### Additional functions

In addition with manipulating the raw parameters of the structures, you can use the `_u_request` and `_u_response` structures by using specific functions designed to facilitate their use and memory management:

```C
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
 * returned value must be free'd after use
 */
struct _u_request * ulfius_duplicate_request(const struct _u_request * request);

/**
 * create a new response based on the source elements
 * return value must be free'd after use
 */
struct _u_response * ulfius_duplicate_response(const struct _u_response * response);
```

#### Memory management

The Ulfius framework will automatically free the variables referenced by the request and responses structures, except for `struct _u_response.shared_data`, so you must use dynamically allocated values for the response pointers.

#### Character encoding

You may be careful with characters encoding if you use non UTF8 characters in your application or webservice, and especially if you use different encodings in the same application. Ulfius has not been fully tested in cases like that.

#### Cookie management

The map_cookie structure will contain a set of key/values for the cookies. The cookie structure is defined as

```C
/**
 * struct _u_cookie
 * the structure containing the response cookie parameters
 */
struct _u_cookie {
  char * key;
  char * value;
  char * expires;
  uint   max_age;
  char * domain;
  char * path;
  int    secure;
  int    http_only;
};
```

You can use the function `ulfius_add_cookie_to_response` in your callback function to facilitate cookies management. This function is defined as:

```C
/**
 * ulfius_add_cookie_to_header
 * add a cookie to the cookie map
 * return U_OK on success
 */
int ulfius_add_cookie_to_response(struct _u_response * response, const char * key, const char * value, const char * expires, const uint max_age, 
                                  const char * domain, const char * path, const int secure, const int http_only);
```

### File upload

Ulifius allows file upload to the server. Beware that an uploaded file will be stored in the request object in memory, so uploading large files may dramatically slow the application or even crash it, depending on your system. An uploaded file is stored in the `request->map_body` structure. You can use `u_map_get_length` to get the exact length of the file as it may not be a string format.

If you want to limit the size of a post parameter, if you want to limit the file size for example, set the value `struct _u_instance.max_post_param_size`. Files or post data exceeding this size will be truncated to the size `struct _u_instance.max_post_param_size`. If this parameter is 0, then no limit is set. Default value is 0.

If you want to handle file upload yourself, you can intercept the file upload process with your own callback function. Before running the webservice with `ulfius_start_framework`, you must call the function `ulfius_set_upload_file_callback_function` with a pointer to your file upload callback function. By using this method, the specified callback function will be executed as much as needed with a chunk of the file upload each time.

This function `ulfius_set_upload_file_callback_function` has the following prototype:

```C
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
```

This callback function will be called before all the other callback functions, and be aware that not all parameters, especially url parameters, will be present during the file upload callback function executions.

See `examples/sheep_counter` for a file upload example.

### Streaming data

If you need to stream data, i.e. send a variable and potentially large amount of data, you can define and use `stream_callback_function` in the `struct _u_response`.

Not that if you stream data to the client, any data that was in the `response->binary_body` will be ignored. You must at least set the function pointer `struct _u_response.stream_callback` to stream data. Set `stream_size` to U_STREAM_SIZE_UNKOWN if you don't know the size of the data you need to send, like in audio stream for example. Set `stream_block_size` according to you system resources to avoid out of memory errors, also, set `stream_callback_free` with a pointer to a function that will free values allocated by your stream callback function, as a `close()` file for example, and finally, you can set `stream_user_data` to a pointer.

You can use the function `ulfius_set_stream_response` to set those parameters.

The prototype of the `stream_callback` function is the following:

```C
ssize_t stream_callback (void * stream_user_data,  // Your predefined user_data
                         uint64_t offset,          // the position of the current data to send
                         char * out_buf,           // The output buffer to fill with data
                         size_t max);              // the max size of data to be put in the out_buf
```

The return value must be the size of the data put in `out_buf`.

This function will be called over and over in loop as long as the client has the connection opened.

If you want to close the stream from the server side, return `U_STREAM_END` in the `stream_callback` function. If a problem occured, you can close the connection with a `U_STREAM_ERROR` return value.

While the `stream_callback_free` function is as simple as:

```C
void stream_callback_free (void * stream_user_data);
```

Check the application `stream_example` in the example folder.

### Websockets communication

The websocket protocol is defined in the [RFC6455](https://tools.ietf.org/html/rfc6455). A websocket is a full-duplex communication layer between a server and a client initiated by a HTTP request. Once the websocket handshake is complete between the client and the server, the tcp socket between them is kept open and messages in a specific format can be exchanged. Any side of the socket can send a message to the other side, which allows the server to push messages to the client.

Ulfius implements websocket communication, both server-side and client-side. The following chapter will describe how to create a websocket service or a websocket client by using callback functions. The framework will handle sending and receiving messages with the clients, and your application will deal with high level functions to facilitate the communication process.

#### Websocket management

During the websocket connection, you can either send messages, read the incoming messages, close the connection, or wait until the connection is closed by the client or by a network problem.

#### Messages manipulation

A websocket message has the following structure:

```C
/**
 * websocket message structure
 * contains all the data of a websocket message
 * and the timestamp of when it was sent of received
 */
struct _websocket_message {
  time_t  datestamp; // datestamp when the message was transmitted
  uint8_t opcode;    // opcode of the message: U_WEBSOCKET_OPCODE_TEXT, U_WEBSOCKET_OPCODE_BINARY, U_WEBSOCKET_OPCODE_PING, U_WEBSOCKET_OPCODE_PONG
  uint8_t has_mask;  // Flag to specify if the message has a mask
  uint8_t mask[4];   // mask
  size_t  data_len;  // Length of the data payload
  char  * data;      // data payload
};
```

The different opcode values available are the following:

```C
U_WEBSOCKET_OPCODE_TEXT
U_WEBSOCKET_OPCODE_BINARY
U_WEBSOCKET_OPCODE_CLOSE
U_WEBSOCKET_OPCODE_PING
```

If you want to send a message to the client, you must use the dedicated functions `ulfius_websocket_send_message` or `ulfius_websocket_send_fragmented_message`:

```C
/**
 * Send a message in the websocket
 * Return U_OK on success
 */
int ulfius_websocket_send_message(struct _websocket_manager * websocket_manager,
                                  const uint8_t opcode,
                                  const uint64_t data_len,
                                  const char * data);

/**
 * Send a fragmented message in the websocket
 * each fragment size will be at most fragment_len
 * Return U_OK on success
 */
int ulfius_websocket_send_fragmented_message(struct _websocket_manager * websocket_manager,
                                             const uint8_t opcode,
                                             const uint64_t data_len,
                                             const char * data,
                                             const size_t fragment_len);
```

To get the first message of the incoming or outcoming if you need to with `ulfius_websocket_pop_first_message`, this will remove the first message of the list, and return it as a pointer. You must free the message using the function `ulfius_clear_websocket_message` after use:

```C
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
```

##### Fragmented messages limitation in browsers

It seems that some browsers like Firefox or Chromium don't like to receive fragmented messages, they will close the connection with a fragmented message is received. Use `ulfius_websocket_send_fragmented_message` with caution then.

#### Server-side websocket

##### Opening a websocket communication

To start a websocket communication between the client and your application, you must use the dedicated function `ulfius_start_websocket_cb` with proper values:

```C
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
```

According to the Websockets RFC, parameters `websocket_protocol` and `websocket_extensions` are specific for your application.

In Ulfius Implementation, if you specify a list of protocols as a string of protocol names, separated by a comma (`,`), Ulfius framework will check each one and see if they match the list of protocols specified by the client. The resulting protocol list will be sent back to the client.
Likewise, the websocket extension is specific to your application, you can specify a list of websocket extension separated by a semicolon (`;`).

If no protocol match your list, the connection will be closed by the framework and will return an error 400 to the client.
If you set a `NULL` value for the protocol and/or the extension, Ulfius will accept any protocols and/or extension sent by the client.

3 callback functions are available for the websocket implementation:

- `websocket_manager_callback`: This function will be called in a separate thread, and the websocket will remain open as long as this callback function is not completed. In this function, your program will have access to the websocket status (connected or not), and the list of messages sent and received. When this function ends, the websocket will close itself automatically.
- `websocket_incoming_message_callback`: This function will be called every time a new message is sent by the client. Although, it is in synchronous mode, which means that you won't have 2 different `websocket_incoming_message_callback` of the same websocket executed at the same time.
- `websocket_onclose_callback`: This optional function will be called right after the websocket connection is closed, but before the websocket structure is cleaned.

You must specify at least one of the callback functions between `websocket_manager_callback` or `websocket_incoming_message_callback`.

When the function `ulfius_stop_framework` is called, it will wait for all running websockets to end by themselves, there is no force close. So if you have a `websocket_manager_callback` function running, you *MUST* end this function in order to make a clean stop of the http daemon.

For each of these callback function, you can specify a `*_user_data` pointer containing any data you need.

##### Closing a websocket communication

To close a websocket communication from the server, you can do one of the following:

- End the function `websocket_manager_callback`, it will result in closing the websocket connection
- Send a message with the opcode `U_WEBSOCKET_OPCODE_CLOSE`
- Call the function `ulfius_websocket_wait_close` or `ulfius_websocket_send_close_signal` described below

If no `websocket_manager_callback` is specified, you can send a `U_WEBSOCKET_OPCODE_CLOSE` in the `websocket_incoming_message_callback` function when you need, or call the function `ulfius_websocket_send_close_signal`:

##### Websocket status

The following functions allow the application to know if the the websocket is still open, to enforce closing the websocket or to wait until the websocket is closed by the client:

```C
/**
 * Sets the websocket in closing mode
 * The websocket will not necessarily be closed at the return of this function,
 * it will process through the end of the `websocket_manager_callback`
 * and the `websocket_onclose_callback` calls first.
 * return U_OK on success
 * or U_ERROR on error
 */
int ulfius_websocket_send_close_signal(struct _websocket_manager * websocket_manager);

/**
 * Returns the status of the websocket connection
 * Returned values can be U_WEBSOCKET_STATUS_OPEN or U_WEBSOCKET_STATUS_CLOSE
 * wether the websocket is open or closed, or U_WEBSOCKET_STATUS_ERROR on error
 */
int ulfius_websocket_status(struct _websocket_manager * websocket_manager);

/**
 * Wait until the websocket connection is closed or the timeout in milliseconds is reached
 * if timeout is 0, no timeout is set
 * Returned values can be U_WEBSOCKET_STATUS_OPEN or U_WEBSOCKET_STATUS_CLOSE
 * wether the websocket is open or closed, or U_WEBSOCKET_STATUS_ERROR on error
 */
int ulfius_websocket_wait_close(struct _websocket_manager * websocket_manager, unsigned int timeout);
```

#### Client-side websocket

Ulfius allows to create a websocket connection as a client. The behavior is quite similar to the server-side websocket. The application will open a websocket connection specified by a `struct _u_request`, and a set of callback functions to manage the websocket once connected.

##### Prepare the request

You can manually fill the `struct _u_request` with your parameters or use the dedicated function `ulfius_set_websocket_request`:

```C
/**
 * Set values for a struct _u_request to open a websocket
 * request must be previously initialized
 * Return U_OK on success
 */
int ulfius_set_websocket_request(struct _u_request * request,
                                 const char * url,
                                 const char * websocket_protocol,
                                 const char * websocket_extensions);
```

The `url` specified must have one of the following form:
- `http://<websocket_url>`
- `ws://<websocket_url>`
- `https://<websocket_url>`
- `wss://<websocket_url>`

The `websocket_protocol` and `websocket_extensions` values are optional. To specify multiple protocol, you must separate them with the comma `,` character. To specify multiple extensions, you must separate them with the semicolon `;` character.

You can also specify additional headers or cookies to the request.

Any body parameter or body raw value will be ignored, the header `Content-Length` will be set to 0.

The header `User-Agent` value will be `Ulfius Websocket Client Framework`, feel free to modify it afterwards if you need.

##### Opening the websocket connection

Once the request is completed, you can open the websocket connection with `ulfius_open_websocket_client_connection`:

```C
/**
 * Open a websocket client connection
 * Return U_OK on success
 * @parameters
 * struct _u_request * request: request used to specify the input parameters
 * websocket_manager_callback: main websocket callback function
 * websocket_manager_user_data: a pointer that will be available in websocket_manager_callback
 * websocket_incoming_message_callback: callback function that will be called each time a message is received
 * websocket_incoming_user_data: a pointer that will be available in websocket_incoming_message_callback
 * websocket_onclose_callback: callback function that will be called right after the websocket is closed
 * websocket_onclose_user_data: a pointer that will be available in websocket_onclose_callback
 * websocket_client_handler: The handler for the websocket
 *
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
                                            struct _websocket_client_handler * websocket_client_handler);
```

If the websocket connection is established, `U_OK` will be returned and the websocket connection will be executed in a separate thread. 

##### Closing a websocket communication

To close a websocket communication, you can do one of the following:

- End the function `websocket_manager_callback`, it will result in closing the websocket connection
- Send a message with the opcode `U_WEBSOCKET_OPCODE_CLOSE`
- Call the function `ulfius_websocket_wait_close` described below, this function will return U_OK when the websocket is closed
- Call the function `ulfius_websocket_client_connection_send_close_signal` described below, this function is non-blocking, it will send a closing signal to the websocket and will return even if the websocket is still open. You can use `ulfius_websocket_wait_close` or `ulfius_websocket_client_connection_status` to check if the websocket is closed.

##### Websocket status

The following functions allow the application to know if the the websocket is still open, to enforce closing the websocket or to wait until the websocket is closed by the server:

```C
/**
 * Send a close signal to the websocket
 * return U_OK when the signal is sent
 * or U_ERROR on error
 */
int ulfius_websocket_client_connection_send_close_signal(struct _websocket_client_handler * websocket_client_handler);

/**
 * Closes a websocket client connection
 * return U_OK when the websocket is closed
 * or U_ERROR on error
 */
int ulfius_websocket_client_connection_close(struct _websocket_client_handler * websocket_client_handler);

/**
 * Returns the status of the websocket client connection
 * Returned values can be U_WEBSOCKET_STATUS_OPEN or U_WEBSOCKET_STATUS_CLOSE
 * wether the websocket is open or closed, or U_WEBSOCKET_STATUS_ERROR on error
 */
int ulfius_websocket_client_connection_status(struct _websocket_client_handler * websocket_client_handler);

/**
 * Wait until the websocket client connection is closed or the timeout in milliseconds is reached
 * if timeout is 0, no timeout is set
 * Returned values can be U_WEBSOCKET_STATUS_OPEN or U_WEBSOCKET_STATUS_CLOSE
 * wether the websocket is open or closed, or U_WEBSOCKET_STATUS_ERROR on error
 */
int ulfius_websocket_client_connection_wait_close(struct _websocket_client_handler * websocket_client_handler, unsigned int timeout);
```

## Outgoing request functions

Ulfius allows output functions to send HTTP or SMTP requests. These functions use `libcurl`. You can disable these functions by appending the argument `CURLFLAG=-DU_DISABLE_CURL` when you build the library with Makefile or by disabling the flag in CMake build:

```
$ make CURLFLAG=-DU_DISABLE_CURL # Makefile
$ cmake -DWITH_CURL=off # CMake
```

### Send HTTP request API

The functions `int ulfius_send_http_request(const struct _u_request * request, struct _u_response * response)` and `int ulfius_send_http_streaming_request(const struct _u_request * request, struct _u_response * response, size_t (* write_body_function)(void * contents, size_t size, size_t nmemb, void * user_data), void * write_body_data)` are based on `libcurl` api.

They allow to send an HTTP request with the parameters specified by the `_u_request` structure. Use the parameter `_u_request.http_url` to specify the distant url to call.

You can fill the maps in the `_u_request` structure with parameters, they will be used to build the request. Note that if you fill `_u_request.map_post_body` with parameters, the content-type `application/x-www-form-urlencoded` will be use to encode the data.

The response parameters is stored into the `_u_response` structure. If you specify NULL for the response structure, the http call will still be made but no response details will be returned. If you use `ulfius_send_http_request`, the response body will be stored in the parameter `response->*body*`, if you use `ulfius_send_http_streaming_request`, the response body will be available in the `write_body_function` specified in the call. The `ulfius_send_http_streaming_request` can be used for streaming data or large response.

Return value is `U_OK` on success.

This functions are defined as:

```C
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
int ulfius_send_http_streaming_request(const struct _u_request * request, 
                                       struct _u_response * response, 
                                       size_t (* write_body_function)(void * contents, 
                                                                      size_t size, 
                                                                      size_t nmemb, 
                                                                      void * user_data), 
                                       void * write_body_data);
```

### Send SMTP request API

The function `ulfius_send_smtp_email` is used to send emails using a smtp server. It is based on `libcurl` API.

It's used to send raw emails via a smtp server.

This function is defined as:

```C
/**
 * Send an email
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
```

### struct _u_map API

The `struct _u_map` is a simple key/value mapping API used in the requests and the response for setting parameters. The available functions to use this structure are:

```C
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
size_t u_map_get_length(const struct _u_map * u_map, const char * key);

/**
 * get the value length corresponding to the specified key in the u_map
 * return -1 if no match found
 * search is case insensitive
 */
size_t u_map_get_case_length(const struct _u_map * u_map, const char * key);

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
 * Copy all key/values pairs of source into target
 * If key is already present in target, it's overwritten
 * return U_OK on success, error otherwise
 */
int u_map_copy_into(const struct _u_map * source, struct _u_map * target);

/**
 * Return the number of key/values pair in the specified struct _u_map
 * Return -1 on error
 */
int u_map_count(const struct _u_map * source);
```

## What's new in Ulfius 2.4?

Improve websocket service features with lots of bugfixes and add the possibility to send a fragmented message.

Add websocket client functionality. Allow to create a websocket client connection and exchange messages with the websocket service. In `http://`/`ws://` non-secure mode or `https://`/`wss://` secure mode.

Add a command-line websocket client: `uwsc`.

## What's new in Ulfius 2.3?

Install via CMake script.

## What's new in Ulfius 2.2?

Allow to use your own callback function when uploading files with `ulfius_set_upload_file_callback_function`, so a large file can be uploaded, even with the option `struct _u_instance.max_post_param_size` set.

## What's new in Ulfius 2.1?

I know it wasn't long since Ulfius 2.0 was released. But after some review and tests, I realized some adjustments had to be made to avoid bugs and to clean the framework a little bit more.

Some of the adjustments made in the new release:
- An annoying bug has been fixed that made streaming data a little buggy when used on raspbian. Now if you don't know the data size you're sending, use the macro U_STREAM_SIZE_UNKOWN instead of the previous value -1. There is some updates in the stream callback function parameter types. Check the [streaming data documentation](API.md#streaming-data).
- Fix bug on `ulfius_send_http_request` that didn't send back all headers value with the same name (#19)
- Fix websocket declaration structures to have them outside of the `ulfius.h`, because it could lead to horrifying bugs when you compile ulfius with websocket but add `#define U_DISABLE_WEBSOCKET` in your application.
- Add proxy value for outgoing requests (#18)
- Unify and update functions name `ulfius_set_[string|json|binary]_body`. You may have to update your legacy code.

The minor version number has been incremented, from 2.0 to 2.1 because some of the changes may require changes in your own code.

## What's new in Ulfius 2.0?

Ulfius 2.0 brings several changes that make the library incompatible with Ulfius 1.0.x branch. The goal of making Ulfius 2.0 is to make a spring cleaning of some functions, remove what is apparently useless, and should bring bugs and memory loss. The main new features are multiple callback functions and websockets implementation.

### Multiple callback functions

Instead of having an authentication callback function, then a main callback function, you can now have as much callback functions as you want for the same endpoint. A `priority` number has been added in the `struct _u_endpoint` and the auth_callback function and its dependencies have been removed.

For example, let's say you have the following endpoints defined:

- `GET` `/api/tomato/:tomato` => `tomato_get_callback` function, priority 10
- `GET` `/api/potato/:potato` => `potato_get_callback` function, priority 10
- `GET` `/api/*` => `api_validate_callback` function, priority 5
- `*` `*` => `authentication_callback` function, priority 1
- `GET` `*` => `gzip_body_callback` function, priority 99

Then if the client calls the url `GET` `/api/potato/myPotato`, the following callback functions will be called in that order:

- `authentication_callback`
- `api_validate_callback`
- `potato_get_callback`
- `gzip_body_callback`

*Warning:* In this example, the url parameter `myPotato` will be availabe only in the `potato_get_callback` function, because the other endpoints did not defined a url parameter after `/potato`.

If you need to communicate between callback functions for any purpose, you can use the new parameter `struct _u_response.shared_data`. This is a `void *` pointer initialized to `NULL`. If you use it, remember to free it after use, because the framework won't.

### Keep only binary_body in struct _u_request and struct _u_response

the values `string_body` and `json_body` have been removed from the structures `struct _u_request` and `struct _u_response`. This may be painless in the response if you used only the functions `ulfius_set_xxx_body_response`. Otherwise, you should make small arrangements to your code.

### Websocket service

Ulfius now allows websockets communication between the client and the server. Check the [API.md](API.md#websockets-communication) file for implementation details.

Using websocket requires [libgnutls](https://www.gnutls.org/). It also requires a recent version of [Libmicrohttpd](https://www.gnu.org/software/libmicrohttpd/), at least 0.9.53.

If you dont need or can't use this feature, you can disable it by adding the option `WEBSOCKETFLAG=-DU_DISABLE_WEBSOCKET` to the make command when you build Ulfius:

```shell
$ make WEBSOCKETFLAG=-DU_DISABLE_WEBSOCKET
```

### Remove libjansson and libcurl hard dependency

In Ulfius 1.0, libjansson and libcurl were mandatory to build the library, but their usage was not in the core of the framework. Although they can be very useful, so the dependency is now optional.

They are enabled by default, but if you don't need them, you can disable them when you build Ulfius library.

#### libjansson dependency

This dependency allows to use the following functions:

```c
/**
 * ulfius_get_json_body_request
 * Get JSON structure from the request body if the request is valid
 */
json_t * ulfius_get_json_body_request(const struct _u_request * request, json_error_t * json_error);

/**
 * ulfius_set_json_body_request
 * Add a json_t body to a request
 * return U_OK on success
 */
int ulfius_set_json_body_request(struct _u_request * request, json_t * body);

/**
 * ulfius_set_json_body_response
 * Add a json_t body to a response
 * return U_OK on success
 */
int ulfius_set_json_body_response(struct _u_response * response, const uint status, const json_t * body);

/**
 * ulfius_get_json_body_response
 * Get JSON structure from the response body if the request is valid
 */
json_t * ulfius_get_json_body_response(struct _u_response * response, json_error_t * json_error);
```

If you want to disable these functions, append `JANSSONFLAG=-DU_DISABLE_JANSSON` when you build Ulfius library.

```
$ git clone https://github.com/babelouest/ulfius.git
$ cd ulfius/
$ git submodule update --init
$ make JANSSONFLAG=-DU_DISABLE_JANSSON
$ sudo make install
```

#### libcurl dependency

This dependency allows to use the following functions:

```c
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
```

If you want to disable these functions, append `CURLFLAG=-DU_DISABLE_CURL` when you build Ulfius library.

```
$ git clone https://github.com/babelouest/ulfius.git
$ cd ulfius/
$ git submodule update --init
$ make CURLFLAG=-DU_DISABLE_CURL
$ sudo make install
```

If you wan to disable libjansson and libcurl, you can append both parameters.

```
$ git clone https://github.com/babelouest/ulfius.git
$ cd ulfius/
$ git submodule update --init
$ make CURLFLAG=-DU_DISABLE_CURL JANSSONFLAG=-DU_DISABLE_JANSSON
$ sudo make install
```

### Ready-to-use callback functions

You can find some ready-to-use callback functions in the folder [example_callbacks](https://github.com/babelouest/ulfius/blob/master/example_callbacks).

## Update existing programs from Ulfius 2.0 to 2.1

- An annoying bug has been fixed that made streaming data a little buggy when used on raspbian. Now if you don't know the data size you're sending, use the macro U_STREAM_SIZE_UNKOWN instead of the previous value -1.
- There are some updates in the stream callback function parameter types. Check the [streaming data documentation](#streaming-data).
- The websocket data structures are no longer available directly in `struct _u_response` or `struct _u_instance`. But you shouldn't use them like this anyway so it won't be a problem.
- Unify and update functions name `ulfius_set_*_body_response`. You may have to update your legacy code.
The new functions names are:
```c
int ulfius_set_string_body_response(struct _u_response * response, const uint status, const char * body);
int ulfius_set_binary_body_response(struct _u_response * response, const uint status, const char * body, const size_t length);
int ulfius_set_empty_body_response(struct _u_response * response, const uint status);
```

## Update existing programs from Ulfius 1.x to 2.0

If you already have programs that use Ulfius 1.x and want to update them to the brand new fresh Ulfius 2.0, it may require the following minor changes.

### Endpoints definitions

Endpoints structure have changed, `ulfius_add_endpoint_by_val` now requires only one callback function, but requires a priority number.

If you don't use authentication callback functions, you can simply remove the `NULL, NULL, NULL` parameters corresponding to the former authentication callback function pointer, the authentication callback user data, and the realm value. Then add any number as a priority, 0 for example.

If you use authentication callback functions, split your `ulfius_add_endpoint_by_val` call in 2 separate calls, one for the authentication function, one for the main callback function. For example:

```C
// An Ulfius 1.x call
ulfius_add_endpoint_by_val(&instance, "GET", "/", NULL, &auth_callback, my_auth_data, "my realm", &main_callback, my_main_data);

// The same behaviour with Ulfius 2.0
ulfius_add_endpoint_by_val(&instance, "GET", "/", NULL, 0, &auth_callback, my_auth_data);
ulfius_add_endpoint_by_val(&instance, "GET", "/", NULL, 1, &main_callback, my_main_data);
// In this case, the realm value "my realm" must be specified in the response
```

### Callback return value

The return value for the callback functions must be adapted, instead of U_OK, U_ERROR or U_ERROR_UNAUTHORIZED, you must use one of the following:

```C
#define U_CALLBACK_CONTINUE     0 // Will replace U_OK
#define U_CALLBACK_COMPLETE     1
#define U_CALLBACK_UNAUTHORIZED 2 // Will replace U_ERROR_UNAUTHORIZED
#define U_CALLBACK_ERROR        3 // Will replace U_ERROR
```

If you want more details on the multiple callback functions, check the [documentation](#callback-functions-return-value).

Other functions may have change their name or signature, check the documentation for more information.
