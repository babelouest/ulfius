/**
 *
 * Static file server Ulfius callback
 *
 * Copyright 2017-2020 Nicolas Mora <mail@babelouest.org>
 *
 * Version 20201028
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
 * struct static_file_config must be initialized with proper values
 * files_path: path (relative or absolute) to the DocumentRoot folder
 * url_prefix: prefix used to access the callback function
 * mime_types: a struct _u_map filled with all the mime-types needed for a static file server
 * redirect_on_404: redirct uri on error 404, if NULL, send 404
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

#ifndef _U_STATIC_FILE
#define _U_STATIC_FILE

#define STATIC_FILE_CHUNK 256

struct _static_file_config {
  char          * files_path;
  char          * url_prefix;
  struct _u_map * mime_types;
  struct _u_map * map_header;
  char          * redirect_on_404;
};

int callback_static_file (const struct _u_request * request, struct _u_response * response, void * user_data);
const char * get_filename_ext(const char *path);

#endif
