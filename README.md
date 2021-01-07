# Ulfius HTTP Framework

![.github/workflows/ccpp.yml](https://github.com/babelouest/ulfius/workflows/.github/workflows/ccpp.yml/badge.svg)
![CodeQL](https://github.com/babelouest/ulfius/workflows/CodeQL/badge.svg)
[![CII Best Practices](https://bestpractices.coreinfrastructure.org/projects/3195/badge)](https://bestpractices.coreinfrastructure.org/projects/3195)

HTTP Framework for REST Applications in C.

Based on [GNU Libmicrohttpd](https://www.gnu.org/software/libmicrohttpd/) for the backend web server, [Jansson](http://www.digip.org/jansson/) for the json manipulation library, and [Libcurl](http://curl.haxx.se/libcurl/) for the http/smtp client API.

Used to facilitate creation of web applications in C programs with a small memory footprint, as in embedded systems applications.

You can create webservices in HTTP or HTTPS mode, stream data, or implement server websockets.

## Hello World! example application

The source code of a hello world using Ulfius is the following:

```C
/**
 * test.c
 * Small Hello World! example
 * to compile with gcc, run the following command
 * gcc -o test test.c -lulfius
 */
#include <stdio.h>
#include <ulfius.h>

#define PORT 8080

/**
 * Callback function for the web application on /helloworld url call
 */
int callback_hello_world (const struct _u_request * request, struct _u_response * response, void * user_data) {
  ulfius_set_string_body_response(response, 200, "Hello World!");
  return U_CALLBACK_CONTINUE;
}

/**
 * main function
 */
int main(void) {
  struct _u_instance instance;

  // Initialize instance with the port number
  if (ulfius_init_instance(&instance, PORT, NULL, NULL) != U_OK) {
    fprintf(stderr, "Error ulfius_init_instance, abort\n");
    return(1);
  }

  // Endpoint list declaration
  ulfius_add_endpoint_by_val(&instance, "GET", "/helloworld", NULL, 0, &callback_hello_world, NULL);

  // Start the framework
  if (ulfius_start_framework(&instance) == U_OK) {
    printf("Start framework on port %d\n", instance.port);

    // Wait for the user to press <enter> on the console to quit the application
    getchar();
  } else {
    fprintf(stderr, "Error starting framework\n");
  }
  printf("End framework\n");

  ulfius_stop_framework(&instance);
  ulfius_clean_instance(&instance);

  return 0;
}
```

## Main features

### Webservice

- Create a webservice in a separate thread, the endpoint is identified by its method (ex: `GET`, `POST`, `PUT`, `DELETE`, etc.) and its URL path with its optional parameters (ex: `/api/doc/@id`). The webservice is executed in a callback function.

- Stream large amount of data with a reduced memory footprint.

- Websocket service, the websocket messages exchange is executed in dedicated callback functions.

### Client requests

- Client http[s] and smtp requests execution, the response is parsed in a dedicated structure.

- Client websocket request execution, the websocket messages exchange is executed in dedicated callback functions.

### Websockets

- Create a websocket service application

- Create websocket client application

- CLI to connect to a remote websocket: [uwsc](https://github.com/babelouest/ulfius/tree/master/tools/uwsc)

## Installation

See [INSTALL.md](INSTALL.md) file for installation details

## Documentation

See [API.md](API.md) file for API documentation details

See the [online documentation](https://babelouest.github.io/ulfius/) for a doxygen format of the API documentation.

## Example programs source code

Example programs are available to understand the different functionalities available, see [example_programs](https://github.com/babelouest/ulfius/blob/master/example_programs) folder for detailed sample source codes and documentation.

## Example callback functions

Example callback functions are available in the folder [example_callbacks](https://github.com/babelouest/ulfius/blob/master/example_callbacks). The example callback functions available are:
- static file server: to provide static files of a specific folder
- oauth2 bearer: to check the validity of a Oauth2 bearer jwt token. Requires [libjwt](https://github.com/benmcollins/libjwt).

## Projects using Ulfius framework

- [Glewlwyd](https://github.com/babelouest/glewlwyd), a lightweight SSO server that provides OAuth2 and OpenID Connect authentication protocols
- [Le Biniou](https://biniou.net/), displays images that evolve with sound
- [Angharad](https://github.com/babelouest/angharad), House automation system for ZWave and other types of devices
- [Hutch](https://github.com/babelouest/hutch), a safe locker for passwords and other secrets, using JavaScript client side encryption only
- [Taliesin](https://github.com/babelouest/taliesin), a lightweight audio streaming server
- [Taulas Raspberry Pi Serial interface](https://github.com/babelouest/taulas/tree/master/taulas_raspberrypi_serial), an interface for Arduino devices that implement [Taulas](https://github.com/babelouest/taulas/) protocol, a house automation protocol for Angharad

## Questions, problems ?

I'm open for questions and suggestions, feel free to open an [issue](https://github.com/babelouest/ulfius/issues), a [pull request](https://github.com/babelouest/ulfius/pulls) or send me an [e-mail](mailto:mail@babelouest.org) if you feel like it!

You can visit the IRC channel #ulfius on the [Freenode](https://freenode.net/) network.
