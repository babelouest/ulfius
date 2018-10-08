# websocket_server

Run a webservice service or client.

## Compile and run

### HTTP mode

Run a HTTP (non secure) webservice

```bash
$ make debug
$ LD_LIBRARY_PATH=../../src: ./websocket_server
```

### HTTPS mode

You must have a certificate key and pem files available. To generate one, you can for example use the following command:

```bash
$ openssl req -new -x509 -days 365 -nodes -out cert.pem -keyout cert.key -sha256
```

Run a HTTPS (secure) webservice

```bash
$ make debug
$ LD_LIBRARY_PATH=../../src: ./websocket_server -https cert.key cert.pem
```

## Endpoints available:

### Websocket

- `GET http://localhost:9275/websocket`: Websocket basic endpoint that will send binary or text messages every 2 seconds
- `GET http://localhost:9275/websocket/file`: Websocket endpoint that will accept a binary message that can contain a file
- `GET http://localhost:9275/websocket/echo`: Websocket echo endpoint, all received messages will be resend to the client

### Static file

- `GET http://localhost:9275/static`: will serve the files located in the `static` folder

## Test the websocket

### Browser

Open in your browser the url [http[s]://localhost:9275/static/index.html](http://localhost:9275/static/index.html), this will open an HTML page where you can test the websockets behaviour.

### Ulfius client

Run the `websocket_client` program while the `websocket_server` is running.

### HTTP mode

Run a HTTP (non secure) webservice

```bash
$ make debug
$ LD_LIBRARY_PATH=../../src: ./websocket_client
```

### HTTPS mode

Run a HTTPS (secure) webservice

```bash
$ make debug
$ LD_LIBRARY_PATH=../../src: ./websocket_client -https
```
