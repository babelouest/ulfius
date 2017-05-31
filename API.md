# Ulfius API Documentation

## Header file

Include file `ulfius.h` in your source file:

```C
#include <ulfius.h>
```

If you have disabled `libcurl` during the build, define the macro `U_DISABLE_CURL` before including `ulfius.h`:

```C
#define U_DISABLE_CURL
#include <ulfius.h>
```

If you have disabled `libjansson` during the build, define the macro `U_DISABLE_JANSSON` before including `ulfius.h`:

```C
#define U_DISABLE_JANSSON
#include <ulfius.h>
```

If you have disabled `libgnutls` during the build, define the macro `U_DISABLE_WEBSOCKET` before including `ulfius.h`:

```C
#define U_DISABLE_WEBSOCKET
#include <ulfius.h>
```

If you have disabled more than one library, for example both `libcurl` and `libjansson`, define both macros before including `ulfius.h`:

```C
#define U_DISABLE_CURL
#define U_DISABLE_JANSSON
#include <ulfius.h>
```

On your linker command, add ulfius as a dependency library, e.g. `-lulfius` for gcc.

## API Documentation

### Update existing programs from Ulfius 1.x to 2.0

If you already have programs that use Ulfius 1.x and want to update them to the brand new fresh Ulfius 2.0, it may require the following minor changes.

#### Endpoints definitions

Endpoints structure have change, `ulfius_add_endpoint_by_val` now requires only one callback function, but requires a priority number.

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

#### Callback return value

The return value for the callback functions must be adapted, instead of U_OK, U_ERROR or U_ERROR_UNAUTHORIZED, you must use one of the following:

```C
#define U_CALLBACK_CONTINUE     0 // Will replace U_OK
#define U_CALLBACK_COMPLETE     1
#define U_CALLBACK_UNAUTHORIZED 2 // Will replace U_ERROR_UNAUTHORIZED
#define U_CALLBACK_ERROR        3 // Will replace U_ERROR
```

