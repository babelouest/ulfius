/**
 * 
 * Ulfius Framework
 * 
 * REST framework library
 * 
 * u_request.c: request related functions defintions
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
 * Splits the url to an array of char *
 */
char ** split_url(const char * url) {
	char * saveptr=NULL, * cur_word=NULL, ** to_return = malloc(sizeof(char*)), * url_cpy = NULL, * url_cpy_addr = NULL;
	int counter = 1;
	
  url_cpy = url_cpy_addr = strdup(url);
	if (to_return != NULL) {
		to_return[0] = NULL;
		cur_word = strtok_r( url_cpy, ULFIUS_URL_SEPARATOR, &saveptr );
		while (cur_word != NULL) {
			if (0 != strcmp("", cur_word) && cur_word[0] != '?') {
				to_return = realloc(to_return, (counter+1)*sizeof(char*));
				to_return[counter-1] = strdup(cur_word);
				to_return[counter] = NULL;
				counter++;
			}
			cur_word = strtok_r( NULL, ULFIUS_URL_SEPARATOR, &saveptr );
		}
	}
  free(url_cpy_addr);
  url_cpy_addr = NULL;
	return to_return;
}

/**
 * Look in all endpoints if one matches the call specified by verb and url
 */
struct _u_endpoint * endpoint_match(const char * method, const char * url, struct _u_endpoint * endpoint_list) {
	char ** splitted_url, ** splitted_url_format;
	struct _u_endpoint * endpoint = NULL;
	int i;
	
	if (method != NULL && url != NULL && endpoint_list != NULL) {
		splitted_url = split_url(url);
		for (i=0; (endpoint_list[i].http_method != NULL && endpoint_list[i].url_format != NULL); i++) {
			if (0 == strcmp(endpoint_list[i].http_method, method)) {
				splitted_url_format = split_url(endpoint_list[i].url_format);
				if (url_format_match((const char **)splitted_url, (const char **)splitted_url_format)) {
					endpoint = (endpoint_list + i);
				}
				u_map_clean_enum(splitted_url_format);
				splitted_url_format = NULL;
			}
		}
		u_map_clean_enum(splitted_url);
		splitted_url = NULL;
	}
	return endpoint;
}

/**
 * Compare the current url to the specified url_format
 * return 1 if it's a match, otherwise 0
 * Match means that url is contained in url_format
 */
int url_format_match(const char ** splitted_url, const char ** splitted_url_format) {
	int i;
	
	for (i=0; splitted_url_format[i] != NULL; i++) {
		if (splitted_url_format[i][0] == '*' && splitted_url_format[i+1] == NULL) {
			return 1;
		}
		if (splitted_url[i] == NULL || (splitted_url_format[i][0] != '@' && splitted_url_format[i][0] != ':' && 0 != strcmp(splitted_url_format[i], splitted_url[i]))) {
			return 0;
		}
	}
	return (splitted_url[i] == NULL && splitted_url_format[i] == NULL);
}

/**
 * Fill a map with url parameters using the format url
 */
int parse_url(const char * url, struct _u_endpoint * endpoint, struct _u_map * map) {
	char * saveptr = NULL, * cur_word = NULL, * url_cpy = NULL, * url_cpy_addr = NULL;
	char * saveptr_format = NULL, * cur_word_format = NULL, * url_format_cpy = NULL, * url_format_cpy_addr = NULL;
	int ret = 0;

	if (map != NULL && endpoint != NULL) {
		url_cpy = url_cpy_addr = strdup(url);
		url_format_cpy = url_format_cpy_addr = strdup(endpoint->url_format);
		cur_word = strtok_r( url_cpy, ULFIUS_URL_SEPARATOR, &saveptr );
		cur_word_format = strtok_r( url_format_cpy, ULFIUS_URL_SEPARATOR, &saveptr_format );
		while (cur_word_format != NULL && cur_word != NULL) {
			if (cur_word_format[0] == ':' || cur_word_format[0] == '@') {
				u_map_put(map, cur_word_format+1, cur_word);
			}
			cur_word = strtok_r( NULL, ULFIUS_URL_SEPARATOR, &saveptr );
			cur_word_format = strtok_r( NULL, ULFIUS_URL_SEPARATOR, &saveptr_format );
		}
		ret = 1;
	}
	free(url_cpy_addr);
	url_cpy_addr = NULL;
	free(url_format_cpy_addr);
	url_format_cpy_addr = NULL;
	return ret;
}
