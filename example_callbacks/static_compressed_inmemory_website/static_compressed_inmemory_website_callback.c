/**
 *
 * Static file server with compression Ulfius callback
 *
 * Copyright 2020-2021 Nicolas Mora <mail@babelouest.org>
 *
 * Version 20211026
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
#include <pthread.h>
#include <zlib.h>
#include <string.h>
#include <ulfius.h>

#include "static_compressed_inmemory_website_callback.h"

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

/**
 * Return the filename extension
 */
static const char * get_filename_ext(const char *path) {
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
static ssize_t callback_static_file_uncompressed_stream(void * cls, uint64_t pos, char * buf, size_t max) {
  (void)(pos);
  if (cls != NULL) {
    return fread (buf, sizeof(char), max, (FILE *)cls);
  } else {
    return U_STREAM_END;
  }
}

/**
 * Cleanup FILE* structure when streaming is complete
 */
static void callback_static_file_uncompressed_stream_free(void * cls) {
  if (cls != NULL) {
    fclose((FILE *)cls);
  }
}

/**
 * static file callback endpoint
 */
static int callback_static_file_uncompressed (const struct _u_request * request, struct _u_response * response, void * user_data) {
  size_t length;
  FILE * f;
  char * file_requested, * file_path, * url_dup_save;
  const char * content_type;
  int ret = U_CALLBACK_CONTINUE;

  if (user_data != NULL && ((struct _u_compressed_inmemory_website_config *)user_data)->files_path != NULL) {
    file_requested = o_strdup(request->http_url);
    url_dup_save = file_requested;

    file_requested += o_strlen(((struct _u_compressed_inmemory_website_config *)user_data)->url_prefix);
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

    file_path = msprintf("%s/%s", ((struct _u_compressed_inmemory_website_config *)user_data)->files_path, file_requested);

    f = fopen (file_path, "rb");
    if (f) {
      fseek (f, 0, SEEK_END);
      length = ftell (f);
      fseek (f, 0, SEEK_SET);

      content_type = u_map_get_case(&((struct _u_compressed_inmemory_website_config *)user_data)->mime_types, get_filename_ext(file_requested));
      if (content_type == NULL) {
        content_type = u_map_get(&((struct _u_compressed_inmemory_website_config *)user_data)->mime_types, "*");
        y_log_message(Y_LOG_LEVEL_WARNING, "Static File Server - Unknown mime type for extension %s", get_filename_ext(file_requested));
      }
      u_map_put(response->map_header, "Content-Type", content_type);
      u_map_copy_into(response->map_header, &((struct _u_compressed_inmemory_website_config *)user_data)->map_header);

      if (ulfius_set_stream_response(response, 200, callback_static_file_uncompressed_stream, callback_static_file_uncompressed_stream_free, length, CHUNK, f) != U_OK) {
        y_log_message(Y_LOG_LEVEL_ERROR, "Static File Server - Error ulfius_set_stream_response");
      }
    } else {
      if (((struct _u_compressed_inmemory_website_config *)user_data)->redirect_on_404 == NULL) {
        ret = U_CALLBACK_IGNORE;
      } else {
        ulfius_add_header_to_response(response, "Location", ((struct _u_compressed_inmemory_website_config *)user_data)->redirect_on_404);
        response->status = 302;
      }
    }
    o_free(file_path);
    o_free(url_dup_save);
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "Static File Server - Error, user_data is NULL or inconsistent");
    ret = U_CALLBACK_ERROR;
  }
  return ret;
}

int u_init_compressed_inmemory_website_config(struct _u_compressed_inmemory_website_config * config) {
  int ret = U_OK;
  pthread_mutexattr_t mutexattr;

  if (config != NULL) {
    config->files_path                 = NULL;
    config->url_prefix                 = NULL;
    config->redirect_on_404            = NULL;
    config->allow_gzip                 = 1;
    config->allow_deflate              = 1;
    config->mime_types_compressed      = NULL;
    config->mime_types_compressed_size = 0;
    config->allow_cache_compressed     = 1;
    if ((ret = u_map_init(&(config->mime_types))) != U_OK) {
      y_log_message(Y_LOG_LEVEL_ERROR, "u_init_compressed_inmemory_website_config - Error u_map_init mime_types");
    } else if ((ret = u_map_init(&(config->map_header))) != U_OK) {
      y_log_message(Y_LOG_LEVEL_ERROR, "u_init_compressed_inmemory_website_config - Error u_map_init map_header");
    } else if ((ret = u_map_init(&(config->gzip_files))) != U_OK) {
      y_log_message(Y_LOG_LEVEL_ERROR, "u_init_compressed_inmemory_website_config - Error u_map_init gzip_files");
    } else if ((ret = u_map_init(&(config->deflate_files))) != U_OK) {
      y_log_message(Y_LOG_LEVEL_ERROR, "u_init_compressed_inmemory_website_config - Error u_map_init deflate_files");
    } else {
      pthread_mutexattr_init (&mutexattr);
      pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE);
      if (pthread_mutex_init(&(config->lock), &mutexattr) != 0) {
        y_log_message(Y_LOG_LEVEL_ERROR, "u_init_compressed_inmemory_website_config - Error pthread_mutex_init");
        ret = U_ERROR;
      }
    }
  }
  return ret;
}

