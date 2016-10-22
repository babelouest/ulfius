# Ulfius

Web Framework for REST Applications in C.

Based on [GNU Libmicrohttpd](https://www.gnu.org/software/libmicrohttpd/) for the backend web server, [Jansson](http://www.digip.org/jansson/) for the json manipulation library, and [Libcurl](http://curl.haxx.se/libcurl/) for the http/smtp client API.

Used to facilitate creation of web applications in C programs with a small memory footprint, as in embedded systems applications.

## Hello World! example application

The source code of a hello world using Ulfius is the following:

```c
/**
 * test.c
 * Small Hello World! example
 * to compile with gcc, run the following command
 * gcc -o test test.c -lulfius
 */
#include <ulfius.h>
#include <string.h>
#include <stdio.h>

#define PORT 8080

/**
 * Callback function for the web application on /helloworld url call
 */
int callback_hello_world (const struct _u_request * request, struct _u_response * response, void * user_data) {
  ulfius_set_string_response(response, 200, "Hello World!");
  return U_OK;
}

/**
 * main function
 */
int main(void) {
  struct _u_instance instance;

  // Initialize instance with the port number
  if (ulfius_init_instance(&instance, PORT, NULL) != U_OK) {
    fprintf(stderr, "Error ulfius_init_instance, abort\n");
    return(1);
  }

  // Endpoint list declaration
  ulfius_add_endpoint_by_val(&instance, "GET", "/helloworld", NULL, NULL, NULL, NULL, &callback_hello_world, NULL);

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

## Installation

See `INSTALL.md` file for installation details

## Documentation

See `API.md` file for API documentation details

## Example source code

Example programs are available to see the different functionalities available, see `example` folder for detailed sample source codes and documentation.

## Questions, problems ?

I'm open for questions and suggestions, feel free to open an issue or send a pull request if you feel like it!
