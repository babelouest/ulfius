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
 * Set the response headers defined by the user
 */
int set_response_header(struct MHD_Response * response, const struct _u_map * response_map_header) {
  char ** header_keys = u_map_enum_keys(response_map_header);
  char * header_value;
  int i = -1, ret;
  if (response != NULL && response_map_header != NULL) {
    for (i=0; header_keys != NULL && header_keys[i] != NULL; i++) {
      header_value = u_map_get(response_map_header, header_keys[i]);
      ret = MHD_add_response_header (response, header_keys[i], header_value);
      free(header_value);
      if (ret == MHD_NO) {
        i = -1;
        break;
      }
    }
    u_map_clean_enum(header_keys);
  }
  return i;
}

/**
 * Set the response cookies defined by the user
 */
int set_response_cookie(struct MHD_Response * mhd_response, const struct _u_response * response) {
	int i, ret;
	char * header;
	if (mhd_response != NULL && response != NULL) {
		for (i=0; i<response->nb_cookies; i++) {
			header = get_cookie_header(&response->map_cookie[i]);
			ret = MHD_add_response_header (mhd_response, MHD_HTTP_HEADER_SET_COOKIE, header);
			free(header);
      if (ret == MHD_NO) {
        i = -1;
        break;
      }
		}
		return response->nb_cookies;
	} else {
		return -1;
	}
}

