# Ulfius API Documentation

- [Use Ulfius in a C program](#use-ulfius-in-a-c-program)
  - [Header file](#header-file)
  - [Build options](#build-options)
- [Return values](#return-values)
- [Memory management](#memory-management)
- [Webservice initialization](#webservice-initialization)
  - [Instance structure](#instance-structure)
  - [Endpoint structure](#endpoint-structure)
  - [Multiple callback functions](#multiple-callback-functions)
  - [Multiple URLs with similar pattern](#multiple-urls-with-similar-pattern)
- [Start and stop webservice](#start-and-stop-webservice)
- [Callback functions management](#callback-functions-management)
  - [Request structure](#request-structure)
	- [ulfius_set_request_properties](#ulfius_set_request_properties)
  - [Response structure](#response-structure)
	- [ulfius_set_response_properties](#ulfius_set_response_properties)
  - [Callback functions return value](#callback-functions-return-value)
  - [Use JSON in request and response body](#use-json-in-request-and-response-body)
  - [Additional functions](#additional-functions)
  - [Memory management](#memory-management)
  - [Character encoding](#character-encoding)
  - [Accessing POST parameters](#accessing-post-parameters)
  - [Accessing query string and URL parameters](#accessing-query-string-and-url-parameters)
  - [Accessing header parameters](#accessing-header-parameters)
  - [Cookie management](#cookie-management)
  - [File upload](#file-upload)
    - [Binary file upload](#binary-file-upload)
  - [Streaming data](#streaming-data)
  - [Websockets communication](#websockets-communication)
    - [Websocket management](#websocket-management)
    - [Messages manipulation](#messages-manipulation)
    - [Server-side websocket](#server-side-websocket)
      - [Open a websocket communication](#open-a-websocket-communication)
      - [Advanced websocket extension](#advanced-websocket-extension)
      - [Built-in server extension permessage-deflate](#built-in-server-extension-permessage-deflate)
      - [Close a websocket communication](#close-a-websocket-communication)
      - [Websocket status](#websocket-status)
    - [Client-side websocket](#client-side-websocket)
      - [Prepare the request](#prepare-the-request)
      - [Built-in client extension permessage-deflate](#built-in-client-extension-permessage-deflate)
      - [Open the websocket](#open-the-websocket)
      - [Client Websocket status](#client-websocket-status)
- [Outgoing request functions](#outgoing-request-functions)
  - [Send HTTP request API](#send-http-request-api)
  - [Send SMTP request API](#send-http-request-api)
- [struct _u_map API](#struct-_u_map-api)
- [What's new in Ulfius 2.7?](#whats-new-in-ulfius-27)
- [What's new in Ulfius 2.6?](#whats-new-in-ulfius-26)
- [What's new in Ulfius 2.5?](#whats-new-in-ulfius-25)
- [What's new in Ulfius 2.4?](#whats-new-in-ulfius-24)
- [What's new in Ulfius 2.3?](#whats-new-in-ulfius-23)
- [What's new in Ulfius 2.2?](#whats-new-in-ulfius-22)
- [What's new in Ulfius 2.1?](#whats-new-in-ulfius-21)
- [What's new in Ulfius 2.0?](#whats-new-in-ulfius-20)
  - [Multiple callback functions](#multiple-callback-functions-1)
  - [Keep only binary_body in struct _u_request and struct _u_response](#keep-only-binary_body-in-struct-_u_request-and-struct-_u_response)
  - [Websocket service](#websocket-service)
  - [Remove libjansson and libcurl hard dependency](#remove-libjansson-and-libcurl-hard-dependency)
  - [Ready-to-use callback functions](#ready-to-use-callback-functions)
- [Update existing programs from Ulfius 2.0 to 2.1](#update-existing-programs-from-ulfius-20-to-21)
- [Update existing programs from Ulfius 1.x to 2.0](#update-existing-programs-from-ulfius-1x-to-20)

## Use Ulfius in a C program <a name="use-ulfius-in-a-c-program"></a>

### Header file <a name="header-file"></a>

Include file `ulfius.h` in your source file:

```C
#include <ulfius.h>
```

### Build options <a name="build-options"></a>

You can use `pkg-config` to provide the compile and link options for Ulfius:

```shell
$ # compile flags
$ pkg-config --cflags libulfius
-I/usr/include
$ # linker flags
$ pkg-config --libs libulfius
-L/usr/lib -lulfius -lorcania -lyder
```

If you don't or can't have pkg-config for the build, you can set the linker options `-lulfius -lorcania -lyder`.

The options `-lorcania` and `-lyder` are not necessary if you don't directly use Orcania or Yder functions. But in doubt, add them anyway.

On your linker command, add Ulfius as a dependency library, e.g. `-lulfius` for gcc.

## Return values <a name="return-values"></a>

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

## Memory management <a name="memory-management"></a>

Ulfius uses the memory allocation functions `malloc/realloc/calloc/free` by default, but you can overwrite this and use any other memory allocation functions of your choice. Use Orcania's functions `o_set_alloc_funcs` and `o_get_alloc_funcs` to set and get memory allocation functions.

```C
void o_set_alloc_funcs(o_malloc_t malloc_fn, o_realloc_t realloc_fn, o_free_t free_fn);
void o_get_alloc_funcs(o_malloc_t * malloc_fn, o_realloc_t * realloc_fn, o_free_t * free_fn);
```

Accessing those functions requires you to directly link your application also against the Orcania library. To do so, add `-lorcania` to your linking command.

If you use a version of `libmicrohttpd` older than `0.9.61`, you need to set the `mhd_response_copy_data = 1` in your `_u_instance` if you use a memory allocator whose allocated return values may not directly be passed to free() or if you want to make sure all free() will always go via your user-provided free callback.

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

It's **recommended** to use `ulfius_global_init` and `ulfius_global_close` at the beginning and at the end of your program to initialize and cleanup internal values and settings. This will make outgoing requests faster, especially if you use lots of them, and dispatch your memory allocation functions in curl and Jansson if you changed them. These functions are **NOT** thread-safe, so you must use them in a single thread context.

```C
/**
 * Initialize global parameters
 * This function isn't thread-safe so it must be called once before any call to
 * ulfius_send_http_request, ulfius_send_http_streaming_request, ulfius_send_smtp_email or ulfius_send_smtp_rich_email
 * The function ulfius_send_request_close must be called when ulfius send request functions are no longer needed
 * @return U_OK on success
 */
int ulfius_global_init();

/**
 * Close global parameters
 */
void ulfius_global_close();
```

## Webservice initialization <a name="webservice-initialization"></a>

Ulfius framework runs as an async task in the background. When initialized, a thread is executed in the background. This thread will listen to the specified port and dispatch the calls to the specified callback functions. Ulfius allows adding and removing new endpoints during the instance execution.

To run a webservice, you must initialize a `struct _u_instance` and add your endpoints.

### Instance structure <a name="instance-structure"></a>

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
 * network_type:           Listen to ipv4 and or ipv6 connections, values available are U_USE_ALL, U_USE_IPV4 or U_USE_IPV6
 * bind_address:           ipv4 address to listen to (optional)
 * bind_address6:          ipv6 address to listen to (optional)
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
 * mhd_response_copy_data: to choose between MHD_RESPMEM_MUST_COPY and MHD_RESPMEM_MUST_FREE, only if you use MHD < 0.9.61, 
 *                         otherwise this option is skipped because it's useless
 * check_utf8:             check that all parameters values in the request (url, header and post_body)
 *                         are valid utf8 strings, if a parameter value has non utf8 character, the value
 *                         will be ignored, default 1
 * use_client_cert_auth:   Internal variable use to indicate if the instance uses client certificate authentication
 *                         Do not change this value, available only if websocket support is enabled
 * allowed_post_processor: Specifies which content-type are allowed to process in the request->map_post_body parameters list,
 *                         default value is U_POST_PROCESS_URL_ENCODED|U_POST_PROCESS_MULTIPART_FORMDATA, to disable all, use U_POST_PROCESS_NONE
 */
struct _u_instance {
  struct MHD_Daemon          *  mhd_daemon;
  int                           status;
  unsigned int                  port;
#if MHD_VERSION >= 0x00095208
  unsigned short                network_type;
#endif
  struct sockaddr_in          * bind_address;
  struct sockaddr_in6         * bind_address6;
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
  int                           check_utf8;
#ifndef U_DISABLE_GNUTLS
  int                           use_client_cert_auth;
#endif
  int                           allowed_post_processor;
};
```

In the `struct _u_instance` structure, the element `port` must be set to the port number you want to listen to, the element `bind_address` is used if you want to listen only to a specific IP address. The element `mhd_daemon` is used by the framework, don't modify it.

You can use the functions `ulfius_init_instance`, `ulfius_init_instance_ipv6` and `ulfius_clean_instance` to facilitate the manipulation of the structure:

```C
/**
 * ulfius_init_instance
 *
 * Initialize a struct _u_instance * with default values
 * Binds to IPV4 addresses only
 * @param u_instance the ulfius instance to initialize
 * @param port tcp port to bind to, must be between 1 and 65535
 * @param bind_address IPv4 address to listen to, optional, the reference is borrowed, the structure isn't copied
 * @param default_auth_realm default realm to send to the client on authentication error
 * @return U_OK on success
 */
int ulfius_init_instance(struct _u_instance * u_instance, unsigned int port, struct sockaddr_in * bind_address, const char * default_auth_realm);

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

/**
 * ulfius_clean_instance
 * 
 * Clean memory allocated by a struct _u_instance *
 */
void ulfius_clean_instance(struct _u_instance * u_instance);
```

Since Ulfius 2.6, you can bind to IPv4 connections, IPv6 or both. By default, `ulfius_init_instance` binds to IPv4 addresses only. If you want to bind to both IPv4 and IPv6 addresses, use `ulfius_init_instance_ipv6` with the value parameter `network_type` set to `U_USE_ALL`. If you want to bind to IPv6 addresses only, use `ulfius_init_instance_ipv6` with the value parameter `network_type` set to `U_USE_IPV6`.

If you bind your instance to an address, you **MUST** set the port number to `struct sockaddr_in.sport` because the `struct _u_instance.port` will be ignored.

### Endpoint structure <a name="endpoint-structure"></a>

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
  char       * http_method;
  char       * url_prefix;
  char       * url_format;
  unsigned int priority;
  int       (* callback_function)(const struct _u_request * request, // Input parameters (set by the framework)
                            struct _u_response * response,     // Output parameters (set by the user)
                            void * user_data);
  void       * user_data;
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

If you fill your array of endpoints manually, your `struct _u_endpoint` array **MUST** end with an empty `struct _u_endpoint`.

You can manually declare an endpoint or use the dedicated functions as `int ulfius_add_endpoint` or `int ulfius_add_endpoint_by_val`. It's recommended to use the dedicated functions to fill this array though.

If you manipulate the attribute `u_instance.endpoint_list`, you must end the list with an empty endpoint (see `const struct _u_endpoint * ulfius_empty_endpoint()`), and you must set the attribute `u_instance.nb_endpoints` accordingly. Also, you must use dynamically allocated values (`malloc`) for attributes `http_method`, `url_prefix` and `url_format`.

### Multiple callback functions <a name="multiple-callback-functions"></a>

Ulfius allows multiple callbacks for the same endpoint. This is helpful when you need to execute several actions in sequence, for example check authentication, get resource, set cookie, then gzip response body. That's also why a priority must be set for each callback.

The priority is in descending order, which means that it starts with 0 (highest priority) and priority decreases when priority number increases. There is no more signification to the priority number, which means you can use any increments of your choice.

`Warning`: Having 2 callback functions with the same priority number will result in an undefined execution order result.

To help passing parameters between callback functions of the same request, the value `struct _u_response.shared_data` can be used. It's recommended to use the function `ulfius_set_response_shared_data` with a pointer to a free function for `shared_data`, therefore the framework will automatically clean `struct _u_response.shared_data` at the end of the callback list.

### Multiple URLs with similar pattern <a name="multiple-urls-with-similar-pattern"></a>

If you need to differentiate multiple URLs with similar pattern, you can use priorities among multiple callback function.

For example, if you have 2 endpoints with the following patterns:

1- `/example/:id`
2- `/example/findByStatus`

You'll probably need the callback referred in 2- to be called and the callback referred in 1- not when the URL called is the exact pattern as in 2-. Nevertheless, you'll need callback referred in 1- in all the other cases.

In that case, you'll have to set a higher priority to the endpoint with the URL 2- and return its callback function with the value `U_CALLBACK_COMPLETE`. Remember, if the first callback returns `U_CALLBACK_CONTINUE`, the second callback will be called afterwards.

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

## Start and stop webservice <a name="start-and-stop-webservice"></a>

### Start webservice <a name="start-webservice"></a>

The starting point function are `ulfius_start_framework`, `ulfius_start_secure_framework`, `ulfius_start_secure_ca_trust_framework` or `ulfius_start_framework_with_mhd_options`:

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
int ulfius_start_secure_ca_trust_framework(struct _u_instance * u_instance, const char * key_pem, const char * cert_pem, const char * root_ca_pem);

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
```

In your program, where you want to start the web server, execute the function `ulfius_start_framework(struct _u_instance * u_instance)` for a non-secure http connection.

Use the function `ulfius_start_secure_framework(struct _u_instance * u_instance, const char * key_pem, const char * cert_pem)` for a secure https connection, using a valid private key and a valid corresponding server certificate, see GnuTLS documentation for certificate generation.

Finally, use the function `int ulfius_start_secure_ca_trust_framework(struct _u_instance * u_instance, const char * key_pem, const char * cert_pem, const char * root_ca_pem)` to start a secure https connection and be able to authenticate clients with a certificate.

Those function accept the previously declared `instance` as first parameter. You can reuse the same callback function as much as you want for different endpoints. On success, these functions returns `U_OK`, otherwise an error code.

Note: for security concerns, after running `ulfius_start_secure_framework` or `ulfius_start_secure_ca_trust_framework`, you can free the parameters `key_pem`, `cert_pem` and `root_ca_pem` if you want to.

### Stop webservice <a name="stop-webservice"></a>

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

## Callback functions management <a name="callback-functions-management"></a>

The callback function is the function executed when a user calls an endpoint managed by your webservice (as defined in your `struct _u_endpoint` list).

The callback function has the following signature:

```C
int (* callback_function)(const struct _u_request * request, // Input parameters (set by the framework)
                          struct _u_response * response,     // Output parameters (set by the user)
                          void * user_data);
```

In the callback function definition, the variables `request` and `response` will be initialized by the framework, and the `user_data` variable will be assigned to the user_data defined in your endpoint list definition.

### Request structure <a name="request-structure"></a>

The request variable is defined as:

```C
/**
 * 
 * Structure of request parameters
 * 
 * Contains request data
 * http_protocol:                  http protocol used (1.0 or 1.1)
 * http_verb:                      http method (GET, POST, PUT, DELETE, etc.)
 * http_url:                       full url used to call this callback function or full url to call when used in a ulfius_send_http_request
 * url_path:                       url path only used to call this callback function (ex, if http_url is /path/?param=1, url_path is /path/)
 * proxy:                          proxy address to use for outgoing connections, used by ulfius_send_http_request
 * network_type:                   Force connect to ipv4, ipv6 addresses or both, values available are U_USE_ALL, U_USE_IPV4 or U_USE_IPV6
 * check_server_certificate:       check server certificate and hostname, default true, used by ulfius_send_http_request
 * check_server_certificate_flag:  check certificate peer and or server hostname if check_server_certificate is enabled, values available are U_SSL_VERIFY_PEER, U_SSL_VERIFY_HOSTNAME or both
                                   default value is both (U_SSL_VERIFY_PEER|U_SSL_VERIFY_HOSTNAME), used by ulfius_send_http_request
 * check_proxy_certificate:        check proxy certificate and hostname, default true, used by ulfius_send_http_request, requires libcurl >= 7.52
 * check_proxy_certificate_flag:   check certificate peer and or proxy hostname if check_proxy_certificate is enabled, values available are U_SSL_VERIFY_PEER, U_SSL_VERIFY_HOSTNAME or both
                                   default value is both (U_SSL_VERIFY_PEER|U_SSL_VERIFY_HOSTNAME), used by ulfius_send_http_request, requires libcurl >= 7.52
 * follow_redirect:                follow url redirections, used by ulfius_send_http_request
 * ca_path                         specify a path to CA certificates instead of system path, used by ulfius_send_http_request
 * timeout                         connection timeout used by ulfius_send_http_request, default is 0
 * client_address:                 IP address of the client
 * auth_basic_user:                basic authentication username
 * auth_basic_password:            basic authentication password
 * map_url:                        map containing the url variables, both from the route and the ?key=value variables
 * map_header:                     map containing the header variables
 * map_cookie:                     map containing the cookie variables
 * map_post_body:                  map containing the post body variables (if available)
 * binary_body:                    pointer to raw body
 * binary_body_length:             length of raw body
 * callback_position:              position of the current callback function in the callback list, starts at 0
 * client_cert:                    x509 certificate of the client if the instance uses client certificate authentication and the client is authenticated
 *                                 available only if websocket support is enabled
 * client_cert_file:               path to client certificate file for sending http requests with certificate authentication
 *                                 available only if websocket support is enabled
 * client_key_file:                path to client key file for sending http requests with certificate authentication
 *                                 available only if websocket support is enabled
 * client_key_password:            password to unlock client key file
 *                                 available only if websocket support is enabled
 */
struct _u_request {
  char *               http_protocol;
  char *               http_verb;
  char *               http_url;
  char *               url_path;
  char *               proxy;
#if MHD_VERSION >= 0x00095208
  unsigned short       network_type;
#endif
  int                  check_server_certificate;
  int                  check_server_certificate_flag;
  int                  check_proxy_certificate;
  int                  check_proxy_certificate_flag;
  int                  follow_redirect;
  char *               ca_path;
  unsigned long        timeout;
  struct sockaddr *    client_address;
  char *               auth_basic_user;
  char *               auth_basic_password;
  struct _u_map *      map_url;
  struct _u_map *      map_header;
  struct _u_map *      map_cookie;
  struct _u_map *      map_post_body;
  unsigned char *      binary_body;
  size_t               binary_body_length;
  unsigned int         callback_position;
#ifndef U_DISABLE_GNUTLS
  gnutls_x509_crt_t    client_cert;
  char *               client_cert_file;
  char *               client_key_file;
  char *               client_key_password;
#endif
};
```

Functions dedicated to handle the request:

```C
/**
 * ulfius_set_string_body_request
 * Set a string string_body to a request
 * string_body must end with a '\0' character
 * return U_OK on success
 */
int ulfius_set_string_body_request(struct _u_response * request, const char * string_body);

/**
 * ulfius_set_binary_body_request
 * Set a binary binary_body to a request, replace any existing body in the request
 * @param request the request to be updated
 * @param binary_body an array of char to set to the body response
 * @param length the length of binary_body to set to the request body
 * return U_OK on success
 */
int ulfius_set_binary_body_request(struct _u_request * request, const unsigned char * binary_body, const size_t length);

/**
 * ulfius_set_empty_body_request
 * Set an empty request body
 * return U_OK on success
 */
int ulfius_set_empty_body_request(struct _u_request * request);
```

### ulfius_set_request_properties <a name="ulfius_set_request_properties"></a>

The function `ulfius_set_request_properties` allows to put a variable set of request properties in a single-line. The parameter list MUST end with the option `U_OPT_NONE`

```
/**
 * ulfius_set_request_properties
 * Set a list of properties to a request
 * return U_OK on success
 */
int ulfius_set_request_properties(struct _u_request * request, ...);
```

Options available:

| Option | Description |
|---|---|
| U_OPT_NONE | Empty option to complete a ulfius_set_request_properties or ulfius_set_request_properties |
| U_OPT_HTTP_VERB | http method (GET, POST, PUT, DELETE, etc.), expected option value type: const char * |
| U_OPT_HTTP_URL | full URL used to call this callback function or full URL to call when used in a ulfius_send_http_request, expected option value type: const char * |
| U_OPT_HTTP_URL_APPEND | append char * value to the current url, expected option value type: const char * |
| U_OPT_HTTP_PROXY | proxy address to use for outgoing connections, used by ulfius_send_http_request, expected option value type: const char * |
| U_OPT_NETWORK_TYPE | Force connect to IPv4, IPv6 addresses or both, values available are U_USE_ALL, U_USE_IPV4 or U_USE_IPV6, expected option value type: unsigned short |
| U_OPT_CHECK_SERVER_CERTIFICATE | check server certificate and hostname, default true, used by ulfius_send_http_request, expected option value type: int |
| U_OPT_CHECK_SERVER_CERTIFICATE_FLAG | check certificate peer and or server hostname if check_server_certificate is enabled, values available are U_SSL_VERIFY_PEER, U_SSL_VERIFY_HOSTNAME or both, default value is both (U_SSL_VERIFY_PEER\|U_SSL_VERIFY_HOSTNAME), used by ulfius_send_http_request, expected option value type: int |
| U_OPT_CHECK_PROXY_CERTIFICATE | check proxy certificate and hostname, default true, used by ulfius_send_http_request, requires libcurl >= 7.52, expected option value type: int |
| U_OPT_CHECK_PROXY_CERTIFICATE_FLAG | check certificate peer and or proxy hostname if check_proxy_certificate is enabled, values available are U_SSL_VERIFY_PEER, U_SSL_VERIFY_HOSTNAME or both, default value is both (U_SSL_VERIFY_PEER\|U_SSL_VERIFY_HOSTNAME), used by ulfius_send_http_request, requires libcurl >= 7.52, expected option value type: int |
| U_OPT_FOLLOW_REDIRECT | follow URL redirections, used by ulfius_send_http_request, expected option value type: int |
| U_OPT_CA_PATH | specify a path to CA certificates instead of system path, used by ulfius_send_http_request, expected option value type: const char * |
| U_OPT_TIMEOUT | connection timeout used by ulfius_send_http_request, default is 0 _or_ Timeout in seconds to close the connection because of inactivity between the client and the server, expected option value type: unsigned long |
| U_OPT_AUTH_BASIC_USER | basic authentication username, expected option value type: const char * |
| U_OPT_AUTH_BASIC_PASSWORD | basic authentication password, expected option value type: const char * |
| U_OPT_URL_PARAMETER | Add to the map containing the URL variables, both from the route and the ?key=value variables, expected option value type: const char *, const char * |
| U_OPT_HEADER_PARAMETER | Add to the map containing the header variables, expected option value type: const char *, const char * |
| U_OPT_COOKIE_PARAMETER | Add to the map containing the cookie variables, expected option value type: const char *, const char * |
| U_OPT_POST_BODY_PARAMETER | Add to the map containing the post body variables (if available), expected option value type: const char *, const char * |
| U_OPT_URL_PARAMETER_REMOVE | Remove from the map containing the URL variables, both from the route and the ?key=value variables, expected option value type: const char * |
| U_OPT_HEADER_PARAMETER_REMOVE | Remove from map containing the header variables, expected option value type: const char * |
| U_OPT_COOKIE_PARAMETER_REMOVE | Remove from map containing the cookie variables, expected option value type: const char * |
| U_OPT_POST_BODY_PARAMETER_REMOVE | Remove from map containing the post body variables (if available), expected option value type: const char * |
| U_OPT_BINARY_BODY | Set a raw body to the request or the response, expected option value type: const char *, size_t |
| U_OPT_STRING_BODY | Set a char * body to the request or the response, expected option value type: const char * |
| U_OPT_JSON_BODY | Set a stringified json_t * body to the request or the response, expected option value type: json_t * |
| U_OPT_CLIENT_CERT_FILE | path to client certificate file for sending http requests with certificate authentication, available only if GnuTLS support is enabled, expected option value type: const char * |
| U_OPT_CLIENT_KEY_FILE | path to client key file for sending http requests with certificate authentication, available only if GnuTLS support is enabled, expected option value type: const char * |
| U_OPT_CLIENT_KEY_PASSWORD | password to unlock client key file, available only if GnuTLS support is enabled, expected option value type: const char * |

Example:

```C
ulfius_set_request_properties(&req, U_OPT_HTTP_VERB, "POST", U_OPT_HTTP_URL, "https://www.example.com/", U_OPT_CHECK_SERVER_CERTIFICATE, 0, U_OPT_STRING_BODY, "Hello World!", U_OPT_HEADER_PARAMETER, "Content-Type", "PlainText", U_OPT_NONE);
```

### Response structure <a name="response-structure"></a>

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
 * stream_size:          size of the streamed data (U_STREAM_SIZE_UNKNOWN if unknown)
 * stream_block_size:    size of each block to be streamed, set according to your system
 * stream_user_data:     user defined data that will be available in your callback stream functions
 * websocket_handle:     handle for websocket extension
 * shared_data:          any data shared between callback functions, must be allocated and freed by the callback functions
 * free_shared_data:     pointer to a function that will free shared_data
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
  unsigned char    * binary_body;
  size_t             binary_body_length;
  ssize_t         (* stream_callback) (void * stream_user_data, uint64_t offset, char * out_buf, size_t max);
  void            (* stream_callback_free) (void * stream_user_data);
  uint64_t           stream_size;
  size_t             stream_block_size;
  void             * stream_user_data;
  void             * websocket_handle;
  void *             shared_data;
  void            (* free_shared_data)(void * shared_data);
  unsigned int       timeout;
};
```

In the response variable set by the framework to the callback function, the structure is initialized with no data.

The user can set the `binary_body` before the return statement, or no response body at all if no need. If a `binary_body` is set, its size must be set to `binary_body_length`. `binary_body` is freed by the framework when the response has been sent to the client, so you must use dynamically allocated values. If no status is set, status 200 will be sent to the client.

Some functions are dedicated to handle the response:

```C
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
int ulfius_set_binary_body_response(struct _u_response * response, const unsigned int status, const unsigned char * body, const size_t length);

/**
 * ulfius_set_empty_body_response
 * Set an empty response with only a status
 * @param response the response to be updated
 * @param status the http status code to set to the response
 * @return U_OK on success
 */
int ulfius_set_empty_body_response(struct _u_response * response, const unsigned int status);

/**
 * ulfius_set_stream_response
 * Set an stream response with a status
 * @param response the response to be updated
 * @param status the http status code to set to the response
 * @param stream_callback a pointer to a function that will handle the response stream
 * @param stream_callback_free a pointer to a function that will free its allocated resources during stream_callback
 * @param stream_size size of the streamed data (U_STREAM_SIZE_UNKNOWN if unknown)
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

### ulfius_set_response_properties <a name="ulfius_set_response_properties"></a>

The function `ulfius_set_response_properties` allows to put a variable set of response properties in a single-line. The parameter list MUST end with the option `U_OPT_NONE`

```C
/**
 * ulfius_set_response_properties
 * Set a list of properties to a response
 * return U_OK on success
 */
int ulfius_set_response_properties(struct _u_response * response, ...);
```

Options available:

| Option | Description |
|---|---|
| U_OPT_NONE | Empty option to complete a ulfius_set_request_properties or ulfius_set_request_properties |
| U_OPT_STATUS | HTTP response status code (200, 404, 500, etc), expected option value type: long |
| U_OPT_AUTH_REALM | realm to send to the client response on authentication failed, expected option value type: const char * |
| U_OPT_SHARED_DATA | any data shared between callback functions, must be allocated and freed by the callback functions, expected option value type: void * |
| U_OPT_TIMEOUT | Timeout in seconds to close the connection because of inactivity between the client and the server, expected option value type: long |
| U_OPT_HEADER_PARAMETER | Add to the map containing the header variables, expected option value type: const char *, const char * |
| U_OPT_HEADER_PARAMETER_REMOVE | Remove from map containing the header variables, expected option value type: const char * |
| U_OPT_BINARY_BODY | Set a raw body to the request or the response, expected option value type: const char *, size_t |
| U_OPT_STRING_BODY | Set a char * body to the request or the response, expected option value type: const char * |
| U_OPT_JSON_BODY | Set a stringified json_t * body to the request or the response, expected option value type: json_t * |

Example:

```C
ulfius_set_response_properties(&req, U_OPT_STATUS, 200, U_OPT_STRING_BODY, "Hello World!", U_OPT_HEADER_PARAMETER, "Content-Type", "PlainText", U_OPT_NONE);
```

### Callback functions return value <a name="callback-functions-return-value"></a>

The callback returned value can have the following values:

- `U_CALLBACK_CONTINUE`: The framework can transfer the request and the response to the next callback function in priority order if there is one, or complete the transaction and send back the response to the client.
- `U_CALLBACK_IGNORE`: The framework can transfer the request and the response to the next callback function in priority order if there is one, or complete the transaction and send back the response to the client, the counter `request->callback_position` will not be incremented. If at the end of the callback list `request->callback_position` is 0, the default callback (if set) will be called.
- `U_CALLBACK_COMPLETE`: The framework must complete the transaction and send the response to the client without calling any further callback function.
- `U_CALLBACK_UNAUTHORIZED`: The framework must complete the transaction without calling any further callback function and send an unauthorized response to the client with the status 401, the body specified and the `auth_realm` value if specified.
- `U_CALLBACK_ERROR`: An error occurred during execution, the framework will complete the transaction without calling any further callback function and send an error 500 to the client.

Except for the return values `U_CALLBACK_UNAUTHORIZED` and `U_CALLBACK_ERROR`, the callback return value isn't useful to specify the response sent back to the client. Use the `struct _u_response` variable in your callback function to set all values in the HTTP response.

### Use JSON in request and response body <a name="use-json-in-request-and-response-body"></a>

In Ulfius 2.0, hard dependency with `libjansson` has been removed, the Jansson library is now optional but enabled by default.

If you want to remove JSON dependency, build Ulfius library using Makefile with the flag `JANSSONFLAG=1` or CMake with the option `-DWITH_WEBSOCKET=off`.

```
$ make JANSSONFLAG=1 # Makefile
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
int ulfius_set_json_body_response(struct _u_response * response, const unsigned int status, const json_t * body);
```

The `jansson` API documentation is available at the following address: [Jansson documentation](https://jansson.readthedocs.org/).

Note: According to the [JSON RFC section 6](https://tools.ietf.org/html/rfc4627#section-6), the MIME media type for JSON text is `application/json`. Thus, if there is no HTTP header specifying JSON content-type, the functions `ulfius_get_json_body_request` and `ulfius_get_json_body_response` will return NULL.

### Additional functions <a name="additional-functions"></a>

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

### Memory management <a name="memory-management-1"></a>

The Ulfius framework will automatically free the variables referenced by the request and responses structures, so you must use dynamically allocated values for the response pointers.

### Character encoding <a name="character-encoding"></a>

You may be careful with characters encoding if you use non UTF8 characters in your application or webservice source code, and especially if you use different encoding in the same application. Ulfius may not work properly.

### Accessing POST parameters <a name="accessing-post-parameters"></a>

In the callback function, you can access the POST parameters in the `struct _u_request.map_post_body`. The parameters keys are case-sensitive.

This variable is a `struct _u_map`, therefore you can access it using the [struct _u_map documentation](#struct-_u_map-api).

```C
// Example of accessing a POST parameter
int callback_test (const struct _u_request * request, struct _u_response * response, void * user_data) {
  printf("POST parameter id: %s\n", u_map_get(request->map_post_body, "id"));
  return U_CALLBACK_CONTINUE;
}
```

### Accessing query string and URL parameters <a name="accessing-query-string-and-url-parameters"></a>

In the callback function, you can access the URL and query parameters in the `struct _u_request.map_url`. This variable contains both URL parameters and query string parameters, the parameters keys are case-sensitive. If a parameter appears multiple times in the URL and the query string, the values will be chained in the `struct _u_request.map_url`, separated by a comma `,`.

This variable is a `struct _u_map`, therefore you can access it using the [struct _u_map documentation](#struct-_u_map-api).

```C
// Example of accessing URL parameters
int callback_test (const struct _u_request * request, struct _u_response * response, void * user_data) {
  /**
   * The url could be either
   * http://localhost:8080/?id=xxx
   * http://localhost:8080/id/xxx/ (with a url format like '/id/:id')
   * http://localhost:8080/id/xxx/?id=xxx (with a url format like '/id/:id')
   */
  printf("URL parameter id: %s\n", u_map_get(request->map_url, "id"));
  return U_CALLBACK_CONTINUE;
}
```

### Accessing header parameters <a name="accessing-header-parameters"></a>

In the callback function, you can access the header parameters in the `struct _u_request.map_header`.

In the callback function, you can access the header parameters in the `struct _u_request.map_header`. The parameters keys are case-sensitive. If a parameter appears multiple times in the header, the values will be chained in the `struct _u_request.map_header`, separated by a comma `,`.

This variable is a `struct _u_map`, therefore you can access it using the [struct _u_map documentation](#struct-_u_map-api).

```C
// Example of accessing a POST parameter
int callback_test (const struct _u_request * request, struct _u_response * response, void * user_data) {
  printf("Heder parameter id: %s\n", u_map_get(request->map_header, "id"));
  return U_CALLBACK_CONTINUE;
}
```

### Cookie management <a name="cookie-management"></a>

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
  unsigned int   max_age;
  char * domain;
  char * path;
  int    secure;
  int    http_only;
  int    same_site;
};
```

You can use the functions `ulfius_add_cookie_to_response` or `ulfius_add_same_site_cookie_to_response` in your callback function to facilitate cookies management. These functions are defined as:

```C
/**
 * ulfius_add_cookie_to_response
 * add a cookie to the cookie map
 * return U_OK on success
 */
int ulfius_add_cookie_to_response(struct _u_response * response, const char * key, const char * value, const char * expires, const unsigned int max_age, 
                                  const char * domain, const char * path, const int secure, const int http_only);

/**
 * ulfius_add_same_site_cookie_to_response
 * add a cookie to the cookie map with a SameSite attribute
 * the same_site parameter must have one of the following values:
 * - U_COOKIE_SAME_SITE_NONE   - No SameSite attribute
 * - U_COOKIE_SAME_SITE_STRICT - SameSite attribute set to 'Strict'
 * - U_COOKIE_SAME_SITE_LAX    - SameSite attribute set to 'Lax'
 * return U_OK on success
 */
int ulfius_add_same_site_cookie_to_response(struct _u_response * response, const char * key, const char * value, const char * expires, const unsigned int max_age,
                                            const char * domain, const char * path, const int secure, const int http_only, const int same_site);
```

If you need to remove a cookie on the client, you can send a cookie with the same key but an empty value and a expiration in the past.

```C
/*
 * Example of a cookie removal, you must change the values domain, path, secure, http_only
 * according to your settings
 */
char * expires = "Sat, 10 Jan 1970 12:00:00 GMT"
ulfius_add_cookie_to_response(response, "cookie_key", "", expires, 0, "your_domain.tld", "/", 0, 0);
```

Please note that the client (browser, app, etc.) doesn't have to remove the cookies if it doesn't want to.

## File upload <a name="file-upload"></a>

Ulfius allows file upload to the server. Beware that an uploaded file will be stored in the request object in memory, so uploading large files may dramatically slow the application or even crash it, depending on your system. An uploaded file is stored in the `request->map_body` structure. You can use `u_map_get_length` to get the exact length of the file as it may not be a string format.

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

This callback function will be called before all the other callback functions, and be aware that not all parameters, especially URL parameters, will be present during the file upload callback function executions.

See `examples/sheep_counter` for a file upload example.

### Binary file upload <a name="binary-file-upload"></a>

By default, Ulfius will check input body data to be valid utf8 characters. This will most likely break binary files uploaded via `multipart/form-data` transfert-encoding.

If you don't or can't use `ulfius_set_upload_file_callback_function`, you must disable utf8 check before starting ulfius instance, then in the callback function, make sure to use `u_map_get_length` for every `request->map_post_body` element.

```C
int callback_function(const struct _u_request * request, struct _u_response * response, void * user_data) {
  const char * file_data = u_map_get(request->map_post_body, "file_parameter");
  size_t file_size = u_map_get_length(request->map_post_body, "file_parameter");
  // process http request
  // [...]
  return U_CALLBACK_CONTINUE;
}

int main() {
  // initialize instance
  struct _u_instance u_instance;

  ulfius_init_instance(&u_instance, 8080, NULL, NULL);
  u_instance.check_utf8 = 0;
  ulfius_add_endpoint_by_val(&u_instance, "POST", "/upload", NULL, 0, &callback_function, NULL);
  ulfius_start_framework(&u_instance);
  
  // The program continues...
}
```

## Streaming data <a name="streaming-data"></a>

If you need to stream data, i.e. send a variable and potentially large amount of data, or if you need to send a chunked response, you can define and use `stream_callback_function` in the `struct _u_response`.

Not that if you stream data to the client, any data that was in the `response->binary_body` will be ignored. You must at least set the function pointer `struct _u_response.stream_callback` to stream data. Set `stream_size` to U_STREAM_SIZE_UNKNOWN if you don't know the size of the data you need to send, like in audio stream for example. Set `stream_block_size` according to you system resources to avoid out of memory errors, also, set `stream_callback_free` with a pointer to a function that will free values allocated by your stream callback function, as a `close()` file for example, and finally, you can set `stream_user_data` to a pointer.

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

If you want to close the stream from the server side, return `U_STREAM_END` in the `stream_callback` function. If a problem occurred, you can close the connection with a `U_STREAM_ERROR` return value.

While the `stream_callback_free` function is as simple as:

```C
void stream_callback_free (void * stream_user_data);
```

Check the application `stream_example` in the example folder.

## Websockets communication <a name="websockets-communication"></a>

The websocket protocol is defined in the [RFC6455](https://tools.ietf.org/html/rfc6455). A websocket is a full-duplex communication layer between a server and a client initiated by a HTTP request. Once the websocket handshake is complete between the client and the server, the TCP socket between them is kept open and messages in a specific format can be exchanged. Any side of the socket can send a message to the other side, which allows the server to push messages to the client.

Ulfius implements websocket communication, both server-side and client-side. The following chapter will describe how to create a websocket service or a websocket client by using callback functions. The framework will handle sending and receiving messages with the clients, and your application will deal with high level functions to facilitate the communication process.

### Websocket management <a name="websocket-management"></a>

During the websocket connection, you can either send messages, read the incoming messages, close the connection, or wait until the connection is closed by the client or by a network problem.

### Messages manipulation <a name="messages-manipulation"></a>

A websocket message has the following structure:

```C
/**
 * websocket message structure
 * contains all the data of a websocket message
 * and the timestamp of when it was sent of received
 */
struct _websocket_message {
  time_t  datestamp; // datestamp when the message was transmitted
  uint8_t rsv;       // RSV (extension) flags, must be binary tested over U_WEBSOCKET_RSV1, U_WEBSOCKET_RSV2, U_WEBSOCKET_RSV3
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

To send a message to the client, use the dedicated functions `ulfius_websocket_send_message`, `ulfius_websocket_send_fragmented_message` or `ulfius_websocket_send_json_message`:

```C
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
 * Sends a JSON message in the websocket
 * @param websocket_manager the websocket manager to use for sending the message
 * @param j_message the message to send
 * @return U_OK on success
 */
int ulfius_websocket_send_json_message(struct _websocket_manager * websocket_manager,
                                       json_t * j_message);
```

To get the first message of the incoming or outcoming if you need to with `ulfius_websocket_pop_first_message`, this will remove the first message of the list, and return it as a pointer. You must free the message using the function `ulfius_clear_websocket_message` after use:

All the sent or received messages are stored by default in the `struct _websocket_manager` attributes `message_list_incoming` and `message_list_outcoming`. To skip storing incoming and/or outcoming messages, you can set the flag `struct _websocket_manager.keep_messages` with the values `U_WEBSOCKET_KEEP_INCOMING`, `U_WEBSOCKET_KEEP_OUTCOMING` or `U_WEBSOCKET_KEEP_NONE`. The flag is set to default with `U_WEBSOCKET_KEEP_INCOMING|U_WEBSOCKET_KEEP_OUTCOMING`.

if you exchange messages in JSON format, you can use `ulfius_websocket_parse_json_message` to parse a `struct _websocket_message *` payload into a `json_t *` object.

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

/**
 * Parses the struct _websocket_message * payload into a json_t object if possible
 * @param message the websocket message to parse
 * @param json_error the parsing error output, optional
 * @return a json_t * object on success, NULL on error, must be json_decref'd after use
 */
json_t * ulfius_websocket_parse_json_message(const struct _websocket_message * message, json_error_t * json_error);
```

#### Fragmented messages limitation in browsers <a name="fragmented-messages-limitation-in-browsers"></a>

It seems that some browsers like Firefox or Chromium don't like to receive fragmented messages, they will close the connection with a fragmented message is received. Use `ulfius_websocket_send_fragmented_message` with caution then.

### Server-side websocket <a name="server-side-websocket"></a>

#### Open a websocket communication <a name="open-a-websocket-communication"></a>

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
Likewise, the websocket extension is specific to your application, you can specify a list of websocket extension separated by a comma (`,`).

If no protocol match your list, the connection will be closed by the framework and will return an error 400 to the client.
If you set a `NULL` value for the protocol and/or the extension, Ulfius will not accept any protocols and/or extension sent by the client, but the websocket connexion will be opened.

3 callback functions are available for the websocket implementation:

- `websocket_manager_callback`: This function will be called in a separate thread, and the websocket will remain open as long as this callback function is not completed. In this function, your program will have access to the websocket status (connected or not), and the list of messages sent and received. When this function ends, the websocket will close itself automatically.
- `websocket_incoming_message_callback`: This function will be called every time a new message is sent by the client. Although, it is in synchronous mode, which means that you won't have 2 different `websocket_incoming_message_callback` of the same websocket executed at the same time.
- `websocket_onclose_callback`: This optional function will be called right after the websocket connection is closed, but before the websocket structure is cleaned.

You must specify at least one of the callback functions between `websocket_manager_callback` or `websocket_incoming_message_callback`.

When the function `ulfius_stop_framework` is called, it will wait for all running websockets to end by themselves, there is no force close. So if you have a `websocket_manager_callback` function running, you *MUST* end this function in order to make a clean stop of the http daemon.

For each of these callback function, you can specify a `*_user_data` pointer containing any data you need.

#### Advanced websocket extension <a name="advanced-websocket-extension"></a>

Since Ulfius 2.7.0, you have advanced functions to handle websocket extensions based on the functions `ulfius_add_websocket_extension_message_perform` for the server websockets and `ulfius_add_websocket_client_extension_message_perform` for the clients websockets.

```C
/**
 * Adds a set of callback functions to perform a message transformation via an extension
 * @param response struct _u_response to send back the websocket initialization, mandatory
 * @param extension the expected extension value
 * @param websocket_extension_message_out_perform a callback function called before a message is sent to the server
 * @param websocket_extension_message_out_perform_user_data a user-defined pointer passed to websocket_extension_message_out_perform
 * @param websocket_extension_message_in_perform a callback function called after a message is received from the server
 * @param websocket_extension_message_in_perform_user_data a user-defined pointer passed to websocket_extension_message_in_perform
 * @param websocket_extension_server_match a callback function called on handshake response to match an extensions value with the given callback message perform extensions
 *        if NULL, then extension_client and the extension sent by the server will be compared for an exact match to enable this extension
 * @param websocket_extension_server_match_user_data a user-defined pointer passed to websocket_extension_server_match
 * @param websocket_extension_free_context a callback function called during the websocket close to free the context created in websocket_extension_server_match
 * @param websocket_extension_free_context_user_data a user-defined pointer passed to websocket_extension_free_context
 * @return U_OK on success
 */
int ulfius_add_websocket_extension_message_perform(struct _u_response * response,
                                                   const char * extension_server,
                                                   int (* websocket_extension_message_out_perform)(const uint8_t opcode,
                                                                                                   uint8_t * rsv,
                                                                                                   const uint64_t data_len_in,
                                                                                                   const char * data_in,
                                                                                                   uint64_t * data_len_out,
                                                                                                   char ** data_out,
                                                                                                   const uint64_t fragment_len,
                                                                                                   void * user_data,
                                                                                                   void * context),
                                                   void * websocket_extension_message_out_perform_user_data,
                                                   int (* websocket_extension_message_in_perform)(const uint8_t opcode,
                                                                                                  uint8_t rsv,
                                                                                                  const uint64_t data_len_in,
                                                                                                  const char * data_in,
                                                                                                  uint64_t * data_len_out,
                                                                                                  char ** data_out,
                                                                                                  const uint64_t fragment_len,
                                                                                                  void * user_data,
                                                                                                  void * context),
                                                   void * websocket_extension_message_in_perform_user_data,
                                                   int (* websocket_extension_server_match)(const char * extension_client,
                                                                                            const char ** extension_client_list,
                                                                                            char ** extension_server,
                                                                                            void * user_data,
                                                                                            void ** context),
                                                   void * websocket_extension_server_match_user_data,
                                                   void (* websocket_extension_free_context)(void * user_data,
                                                                                             void * context),
                                                   void  * websocket_extension_free_context_user_data);

/**
 * Adds a set of callback functions to perform a message transformation via an extension
 * @param websocket_client_handler the handler of the websocket
 * @param extension the expected extension value
 * @param websocket_extension_message_out_perform a callback function called before a message is sent to the client
 * @param websocket_extension_message_out_perform_user_data a user-defined pointer passed to websocket_extension_message_out_perform
 * @param websocket_extension_message_in_perform a callback function called after a message is received from the client
 * @param websocket_extension_message_in_perform_user_data a user-defined pointer passed to websocket_extension_message_in_perform
 * @param websocket_extension_client_match a callback function called on handshake response to match an extensions value with the given callback message perform extensions
 * if NULL, then extension and the extension sent by the client will be compared for an exact match to enable this extension
 * @param websocket_extension_client_match_user_data a user-defined pointer passed to websocket_extension_client_match
 * @param websocket_extension_free_context a callback function called during the websocket close to free the context created in websocket_extension_server_match
 * @param websocket_extension_free_context_user_data a user-defined pointer passed to websocket_extension_free_context
 * @return U_OK on success
 */
int ulfius_add_websocket_client_extension_message_perform(struct _websocket_client_handler * websocket_client_handler,
                                                          const char * extension,
                                                          int (* websocket_extension_message_out_perform)(const uint8_t opcode,
                                                                                                          uint8_t * rsv,
                                                                                                          const uint64_t data_len_in,
                                                                                                          const char * data_in,
                                                                                                          uint64_t * data_len_out,
                                                                                                          char ** data_out,
                                                                                                          const uint64_t fragment_len,
                                                                                                          void * user_data,
                                                                                                          void * context),
                                                          void * websocket_extension_message_out_perform_user_data,
                                                          int (* websocket_extension_message_in_perform)(const uint8_t opcode,
                                                                                                         uint8_t rsv,
                                                                                                         const uint64_t data_len_in,
                                                                                                         const char * data_in,
                                                                                                         uint64_t * data_len_out,
                                                                                                         char ** data_out,
                                                                                                         const uint64_t fragment_len,
                                                                                                         void * user_data,
                                                                                                         void * context),
                                                          void * websocket_extension_message_in_perform_user_data,
                                                          int (* websocket_extension_client_match)(const char * extension_server,
                                                                                                   void * user_data,
                                                                                                   void ** context),
                                                          void * websocket_extension_client_match_user_data,
                                                          void (* websocket_extension_free_context)(void * user_data,
                                                                                                    void * context),
                                                          void  * websocket_extension_free_context_user_data);
```

These functions add the possibility to run a callback function before a message is sent and/or after a message is received.

The callback functions `websocket_extension_server_match` and `websocket_extension_client_match` can be use if you expect to match an extension with parameters. If `NULL`, then instead an exact match between `const char * extension` and the extension received will be checked to enable or not this extension callback functions.

```C
/**
 * Checks an extension sent by the client if it matches the expected extension
 * if the function return U_OK, the extension will be enabled and the functions
 * websocket_extension_message_out_perform and websocket_extension_message_in_perform available
 * @param extension_client the extension value to check
 * @param extension_client_list the list of all extension_client to match
 * @param extension_server the extension value to return to the client
 * @param user_data user-defined data
 * @param context context to allocate
 * @return U_OK on success
 */
int websocket_extension_server_match(const char * extension_client,
                                     const char ** extension_client_list,
                                     char ** extension_server,
                                     void * user_data,
                                     void ** context);

/**
 * Checks an extension sent by the server if it matches the expected extension
 * if the function return U_OK, the extension will be enabled and the functions
 * websocket_extension_message_out_perform and websocket_extension_message_in_perform available
 * @param extension_server the extension value to check
 * @param user_data user-defined data
 * @param context context to allocate
 * @return U_OK on success
 */
int websocket_extension_client_match(const char * extension_server, void * user_data, void ** context);
```

The callback function `websocket_extension_message_out_perform` can modify the message data and data lenght and the RSV flags. The callback function `websocket_extension_message_in_perform` can modify the message data only. Inside these functions, `data_in` and `data_len_in` are the current data, your extension callback function must update `data_out` with a `o_malloc`'ed data and set the new data length using `data_len_out` and return `U_OK` on success.
If your function doesn't return `U_OK`, the message data won't be updated and `data_out` won't be free'd if set.

You can call `ulfius_add_websocket_extension_message_perform` or `ulfius_add_websocket_client_extension_message_perform` multiple times for a websocket definition. In that case the extension callbacks function will be called in the same order for the `websocket_extension_message_out_perform` callbacks, and in reverse order for the `websocket_extension_message_in_perform` callbacks.

#### Built-in server extension permessage-deflate <a name="built-in-server-extension-permessage-deflate"></a>

The Websocket extension [permessage-deflate](https://tools.ietf.org/html/rfc7692) used to compress the message data is available in Ulfius. To use this extension in your websocket server, you must call `ulfius_add_websocket_deflate_extension` after calling `ulfius_set_websocket_response` and before finishing the callback endpoint.

```C
/**
 * Adds the required extension message perform to implement message compression according to
 * RFC 7692: Compression Extensions for WebSocket
 * https://tools.ietf.org/html/rfc7692
 * Due to limited implementation, will force response parameters to server_no_context_takeover; client_no_context_takeover
 * @param response struct _u_response to send back the websocket initialization, mandatory
 * @return U_OK on success
 */
int ulfius_add_websocket_deflate_extension(struct _u_response * response);
```

See the sample code in [websocket_example/websocket_server.c](example_programs/websocket_example/websocket_server.c)

#### Close a websocket communication <a name="close-a-websocket-communication"></a>

To close a websocket communication from the server, you can do one of the following:

- End the function `websocket_manager_callback`, it will result in closing the websocket connection
- Send a message with the opcode `U_WEBSOCKET_OPCODE_CLOSE`
- Call the function `ulfius_websocket_wait_close` or `ulfius_websocket_send_close_signal` described below

If no `websocket_manager_callback` is specified, you can send a `U_WEBSOCKET_OPCODE_CLOSE` in the `websocket_incoming_message_callback` function when you need, or call the function `ulfius_websocket_send_close_signal`.

If a callback function `websocket_onclose_callback` has been specified, this function will be executed on every case at the end of the websocket connection.

If the websocket handshake hasn't been correctly completed or if an error appears during the handshake connection, the callback `websocket_onclose_callback` will be called anyway, even if the callback functions `websocket_manager_callback` or `websocket_incoming_message_callback` are skipped due to no websocket connection.
This is to allow the calling program to close opened resources or clean allocated memory. Beware that in this specific case, the parameter `struct _websocket_manager * websocket_manager` may be `NULL`.

#### Websocket status <a name="websocket-status"></a>

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

### Client-side websocket <a name="client-side-websocket"></a>

Ulfius allows to create a websocket connection as a client. The behavior is quite similar to the server-side websocket. The application will open a websocket connection specified by a `struct _u_request`, and a set of callback functions to manage the websocket once connected.

#### Prepare the request <a name="prepare-the-request"></a>

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

#### Built-in client extension permessage-deflate <a name="built-in-client-extension-permessage-deflate"></a>

The Websocket extension [permessage-deflate](https://tools.ietf.org/html/rfc7692) used to compress the message data is available in Ulfius. To use this extension in your websocket client, you must call `ulfius_add_websocket_client_deflate_extension` after calling `ulfius_set_websocket_request` and before calling `ulfius_open_websocket_client_connection`. Also, the parameter `struct _websocket_client_handler` must be initialized to `{NULL, NULL}` before calling `ulfius_set_websocket_request`.

```
/**
 * Adds the required extension message perform to implement message compression according to
 * RFC 7692: Compression Extensions for WebSocket
 * https://tools.ietf.org/html/rfc7692
 * @param websocket_client_handler the handler of the websocket
 * @return U_OK on success
 */
int ulfius_add_websocket_client_deflate_extension(struct _websocket_client_handler * websocket_client_handler);
```

See the sample code in [websocket_example/websocket_client.c](example_programs/websocket_example/websocket_client.c)

#### Opening the websocket connection <a name="opening-the-websocket-connection"></a>

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

#### Closing a websocket communication <a name="closing-a-websocket-communication"></a>

To close a websocket communication, you can do one of the following:

- End the function `websocket_manager_callback`, it will result in closing the websocket connection
- Send a message with the opcode `U_WEBSOCKET_OPCODE_CLOSE`
- Call the function `ulfius_websocket_wait_close` described below, this function will return U_OK when the websocket is closed
- Call the function `ulfius_websocket_client_connection_send_close_signal` described below, this function is non-blocking, it will send a closing signal to the websocket and will return even if the websocket is still open. You can use `ulfius_websocket_wait_close` or `ulfius_websocket_client_connection_status` to check if the websocket is closed.

*Note - broken pipe*

In some cases, when the client websocket connection is secured via TLS. If the connection is already closed, or broken, a `SIGPIPE` signal can occur, leading to a program crash. To avoid this issue, you can handle `SIGPIPE` signals using `sigaction`.

The following code is a simple example where all SIGPIPE signals are simply ignored:

```C
#include <signal.h>
sigaction(SIGPIPE, &(struct sigaction){SIG_IGN}, NULL);
```

Check the `sigaction` documentation for more details.

### Client Websocket status <a name="client-websocket-status"></a>

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

## Outgoing request functions <a name="outgoing-request-functions"></a>

Ulfius allows output functions to send HTTP or SMTP requests. These functions use `libcurl`. You can disable these functions by appending the argument `CURLFLAG=1` when you build the library with Makefile or by disabling the flag in CMake build:

```
$ make CURLFLAG=1 # Makefile
$ cmake -DWITH_CURL=off # CMake
```

### Send HTTP request API <a name="send-http-request-api"></a>

The functions `int ulfius_send_http_request(const struct _u_request * request, struct _u_response * response)` and `int ulfius_send_http_streaming_request(const struct _u_request * request, struct _u_response * response, size_t (* write_body_function)(void * contents, size_t size, size_t nmemb, void * user_data), void * write_body_data)` are based on `libcurl` API.

They allow to send an HTTP request with the parameters specified by the `_u_request` structure. Use the parameter `_u_request.http_url` to specify the distant URL to call.

You can fill the maps in the `_u_request` structure with parameters, they will be used to build the request. Note that if you fill `_u_request.map_post_body` with parameters, the content-type `application/x-www-form-urlencoded` will be used to encode the data.

The response parameters is stored into the `_u_response` structure. If you specify NULL for the response structure, the http call will still be made but no response details will be returned. If you use `ulfius_send_http_request`, the response body will be stored in the parameter `response->*body*`.

If you use `ulfius_send_http_streaming_request`, the response body will be available in the `write_body_function` specified in the call. The `ulfius_send_http_streaming_request` can be used for streaming data or large response, or if you need to receive a checked response from the server.

Return value is `U_OK` on success.

This functions are defined as:

```C
/**
 * ulfius_send_http_request
 * Send a HTTP request and store the result into a _u_response
 * @param request the struct _u_request that contains all the input parameters to perform the HTTP request
 * @param response the struct _u_response that will be filled with all response parameter values, optional, may be NULL
 * @return U_OK on success
 */
int ulfius_send_http_request(const struct _u_request * request, struct _u_response * response);

/**
 * ulfius_send_http_request_with_limit
 * Send a HTTP request and store the result into a _u_response
 * The response body lenght and number of parsed headers can be limited
 * @param request the struct _u_request that contains all the input parameters to perform the HTTP request
 * @param response the struct _u_response that will be filled with all response parameter values, optional, may be NULL
 * @param response_body_limit the maximum size of the response body to retrieve, 0 means no limit
 * @param max_header maximum number of headers to parse in the response, 0 means no limit
 * @return U_OK on success
 */
int ulfius_send_http_request_with_limit(const struct _u_request * request, struct _u_response * response, size_t response_body_limit, size_t max_header);

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
int ulfius_send_http_streaming_request(const struct _u_request * request,
                                       struct _u_response * response,
                                       size_t (* write_body_function)(void * contents, size_t size, size_t nmemb, void * user_data),
                                       void * write_body_data);

/**
 * ulfius_send_http_streaming_request_max_header
 * Send a HTTP request and store the result into a _u_response
 * Except for the body which will be available using write_body_function in the write_body_data
 * The number of parsed headers in the response can be limited by max_header
 * @param request the struct _u_request that contains all the input parameters to perform the HTTP request
 * @param response the struct _u_response that will be filled with all response parameter values, optional, may be NULL
 * @param write_body_function a pointer to a function that will be used to receive response body in chunks
 * @param write_body_data a user-defined poitner that will be passed in parameter to write_body_function
 * @param max_header maximum number of headers to parse in the response, 0 means no limit
 * @return U_OK on success
 */
int ulfius_send_http_streaming_request_max_header(const struct _u_request * request,
                                                  struct _u_response * response,
                                                  size_t (* write_body_function)(void * contents, size_t size, size_t nmemb, void * user_data),
                                                  void * write_body_data,
                                                  size_t max_header);
```

### Send SMTP request API <a name="send-smtp-request-api"></a>

The function `ulfius_send_smtp_email` is used to send emails using a smtp server. It is based on `libcurl` API. It's used to send plain/text emails via a smtp server.
The function `ulfius_send_smtp_rich_email` is used to send an e-mail with a specified content-type.

The functions are defined as:

```C
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

/**
 * Send an email using libcurl
 * email has the content-type specified in parameter
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
 * content_type: content-type to add to the e-mail body
 * subject: email subject (mandatory)
 * mail_body: email body (mandatory)
 * return U_OK on success
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
```

## Miscellaneous functions

### Escape/Unescape string for url

Ulfius includes the functions `ulfius_url_decode` and `ulfius_url_encode` to transform a string into url-safe string and vice-versa. Both functions take a string in input and return a head-allocated string as output that must be u_free'd after use.
Note: In the callback functions, the `request->map_url` values are already url-decoded by the framework.

```C
/**
 * Returns a url-decoded version of str
 * returned value must be u_free'd after use
 * Thanks Geek Hideout!
 * http://www.geekhideout.com/urlcode.shtml
 * @param str the string to decode
 * @return a heap-allocated string
 */
char * ulfius_url_decode(const char * str);

/**
 * Returns a url-encoded version of str
 * returned value must be u_free'd after use
 * Thanks Geek Hideout!
 * http://www.geekhideout.com/urlcode.shtml
 * @param str the string to encode
 * @return a heap-allocated string
 */
char * ulfius_url_encode(const char * str);
```

### Export struct _u_request * and struct _u_response * in HTTP/1.1 stream format

Those functions can be useful to debug requests or responses, or can be used to demonstrate how their respective parameters are translated in HTTP language.

```C
/**
 * Exports a struct _u_request * into a readable HTTP request
 * This function is for debug or educational purpose
 * And the output is probably incomplete for some edge cases
 * So don't think this is the right way
 * Example:
 * PUT /api/write HTTP/1.1\r\n
 * Host: domain.tld\r\n
 * Accept: gzip\r\n
 * Content-Type: application/x-www-form-urlencoded\r\n
 * Content-length: 4321\r\n
 * \r\n
 * key1=value1&key2=value2[...]
 * 
 * @param request the request to export
 * returned value must be u_free'd after use
 */
char * ulfius_export_request_http(const struct _u_request * request);

/**
 * Exports a struct _u_response * into a readable HTTP response
 * This function is for debug or educational purpose
 * And the output is probably incomplete for some edge cases
 * So don't think this is the right way
 * Example:
 * HTTP/1.1 200 OK\r\n
 * Content-type: text/html; charset=utf-8\r\n
 * Set-Cookie: cookieXyz1234...\r\n
 * Content-length: 1234\r\n
 * \r\n
 * <html>\r\n
 * <head>\r\n
 * <title>Hello World!</title>\r\n
 * </head>\r\n
 * <body>\r\n
 * <h2>Welcome</h2>\r\n
 * ....
 * 
 * @param response the response to export
 * returned value must be u_free'd after use
 */
char * ulfius_export_response_http(const struct _u_response * response);
```

## struct _u_map API <a name="struct-_u_map-api"></a>

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

## What's new in Ulfius 2.7? <a name="whats-new-in-ulfius-27"></a>

Allow `Content-Enconding` header with `ulfius_send_http_request` to compress the response body
Add http_compression callback example
Add static_compressed_inmemory_website callback example
Add callback return value `U_CALLBACK_IGNORE` to igore incrementation of `request->callback_position`
Add `ulfius_add_websocket_extension_message_perform` and `ulfius_add_websocket_client_extension_message_perform` for advanced websocket extensions management
Add [Compression Extensions for WebSocket](https://tools.ietf.org/html/rfc7692)

### Breaking change with struct _websocket_client_handler

When declaring a `struct _websocket_client_handler` for websocket client API, you must initialize its members content to `NULL` before using it:

```C
struct _websocket_client_handler websocket_client_handler = {NULL, NULL};
```

### Breaking change with ulfius_set_websocket_response

When using `ulfius_set_websocket_response` with parameters `websocket_protocol` or `websocket_extensions` set to `NULL`, Ulfius will no longer accept any protocol or extension sent by the client, but will return no extension nor protocol to the client, and the websocket connexion will be "raw".

## What's new in Ulfius 2.6? <a name="whats-new-in-ulfius-26"></a>

Add IPv6 support.

Add `struct _u_request->callback_position` to know the position of the current callback in the callback list.

## What's new in Ulfius 2.5? <a name="whats-new-in-ulfius-25"></a>

Add option to ignore non UTF8 strings in incoming requests.

Allow client certificate authentication

## What's new in Ulfius 2.4? <a name="whats-new-in-ulfius-24"></a>

Improve websocket service features with lots of bugfixes and add the possibility to send a fragmented message.

Add websocket client functionality. Allow to create a websocket client connection and exchange messages with the websocket service. In `http://`/`ws://` non-secure mode or `https://`/`wss://` secure mode.

Add a command-line websocket client: `uwsc`.

## What's new in Ulfius 2.3? <a name="whats-new-in-ulfius-23"></a>

Not much on the API, a lot on the build process.
Install via CMake script.

## What's new in Ulfius 2.2? <a name="whats-new-in-ulfius-22"></a>

Allow to use your own callback function when uploading files with `ulfius_set_upload_file_callback_function`, so a large file can be uploaded, even with the option `struct _u_instance.max_post_param_size` set.

## What's new in Ulfius 2.1? <a name="whats-new-in-ulfius-21"></a>

I know it wasn't long since Ulfius 2.0 was released. But after some review and tests, I realized some adjustments had to be made to avoid bugs and to clean the framework a little bit more.

Some of the adjustments made in the new release:
- An annoying bug has been fixed that made streaming data a little buggy when used on Raspbian. Now if you don't know the data size you're sending, use the macro U_STREAM_SIZE_UNKNOWN instead of the previous value -1. There is some updates in the stream callback function parameter types. Check the [streaming data documentation](API.md#streaming-data).
- Fix bug on `ulfius_send_http_request` that didn't send back all headers value with the same name (#19)
- Fix websocket declaration structures to have them outside of the `ulfius.h`, because it could lead to horrifying bugs when you compile Ulfius with websocket but add `#define U_DISABLE_WEBSOCKET` in your application.
- Add proxy value for outgoing requests (#18)
- Unify and update functions name `ulfius_set_[string|json|binary]_body`. You may have to update your legacy code.

The minor version number has been incremented, from 2.0 to 2.1 because some of the changes may require changes in your own code.

## What's new in Ulfius 2.0? <a name="whats-new-in-ulfius-20"></a>

Ulfius 2.0 brings several changes that make the library incompatible with Ulfius 1.0.x branch. The goal of making Ulfius 2.0 is to make a spring cleaning of some functions, remove what is apparently useless, and should bring bugs and memory loss. The main new features are multiple callback functions and websockets implementation.

### Multiple callback functions <a name="multiple-callback-functions-1"></a>

Instead of having an authentication callback function, then a main callback function, you can now have as much callback functions as you want for the same endpoint. A `priority` number has been added in the `struct _u_endpoint` and the auth_callback function and its dependencies have been removed.

For example, let's say you have the following endpoints defined:

- `GET` `/api/tomato/:tomato` => `tomato_get_callback` function, priority 10
- `GET` `/api/potato/:potato` => `potato_get_callback` function, priority 10
- `GET` `/api/*` => `api_validate_callback` function, priority 5
- `*` `*` => `authentication_callback` function, priority 1
- `GET` `*` => `gzip_body_callback` function, priority 99

Then if the client calls the URL `GET` `/api/potato/myPotato`, the following callback functions will be called in that order:

- `authentication_callback`
- `api_validate_callback`
- `potato_get_callback`
- `gzip_body_callback`

*Warning:* In this example, the URL parameter `myPotato` will be available only in the `potato_get_callback` function, because the other endpoints did not defined a URL parameter after `/potato`.

If you need to communicate between callback functions for any purpose, you can use the new parameter `struct _u_response.shared_data`. This is a `void *` pointer initialized to `NULL`.

The dedicated function `ulfius_set_response_shared_data` can be used to set `struct _u_response.shared_data` and `struct _u_response.free_shared_data`. If `struct _u_response.free_shared_data` is set, the function will be used to free `struct _u_response.shared_data` at the end of the callback list.

Note: If you call `ulfius_set_response_shared_data` multiple times in the same request, before replacing `_u_response.shared_data`, the function `ulfius_set_response_shared_data` will free the previous pointer with the previous `struct _u_response.free_shared_data` if set.

### Keep only binary_body in struct _u_request and struct _u_response <a name="keep-only-binary_body-in-struct-_u_request-and-struct-_u_response"></a>

the values `string_body` and `json_body` have been removed from the structures `struct _u_request` and `struct _u_response`. This may be painless in the response if you used only the functions `ulfius_set_xxx_body_response`. Otherwise, you should make small arrangements to your code.

### Websocket service <a name="websocket-service"></a>

Ulfius now allows websockets communication between the client and the server. Check the [API.md](API.md#websockets-communication) file for implementation details.

Using websocket requires [libgnutls](https://www.gnutls.org/). It also requires a recent version of [Libmicrohttpd](https://www.gnu.org/software/libmicrohttpd/), at least 0.9.53.

If you don't need or can't use this feature, you can disable it by adding the option `WEBSOCKETFLAG=1` to the make command when you build Ulfius:

```shell
$ make WEBSOCKETFLAG=1
```

### Remove libjansson and libcurl hard dependency <a name="remove-libjansson-and-libcurl-hard-dependency"></a>

In Ulfius 1.0, libjansson and libcurl were mandatory to build the library, but their usage was not in the core of the framework. Although they can be very useful, so the dependency is now optional.

They are enabled by default, but if you don't need them, you can disable them when you build Ulfius library.

#### libjansson dependency <a name="libjansson-dependency"></a>

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
int ulfius_set_json_body_response(struct _u_response * response, const unsigned int status, const json_t * body);

/**
 * ulfius_get_json_body_response
 * Get JSON structure from the response body if the request is valid
 */
json_t * ulfius_get_json_body_response(struct _u_response * response, json_error_t * json_error);
```

If you want to disable these functions, append `JANSSONFLAG=1` when you build Ulfius library.

```
$ git clone https://github.com/babelouest/ulfius.git
$ cd ulfius/
$ git submodule update --init
$ make JANSSONFLAG=1
$ sudo make install
```

#### libcurl dependency <a name="libjansson-dependency"></a>

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

/**
 * Send an email using libcurl
 * email has the content-type specified in parameter
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
 * content_type: content-type to add to the e-mail body
 * subject: email subject (mandatory)
 * mail_body: email body (mandatory)
 * return U_OK on success
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
```

If you want to disable these functions, append `CURLFLAG=1` when you build Ulfius library.

```
$ git clone https://github.com/babelouest/ulfius.git
$ cd ulfius/
$ git submodule update --init
$ make CURLFLAG=1
$ sudo make install
```

If you wan to disable libjansson and libcurl, you can append both parameters.

```
$ git clone https://github.com/babelouest/ulfius.git
$ cd ulfius/
$ git submodule update --init
$ make CURLFLAG=1 JANSSONFLAG=1
$ sudo make install
```

### Ready-to-use callback functions <a name="ready-to-use-callback-functions"></a>

You can find some ready-to-use callback functions in the folder [example_callbacks](https://github.com/babelouest/ulfius/blob/master/example_callbacks).

## Update existing programs from Ulfius 2.0 to 2.1 <a name="update-existing-programs-from-ulfius-20-to-21"></a>

- An annoying bug has been fixed that made streaming data a little buggy when used on Raspbian. Now if you don't know the data size you're sending, use the macro U_STREAM_SIZE_UNKNOWN instead of the previous value -1.
- There are some updates in the stream callback function parameter types. Check the [streaming data documentation](#streaming-data).
- The websocket data structures are no longer available directly in `struct _u_response` or `struct _u_instance`. But you shouldn't use them like this anyway so it won't be a problem.
- Unify and update functions name `ulfius_set_*_body_response`. You may have to update your legacy code.
The new functions names are:
```c
int ulfius_set_string_body_response(struct _u_response * response, const unsigned int status, const char * body);
int ulfius_set_binary_body_response(struct _u_response * response, const unsigned int status, const char * body, const size_t length);
int ulfius_set_empty_body_response(struct _u_response * response, const unsigned int status);
```

## Update existing programs from Ulfius 1.x to 2.0 <a name="update-existing-programs-from-ulfius-1x-to-20"></a>

If you already have programs that use Ulfius 1.x and want to update them to the brand new fresh Ulfius 2.0, it may require the following minor changes.

### Endpoints definitions <a name="endpoints-definitions"></a>

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

### Callback return value <a name="callback-return-value"></a>

The return value for the callback functions must be adapted, instead of U_OK, U_ERROR or U_ERROR_UNAUTHORIZED, you must use one of the following:

```C
#define U_CALLBACK_CONTINUE     0 // Will replace U_OK
#define U_CALLBACK_IGNORE       1
#define U_CALLBACK_COMPLETE     2
#define U_CALLBACK_UNAUTHORIZED 3 // Will replace U_ERROR_UNAUTHORIZED
#define U_CALLBACK_ERROR        4 // Will replace U_ERROR
```

If you want more details on the multiple callback functions, check the [documentation](#callback-functions-return-value).

Other functions may have change their name or signature, check the documentation for more information.
