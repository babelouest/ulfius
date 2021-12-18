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

#include <string.h>
#include <ulfius.h>
#include <zlib.h>

#include "http_compression_callback.h"

#define U_COMPRESS_NONE 0
#define U_COMPRESS_GZIP 1
#define U_COMPRESS_DEFL 2

#define U_ACCEPT_HEADER  "Accept-Encoding"
#define U_CONTENT_HEADER "Content-Encoding"

#define U_ACCEPT_GZIP    "gzip"
#define U_ACCEPT_DEFLATE "deflate"

#define U_GZIP_WINDOW_BITS 15
#define U_GZIP_ENCODING    16

#define CHUNK 0x4000

static void * u_zalloc(void * q, unsigned n, unsigned m) {
  (void)q;
  return o_malloc((size_t) n * m);
}

static void u_zfree(void *q, void *p) {
  (void)q;
  o_free(p);
}

int callback_http_compression (const struct _u_request * request, struct _u_response * response, void * user_data) {
  struct _http_compression_config * config = (struct _http_compression_config *)user_data;
  char ** accept_list = NULL;
  int ret = U_CALLBACK_IGNORE, compress_mode = U_COMPRESS_NONE, res;
  z_stream defstream;
  char * data_zip = NULL;
  size_t data_zip_len = 0;

  if (response->binary_body_length && u_map_has_key_case(request->map_header, U_ACCEPT_HEADER)) {
    if (split_string(u_map_get_case(request->map_header, U_ACCEPT_HEADER), ",", &accept_list)) {
      if ((config == NULL || config->allow_gzip) && string_array_has_trimmed_value((const char **)accept_list, U_ACCEPT_GZIP)) {
        compress_mode = U_COMPRESS_GZIP;
      } else if ((config == NULL || config->allow_deflate) && string_array_has_trimmed_value((const char **)accept_list, U_ACCEPT_DEFLATE)) {
        compress_mode = U_COMPRESS_DEFL;
      }

      if (compress_mode != U_COMPRESS_NONE) {
        defstream.zalloc = u_zalloc;
        defstream.zfree = u_zfree;
        defstream.opaque = Z_NULL;
        defstream.avail_in = (uInt)response->binary_body_length;
        defstream.next_in = (Bytef *)response->binary_body;

        if (compress_mode == U_COMPRESS_GZIP) {
          if (deflateInit2(&defstream, 
                           Z_BEST_COMPRESSION, 
                           Z_DEFLATED,
                           U_GZIP_WINDOW_BITS | U_GZIP_ENCODING,
                           8,
                           Z_DEFAULT_STRATEGY) != Z_OK) {
            y_log_message(Y_LOG_LEVEL_ERROR, "callback_http_compression - Error deflateInit (gzip)");
            ret = U_CALLBACK_ERROR;
          }
        } else {
          if (deflateInit(&defstream, Z_BEST_COMPRESSION) != Z_OK) {
            y_log_message(Y_LOG_LEVEL_ERROR, "callback_http_compression - Error deflateInit (deflate)");
            ret = U_CALLBACK_ERROR;
          }
        }
        if (ret == U_CALLBACK_IGNORE) {
          do {
            if ((data_zip = o_realloc(data_zip, data_zip_len+_U_C_BLOCK_SIZE)) != NULL) {
              defstream.avail_out = _U_C_BLOCK_SIZE;
              defstream.next_out = ((Bytef *)data_zip)+data_zip_len;
              switch ((res = deflate(&defstream, Z_FINISH))) {
                case Z_OK:
                case Z_STREAM_END:
                case Z_BUF_ERROR:
                  break;
                default:
                  y_log_message(Y_LOG_LEVEL_ERROR, "callback_http_compression - Error deflate %d", res);
                  ret = U_CALLBACK_ERROR;
                  break;
              }
              data_zip_len += _U_C_BLOCK_SIZE - defstream.avail_out;
            } else {
              y_log_message(Y_LOG_LEVEL_ERROR, "callback_http_compression - Error allocating resources for data_zip");
              ret = U_CALLBACK_ERROR;
            }
          } while (U_CALLBACK_IGNORE == ret && defstream.avail_out == 0);

          if (ret == U_CALLBACK_IGNORE) {
            ulfius_set_binary_body_response(response, response->status, (const char *)data_zip, defstream.total_out);
            u_map_put(response->map_header, U_CONTENT_HEADER, compress_mode==U_COMPRESS_GZIP?U_ACCEPT_GZIP:U_ACCEPT_DEFLATE);
          }
          deflateEnd(&defstream);
          o_free(data_zip);
        }
      }
    }
    free_string_array(accept_list);
  }

  return ret;
}
