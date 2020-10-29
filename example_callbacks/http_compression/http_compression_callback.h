/**
 *
 * Response body compression callback function for Ulfius Framework
 *
 * Copyright 2020 Nicolas Mora <mail@babelouest.org>
 *
 * Version 20201028
 *
 * Compress the response body using `deflate` or `gzip` depending on the request header `Accept-Encoding` and the callback configuration.
 * The rest of the response, status, headers, cookies won't change.
 * After compressing response body, the response header Content-Encoding will be set accordingly.
 * 
 */

#ifndef _U_HTTP_COMPRESSION
#define _U_HTTP_COMPRESSION

/**
 * If both values are set to true, the first compression algorithm used will be gzip
 */
struct _http_compression_config {
  int allow_gzip;
  int allow_deflate;
};

/**
 * Compress response->binary_body using gzip or deflate algorithm
 * depending on the request header Accept-Encoding and the
 * struct _http_compression_config configuration value
 * If user_data is NULL, it will considered as allow_gzip and allow_deflate to true
 * After compressing response body, will set response header Content-Encoding accordingly
 */
int callback_http_compression (const struct _u_request * request, struct _u_response * response, void * user_data);

#endif
