# websocket_example

Run a webservice with a websocket endpoint that will print the client messages, and send 10 messages at the same time, one per seconds, then close the connection.

## Compile and run

### HTTP mode

Run a HTTP (non secure) webservice

```bash
$ make debug
$ LD_LIBRARY_PATH=../../src: ./websocket_example
```

### HTTPS mode

You must have a certificate key and pem files available. To generate one, you can for example use the following command:

```bash
$ openssl req -new -x509 -days 365 -nodes -out cert.pem -keyout cert.key -sha256
```

Run a HTTPS (secure) webservice

```bash
$ make debug
$ LD_LIBRARY_PATH=../../src: ./websocket_example -https cert.key cert.pem
```

## Endpoints available:

### Websocket

- `GET http://localhost:9275/websocket`: Websocket endpoint

### Static file

- `GET http://localhost:9275/static`: will serve the files located in the `static` folder

## Test the websocket

Open in your browser the url [http[s]://localhost:9275/static/index.html](http://localhost:9275/static/index.html), this will open an HTML page where you can test the websockets behaviour.
