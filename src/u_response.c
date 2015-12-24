/**
 * 
 * Ulfius Framework
 * 
 * REST framework library
 * 
 * u_response.c: response related functions defintions
 * 
 * Copyright 2015 Nicolas Mora <mail@babelouest.org>
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

#include "ulfius.h"

/**
 * set_response_header
 * adds headers defined in the response_map_header to the response
 * return the number of added headers, -1 on error
 */
int set_response_header(struct MHD_Response * response, const struct _u_map * response_map_header) {
  const char ** header_keys = u_map_enum_keys(response_map_header);
  const char * header_value;
  int i = -1, ret;
  if (header_keys != NULL && response != NULL && response_map_header != NULL) {
    for (i=0; header_keys != NULL && header_keys[i] != NULL; i++) {
      header_value = u_map_get(response_map_header, header_keys[i]);
      if (header_value == NULL) {
        return -1;
      }
      ret = MHD_add_response_header (response, header_keys[i], header_value);
      if (ret == MHD_NO) {
        i = -1;
        break;
      }
    }
  }
  return i;
}

/**
 * set_response_cookie
 * adds cookies defined in the response_map_cookie
 * return the number of added headers, -1 on error
 */
int set_response_cookie(struct MHD_Response * mhd_response, const struct _u_response * response) {
  int i, ret;
  char * header;
  if (mhd_response != NULL && response != NULL) {
    for (i=0; i<response->nb_cookies; i++) {
      header = get_cookie_header(&response->map_cookie[i]);
      if (header != NULL) {
        ret = MHD_add_response_header (mhd_response, MHD_HTTP_HEADER_SET_COOKIE, header);
        free(header);
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

/**
 * ulfius_add_cookie_to_header
 * add a cookie to the cookie map
 * return U_OK on success
 */
int ulfius_add_cookie_to_response(struct _u_response * response, const char * key, const char * value, const char * expires, const uint max_age, 
                                  const char * domain, const char * path, const int secure, const int http_only) {
  int i;
  if (response != NULL && key != NULL && value != NULL) {
    // Look for cookies with the same key
    for (i=0; i<response->nb_cookies; i++) {
      if (0 == strcmp(response->map_cookie[i].key, key)) {
        // Key found, replace cookie
        free(response->map_cookie[i].value);
        free(response->map_cookie[i].expires);
        free(response->map_cookie[i].domain);
        free(response->map_cookie[i].path);
        response->map_cookie[i].value = u_strdup(value);
        response->map_cookie[i].expires = u_strdup(expires);
        response->map_cookie[i].domain = u_strdup(domain);
        response->map_cookie[i].path = u_strdup(path);
        response->map_cookie[i].max_age = max_age;
        response->map_cookie[i].secure = secure;
        response->map_cookie[i].http_only = http_only;
        if ((value != NULL && response->map_cookie[i].value == NULL) ||
            (expires != NULL && response->map_cookie[i].expires == NULL) ||
            (domain != NULL && response->map_cookie[i].domain == NULL) ||
            (path != NULL && response->map_cookie[i].path == NULL)) {
          return U_ERROR_MEMORY;
        } else {
          return U_OK;
        }
      }
    }
    
    // Key not found, inserting a new cookie
    if (response->nb_cookies == 0) {
      response->map_cookie = malloc(sizeof(struct _u_cookie));
      if (response->map_cookie == NULL) {
        return U_ERROR_MEMORY;
      }
    } else {
      response->map_cookie = realloc(response->map_cookie, (response->nb_cookies + 1) * sizeof(struct _u_cookie));
      if (response->map_cookie == NULL) {
        return U_ERROR_MEMORY;
      }
    }
    response->map_cookie[response->nb_cookies].key = u_strdup(key);
    response->map_cookie[response->nb_cookies].value = u_strdup(value);
    response->map_cookie[response->nb_cookies].expires = u_strdup(expires);
    response->map_cookie[response->nb_cookies].max_age = max_age;
    response->map_cookie[response->nb_cookies].domain = u_strdup(domain);
    response->map_cookie[response->nb_cookies].path = u_strdup(path);
    response->map_cookie[response->nb_cookies].secure = secure;
    response->map_cookie[response->nb_cookies].http_only = http_only;
    response->nb_cookies++;
    return U_OK;
  } else {
    return U_ERROR_PARAMS;
  }
}

/**
 * Add a cookie in the cookie map as defined in the RFC 6265
 * Returned value must be free'd after use
 */
char * get_cookie_header(const struct _u_cookie * cookie) {
  char * attr_expires = NULL, * attr_max_age = NULL, * attr_domain = NULL, * attr_path = NULL;
  char * attr_secure = NULL, * attr_http_only = NULL, * cookie_header_value = NULL;
  int len;
  if (cookie != NULL) {
    if (cookie->expires != NULL) {
      len = snprintf(NULL, 0, "; %s=%s", ULFIUS_COOKIE_ATTRIBUTE_EXPIRES, cookie->expires);
      attr_expires = malloc((len+1)*sizeof(char));
      if (attr_expires == NULL) {
        return NULL;
      }
      snprintf(attr_expires, (len+1), "; %s=%s", ULFIUS_COOKIE_ATTRIBUTE_EXPIRES, cookie->expires);
    } else {
      attr_expires = u_strdup("");
    }
    if (cookie->max_age > 0) {
      len = snprintf(NULL, 0, "; %s=%d", ULFIUS_COOKIE_ATTRIBUTE_MAX_AGE, cookie->max_age);
      attr_max_age = malloc((len+1)*sizeof(char));
      if (attr_max_age == NULL) {
        free(attr_expires);
        attr_expires = NULL;
        return NULL;
      }
      snprintf(attr_max_age, (len+1), "; %s=%d", ULFIUS_COOKIE_ATTRIBUTE_MAX_AGE, cookie->max_age);
    } else {
      attr_max_age = u_strdup("");
    }
    if (cookie->domain != NULL) {
      len = snprintf(NULL, 0, "; %s=%s", ULFIUS_COOKIE_ATTRIBUTE_DOMAIN, cookie->domain);
      attr_domain = malloc((len+1)*sizeof(char));
      if (attr_domain == NULL) {
        free(attr_expires);
        attr_expires = NULL;
        free(attr_max_age);
        attr_max_age = NULL;
        return NULL;
      }
      snprintf(attr_domain, (len+1), "; %s=%s", ULFIUS_COOKIE_ATTRIBUTE_DOMAIN, cookie->domain);
    } else {
      attr_domain = u_strdup("");
    }
    if (cookie->path != NULL) {
      len = snprintf(NULL, 0, "; %s=%s", ULFIUS_COOKIE_ATTRIBUTE_PATH, cookie->path);
      attr_path = malloc((len+1)*sizeof(char));
      if (attr_path == NULL) {
        free(attr_expires);
        free(attr_max_age);
        free(attr_domain);
        attr_expires = NULL;
        attr_max_age = NULL;
        attr_domain = NULL;
        return NULL;
      }
      snprintf(attr_path, (len+1), "; %s=%s", ULFIUS_COOKIE_ATTRIBUTE_DOMAIN, cookie->path);
    } else {
      attr_path = u_strdup("");
    }
    if (cookie->secure) {
      len = snprintf(NULL, 0, "; %s", ULFIUS_COOKIE_ATTRIBUTE_SECURE);
      attr_secure = malloc((len+1)*sizeof(char));
      if (attr_secure == NULL) {
        free(attr_expires);
        free(attr_max_age);
        free(attr_domain);
        free(attr_path);
        attr_expires = NULL;
        attr_max_age = NULL;
        attr_domain = NULL;
        attr_path = NULL;
        return NULL;
      }
      snprintf(attr_secure, (len+1), "; %s", ULFIUS_COOKIE_ATTRIBUTE_SECURE);
    } else {
      attr_secure = u_strdup("");
    }
    if (cookie->http_only) {
      len = snprintf(NULL, 0, "; %s", ULFIUS_COOKIE_ATTRIBUTE_HTTPONLY);
      attr_http_only = malloc((len+1)*sizeof(char));
      if (attr_http_only == NULL) {
        free(attr_expires);
        free(attr_max_age);
        free(attr_domain);
        free(attr_path);
        free(attr_secure);
        attr_expires = NULL;
        attr_max_age = NULL;
        attr_domain = NULL;
        attr_path = NULL;
        attr_secure = NULL;
        return NULL;
      }
      snprintf(attr_http_only, (len+1), "; %s", ULFIUS_COOKIE_ATTRIBUTE_HTTPONLY);
    } else {
      attr_http_only = u_strdup("");
    }
    len = snprintf(NULL, 0, "%s=%s%s%s%s%s%s%s", cookie->key, cookie->value, attr_expires, attr_max_age, attr_domain, attr_path, attr_secure, attr_http_only);
    cookie_header_value = malloc((len+1)*sizeof(char));
    if (cookie_header_value != NULL) {
      snprintf(cookie_header_value, (len+1), "%s=%s%s%s%s%s%s%s", cookie->key, cookie->value, attr_expires, attr_max_age, attr_domain, attr_path, attr_secure, attr_http_only);
    }
    free(attr_expires);
    free(attr_max_age);
    free(attr_domain);
    free(attr_path);
    free(attr_secure);
    free(attr_http_only);
    attr_expires = NULL;
    attr_max_age = NULL;
    attr_domain = NULL;
    attr_path = NULL;
    attr_secure = NULL;
    attr_http_only = NULL;
    return cookie_header_value;
  } else {
    return NULL;
  }
}

/**
 * ulfius_clean_cookie
 * clean the cookie's elements
 * return U_OK on success
 */
int ulfius_clean_cookie(struct _u_cookie * cookie) {
  if (cookie != NULL) {
    free(cookie->key);
    free(cookie->value);
    free(cookie->expires);
    free(cookie->domain);
    free(cookie->path);
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

/**
 * Copy the cookie source elements into dest elements
 * return U_OK on success
 */
int ulfius_copy_cookie(struct _u_cookie * dest, const struct _u_cookie * source) {
  if (source != NULL && dest != NULL) {
    dest->key = u_strdup(source->key);
    dest->value = u_strdup(source->value);
    dest->expires = u_strdup(source->expires);
    dest->max_age = source->max_age;
    dest->domain = u_strdup(source->domain);
    dest->path = u_strdup(source->path);
    dest->secure = source->secure;
    dest->http_only = source->http_only;
    return U_OK;
  }
  return U_ERROR_PARAMS;
}

/**
 * ulfius_clean_response
 * clean the specified response's inner elements
 * user must free the parent pointer if needed after clean
 * or use ulfius_clean_response_full
 * return U_OK on success
 */
int ulfius_clean_response(struct _u_response * response) {
  int i;
  if (response != NULL) {
    free(response->protocol);
    response->protocol = NULL;
    u_map_clean_full(response->map_header);
    response->map_header = NULL;
    for (i=0; i<response->nb_cookies; i++) {
      ulfius_clean_cookie(&response->map_cookie[i]);
    }
    free(response->map_cookie);
    free(response->string_body);
    free(response->binary_body);
    json_decref(response->json_body);
    response->map_cookie = NULL;
    response->string_body = NULL;
    response->json_body = NULL;
    response->binary_body = NULL;
    return U_OK;
  } else {
    return U_ERROR_PARAMS;
  }
}

/**
 * ulfius_clean_response_full
 * clean the specified response and all its elements
 * return U_OK on success
 */
int ulfius_clean_response_full(struct _u_response * response) {
  if (ulfius_clean_response(response) == U_OK) {
    free(response);
    return U_OK;
  } else {
    return U_ERROR_PARAMS;
  }
}

/**
 * ulfius_init_response
 * Initialize a response structure by allocating inner elements
 * return U_OK on success
 */
int ulfius_init_response(struct _u_response * response) {
  if (response != NULL) {
    response->status = 0;
    response->map_header = malloc(sizeof(struct _u_map));
    if (response->map_header == NULL) {
      return U_ERROR_MEMORY;
    }
    if (u_map_init(response->map_header) != U_OK) {
      return U_ERROR_PARAMS;
    }
    response->map_cookie = NULL;
    response->nb_cookies = 0;
    response->protocol = NULL;
    response->string_body = NULL;
    response->json_body = NULL;
    response->binary_body = NULL;
    response->binary_body_length = 0;
    return U_OK;
  } else {
    return U_ERROR_PARAMS;
  }
}

/**
 * create a new response based on the source elements
 * return value must be free'd
 */
struct _u_response * ulfius_duplicate_response(const struct _u_response * response) {
  struct _u_response * new_response = NULL;
  int i;
  if (response != NULL) {
    new_response = malloc(sizeof(struct _u_response));
    if (new_response == NULL) {
      return NULL;
    }
    ulfius_init_response(new_response);
    new_response->status = response->status;
    new_response->protocol = u_strdup(response->protocol);
    u_map_clean_full(new_response->map_header);
    new_response->map_header = u_map_copy(response->map_header);
    new_response->nb_cookies = response->nb_cookies;
    if (response->nb_cookies > 0) {
      new_response->map_cookie = malloc(response->nb_cookies*sizeof(struct _u_cookie));
      if (new_response->map_cookie == NULL) {
        free(new_response);
        return NULL;
      }
      for (i=0; i<response->nb_cookies; i++) {
        ulfius_copy_cookie(&new_response->map_cookie[i], &response->map_cookie[i]);
      }
    } else {
      new_response->map_cookie = NULL;
    }
    new_response->string_body = u_strdup(response->string_body);
    new_response->json_body = (response->json_body==NULL?NULL:json_copy(response->json_body));
    
    if (response->binary_body != NULL && response->binary_body_length > 0) {
      new_response->binary_body = malloc(response->binary_body_length);
      if (new_response->binary_body == NULL) {
        free(new_response->map_cookie);
        free(new_response);
        return NULL;
      }
      new_response->binary_body_length = response->binary_body_length;
      memcpy(new_response->binary_body, response->binary_body, response->binary_body_length);
    }
  }
  return new_response;
}

/**
 * ulfius_copy_response
 * Copy the source response elements into the des response
 * return U_OK on success
 */
int ulfius_copy_response(struct _u_response * dest, const struct _u_response * source) {
  int i;
  if (dest != NULL && source != NULL) {
    dest->status = source->status;
    dest->protocol = u_strdup(source->protocol);
    u_map_clean_full(dest->map_header);
    dest->map_header = u_map_copy(source->map_header);
    if (dest->map_header == NULL) {
      return U_ERROR_MEMORY;
    }
    dest->nb_cookies = source->nb_cookies;
    if (source->nb_cookies > 0) {
      dest->map_cookie = malloc(source->nb_cookies*sizeof(struct _u_cookie));
      if (dest->map_cookie == NULL) {
        return U_ERROR_MEMORY;
      }
      for (i=0; i<source->nb_cookies; i++) {
        ulfius_copy_cookie(&dest->map_cookie[i], &source->map_cookie[i]);
      }
    } else {
      dest->map_cookie = NULL;
    }
    dest->string_body = u_strdup(source->string_body);
    dest->json_body = (source->json_body==NULL?NULL:json_copy(source->json_body));
    
    if (source->binary_body != NULL && source->binary_body_length > 0) {
      dest->binary_body = malloc(source->binary_body_length);
      if (dest->binary_body == NULL) {
        return U_ERROR_MEMORY;
      }
      dest->binary_body_length = source->binary_body_length;
      memcpy(dest->binary_body, source->binary_body, source->binary_body_length);
    }
    return U_OK;
  } else {
    return U_ERROR_PARAMS;
  }
}
