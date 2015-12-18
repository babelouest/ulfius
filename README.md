# Ulfius

Web Framework for REST Applications in C.

Based on [GNU Libmicrohttpd](https://www.gnu.org/software/libmicrohttpd/) for the web server backend, [Jansson](http://www.digip.org/jansson/) for the json manipulation library, and [Libcurl](http://curl.haxx.se/libcurl/) for the send http request API.

Used to facilitate creation of web applications in C programs with a small memory footprint, like embedded systems applications.

## Hello World! example application

The source code of a hello world using Ulfius could be the following:

```c
#include <ulfius.h>

#define PORT 8080

/**
 * Callback function for the web application on /helloworld url call
 */
int callback_hello_world (const struct _u_request * request, struct _u_response * response, void * user_data) {
  response->string_body = strdup("Hello World!");
  response->status = 200;
  return U_OK;
}

/**
 * main function
 */
int main(void) {
  // Endpoint list declaration
  // The last line is mandatory to mark the end of the array
  struct _u_endpoint endpoint_list[] = {
    {"GET", "/helloworld", &callback_get_test, NULL},
    {NULL, NULL, NULL, NULL}
  };
  
  // Set the framework port number
  struct _u_instance instance;
  instance.port = PORT;
  instance.bind_address = NULL;
  
  // Start the framework
  if (ulfius_init_framework(&instance, endpoint_list) == U_OK) {
    printf("Start framework on port %d\n", instance.port);
    // Wait for the user to press <enter> on the console to quit the application
    getchar();
  } else {
    printf("Error starting framework\n");
  }
  
  printf("End framework\n");
  return ulfius_stop_framework(&instance);
}
```

# Prerequisites

To install the dependencies, for Debian based distributions (Debian, Ubuntu, Raspbian, etc.), run as root:

```shell
# apt-get install libmicrohttpd-dev libjansson-dev libcurl4-gnutls-dev
```

## Note

I suggest libcurl4-gnutls-dev for the example, but any `libcurl*-dev` library should be sufficent, depending on your needs and configuration.

# Installation

Download Ulfius source code from Github, go to src directory, compile and install:

```shell
$ git clone https://github.com/babelouest/ulfius.git
$ cd src
$ make
# make install (as root)
```

By default, the shared library and the header file will be installed in the `/usr/local` location. To change this setting, you can modify the `PREFIX` value in the `Makefile`.

# API Documentation

## Header file

Include file `ulfius.h` in your source file:

```c
#include <ulfius.h>
```

On your linker command, add ulfius as a dependency library, e.g. `-lulfius` for gcc.

## API Documentation

### Return values

When specified, some functions return `U_OK` on success, and other values otherwise. `U_OK` is 0, other values are non-0 values. The defined errors list is the following:
```c
#define U_OK            0 // No error
#define U_ERROR_MEMORY  1 // Error in memory allocation
#define U_ERROR_PARAMS  2 // Error in input parameters
#define U_ERROR_LIBMHD  3 // Error in libmicrohttpd execution
#define U_ERROR_LIBCURL 4 // Error in libcurl execution
```

### Initialization

When initialized, Ulfius runs a thread in background that will listen to the specified port and dispatch the calls to the specified functions. Ulfius does not have high-level features like dependency injection, but its goal is to simplify writing webservices usable in embedded devices with a small memory footprint.

To run a webservice, you must declare a `struct _u_instance` and an array of `struct _u_endpoint`.

The `struct _u_instance` is defined as:

```c
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
```

In the `struct _u_instance` structure, the element `port` must be set to the port number you want to listen to, the element `bind_address` is used if you want to listen only to a specific IP address. The element `mhd_daemon` is used by the framework, don't modify it.

The `struct _u_endpoint` is defined as:

```c
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
```

Your `struct _u_endpoint` array **MUST** end with an empty `struct _u_endpoint`.

for example, you can declare an endpoint list like this:

```c
  struct _u_endpoint endpoint_list[] = {
    {"GET", "/test", &callback_get_test, NULL},
    {"POST", "/test", &callback_post_test, NULL},
    {"GET", "/test/:foo", &callback_all_test_foo, "user data 1"},
    {"POST", "/test/:foo", &callback_all_test_foo, "user data 2"},
    {"PUT", "/test/:foo", &callback_all_test_foo, "user data 3"},
    {"DELETE", "/test/:foo", &callback_all_test_foo, "user data 4"},
    {"GET", "/json/test", &callback_get_jsontest, NULL},
    {"GET", "/cookie/test/:lang/:extra", &callback_get_cookietest, NULL},
    {NULL, NULL, NULL, NULL}
  };
```

Please note that each time a call is made to the webservice, endpoints will be tested in the same order. On the first matching endpoint, the other ones won't be tested. So be careful on your endpoints declaration order.

### Start and stop webservice

#### Start webservice

The starting point function is ulfius_init_framework:

```c
/**
 * ulfius_init_framework
 * Initializes the framework and run the webservice based on the parameters given
 * return truze if no error
 * 
 * u_instance:    pointer to a struct _u_instance that describe its port and bind address
 * endpoint_list: array of struct _u_endpoint that will describe endpoints used for the application
 *                the array MUST have an empty struct _u_endpoint at the end of it
 *                {NULL, NULL, NULL, NULL}
 * return U_OK on success
 */
int ulfius_init_framework(struct _u_instance * u_instance, struct _u_endpoint * endpoint_list);
```

In your program where you want to start the web server, simply execute the function `ulfius_init_framework(struct _u_instance * u_instance, struct _u_endpoint * endpoint_list);` with the previously declared `instance` and `endpoint_list`. You can reuse the same callback function as much as you want for different endpoints. On success, this function returns `true`, otherwise `false`.

#### Stop webservice

To stop the webservice, call the following function:

```c
/**
 * ulfius_stop_framework
 * 
 * Stop the webservice
 * u_instance:    pointer to a struct _u_instance that describe its port and bind address
 * return U_OK on success
 */
int ulfius_stop_framework(struct _u_instance * u_instance);
```

### Callback function

The callback function is the function called when a user calls an endpoint managed by your webservice (as defined in your `struct _u_endpoint` list). A callback function must have the following declaration:

```c
int (* callback_function)(const struct _u_request * request, // Input parameters (set by the framework)
                          struct _u_response * response,     // Output parameters (set by the user)
                          void * user_data);
```

In the callback function definition, the variables `request` and `response` will be set by the framework, and the `user_data` variable will be assigned to the user_data defined in your endpoint list definition.

The request variable is defined as:

```c
/**
 * 
 * Structure of request parameters
 * 
 * Contains request data
 * http_verb:      http method (GET, POST, PUT, DELETE, etc.)
 * http_url:       url used to call this callback function or full url to call when used in a ulfius_request_http
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
```

The response variable is defined as:

```c
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
```

In the response variable set by the framework to the callback function, the structure is empty, except for the map_cookie which is set to the same key/values as the request element `map_cookie`.

The user must set the `string_body` or the `json_body` or the `binary_body` before the return statement, otherwise the framework will send an error 500 response. If a `string_body` is set, the `json_body` or the `binary_body` won't be tested. So to return a `json_body` object, you must leave `string_body` with a `NULL` value. Likewise, if a `json_body` is set, the `binary_body` won't be tested. Finally, if a `binary_body` is set, its length must be set to `binary_body_length`.

You can find the `jansson` api documentation at the following address: [Jansson documentation](https://jansson.readthedocs.org/).

The callback function return value is `U_OK` on success. If the return value is other than `U_OK`, an error 500 response will be sent to the client.

In addition with manipulating the raw parameters of the structures, you can use the `_u_request` and `_u_response` structures by using specific functions designed to facilitate their use and memory management:

```c
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
 * returned value must be free'd
 */
struct _u_request * ulfius_duplicate_request(const struct _u_request * request);

/**
 * create a new response based on the source elements
 * return value must be free'd
 */
struct _u_response * ulfius_duplicate_response(const struct _u_response * response);
```

### Memory management

The Ulfius framework will automatically free the variables referenced by the request and responses structures, so you must use dynamically allocated values for the response pointers.

### Cookie management

The map_cookie structure will contain a set of key/values to set the cookies. You can use the function `ulfius_add_cookie_to_response` in your callback function to facilitate cookies management. This function is defined as:

```c
/**
 * ulfius_add_cookie_to_header
 * add a cookie to the cookie map
 * return U_OK on success
 */
int ulfius_add_cookie_to_response(struct _u_response * response, const char * key, const char * value, const char * expires, const uint max_age, 
                                  const char * domain, const char * path, const int secure, const int http_only);
```

### struct _u_map API

The `struct _u_map` is a simple key/value mapping API. The available functions to use this structure are:

```c
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
 * return U_OK on success
 */
int u_map_put(struct _u_map * u_map, const char * key, const char * value);

/**
 * get the value corresponding to the specified key in the u_map
 * return NULL if no match found
 * search is case sensitive
 * returned value must be free'd after use
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
 * returned value must be free'd after use
 */
char * u_map_get_case(const struct _u_map * u_map, const char * key);

/**
 * Create an exact copy of the specified struct _u_map
 * return a reference to the copy, NULL otherwise
 * returned value must be free'd after use
 */
struct _u_map * u_map_copy(const struct _u_map * source);

/**
 * Return the number of key/values pair in the specified struct _u_map
 * Return -1 on error
 */
int u_map_count(const struct _u_map * source);
```

### Send request API

The function `int ulfius_send_http_request(const struct _u_request * request, struct _u_response * response)` is based on `libcul` api.

It allows to send an http request with the parameters specified by the `_u_request` structure. Use the parameter `_u_request.http_url` to specify the distant url to call.

You can fill the maps in the `_u_request` structure with parameters, they will be used to build the request. Note that if you fill `_u_request.map_post_body` with parameters, the content-type `application/x-www-form-urlencoded` will be use to encode the data.

The response parameters is stored into the `_u_response` structure. If you specify NULL for the response structure, the http call will still be made but no response details will be returned.

Return value is U_OK on success.

This function is defined as:

```c
/**
 * ulfius_send_http_request
 * Send a HTTP request and store the result into a _u_response
 * return U_OK on success
 */
int ulfius_send_http_request(const struct _u_request * request, struct _u_response * response);
```

### Send smtp API

The function `ulfius_send_smtp_email` is used to send emails using a smtp server. It is based on `libcurl` API.

It's used to send emails (without attached files) via a smtp server.

This function is defined as:

```c
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

### Example source code

See `example` folder for detailed sample source codes.