If you want more details on the multiple callback functions, check the [documentation](#callback-functions-return-value).

Other functions may have change their name or signature, check the documentation for more information.

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

Ulfius uses memory allocation functions `malloc/realloc/free` by default, but you can overwrite this and use any other memory allocation functions of your choice. Use Orcania's functions `o_set_alloc_funcs` and `o_get_alloc_funcs` to set and get memory allocation functions.

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

### Initialization

When initialized, Ulfius runs a thread in background that will listen to the specified port and dispatch the calls to the specified functions. Ulfius allows adding and removing new endpoints during the instance execution.

To run a webservice, you must initialize a `struct _u_instance` and add your endpoints.

The `struct _u_instance` is defined as:

```C
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
  uint                          port;
  struct sockaddr_in          * bind_address;
  int                           nb_endpoints;
  char                        * default_auth_realm;
  struct _u_endpoint          * endpoint_list;
  struct _u_endpoint          * default_endpoint;
  struct _u_map               * default_headers;
  size_t                        max_post_param_size;
  size_t                        max_post_body_size;
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

HTTP Method can be an existing or not existing method, or * for any method, you must specify a url_prefix, a url_format or both, callback_function is mandatory, user_data is optional.

Your `struct _u_endpoint` array **MUST** end with an empty `struct _u_endpoint`.

You can manually declare an endpoint or use the dedicated functions as `int ulfius_add_endpoint` or `int ulfius_add_endpoint_by_val`.

If you manipulate the attribute `u_instance.endpoint_list`, you must end the list with an empty endpoint (see `const struct _u_endpoint * ulfius_empty_endpoint()`), and you must set the attribute `u_instance.nb_endpoints` accordingly. Also, you must use dynamically allocated values for attributes `http_method`, `url_prefix` and `url_format`.

Ulfius allows multiple callbacks for the same endpoint. This is helpful when you need to execute several actions in sequence, for example check authentication, get resource, set cookie, then gzip response body. That's also why a priority must be set for each callback.

The priority is in descending order, which means that it starts with 0 (highest priority) and priority decreases when priority number increases. There is no more signification to the priority number, which means you can use any incrementation of your choice.

`Warning`: Having 2 callback functions with the same priority number will result in an undefined result for the callback functions execution order.

To help passing parameters between callback functions of the same request, the value `struct _u_response.shared_data` can bse used. But it will not be allocated or freed by the framework, the program using this variable must free by itself.

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

In your program where you want to start the web server, simply execute the function `ulfius_start_framework(struct _u_instance * u_instance)` for a non-secure http connection or `ulfius_start_secure_framework(struct _u_instance * u_instance, const char * key_pem, const char * cert_pem)` for a secure https connection, using a valid private key and a valid corresponding server certificate, see openssl documentation for certificate generation. Those function accept the previously declared `instance` as first parameter. You can reuse the same callback function as much as you want for different endpoints. On success, these functions returns `U_CALLBACK_CONTINUE` or `U_CALLBACK_COMPLETE`, otherwise an error code.

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

### Callback functions

The callback function is the function executed when a user calls an endpoint managed by your webservice (as defined in your `struct _u_endpoint` list).

The callback function has the following signature:

```C
int (* callback_function)(const struct _u_request * request, // Input parameters (set by the framework)
                          struct _u_response * response,     // Output parameters (set by the user)
                          void * user_data);
```

In the callback function definition, the variables `request` and `response` will be initialized by the framework, and the `user_data` variable will be assigned to the user_data defined in your endpoint list definition.

The request variable is defined as:

```C
/**
 * 
 * Structure of request parameters
 * 
 * Contains request data
 * http_verb:                 http method (GET, POST, PUT, DELETE, etc.), use '*' to match all http methods
 * http_url:                  url used to call this callback function or full url to call when used in a ulfius_send_http_request
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
  char *               http_verb;
  char *               http_url;
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
 * stream_size:          size of the streamed data (-1 if unknown)
 * stream_block_size:    size of each block to be streamed, set according to your system
 * stream_user_data:     user defined data that will be available in your callback stream functions
 * shared_data:          any data shared between callback functions, must be allocated and freed by the callback functions
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
  size_t             stream_size;
  unsigned int       stream_block_size;
  void             * stream_user_data;
  void *             shared_data;
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
                                int (* stream_callback) (void * stream_user_data, uint64_t offset, char * out_buf, size_t max),
                                void (* stream_callback_free) (void * stream_user_data),
                                size_t stream_size,
                                unsigned int stream_block_size,
                                void * stream_user_data);
```

### JSON body in request and response

In Ulfius 2.0, hard dependency with libjansson has been removed, the jansson library is now optional but enabled by default.

If you want to remove libjansson dependency, build Ulfius library with the flag `JANSSONFLAG=-DU_DISABLE_JANSSON`

```
$ make JANSSONFLAG=-DU_DISABLE_JANSSON
```

if libjansson library is enabled, the following functions are available in Ulfius:

```C
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

The `jansson` api documentation is available at the following address: [Jansson documentation](https://jansson.readthedocs.org/).

#### Callback functions return value

The callback returned value can have the following values:

- `U_CALLBACK_CONTINUE`: The framework can transfer the request and the response to the next callback function in priority order if there is one, or complete the transaction and send back the response to the client.
- `U_CALLBACK_COMPLETE`: The framework must complete the transaction and send the response to the client without calling any further callback function.
- `U_CALLBACK_UNAUTHORIZED`: The framework must complete the transaction without calling any further callback function and send an unauthorized response to the client with the status 401, the body specified and the `auth_realm` value if specified.
- `U_CALLBACK_ERROR`: An error occured during execution, the framework must complete the transaction without calling any further callback function and send an error 500 to the client.

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

### Memory management

The Ulfius framework will automatically free the variables referenced by the request and responses structures, except for `struct _u_response.shared_data`, so you must use dynamically allocated values for the response pointers.

### Character encoding

You may be careful with characters encoding if you use non UTF8 characters in your application or webservice, and especially if you use different encodings in the same application. Ulfius has not been fully tested in cases like that.

### Cookie management

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

See `examples/sheep_counter` for a file upload example.

### Streaming data

If you need to stream data, i.e. send a variable and potentially large amount of data, you can define and use `stream_callback_function` in the `struct _u_response`.

Not that if you stream data to the client, any data that was in the `response->binary_body` will be ignored. You must at least set the function pointer `struct _u_response.stream_callback` to stream data. Set `stream_size` to -1 if you don't know the size of the data you need to send, like in audio stream for example. Set `stream_block_size` according to you system resources to avoid out of memory errors, also, set `stream_callback_free` with a pointer to a function that will free values allocated by your stream callback function, as a `close()` file for example, and finally, you can set `stream_user_data` to a pointer.

You can use the function `ulfius_set_stream_response` to set those parameters.

The prototype of the `stream_callback` function is the following:

```C
int stream_callback (void * stream_user_data, // Your predefined user_data
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

Check the program `stream_example` in the example folder.

### Websockets communication

Websocket communication is possible via callback functions defined in your program. The framework will handle sending and receiving messages with the clients, and your program will deal with high level functions to facilitate the communication process. The communication will respect the [RFC6455](https://tools.ietf.org/html/rfc6455) of the websocket protocol.

#### Starting a websocket communication

To start a websocket communication between the client and your program, you must use the dedicated function `ulfius_start_websocket_cb` with proper values:

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

In Ulfius Implementation, if you specify a list of protocols as a string of protocol names, separated by a comma, Ulfius will check each one and see if they match the list of protocols specified by the client. The resulting protocol list will be sent back to the client.
If no protocol match your list, the connection will be closed by the framework and will return an error 400 to the client.
If you set a `NULL` value, Ulfius will accept all protocols sent by the client.

This behaviour is the same with websocket extension check.

3 callback functions are available for the websocket implementation:

- `websocket_manager_callback`: This function will be called in a separate thread, and the websocket will remain open as long as this callback function is not completed. In this function, your program will have access to the websocket status (connected or not), and the list of messages sent and received. When this function ends, the websocket will close itself automatically.
- `websocket_incoming_message_callback`: This function will be called every time a new message is sent by the client. Although, it is in synchronous mode, which means that you won't have 2 different `websocket_incoming_message_callback` of the same websocket executed at the same time.
- `websocket_onclose_callback`: This function will be called right after the websocket connection is closed, but before the websocket structure is cleaned.

You must specify at least one of the callback functions `websocket_manager_callback` or `websocket_incoming_message_callback`.

When the function `ulfius_stop_framework` is called, it will wait for all running websockets to end by themselves, there is no force close. So if you have a `websocket_manager_callback` function running, you *MUST* end this function in order to make a clean stop of the http daemon.

For each of these callback function, you can specify a `*_user_data` pointer containing any data you need.

#### Manipulating websocket data structure

The websocket manager data structure definition is the following:

```C
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
};
```

The most important data you will need are `message_list_incoming`, `message_list_outcoming` and `connected`. The other ones are reserved for the framework, do not alterate them.

The `connected` value will tell you wether the websocket is connected or not, `message_list_incoming`, `message_list_outcoming` will contain the message lists. These lists are updated in real time during the execution, do not try to update them directly.

When an incoming message will arrive, the `message_list_incoming` table will be updated, and the `websocket_incoming_message_callback` function will be called too, depending on your application behaviour, use the method you prefer for incoming messages.

#### Messages manipulation

A message has the following structure:

```C
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
```

A message list has the following strucure:

```C

struct _websocket_message_list {
  struct _websocket_message ** list;
  size_t len;
};
```

If you want to send a message to the client, you must use the dedicated function `ulfius_websocket_send_message`:

```C
/**
 * Send a message in the websocket
 * Return U_OK on success
 */
int ulfius_websocket_send_message(struct _websocket_manager * websocket_manager,
                                  const uint8_t opcode,
                                  const uint64_t data_len,
                                  const char * data);
```

You can also pop the first message of the incoming or outcoming if you need to with `ulfius_websocket_pop_first_message`, this will remove the first message of the list, and return it as a pointer. You can free a message using the function `ulfius_clear_websocket_message`:

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
size_t u_map_get_length(const struct _u_map * u_map, const const char * key);

/**
 * get the value length corresponding to the specified key in the u_map
 * return -1 if no match found
 * search is case insensitive
 */
size_t u_map_get_case_length(const struct _u_map * u_map, const const char * key);

/**
 * get the value corresponding to the specified key in the u_map
 * return NULL if no match found
 * search is case sensitive
 */
const char * u_map_get(const struct _u_map * u_map, const const char * key);

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

## Output request functions

Ulfius allows output functions to send HTTP or SMTP requests. These functions use `libcurl`. You can disable these functions by appending the argument `CURLFLAG=-DU_DISABLE_CURL` when you build the library:

```
$ make CURLFLAG=-DU_DISABLE_CURL
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

It's used to send emails (without attached files) via a smtp server.

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
