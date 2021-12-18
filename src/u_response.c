/**
 *
 * Ulfius Framework
 *
 * REST framework library
 *
 * u_response.c: response related functions defintions
 *
 * Copyright 2015-2020 Nicolas Mora <mail@babelouest.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation;
 * version 2.1 of the License.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU GENERAL PUBLIC LICENSE for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include <string.h>
#include <u_private.h>
#include <ulfius.h>

static char * ulfius_generate_cookie_header(const struct _u_cookie * cookie) {
  char * attr_expires = NULL, * attr_max_age = NULL, * attr_domain = NULL, * attr_path = NULL;
  char * attr_secure = NULL, * attr_http_only = NULL, * cookie_header_value = NULL, * same_site = NULL;

  if (cookie != NULL) {
    if (cookie->expires != NULL) {
      attr_expires = msprintf("; %s=%s", ULFIUS_COOKIE_ATTRIBUTE_EXPIRES, cookie->expires);
      if (attr_expires == NULL) {
        y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for attr_expires");
        return NULL;
      }
    } else {
      attr_expires = o_strdup("");
    }
    if (cookie->max_age > 0) {
      attr_max_age = msprintf("; %s=%d", ULFIUS_COOKIE_ATTRIBUTE_MAX_AGE, cookie->max_age);
      if (attr_max_age == NULL) {
        y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for attr_max_age");
        o_free(attr_expires);
        attr_expires = NULL;
        return NULL;
      }
    } else {
      attr_max_age = o_strdup("");
    }
    if (cookie->domain != NULL) {
      attr_domain = msprintf("; %s=%s", ULFIUS_COOKIE_ATTRIBUTE_DOMAIN, cookie->domain);
      if (attr_domain == NULL) {
        y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for attr_domain");
        o_free(attr_expires);
        attr_expires = NULL;
        o_free(attr_max_age);
        attr_max_age = NULL;
        return NULL;
      }
    } else {
      attr_domain = o_strdup("");
    }
    if (cookie->path != NULL) {
      attr_path = msprintf("; %s=%s", ULFIUS_COOKIE_ATTRIBUTE_PATH, cookie->path);
      if (attr_path == NULL) {
        y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for attr_path");
        o_free(attr_expires);
        o_free(attr_max_age);
        o_free(attr_domain);
        attr_expires = NULL;
        attr_max_age = NULL;
        attr_domain = NULL;
        return NULL;
      }
    } else {
      attr_path = o_strdup("");
    }
    if (cookie->secure) {
      attr_secure = msprintf("; %s", ULFIUS_COOKIE_ATTRIBUTE_SECURE);
      if (attr_secure == NULL) {
        y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for attr_secure");
        o_free(attr_expires);
        o_free(attr_max_age);
        o_free(attr_domain);
        o_free(attr_path);
        attr_expires = NULL;
        attr_max_age = NULL;
        attr_domain = NULL;
        attr_path = NULL;
        return NULL;
      }
    } else {
      attr_secure = o_strdup("");
    }
    if (cookie->http_only) {
      attr_http_only = msprintf("; %s", ULFIUS_COOKIE_ATTRIBUTE_HTTPONLY);
      if (attr_http_only == NULL) {
        y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for attr_http_only");
        o_free(attr_expires);
        o_free(attr_max_age);
        o_free(attr_domain);
        o_free(attr_path);
        o_free(attr_secure);
        attr_expires = NULL;
        attr_max_age = NULL;
        attr_domain = NULL;
        attr_path = NULL;
        attr_secure = NULL;
        return NULL;
      }
    } else {
      attr_http_only = o_strdup("");
    }

    if (cookie->same_site  == U_COOKIE_SAME_SITE_STRICT) {
      same_site = o_strdup("; SameSite=Strict");
    } else if (cookie->same_site == U_COOKIE_SAME_SITE_LAX) {
      same_site = o_strdup("; SameSite=Lax");
    } else {
      same_site = o_strdup("");
    }

    if (attr_expires == NULL || attr_max_age == NULL || attr_domain == NULL || attr_path == NULL || attr_secure == NULL || attr_http_only == NULL) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for ulfius_generate_cookie_header");
    } else {
      cookie_header_value = msprintf("%s=%s%s%s%s%s%s%s%s", cookie->key, cookie->value, attr_expires, attr_max_age, attr_domain, attr_path, attr_secure, attr_http_only, same_site);
      if (cookie_header_value == NULL) {
        y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for cookie_header_value");
      }
    }
    o_free(attr_expires);
    o_free(attr_max_age);
    o_free(attr_domain);
    o_free(attr_path);
    o_free(attr_secure);
    o_free(attr_http_only);
    o_free(same_site);
    attr_expires = NULL;
    attr_max_age = NULL;
    attr_domain = NULL;
    attr_path = NULL;
    attr_secure = NULL;
    attr_http_only = NULL;
    same_site = NULL;
    return cookie_header_value;
  } else {
    return NULL;
  }
}

int ulfius_set_response_header(struct MHD_Response * response, const struct _u_map * response_map_header) {
  const char ** header_keys = u_map_enum_keys(response_map_header);
  const char * header_value;
  int i = -1, ret;
  if (header_keys != NULL && response != NULL && response_map_header != NULL) {
    for (i=0; header_keys != NULL && header_keys[i] != NULL; i++) {
      header_value = u_map_get(response_map_header, header_keys[i]);
      if (header_value != NULL) {
        ret = MHD_add_response_header (response, header_keys[i], header_value);
        if (ret == MHD_NO) {
          i = -1;
          break;
        }
      }
    }
  }
  return i;
}

int ulfius_set_response_cookie(struct MHD_Response * mhd_response, const struct _u_response * response) {
  int ret;
  unsigned int i;
  char * header;
  if (mhd_response != NULL && response != NULL) {
    for (i=0; i<response->nb_cookies; i++) {
      header = ulfius_generate_cookie_header(&response->map_cookie[i]);
      if (header != NULL) {
        ret = MHD_add_response_header (mhd_response, MHD_HTTP_HEADER_SET_COOKIE, header);
        o_free(header);
        if (ret == MHD_NO) {
          i = -1;
          break;
        }
      } else {
        i = -1;
        break;
      }
    }
    return i;
  } else {
    return -1;
  }
}

int ulfius_add_cookie_to_response(struct _u_response * response, const char * key, const char * value, const char * expires, const unsigned int max_age,
                                  const char * domain, const char * path, const int secure, const int http_only) {
  return ulfius_add_same_site_cookie_to_response(response, key, value, expires, max_age, domain, path, secure, http_only, U_COOKIE_SAME_SITE_NONE);
}

int ulfius_add_same_site_cookie_to_response(struct _u_response * response, const char * key, const char * value, const char * expires, const unsigned int max_age,
                                            const char * domain, const char * path, const int secure, const int http_only, const int same_site) {
  unsigned int i;
  if (response != NULL && key != NULL && (same_site == U_COOKIE_SAME_SITE_NONE || same_site == U_COOKIE_SAME_SITE_STRICT || same_site == U_COOKIE_SAME_SITE_LAX)) {
    // Look for cookies with the same key
    for (i=0; i<response->nb_cookies; i++) {
      if (0 == o_strcmp(response->map_cookie[i].key, key)) {
        // Key found, replace cookie
        o_free(response->map_cookie[i].value);
        o_free(response->map_cookie[i].expires);
        o_free(response->map_cookie[i].domain);
        o_free(response->map_cookie[i].path);
        response->map_cookie[i].value = o_strdup(value!=NULL?value:"");
        response->map_cookie[i].expires = o_strdup(expires);
        response->map_cookie[i].domain = o_strdup(domain);
        response->map_cookie[i].path = o_strdup(path);
        response->map_cookie[i].max_age = max_age;
        response->map_cookie[i].secure = secure;
        response->map_cookie[i].http_only = http_only;
        response->map_cookie[i].same_site = same_site;
        if ((value != NULL && response->map_cookie[i].value == NULL) ||
            (expires != NULL && response->map_cookie[i].expires == NULL) ||
            (domain != NULL && response->map_cookie[i].domain == NULL) ||
            (path != NULL && response->map_cookie[i].path == NULL)) {
          ulfius_clean_cookie(&response->map_cookie[i]);
          o_free(response->map_cookie[i].value);
          o_free(response->map_cookie[i].expires);
          o_free(response->map_cookie[i].domain);
          o_free(response->map_cookie[i].path);
          return U_ERROR_MEMORY;
        } else {
          return U_OK;
        }
      }
    }

    // Key not found, inserting a new cookie
    if (response->nb_cookies == 0) {
      response->map_cookie = o_malloc(sizeof(struct _u_cookie));
      if (response->map_cookie == NULL) {
        y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for response->map_cookie");
        return U_ERROR_MEMORY;
      }
    } else {
      response->map_cookie = o_realloc(response->map_cookie, (response->nb_cookies + 1) * sizeof(struct _u_cookie));
      if (response->map_cookie == NULL) {
        y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for response->map_cookie");
        return U_ERROR_MEMORY;
      }
    }
    response->map_cookie[response->nb_cookies].key = o_strdup(key);
    response->map_cookie[response->nb_cookies].value = o_strdup(value!=NULL?value:"");
    response->map_cookie[response->nb_cookies].expires = o_strdup(expires);
    response->map_cookie[response->nb_cookies].max_age = max_age;
    response->map_cookie[response->nb_cookies].domain = o_strdup(domain);
    response->map_cookie[response->nb_cookies].path = o_strdup(path);
    response->map_cookie[response->nb_cookies].secure = secure;
    response->map_cookie[response->nb_cookies].http_only = http_only;
    response->map_cookie[response->nb_cookies].same_site = same_site;
    if ((key != NULL && response->map_cookie[response->nb_cookies].key == NULL) || (value != NULL && response->map_cookie[response->nb_cookies].value == NULL) ||
        (expires != NULL && response->map_cookie[response->nb_cookies].expires == NULL) || (domain != NULL && response->map_cookie[response->nb_cookies].domain == NULL) ||
        (path != NULL && response->map_cookie[response->nb_cookies].path == NULL)) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for ulfius_add_cookie_to_response");
      ulfius_clean_cookie(&response->map_cookie[response->nb_cookies]);
      o_free(response->map_cookie[response->nb_cookies].key);
      o_free(response->map_cookie[response->nb_cookies].value);
      o_free(response->map_cookie[response->nb_cookies].expires);
      o_free(response->map_cookie[response->nb_cookies].domain);
      o_free(response->map_cookie[response->nb_cookies].path);
      return U_ERROR_MEMORY;
    }
    response->nb_cookies++;
    return U_OK;
  } else {
    return U_ERROR_PARAMS;
  }
}

int ulfius_clean_cookie(struct _u_cookie * cookie) {
  if (cookie != NULL) {
    o_free(cookie->key);
    o_free(cookie->value);
    o_free(cookie->expires);
    o_free(cookie->domain);
    o_free(cookie->path);
    cookie->key = NULL;
    cookie->value = NULL;
    cookie->expires = NULL;
    cookie->domain = NULL;
    cookie->path = NULL;
    return U_OK;
  } else {
    return U_ERROR_PARAMS;
  }
}

int ulfius_copy_cookie(struct _u_cookie * dest, const struct _u_cookie * source) {
  if (source != NULL && dest != NULL) {
    dest->key = o_strdup(source->key);
    dest->value = o_strdup(source->value);
    dest->expires = o_strdup(source->expires);
    dest->max_age = source->max_age;
    dest->domain = o_strdup(source->domain);
    dest->path = o_strdup(source->path);
    dest->secure = source->secure;
    dest->http_only = source->http_only;
    dest->same_site = source->same_site;
    if (dest->path == NULL || dest->domain == NULL || dest->expires == NULL || dest->value == NULL) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for ulfius_copy_cookie");
      o_free(dest->path);
      o_free(dest->domain);
      o_free(dest->expires);
      o_free(dest->value);
      return U_ERROR_MEMORY;
    } else {
      return U_OK;
    }
  }
  return U_ERROR_PARAMS;
}

int ulfius_clean_response(struct _u_response * response) {
  unsigned int i;
  if (response != NULL) {
    o_free(response->protocol);
    response->protocol = NULL;
    u_map_clean_full(response->map_header);
    response->map_header = NULL;
    for (i=0; i<response->nb_cookies; i++) {
      ulfius_clean_cookie(&response->map_cookie[i]);
    }
    o_free(response->auth_realm);
    o_free(response->map_cookie);
    o_free(response->binary_body);
    response->auth_realm = NULL;
    response->map_cookie = NULL;
    response->binary_body = NULL;
#ifndef U_DISABLE_WEBSOCKET
    /* ulfius_clean_response might be called without websocket_handle being initialized */
    if ((struct _websocket_handle *)response->websocket_handle) {
      o_free(((struct _websocket_handle *)response->websocket_handle)->websocket_protocol);
      o_free(((struct _websocket_handle *)response->websocket_handle)->websocket_extensions);
      pointer_list_clean_free(((struct _websocket_handle *)response->websocket_handle)->websocket_extension_list, &ulfius_free_websocket_extension_pointer_list);
      o_free(((struct _websocket_handle *)response->websocket_handle)->websocket_extension_list);
      o_free(response->websocket_handle);
    }
