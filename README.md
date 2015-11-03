# Ulfius

Web Framework for REST Applications in C.

Based on [GNU Libmicrohttpd](https://www.gnu.org/software/libmicrohttpd/) for the web server backend and [Jansson](http://www.digip.org/jansson/) for the json manipulation library.

Used to facilitate creation of web applications in C programs with a small memory footprint, like in embedded systems applications.

# Prerequisites

To install the dependencies, for Debian based distributions (Debian, Ubuntu, Raspbian, etc.), run as root:

```shell
# apt-get install libmicrohttpd libmicrohttpd-dev libjansson libjansson-dev
```

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

## API functions

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
 *                    on an url_format that ends with /*, the rest of the url will not be tested
 * user_data:         a pointer to a data or a structure that will be available in the callback function
 * callback_function: a pointer to a function that will be executed each time the endpoint is called
 *                    you must declare the function as described.
 * 
 */
struct _u_endpoint {
  char * http_method;
  char * url_format;
  void * user_data;
  int (* callback_function)(const struct _u_request * request, // Input parameters (set by the framework)
       struct _u_response * response,     // Output parameters (set by the user)
       void * user_data);
};
```

Your `struct _u_endpoint` array MUST end with an empty `struct _u_endpoint`.

for example, you can declare an endpoint list like this:

```c
  struct _u_endpoint endpoint_list[] = {
    {"GET", "/test", NULL, &callback_get_test},
    {"POST", "/test", NULL, &callback_post_test},
    {"GET", "/test/:foo", "user data 1", &callback_all_test_foo},
    {"POST", "/test/:foo", "user data 2", &callback_all_test_foo},
    {"PUT", "/test/:foo", "user data 3", &callback_all_test_foo},
    {"DELETE", "/test/:foo", "user data 4", &callback_all_test_foo},
    {"GET", "/json/test", NULL, &callback_get_jsontest},
    {"GET", "/cookie/test/:lang/:extra", NULL, &callback_get_cookietest},
    {NULL, NULL, NULL, NULL}
  };
```

Please note that each time a call is made to the webservice, endpoints will be tested in the same order. On the first matching endpoint, the other ones won't be tested. So be careful on your endpoints declaration order.

The starting point function is ulfius_init_framework, here is its declaration:


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
 */
int ulfius_init_framework(struct _u_instance * u_instance, struct _u_endpoint * endpoint_list);
```

In your program where you want to start the web server, simply execute the function `ulfius_init_framework(struct _u_instance * u_instance, struct _u_endpoint * endpoint_list);` with the previously declared `instance` and `endpoint_list`. You can reuse the same callback function as much as you want for different endpoints. On success, this function returns `true`, otherwise `false`.

### Callback function

A callback function must have the following declaration:

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
 * http_verb:     http method (GET, POST, PUT, DELETE, etc.)
 * http_url:      url used to call this callback function
 * client_ip:     IP address of the client
 * map_url:       map containing the url variables, both from the route and the ?key=value variables
 * map_header:    map containing the header variables
 * map_cookie:    map containing the cookie variables
 * map_post_body: map containing the post body variables (if available)
 * json_body:     json_t * object containing the json body (if available)
 * json_error:    true if the json body was not parsed by jansson (if available)
 * 
 */
struct _u_request {
	char *               http_verb;
	char *               http_url;
	struct sockaddr_in * client_ip;
	struct _u_map *      map_url;
	struct _u_map *      map_header;
	struct _u_map *      map_cookie;
	struct _u_map *      map_post_body;
	json_t *             json_body;
	int                  json_error;
};
```
The response variable is defined as:

```c
/**
 * 
 * Structure of response parameters
 * 
 * Contains response data that must be set by the user
 * status:      HTTP status code (200, 404, 500, etc)
 * map_header:  map containing the header variables
 * map_cookie:  map containing the cookie variables
 * string_body: a string containing the raw body response
 * json_body:   a json_t * object containing the json response
 * 
 */
struct _u_response {
	int             status;
	struct _u_map * map_header;
	struct _u_map * map_cookie;
	char *          string_body;
	json_t *        json_body;
};
```

In the response variable set by the framework to the callback function, the structure is empty, except for the map_cookie which is set to the same key/values as the request `map_cookie`.

The user must set the `status` code and the `string_body` or the `json_body` before the return statement, otherwise the framework will send an error 500 response. If a `string_body` is set, the `json_body` won't be tested. So to return a `json_body` object, you must leave `string_body` with a `NULL` value.

You can find the jansson api documentation at the following address: [Jansson documentation](https://jansson.readthedocs.org/).

The callback function return value is 0 on success. If the return value is other than 0, an error 500 response will be sent to the client.

The Ulfius framework will automatically free the variables referenced by the request and responses structures, so you must use dynamically allocated values.

### Cookie management

The map_cookie structure will contain a set of key/values to set the cookies. You can use the function `ulfius_add_cookie` in your callback function to facilitate cookies management. This function is defined as:

```c

/**
 * ulfius_add_cookie
 * add a cookie to the cookie map
 */
int ulfius_add_cookie(struct _u_map * response_map_cookie, const char * key, const char * value, const char * expires, const uint max_age, 
											const char * domain, const char * path, const int secure, const int http_only);
```

### u_map API

The `struct _u_map` is a simple key/value mapping API. The available functions to use this structure are:

```c

/**
 * initialize a struct _u_map
 * this function MUST be called after a declaration or allocation
 */
void u_map_init(struct _u_map * map);

/**
 * free the struct _u_map and its components
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
```

### Example source code

See `example/example.c` for more detailed sample source codes.

