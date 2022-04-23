/**
 *
 * Static file server with compression Ulfius callback
 *
 * Copyright 2020-2022 Nicolas Mora <mail@babelouest.org>
 *
 * Version 20220423
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
 * `files_path`: path to the DocumentRoot folder, can be relative or absolute
 * `url_prefix`: prefix used to access the callback function
 * `mime_types`: a `struct _u_map` containing a set of mime-types with file extension as key and mime-type as value
 * `mime_types_compressed`: A `string_array` structure containing the list of mime-types allowed for compression
 * `mime_types_compressed_size`: The number of elements in `mime_types_compressed`
 * `map_header`: a `struct _u_map` containing a set of headers that will be added to all responses within the `static_file_callback`
 * `redirect_on_404`: redirct uri on error 404, if NULL, send 404
 * `allow_gzip`: Set to true if you want to allow gzip compression (default true)
 * `allow_deflate`: Set to true if you want to allow deflate compression (default true)
 * `allow_cache_compressed`: set to true if you want to allow memory cache for compressed files (default true)
 * `lock`: mutex lock (do not touch this variable)
 * `gzip_files`: a `struct _u_map` containing cached gzip files
 * `deflate_files`: a `struct _u_map` containing cached deflate files
 * 
 * example of mime-types used in Hutch:
 * {
 *   key = ".html"
 *   value = "text/html"
 * },
 * {
 *   key = ".css"
 *   value = "text/css"
 * },
 * {
 *   key = ".js"
 *   value = "application/javascript"
 * },
 * {
 *   key = ".png"
 *   value = "image/png"
 * },
 * {
 *   key = ".jpg"
 *   value = "image/jpeg"
 * },
 * {
 *   key = ".jpeg"
 *   value = "image/jpeg"
 * },
 * {
 *   key = ".ttf"
 *   value = "font/ttf"
 * },
 * {
 *   key = ".woff"
 *   value = "font/woff"
 * },
 * {
 *   key = ".woff2"
 *   value = "font/woff2"
 * },
 * {
 *   key = ".map"
 *   value = "application/octet-stream"
 * },
 * {
 *   key = "*"
 *   value = "application/octet-stream"
 * }
 * 
 */

#ifndef _U_STATIC_COMPRESSED_INMEMORY_WEBSITE
#define _U_STATIC_COMPRESSED_INMEMORY_WEBSITE

#define _U_W_BLOCK_SIZE 256

struct _u_compressed_inmemory_website_config {
  char          * files_path;
  char          * url_prefix;
  struct _u_map   mime_types;
  char **         mime_types_compressed;
  size_t          mime_types_compressed_size;
  struct _u_map   map_header;
  char          * redirect_on_404;
  int             allow_gzip;
  int             allow_deflate;
  int             allow_cache_compressed;
  pthread_mutex_t lock;
  struct _u_map   gzip_files;
  struct _u_map   deflate_files;
};

int u_init_compressed_inmemory_website_config(struct _u_compressed_inmemory_website_config * config);

void u_clean_compressed_inmemory_website_config(struct _u_compressed_inmemory_website_config * config);

int u_add_mime_types_compressed(struct _u_compressed_inmemory_website_config * config, const char * mime_type);

int callback_static_compressed_inmemory_website (const struct _u_request * request, struct _u_response * response, void * user_data);

#endif
