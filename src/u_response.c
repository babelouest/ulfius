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
 * version 3 of the License.
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
int set_response_header(struct MHD_Response * response, struct _u_map * response_map_header) {
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
int set_response_cookie(struct MHD_Response * response, struct _u_map * response_map_cookie) {
	char ** cookie_keys = u_map_enum_keys(response_map_cookie);
	char * cookie_value, * built_header = NULL;
	int i = -1, ret, len;
	if (response != NULL && response_map_cookie != NULL) {
		for (i=0; cookie_keys != NULL && cookie_keys[i] != NULL; i++) {
			cookie_value = u_map_get(response_map_cookie, cookie_keys[i]);
			len = snprintf(NULL, 0, "%s=%s", cookie_keys[i], cookie_value);
			built_header = malloc((len+1)*sizeof(char));
			snprintf(built_header, (len+1), "%s=%s", cookie_keys[i], cookie_value);
			ret = MHD_add_response_header (response, MHD_HTTP_HEADER_SET_COOKIE, built_header);
			free(cookie_value);
			if (ret == MHD_NO) {
				i = -1;
				break;
			}
			free(built_header);
		}
		u_map_clean_enum(cookie_keys);
	}
	return i;
}

/**
 * Add a cookie in the cookie map as defined in the RFC 6265
 */
int ulfius_add_cookie(struct _u_map * response_map_cookie, const char * key, const char * value, const char * expires, const uint max_age, 
								const char * domain, const char * path, const int secure, const int http_only) {
	char * attr_expires = NULL, * attr_max_age = NULL, * attr_domain = NULL, * attr_path = NULL, * attr_secure = NULL, * attr_http_only = NULL, * cookie_header_value = NULL;
	int len;
	if (response_map_cookie != NULL && key != NULL && value != NULL) {
		if (expires != NULL) {
			len = snprintf(NULL, 0, "; %s=%s", ULFIUS_COOKIE_ATTRIBUTE_EXPIRES, expires);
			attr_expires = malloc((len+1)*sizeof(char));
			snprintf(attr_expires, (len+1), "; %s=%s", ULFIUS_COOKIE_ATTRIBUTE_EXPIRES, expires);
		} else {
			attr_expires = strdup("");
		}
		if (max_age > 0) {
			len = snprintf(NULL, 0, "; %s=%d", ULFIUS_COOKIE_ATTRIBUTE_MAX_AGE, max_age);
			attr_max_age = malloc((len+1)*sizeof(char));
			snprintf(attr_max_age, (len+1), "; %s=%d", ULFIUS_COOKIE_ATTRIBUTE_MAX_AGE, max_age);
		} else {
			attr_max_age = strdup("");
		}
		if (domain != NULL) {
			len = snprintf(NULL, 0, "; %s=%s", ULFIUS_COOKIE_ATTRIBUTE_DOMAIN, domain);
			attr_domain = malloc((len+1)*sizeof(char));
			snprintf(attr_domain, (len+1), "; %s=%s", ULFIUS_COOKIE_ATTRIBUTE_DOMAIN, domain);
		} else {
			attr_domain = strdup("");
		}
		if (path != NULL) {
			len = snprintf(NULL, 0, "; %s=%s", ULFIUS_COOKIE_ATTRIBUTE_PATH, path);
			attr_path = malloc((len+1)*sizeof(char));
			snprintf(attr_path, (len+1), "; %s=%s", ULFIUS_COOKIE_ATTRIBUTE_DOMAIN, path);
		} else {
			attr_path = strdup("");
		}
		if (secure) {
			len = snprintf(NULL, 0, "; %s", ULFIUS_COOKIE_ATTRIBUTE_SECURE);
			attr_secure = malloc((len+1)*sizeof(char));
			snprintf(attr_secure, (len+1), "; %s", ULFIUS_COOKIE_ATTRIBUTE_SECURE);
		} else {
			attr_secure = strdup("");
		}
		if (http_only) {
			len = snprintf(NULL, 0, "; %s", ULFIUS_COOKIE_ATTRIBUTE_HTTPONLY);
			attr_http_only = malloc((len+1)*sizeof(char));
			snprintf(attr_http_only, (len+1), "; %s", ULFIUS_COOKIE_ATTRIBUTE_HTTPONLY);
		} else {
			attr_http_only = strdup("");
		}
		len = snprintf(NULL, 0, "%s%s%s%s%s%s%s", value, attr_expires, attr_max_age, attr_domain, attr_path, attr_secure, attr_http_only);
		cookie_header_value = malloc((len+1)*sizeof(char));
		snprintf(cookie_header_value, (len+1), "%s%s%s%s%s%s%s", value, attr_expires, attr_max_age, attr_domain, attr_path, attr_secure, attr_http_only);
		len = u_map_put(response_map_cookie, key, cookie_header_value);
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
		free(cookie_header_value);
		cookie_header_value = NULL;
		return len;
	} else {
		return 0;
	}
}
