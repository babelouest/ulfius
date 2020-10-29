# Response body compression callback function for Ulfius Framework

Compress the response body using `deflate` or `gzip` depending on the request header `Accept-Encoding` and the callback configuration. The rest of the response, status, headers, cookies won't change. After compressing response body, the response header Content-Encoding will be set accordingly.

By default, this callback will use `gzip` algorithm if possible, `deflate` otherwise.

To use this callback function, declare it in your endpoint list with the lowest priority to make sure it's called at the end of the callback functions list. This callback function won't be called if a websocket or a streaming response is declared in a previous callback.

Simple usage example:

```C
// Will call callback_http_compression for every endpoint
ulfius_add_endpoint_by_val(&instance, "*", NULL, "*", 1000, &callback_http_compression, NULL);
```

Use `callback_http_compression` for `deflate` compression only:

```C
struct _http_compression_config config;
config.allow_gzip = 0;
config.allow_deflate = 1;

ulfius_add_endpoint_by_val(&instance, "*", NULL, "*", 1000, &callback_http_compression, &config);
```