int ulfius_add_cookie_to_response(struct _u_response * response, const char * key, const char * value, const char * expires, const uint max_age, 
                                  const char * domain, const char * path, const int secure, const int http_only) {
  int i;
  if (response != NULL && key != NULL && value != NULL) {
    // Look for cookies with the same key
    for (i=0; i<response->nb_cookies; i++) {
      if (0 == strcmp(response->map_cookie[i].key, key)) {
        // Key found, replace cookie
        free(response->map_cookie[i].value);
        response->map_cookie[i].value = u_strdup(value);
        free(response->map_cookie[i].expires);
        response->map_cookie[i].expires = u_strdup(expires);
        response->map_cookie[i].max_age = max_age;
        free(response->map_cookie[i].domain);
        response->map_cookie[i].domain = u_strdup(domain);
        free(response->map_cookie[i].path);
        response->map_cookie[i].path = u_strdup(path);
        response->map_cookie[i].secure = secure;
        response->map_cookie[i].http_only = http_only;
        return 1;
      }
    }
    
    // Key not found, inserting a new cookie
    if (response->nb_cookies == 0) {
      response->map_cookie = malloc(sizeof(struct _u_cookie));
    } else {
      response->map_cookie = realloc(response->map_cookie, (response->nb_cookies + 1) * sizeof(struct _u_cookie));
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
    return 1;
  } else {
    return 0;
  }
}

/**
 * Add a cookie in the cookie map as defined in the RFC 6265
 */
char * get_cookie_header(const struct _u_cookie * cookie) {
  char * attr_expires = NULL, * attr_max_age = NULL, * attr_domain = NULL, * attr_path = NULL;
  char * attr_secure = NULL, * attr_http_only = NULL, * cookie_header_value = NULL;
  int len;
  if (cookie != NULL) {
    if (cookie->expires != NULL) {
      len = snprintf(NULL, 0, "; %s=%s", ULFIUS_COOKIE_ATTRIBUTE_EXPIRES, cookie->expires);
      attr_expires = malloc((len+1)*sizeof(char));
      snprintf(attr_expires, (len+1), "; %s=%s", ULFIUS_COOKIE_ATTRIBUTE_EXPIRES, cookie->expires);
    } else {
      attr_expires = u_strdup("");
    }
    if (cookie->max_age > 0) {
      len = snprintf(NULL, 0, "; %s=%d", ULFIUS_COOKIE_ATTRIBUTE_MAX_AGE, cookie->max_age);
      attr_max_age = malloc((len+1)*sizeof(char));
      snprintf(attr_max_age, (len+1), "; %s=%d", ULFIUS_COOKIE_ATTRIBUTE_MAX_AGE, cookie->max_age);
    } else {
      attr_max_age = u_strdup("");
    }
    if (cookie->domain != NULL) {
      len = snprintf(NULL, 0, "; %s=%s", ULFIUS_COOKIE_ATTRIBUTE_DOMAIN, cookie->domain);
      attr_domain = malloc((len+1)*sizeof(char));
      snprintf(attr_domain, (len+1), "; %s=%s", ULFIUS_COOKIE_ATTRIBUTE_DOMAIN, cookie->domain);
    } else {
      attr_domain = u_strdup("");
    }
    if (cookie->path != NULL) {
      len = snprintf(NULL, 0, "; %s=%s", ULFIUS_COOKIE_ATTRIBUTE_PATH, cookie->path);
      attr_path = malloc((len+1)*sizeof(char));
      snprintf(attr_path, (len+1), "; %s=%s", ULFIUS_COOKIE_ATTRIBUTE_DOMAIN, cookie->path);
    } else {
      attr_path = u_strdup("");
    }
    if (cookie->secure) {
      len = snprintf(NULL, 0, "; %s", ULFIUS_COOKIE_ATTRIBUTE_SECURE);
      attr_secure = malloc((len+1)*sizeof(char));
      snprintf(attr_secure, (len+1), "; %s", ULFIUS_COOKIE_ATTRIBUTE_SECURE);
    } else {
      attr_secure = u_strdup("");
    }
    if (cookie->http_only) {
      len = snprintf(NULL, 0, "; %s", ULFIUS_COOKIE_ATTRIBUTE_HTTPONLY);
      attr_http_only = malloc((len+1)*sizeof(char));
      snprintf(attr_http_only, (len+1), "; %s", ULFIUS_COOKIE_ATTRIBUTE_HTTPONLY);
    } else {
      attr_http_only = u_strdup("");
    }
    len = snprintf(NULL, 0, "%s=%s%s%s%s%s%s%s", cookie->key, cookie->value, attr_expires, attr_max_age, attr_domain, attr_path, attr_secure, attr_http_only);
    cookie_header_value = malloc((len+1)*sizeof(char));
    snprintf(cookie_header_value, (len+1), "%s=%s%s%s%s%s%s%s", cookie->key, cookie->value, attr_expires, attr_max_age, attr_domain, attr_path, attr_secure, attr_http_only);
    free(attr_expires);
    attr_expires = NULL;
    free(attr_max_age);
    attr_max_age = NULL;
    free(attr_domain);
    attr_domain = NULL;
    free(attr_path);
    attr_path = NULL;
    free(attr_secure);
    attr_secure = NULL;
    free(attr_http_only);
    attr_http_only = NULL;
    return cookie_header_value;
  } else {
    return NULL;
  }
}

/**
 * ulfius_clean_cookie
 * clean the cookie's elements
 */
int ulfius_clean_cookie(struct _u_cookie * cookie) {
	if (cookie != NULL) {
		free(cookie->key);
		cookie->key = NULL;
		free(cookie->value);
		cookie->value = NULL;
		free(cookie->expires);
		cookie->expires = NULL;
		free(cookie->domain);
		cookie->domain = NULL;
		free(cookie->path);
		cookie->path = NULL;
		return 1;
	} else {
		return 0;
	}
}

/**
 * Copy the source's cookie elements into dest
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
		return 1;
	}
	return 0;
}

/**
 * ulfius_clean_response
 * clean the specified response's elements
 * user must free the parent pointer if needed after clean
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
    response->map_cookie = NULL;
    free(response->string_body);
    response->string_body = NULL;
		json_decref(response->json_body);
		response->json_body = NULL;
    free(response->binary_body);
    response->binary_body = NULL;
    return 1;
  } else {
    return 0;
  }
}

/**
 * ulfius_clean_response_full
 * clean the specified response and all its elements
 */
int ulfius_clean_response_full(struct _u_response * response) {
  if (ulfius_clean_response(response)) {
    free(response);
    return 1;
  } else {
    return 0;
  }
}

/**
 * ulfius_init_response
 * Initialize a response structure by allocating inner elements
 * return true if everything went fine, false otherwise
 */
int ulfius_init_response(struct _u_response * response) {
  if (response != NULL) {
		response->status = 0;
    response->map_header = malloc(sizeof(struct _u_map));
    u_map_init(response->map_header);
    response->map_cookie = NULL;
    response->nb_cookies = 0;
    response->protocol = NULL;
    response->string_body = NULL;
    response->json_body = NULL;
    response->binary_body = NULL;
    response->binary_body_length = 0;
    return 1;
  } else {
    return 0;
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
		ulfius_init_response(new_response);
		new_response->status = response->status;
		new_response->protocol = u_strdup(response->protocol);
		u_map_clean_full(new_response->map_header);
		new_response->map_header = u_map_copy(response->map_header);
		new_response->nb_cookies = response->nb_cookies;
		if (response->nb_cookies > 0) {
			new_response->map_cookie = malloc(response->nb_cookies*sizeof(struct _u_cookie));
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
			new_response->binary_body_length = response->binary_body_length;
			memcpy(new_response->binary_body, response->binary_body, response->binary_body_length);
		}
	}
	return new_response;
}

/**
 * ulfius_copy_response
 * Copy the source response elements into the des response
 */
int ulfius_copy_response(struct _u_response * dest, const struct _u_response * source) {
	int i;
	if (dest != NULL && source != NULL) {
		dest->status = source->status;
		dest->protocol = u_strdup(source->protocol);
		u_map_clean_full(dest->map_header);
		dest->map_header = u_map_copy(source->map_header);
		dest->nb_cookies = source->nb_cookies;
		if (source->nb_cookies > 0) {
			dest->map_cookie = malloc(source->nb_cookies*sizeof(struct _u_cookie));
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
			dest->binary_body_length = source->binary_body_length;
			memcpy(dest->binary_body, source->binary_body, source->binary_body_length);
		}
		return 1;
	} else {
		return 0;
	}
}