#endif
    return U_OK;
  } else {
    return U_ERROR_PARAMS;
  }
}

int ulfius_clean_response_full(struct _u_response * response) {
  if (ulfius_clean_response(response) == U_OK) {
    o_free(response);
    return U_OK;
  } else {
    return U_ERROR_PARAMS;
  }
}

int ulfius_init_response(struct _u_response * response) {
  if (response != NULL) {
    response->status = 200;
    response->map_header = o_malloc(sizeof(struct _u_map));
    if (response->map_header == NULL) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for response->map_header");
      return U_ERROR_MEMORY;
    }
    if (u_map_init(response->map_header) != U_OK) {
      return U_ERROR_PARAMS;
    }
    response->auth_realm = NULL;
    response->map_cookie = NULL;
    response->nb_cookies = 0;
    response->protocol = NULL;
    response->binary_body = NULL;
    response->binary_body_length = 0;
    response->stream_callback = NULL;
    response->stream_size = U_STREAM_SIZE_UNKNOWN;
    response->stream_block_size = ULFIUS_STREAM_BLOCK_SIZE_DEFAULT;
    response->stream_callback_free = NULL;
    response->stream_user_data = NULL;
    response->timeout = 0;
    response->shared_data = NULL;
    response->free_shared_data = NULL;
#ifndef U_DISABLE_WEBSOCKET
    response->websocket_handle = o_malloc(sizeof(struct _websocket_handle));
    if (response->websocket_handle == NULL) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for response->websocket_handle");
      return U_ERROR_MEMORY;
    }
    ((struct _websocket_handle *)response->websocket_handle)->websocket_protocol = NULL;
    ((struct _websocket_handle *)response->websocket_handle)->websocket_extensions = NULL;
    ((struct _websocket_handle *)response->websocket_handle)->websocket_manager_callback = NULL;
    ((struct _websocket_handle *)response->websocket_handle)->websocket_manager_user_data = NULL;
    ((struct _websocket_handle *)response->websocket_handle)->websocket_incoming_message_callback = NULL;
    ((struct _websocket_handle *)response->websocket_handle)->websocket_incoming_user_data = NULL;
    ((struct _websocket_handle *)response->websocket_handle)->websocket_onclose_callback = NULL;
    ((struct _websocket_handle *)response->websocket_handle)->websocket_onclose_user_data = NULL;
    ((struct _websocket_handle *)response->websocket_handle)->rsv_expected = 0;
    if ((((struct _websocket_handle *)response->websocket_handle)->websocket_extension_list = o_malloc(sizeof(struct _pointer_list))) == NULL) {
      o_free(response->websocket_handle);
      y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for response->websocket_handle->websocket_extension_list");
      return U_ERROR_MEMORY;
    }
    pointer_list_init(((struct _websocket_handle *)response->websocket_handle)->websocket_extension_list);
