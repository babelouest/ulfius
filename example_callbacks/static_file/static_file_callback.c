/**
 *
 * Static file server Ulfius callback
 *
 * Copyright 2017 Nicolas Mora <mail@babelouest.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * License as published by the Free Software Foundation;
 * version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU GENERAL PUBLIC LICENSE for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <string.h>
#include <orcania>
#include <yder.h>
#include <ulfius.h>

/**
 * struct static_file_config must be initialized with proper values
 * files_path: path (relative or absolute) to the DocumentRoot folder
 * mime_types: a struct _u_map filled with all the mime-types needed for a static file server
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
struct static_file_config {
  char *          files_path;
  struct _u_map * mime_types;
};

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
 * static file callback endpoint
 */
int callback_hutch_static_file (const struct _u_request * request, struct _u_response * response, void * user_data) {
  void * buffer = NULL;
  size_t length, res;
  FILE * f;
  char * file_requested;
  char * file_path;
  const char * content_type;

  if (user_data != NULL && ((struct static_file_config *)user_data)->files_path != NULL && ((struct static_file_config *)user_data)->mime_types != NULL) {
    file_requested = nstrdup(request->http_url + sizeof(char));
    
    if (strchr(file_requested, '#') != NULL) {
      *strchr(file_requested, '#') = '\0';
    }
    
    if (strchr(file_requested, '?') != NULL) {
      *strchr(file_requested, '?') = '\0';
    }
    
    if (file_requested == NULL || strlen(file_requested) == 0 || 0 == nstrcmp("/", file_requested)) {
      file_requested = "/index.html";
    }
    
    file_path = msprintf("%s%s", ((struct static_file_config *)user_data)->files_path, file_requested);

    if (access(file_path, F_OK) != -1) {
      f = fopen (file_path, "rb");
      if (f) {
        fseek (f, 0, SEEK_END);
        length = ftell (f);
        fseek (f, 0, SEEK_SET);
        buffer = malloc(length*sizeof(void));
        if (buffer) {
          res = fread (buffer, 1, length, f);
          if (res != length) {
            y_log_message(Y_LOG_LEVEL_WARNING, "callback_angharad_static_file - fread warning, reading %ld while expecting %ld", res, length);
          }
        }
        fclose (f);
      }

      if (buffer) {
        content_type = u_map_get_case(((struct static_file_config *)user_data)->mime_types, get_filename_ext(file_requested));
        if (content_type == NULL) {
          content_type = u_map_get(((struct static_file_config *)user_data)->mime_types, "*");
          y_log_message(Y_LOG_LEVEL_WARNING, "Static File Server - Unknown mime type for extension %s", get_filename_ext(file_requested));
        }
        response->binary_body = buffer;
        response->binary_body_length = length;
        u_map_put(response->map_header, "Content-Type", content_type);
      } else {
        ulfius_set_string_response(response, 500, "Error processing static file");
        y_log_message(Y_LOG_LEVEL_ERROR, "Static File Server - Internal error in %s", request->http_url);
      }
    } else {
      ulfius_set_string_response(response, 404, "Resource not found");
    }
    free(file_path);
    free(file_requested);
    return U_CALLBACK_CONTINUE;
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "Static File Server - Error, user_data is NULL");
    return U_CALLBACK_ERROR;
  }
}
