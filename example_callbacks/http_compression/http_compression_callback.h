/**
 *
 * Response body compression callback function for Ulfius Framework
 *
 * Copyright 2020 Nicolas Mora <mail@babelouest.org>
 *
 * Version 20201213
 *
 * Compress the response body using `deflate` or `gzip` depending on the request header `Accept-Encoding` and the callback configuration.
 * The rest of the response, status, headers, cookies won't change.
 * After compressing response body, the response header Content-Encoding will be set accordingly.
 *
 * The MIT License (MIT)
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * 
 */

#ifndef _U_HTTP_COMPRESSION
#define _U_HTTP_COMPRESSION

#define _U_C_BLOCK_SIZE 256

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