#endif
    return U_OK;
  } else {
    return U_ERROR_PARAMS;
  }
}

int ulfius_copy_response(struct _u_response * dest, const struct _u_response * source) {
  unsigned int i;
  if (dest != NULL && source != NULL) {
    dest->status = source->status;
    dest->protocol = o_strdup(source->protocol);
    dest->auth_realm = o_strdup(source->auth_realm);
    if (dest->protocol == NULL) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for dest->protocol");
      return U_ERROR_MEMORY;
    }
    u_map_clean_full(dest->map_header);
    dest->map_header = u_map_copy(source->map_header);
    if (dest->map_header == NULL) {
      return U_ERROR_MEMORY;
    }
    dest->nb_cookies = source->nb_cookies;
    if (source->nb_cookies > 0) {
      dest->map_cookie = o_malloc(source->nb_cookies*sizeof(struct _u_cookie));
      if (dest->map_cookie == NULL) {
        y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for dest->map_cookie");
        return U_ERROR_MEMORY;
      }
      for (i=0; i<source->nb_cookies; i++) {
        ulfius_copy_cookie(&dest->map_cookie[i], &source->map_cookie[i]);
      }
    } else {
      dest->map_cookie = NULL;
    }

    if (source->binary_body != NULL && source->binary_body_length > 0) {
      dest->binary_body = o_malloc(source->binary_body_length);
      if (dest->binary_body == NULL) {
        y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for dest->binary_body");
        return U_ERROR_MEMORY;
      }
      dest->binary_body_length = source->binary_body_length;
      memcpy(dest->binary_body, source->binary_body, source->binary_body_length);
    }

    if (source->stream_callback != NULL) {
      dest->stream_callback = source->stream_callback;
      dest->stream_callback_free = source->stream_callback_free;
      dest->stream_size = source->stream_size;
      dest->stream_block_size = source->stream_block_size;
      dest->stream_user_data = source->stream_user_data;
    }

    dest->shared_data = source->shared_data;
    dest->timeout = source->timeout;
#ifndef U_DISABLE_WEBSOCKET
    if (source->websocket_handle != NULL) {
      ((struct _websocket_handle *)dest->websocket_handle)->websocket_protocol = o_strdup(((struct _websocket_handle *)source->websocket_handle)->websocket_protocol);
      ((struct _websocket_handle *)dest->websocket_handle)->websocket_extensions = o_strdup(((struct _websocket_handle *)source->websocket_handle)->websocket_extensions);
      ((struct _websocket_handle *)dest->websocket_handle)->websocket_manager_callback = ((struct _websocket_handle *)source->websocket_handle)->websocket_manager_callback;
      ((struct _websocket_handle *)dest->websocket_handle)->websocket_manager_user_data = ((struct _websocket_handle *)source->websocket_handle)->websocket_manager_user_data;
      ((struct _websocket_handle *)dest->websocket_handle)->websocket_incoming_message_callback = ((struct _websocket_handle *)source->websocket_handle)->websocket_incoming_message_callback;
      ((struct _websocket_handle *)dest->websocket_handle)->websocket_incoming_user_data = ((struct _websocket_handle *)source->websocket_handle)->websocket_incoming_user_data;
      ((struct _websocket_handle *)dest->websocket_handle)->websocket_onclose_callback = ((struct _websocket_handle *)source->websocket_handle)->websocket_onclose_callback;
      ((struct _websocket_handle *)dest->websocket_handle)->websocket_onclose_user_data = ((struct _websocket_handle *)source->websocket_handle)->websocket_onclose_user_data;
    }
#endif
    return U_OK;
  } else {
    return U_ERROR_PARAMS;
  }
}