void u_clean_compressed_inmemory_website_config(struct _u_compressed_inmemory_website_config * config) {
  if (config != NULL) {
    u_map_clean(&(config->mime_types));
    u_map_clean(&(config->map_header));
    u_map_clean(&(config->gzip_files));
    u_map_clean(&(config->deflate_files));
    free_string_array(config->mime_types_compressed);
    pthread_mutex_destroy(&(config->lock));
  }
}

int u_add_mime_types_compressed(struct _u_compressed_inmemory_website_config * config, const char * mime_type) {
  int ret;
  if (config != NULL && o_strlen(mime_type)) {
    if ((config->mime_types_compressed = o_realloc(config->mime_types_compressed, (config->mime_types_compressed_size+2)*sizeof(char*))) != NULL) {
      config->mime_types_compressed[config->mime_types_compressed_size] = o_strdup(mime_type);
      config->mime_types_compressed[config->mime_types_compressed_size+1] = NULL;
      config->mime_types_compressed_size++;
      ret = U_OK;
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "u_add_mime_types_compressed - Error allocating resources for mime_types_compressed");
      ret = U_ERROR;
    }
  } else {
    ret = U_ERROR_PARAMS;
  }
  return ret;
}

int callback_static_compressed_inmemory_website (const struct _u_request * request, struct _u_response * response, void * user_data) {
  struct _u_compressed_inmemory_website_config * config = (struct _u_compressed_inmemory_website_config *)user_data;
  char ** accept_list = NULL;
  int ret = U_CALLBACK_CONTINUE, compress_mode = U_COMPRESS_NONE, res;
  z_stream defstream;
  unsigned char * file_content, * file_content_orig = NULL;
  size_t length, read_length, offset, data_zip_len = 0;
  FILE * f;
  char * file_requested, * file_path, * url_dup_save, * data_zip = NULL;
  const char * content_type;

  /*
   * Comment this if statement if you don't access static files url from root dir, like /app
   */
  if (request->callback_position > 0) {
    return U_CALLBACK_IGNORE;
  } else {
    file_requested = o_strdup(request->http_url);
    url_dup_save = file_requested;

    while (file_requested[0] == '/') {
      file_requested++;
    }
    file_requested += o_strlen((config->url_prefix));
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

    if (!u_map_has_key_case(response->map_header, U_CONTENT_HEADER)) {
      if (split_string(u_map_get_case(request->map_header, U_ACCEPT_HEADER), ",", &accept_list)) {
        if (config->allow_gzip && string_array_has_trimmed_value((const char **)accept_list, U_ACCEPT_GZIP)) {
          compress_mode = U_COMPRESS_GZIP;
        } else if (config->allow_deflate && string_array_has_trimmed_value((const char **)accept_list, U_ACCEPT_DEFLATE)) {
          compress_mode = U_COMPRESS_DEFL;
        }

        content_type = u_map_get_case(&config->mime_types, get_filename_ext(file_requested));
        if (content_type == NULL) {
          content_type = u_map_get(&config->mime_types, "*");
          y_log_message(Y_LOG_LEVEL_WARNING, "Static File Server - Unknown mime type for extension %s", get_filename_ext(file_requested));
        }
        if (!string_array_has_value((const char **)config->mime_types_compressed, content_type)) {
          compress_mode = U_COMPRESS_NONE;
        }

        u_map_put(response->map_header, "Content-Type", content_type);
        u_map_copy_into(response->map_header, &config->map_header);

        if (compress_mode != U_COMPRESS_NONE) {
          if (compress_mode == U_COMPRESS_GZIP && config->allow_cache_compressed && u_map_has_key(&config->gzip_files, file_requested)) {
            ulfius_set_binary_body_response(response, 200, u_map_get(&config->gzip_files, file_requested), u_map_get_length(&config->gzip_files, file_requested));
            u_map_put(response->map_header, U_CONTENT_HEADER, U_ACCEPT_GZIP);
          } else if (compress_mode == U_COMPRESS_DEFL && config->allow_cache_compressed && u_map_has_key(&config->deflate_files, file_requested)) {
            ulfius_set_binary_body_response(response, 200, u_map_get(&config->deflate_files, file_requested), u_map_get_length(&config->deflate_files, file_requested));
            u_map_put(response->map_header, U_CONTENT_HEADER, U_ACCEPT_DEFLATE);
          } else {
            file_path = msprintf("%s/%s", ((struct _u_compressed_inmemory_website_config *)user_data)->files_path, file_requested);

            if (!pthread_mutex_lock(&config->lock)) {
              f = fopen (file_path, "rb");
              if (f) {
                fseek (f, 0, SEEK_END);
                offset = length = ftell (f);
                fseek (f, 0, SEEK_SET);

                if ((file_content_orig = file_content = o_malloc(length)) != NULL && (data_zip = o_malloc((2*length)+20)) != NULL) {
                  defstream.zalloc = u_zalloc;
                  defstream.zfree = u_zfree;
                  defstream.opaque = Z_NULL;
                  defstream.avail_in = (uInt)length;
                  defstream.next_in = (Bytef *)file_content;
                  while ((read_length = fread(file_content, sizeof(char), offset, f))) {
                    file_content += read_length;
                    offset -= read_length;
                  }

                  if (compress_mode == U_COMPRESS_GZIP) {
                    if (deflateInit2(&defstream, 
                                     Z_DEFAULT_COMPRESSION, 
                                     Z_DEFLATED,
                                     U_GZIP_WINDOW_BITS | U_GZIP_ENCODING,
                                     8,
                                     Z_DEFAULT_STRATEGY) != Z_OK) {
                      y_log_message(Y_LOG_LEVEL_ERROR, "callback_static_compressed_inmemory_website - Error deflateInit (gzip)");
                      ret = U_CALLBACK_ERROR;
                    }
                  } else {
                    if (deflateInit(&defstream, Z_BEST_COMPRESSION) != Z_OK) {
                      y_log_message(Y_LOG_LEVEL_ERROR, "callback_static_compressed_inmemory_website - Error deflateInit (deflate)");
                      ret = U_CALLBACK_ERROR;
                    }
                  }
                  if (ret == U_CALLBACK_CONTINUE) {
                    do {
                      if ((data_zip = o_realloc(data_zip, data_zip_len+_U_W_BLOCK_SIZE)) != NULL) {
                        defstream.avail_out = _U_W_BLOCK_SIZE;
                        defstream.next_out = ((Bytef *)data_zip)+data_zip_len;
                        switch ((res = deflate(&defstream, Z_FINISH))) {
                          case Z_OK:
                          case Z_STREAM_END:
                          case Z_BUF_ERROR:
                            break;
                          default:
                            y_log_message(Y_LOG_LEVEL_ERROR, "callback_static_compressed_inmemory_website - Error deflate %d", res);
                            ret = U_CALLBACK_ERROR;
                            break;
                        }
                        data_zip_len += _U_W_BLOCK_SIZE - defstream.avail_out;
                      } else {
                        y_log_message(Y_LOG_LEVEL_ERROR, "callback_static_compressed_inmemory_website - Error allocating resources for data_zip");
                        ret = U_CALLBACK_ERROR;
                      }
                    } while (U_CALLBACK_CONTINUE == ret && defstream.avail_out == 0);
                    
                    if (ret == U_CALLBACK_CONTINUE) {
                      if (compress_mode == U_COMPRESS_GZIP) {
                        if (config->allow_cache_compressed) {
                          u_map_put_binary(&config->gzip_files, file_requested, data_zip, 0, defstream.total_out);
                        }
                        ulfius_set_binary_body_response(response, 200, u_map_get(&config->gzip_files, file_requested), u_map_get_length(&config->gzip_files, file_requested));
                      } else {
                        if (config->allow_cache_compressed) {
                          u_map_put_binary(&config->deflate_files, file_requested, data_zip, 0, defstream.total_out);
                        }
                        ulfius_set_binary_body_response(response, 200, u_map_get(&config->deflate_files, file_requested), u_map_get_length(&config->deflate_files, file_requested));
                      }
                      u_map_put(response->map_header, U_CONTENT_HEADER, compress_mode==U_COMPRESS_GZIP?U_ACCEPT_GZIP:U_ACCEPT_DEFLATE);
                    }
                  }
                  deflateEnd(&defstream);
                  o_free(data_zip);
                } else {
                  y_log_message(Y_LOG_LEVEL_ERROR, "callback_static_compressed_inmemory_website - Error allocating resource for file_content or data_zip");
                  ret = U_CALLBACK_ERROR;
                }
                o_free(file_content_orig);
                fclose(f);
              } else {
                if (((struct _u_compressed_inmemory_website_config *)user_data)->redirect_on_404 == NULL) {
                  ret = U_CALLBACK_IGNORE;
                } else {
                  ulfius_add_header_to_response(response, "Location", ((struct _u_compressed_inmemory_website_config *)user_data)->redirect_on_404);
                  response->status = 302;
                }
              }
              pthread_mutex_unlock(&config->lock);
            } else {
              y_log_message(Y_LOG_LEVEL_ERROR, "callback_static_compressed_inmemory_website - Error pthread_lock_mutex");
              ret = U_CALLBACK_ERROR;
            }
            o_free(file_path);
          }
        } else {
          ret = callback_static_file_uncompressed(request, response, user_data);
        }
        free_string_array(accept_list);
      }
    }
    o_free(url_dup_save);
  }

  return ret;
}
