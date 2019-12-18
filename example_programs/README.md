# Ulfius Framework example programs

Those programs require ulfius to be installed with its dependencies. To do so, in the root folder, run `make && sudo make install` to compile and install the library. Refer to the `README.md` for more information.

Each program implement some functionalities provided by Ulfius, such as url parsing, request filling and response manipulation. For each example program, you can look at its `README.md`. Usually, a simple `make ` or `make test` is sufficient to run the example program. Or you can build all of them using the `CMake` script.

## Example programs available

The example programs were developped to help implementing the functionnalities of the framework and test them. The tested functionnalities are:

- `simple_example`: endpoint tests, url parameters, body parameters and cookies
- `sheep_counter`: file server, upload file, JSON parameters
- `injection_example`: endpoints injection and removal during the program execution
- `proxy_example`: proxyfies calls to another url
- `auth_example`: HTTP Basic Auth and TLS certificates authentication
- `stream_example`: data streaming (server and client side)
- `request_example`: send http and smtp requests
- `test_u_map`: `struct _u_map` tests
- `multiple_callbacks_example`: Run multiple callback functions on a single endpoint
- `websocket_example`: Websocket client and server

## Build

### CMake - Multi architecture

You can build Ulfius example programs using `cmake`, Ulfius library must be already installed, example:

```shell
$ cd example_programs
$ mkdir build
$ cd build
$ cmake ..
$ make
$ ./simple_example
```

The available options for `cmake` are:
- `-DWITH_JANSSON=[on|off]` (default `on`): Build example programs with Jansson dependency
- `-DWITH_CURL=[on|off]` (default `on`): Build example programs with libcurl dependency
- `-DWITH_WEBSOCKET=[on|off]` (default `on`): Build example program with websocket functions, not available for Windows, requires libmicrohttpd 0.9.53 minimum and GnuTLS installed.

### Good ol' Makefile

You can either use the `Makefile` available in the `example_programs` folder or each `Makefile` in each subfolders. If your ulfius library is built with options like `make CURLFLAG=1`, you must use the same option to build the examples.

```shell
$ # Build all example programs
$ cd example_programs
$ make
$ ./simple_example/simple_example
$ # Build only simple_example
$ cd example_programs/simple_example
$ make && make test
```