struct _u_response * ulfius_duplicate_response(const struct _u_response * response) {
  struct _u_response * new_response = NULL;
  if (response != NULL) {
    new_response = o_malloc(sizeof(struct _u_response));
    if (new_response == NULL) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for new_response");
      return NULL;
    }
    if (ulfius_init_response(new_response) == U_OK) {
      if (ulfius_copy_response(new_response, response) != U_OK) {
        y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error ulfius_copy_response");
        ulfius_clean_response_full(new_response);
        new_response = NULL;
      }
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error ulfius_init_response");
      o_free(new_response);
      new_response = NULL;
    }
  }
  return new_response;
}

int ulfius_set_string_body_response(struct _u_response * response, const unsigned int status, const char * string_body) {
  if (response != NULL && string_body != NULL) {
    // Free all the bodies available
    o_free(response->binary_body);
    response->binary_body = o_strdup(string_body);
    if (response->binary_body == NULL) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for response->binary_body");
      return U_ERROR_MEMORY;
    } else {
      response->status = status;
      response->binary_body_length = o_strlen(string_body);
      return U_OK;
    }
  } else {
    return U_ERROR_PARAMS;
  }
}

int ulfius_set_binary_body_response(struct _u_response * response, const unsigned int status, const char * binary_body, const size_t length) {
  if (response != NULL && binary_body != NULL && length > 0) {
    // Free all the bodies available
    o_free(response->binary_body);
    response->binary_body = NULL;
    response->binary_body_length = 0;

    response->binary_body = o_malloc(length);
    if (response->binary_body == NULL) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for response->binary_body");
      return U_ERROR_MEMORY;
    }
    memcpy(response->binary_body, binary_body, length);
    response->binary_body_length = length;
    response->status = status;
    return U_OK;
  } else {
    return U_ERROR_PARAMS;
  }
}

