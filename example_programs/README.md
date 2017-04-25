# Ulfius Franework example programs

Those programs require ulfius to be installed with its dependencies. To do so, in the root folder, run `make && sudo make install` to compile and install the library. Refer to the `README.md` for more information.

Each program implement some functionalities provided by Ulfius, such as url parsing, request filling and response manipulation. For each example program, you can look at its `README.md`. In general, a simple `make ` or `make test` is sufficient to run the example program.

## Example programs available

The example programs were developped to help implementing the functionnalities of the framework and test them. The tested functionnalities are:

- `simple_example`: endpoint tests, url parameters, body parameters, json body parameters and cookies
- `sheep_counter`: file server, upload file
- `injection_example`: endpoints injection and remove during the program execution
- `proxy_example`: proxyfies calls to anither url
- `auth_example`: HTTP Basic Auth
- `stream_example`: data streaming (server and client side)
- `request_example`: send http and smtp requests
- `test_u_map`: struct _u_map tests
- `multiple_callbacks_example`: Test new feature in Ulfius 2.0 where you can access multiple callback functions on a single endpoint, one after the other
