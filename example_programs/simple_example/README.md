# simple_example

Run a simple webservice that listen on port 8537.

## Compile and run

```bash
$ make test
```

### https connection

If you want this program to listen to a secure (https) connection, create a certificate with its certificate and private keys in to separate files, and run the program with the options `-secure <key_file> <cert_file>`.

## Endpoints available:

- `GET http://localhost:8537/test`: A "Hello World!" response (status 200)
- `GET http://localhost:8537/test/empty`: An empty response (status 200)
- `POST http://localhost:8537/test`: Will display all requests parameters (body, headers, cookies, etc.) given by the client
- `GET http://localhost:8537/test/:foo`: Will display all requests parameters (body, headers, cookies, etc.) given by the client
- `POST http://localhost:8537/test/:foo`: Will display all requests parameters (body, headers, cookies, etc.) given by the client
- `PUT http://localhost:8537/test/:foo`: Will display all requests parameters (body, headers, cookies, etc.) given by the client
- `DELETE http://localhost:8537/test/:foo`: Will display all requests parameters (body, headers, cookies, etc.) given by the client
- `GET http://localhost:8537/test/multiple/:multiple/:multiple/:not_multiple`: Will display all requests parameters (body, headers, cookies, etc.) given by the client, and multiple values in `map_url:multiple` key
- `GET http://localhost:8537/testcookie/:lang/:extra` Will send a cookie to the client with `lang` and `extra` values in it