int ulfius_set_empty_body_response(struct _u_response * response, const unsigned int status) {
  if (response != NULL) {
    // Free all the bodies available
    o_free(response->binary_body);
    response->binary_body = NULL;
    response->binary_body_length = 0;

    response->status = status;
    return U_OK;
  } else {
    return U_ERROR_PARAMS;
  }
}

int ulfius_set_stream_response(struct _u_response * response,
                                const unsigned int status,
                                ssize_t (* stream_callback) (void * stream_user_data, uint64_t offset, char * out_buf, size_t max),
                                void (* stream_callback_free) (void * stream_user_data),
                                uint64_t stream_size,
                                size_t stream_block_size,
                                void * stream_user_data) {
  if (response != NULL && stream_callback != NULL) {
    // Free all the bodies available
    o_free(response->binary_body);
    response->binary_body = NULL;
    response->binary_body_length = 0;

    response->status = status;
    response->stream_callback = stream_callback;
    response->stream_callback_free = stream_callback_free;
    response->stream_size = stream_size;
    response->stream_block_size = stream_block_size;
    response->stream_user_data = stream_user_data;
    return U_OK;
  } else {
    return U_ERROR_PARAMS;
  }
}

int ulfius_set_response_properties(struct _u_response * response, ...) {
  u_option option;
  int ret = U_OK;
  const char * str_key, * str_value;
  size_t size_value;
#ifndef U_DISABLE_JANSSON
  json_t * j_value;
#endif
  va_list vl;

  if (response != NULL) {
    va_start(vl, response);
    for (option = va_arg(vl, uint); option != U_OPT_NONE && ret == U_OK; option = va_arg(vl, uint)) {
      switch (option) {
        case U_OPT_STATUS:
          response->status = va_arg(vl, long);
          break;
        case U_OPT_HEADER_PARAMETER:
          str_key = va_arg(vl, const char *);
          str_value = va_arg(vl, const char *);
          ret = u_map_put(response->map_header, str_key, str_value);
          break;
        case U_OPT_HEADER_PARAMETER_REMOVE:
          str_key = va_arg(vl, const char *);
          ret = u_map_remove_from_key(response->map_header, str_key);
          break;
        case U_OPT_AUTH_REALM:
          str_value = va_arg(vl, const char *);
          o_free(response->auth_realm);
          if (o_strlen(str_value)) {
            response->auth_realm = o_strdup(str_value);
          } else {
            response->auth_realm = NULL;
          }
          break;
        case U_OPT_BINARY_BODY:
          str_value = va_arg(vl, const char *);
          size_value = va_arg(vl, size_t);
          ret = ulfius_set_binary_body_response(response, response->status, str_value, size_value);
          break;
        case U_OPT_STRING_BODY:
          str_value = va_arg(vl, const char *);
          ret = ulfius_set_string_body_response(response, response->status, str_value);
          break;
#ifndef U_DISABLE_JANSSON
        case U_OPT_JSON_BODY:
          j_value = va_arg(vl, json_t *);
          ret = ulfius_set_json_body_response(response, response->status, j_value);
          break;
#endif
        case U_OPT_SHARED_DATA:
          response->shared_data = va_arg(vl, void *);
          break;
        case U_OPT_TIMEOUT:
          response->timeout = va_arg(vl, unsigned int);
          break;
        default:
          ret = U_ERROR_PARAMS;
          break;
      }
    }
    va_end(vl);
  } else {
    y_log_message(Y_LOG_LEVEL_DEBUG, "Ulfius - Error input parameter");
    ret = U_ERROR_PARAMS;
  }
  return ret;
}

