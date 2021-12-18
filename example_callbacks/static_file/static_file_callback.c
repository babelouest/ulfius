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

#include <orcania.h>
#include <string.h>
#include <ulfius.h>
#include <yder.h>

#include "static_file_callback.h"

/**
 * Return the filename extension
 */
const char * get_filename_ext(const char *path) {
    const char *dot = strrchr(path, '.');
    if(!dot || dot == path) return "*";
    if (strchr(dot, '?') != NULL) {
      *strchr(dot, '?') = '\0';
    }
    return dot;
}

/**
 * Streaming callback function to ease sending large files
 */
static ssize_t callback_static_file_stream(void * cls, uint64_t pos, char * buf, size_t max) {
  (void)(pos);
  if (cls != NULL) {
    return fread (buf, 1, max, (FILE *)cls);
  } else {
    return U_STREAM_END;
  }
}

/**
 * Cleanup FILE* structure when streaming is complete
 */
static void callback_static_file_stream_free(void * cls) {
  if (cls != NULL) {
    fclose((FILE *)cls);
  }
}

/**
 * static file callback endpoint
 */
int callback_static_file (const struct _u_request * request, struct _u_response * response, void * user_data) {
  size_t length;
  FILE * f;
  char * file_requested, * file_path, * url_dup_save;
  const char * content_type;

  /*
   * Comment this if statement if you don't access static files url from root dir, like /app
   */
  if (request->callback_position > 0) {
    return U_CALLBACK_CONTINUE;
  } else if (user_data != NULL && ((struct _static_file_config *)user_data)->files_path != NULL) {
    file_requested = o_strdup(request->http_url);
    url_dup_save = file_requested;
    
    while (file_requested[0] == '/') {
      file_requested++;
    }
    file_requested += o_strlen(((struct _static_file_config *)user_data)->url_prefix);
    while (file_requested[0] == '/') {
      file_requested++;
    }
    
    if (strchr(file_requested, '#') != NULL) {
      *strchr(file_requested, '#') = '\0';
    }
    
    if (strchr(file_requested, '?') != NULL) {
      *strchr(file_requested, '?') = '\0';
    }
    
    if (file_requested == NULL || o_strlen(file_requested) == 0 || 0 == o_strcmp("/", file_requested)) {
      o_free(url_dup_save);
      url_dup_save = file_requested = o_strdup("index.html");
    }
    
    file_path = msprintf("%s/%s", ((struct _static_file_config *)user_data)->files_path, file_requested);

    if (access(file_path, F_OK) != -1) {
      f = fopen (file_path, "rb");
      if (f) {
        fseek (f, 0, SEEK_END);
        length = ftell (f);
        fseek (f, 0, SEEK_SET);
        
        content_type = u_map_get_case(((struct _static_file_config *)user_data)->mime_types, get_filename_ext(file_requested));
        if (content_type == NULL) {
          content_type = u_map_get(((struct _static_file_config *)user_data)->mime_types, "*");
          y_log_message(Y_LOG_LEVEL_WARNING, "Static File Server - Unknown mime type for extension %s", get_filename_ext(file_requested));
        }
        u_map_put(response->map_header, "Content-Type", content_type);
        u_map_copy_into(response->map_header, ((struct _static_file_config *)user_data)->map_header);
        
        if (ulfius_set_stream_response(response, 200, callback_static_file_stream, callback_static_file_stream_free, length, STATIC_FILE_CHUNK, f) != U_OK) {
          y_log_message(Y_LOG_LEVEL_ERROR, "callback_static_file - Error ulfius_set_stream_response");
        }
      }
    } else {
      if (((struct _static_file_config *)user_data)->redirect_on_404 == NULL) {
        ulfius_set_string_body_response(response, 404, "File not found");
      } else {
        ulfius_add_header_to_response(response, "Location", ((struct _static_file_config *)user_data)->redirect_on_404);
        response->status = 302;
      }
    }
    o_free(file_path);
    o_free(url_dup_save);
    return U_CALLBACK_CONTINUE;
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "Static File Server - Error, user_data is NULL or inconsistent");
    return U_CALLBACK_ERROR;
  }
}
