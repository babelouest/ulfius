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
#include <orcania.h>
#include <yder.h>
#include <ulfius.h>

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
 * static file callback endpoint
 */
int callback_static_file (const struct _u_request * request, struct _u_response * response, void * user_data) {
  void * buffer = NULL;
  size_t length, res;
  FILE * f;
  char * file_requested;
  char * file_path;
  const char * content_type;

  if (user_data != NULL && ((struct static_file_config *)user_data)->files_path != NULL) {
    file_requested = o_strdup((request->http_url + (((struct static_file_config *)user_data)->url_prefix!=NULL?strlen(((struct static_file_config *)user_data)->url_prefix):0)));
    
    if (strchr(file_requested, '#') != NULL) {
      *strchr(file_requested, '#') = '\0';
    }
    
    if (strchr(file_requested, '?') != NULL) {
      *strchr(file_requested, '?') = '\0';
    }
    
    if (file_requested == NULL || strlen(file_requested) == 0 || 0 == o_strcmp("/", file_requested)) {
      o_free(file_requested);
      file_requested = o_strdup("index.html");
    }
    
    file_path = msprintf("%s/%s", ((struct static_file_config *)user_data)->files_path, file_requested);

    if (access(file_path, F_OK) != -1) {
      f = fopen (file_path, "rb");
      if (f) {
        fseek (f, 0, SEEK_END);
        length = ftell (f);
        fseek (f, 0, SEEK_SET);
        buffer = o_malloc(length*sizeof(void));
        if (buffer) {
          res = fread (buffer, 1, length, f);
          if (res != length) {
            y_log_message(Y_LOG_LEVEL_WARNING, "callback_angharad_static_file - fread warning, reading %ld while expecting %ld", res, length);
          }
        }
        fclose (f);
      }

      if (buffer) {
        content_type = u_map_get_case(&((struct static_file_config *)user_data)->mime_types, get_filename_ext(file_requested));
        if (content_type == NULL) {
          content_type = u_map_get(&((struct static_file_config *)user_data)->mime_types, "*");
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
      ulfius_set_string_response(response, 404, "File not found");
    }
    o_free(file_path);
    o_free(file_requested);
    return U_CALLBACK_CONTINUE;
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "Static File Server - Error, user_data is NULL or inconsistent");
    return U_CALLBACK_ERROR;
  }
}