int ulfius_set_response_shared_data(struct _u_response * response, void * shared_data, void (* free_shared_data) (void * shared_data)) {
  if (response != NULL && shared_data != NULL) {
    if (response->free_shared_data != NULL && response->shared_data != NULL) {
      response->free_shared_data(response->shared_data);
    }
    response->shared_data = shared_data;
    response->free_shared_data = free_shared_data;
    return U_OK;
  } else {
    return U_ERROR_PARAMS;
  }
}

#ifndef U_DISABLE_JANSSON
/**
 * ulfius_set_json_body_response
 * Add a json_t j_body to a response
 * return U_OK on success
 */
int ulfius_set_json_body_response(struct _u_response * response, const unsigned int status, const json_t * j_body) {
  if (response != NULL && j_body != NULL && (json_is_array(j_body) || json_is_object(j_body))) {
    // Free all the bodies available
    o_free(response->binary_body);
    response->binary_body = NULL;
    response->binary_body_length = 0;

    response->binary_body = (void*) json_dumps(j_body, JSON_COMPACT);
    if (response->binary_body == NULL) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for response->binary_body");
      return U_ERROR_MEMORY;
    }
    response->binary_body_length = o_strlen((char*)response->binary_body);
    response->status = status;
    u_map_put(response->map_header, ULFIUS_HTTP_HEADER_CONTENT, ULFIUS_HTTP_ENCODING_JSON);
    return U_OK;
  } else {
    return U_ERROR_PARAMS;
  }
}

/**
 * ulfius_get_json_body_response
 * Get JSON structure from the request body if the request is valid
 * request: struct _u_request used
 * json_error: structure to store json_error_t if specified
 */
json_t * ulfius_get_json_body_response(struct _u_response * response, json_error_t * json_error) {
  if (response != NULL && response->map_header != NULL && NULL != o_strstr(u_map_get_case(response->map_header, ULFIUS_HTTP_HEADER_CONTENT), ULFIUS_HTTP_ENCODING_JSON)) {
    return json_loadb(response->binary_body, response->binary_body_length, JSON_DECODE_ANY, json_error);
  }
  return NULL;
}
#endif

int ulfius_add_header_to_response(struct _u_response * response, const char * key, const char * value) {
  if (response != NULL && key != NULL && value != NULL) {
    return u_map_put(response->map_header, key, value);
  } else {
    return U_ERROR_PARAMS;
  }
}
