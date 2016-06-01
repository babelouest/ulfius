# Authentication example

Provides an application that implement basic authentication

## Compile and run

```bash
$ make
$ ./auth_server
```

## Usage

Open in your browser the url `http://localhost:2884/auth/basic`, then on the authentication prompt, enter the `USER` and `PASSWORD` specified in `auth_server.c` (default test/testpassword) to be authenticated and allowed to access the endpoint.

In another console, you can run the client program to test the use cases

```bash
$ ./auth_client
```
