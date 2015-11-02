/**
 * 
 * Ulfius Framework
 * 
 * REST framework library
 * 
 * u_umap.c: Simple map structure functions definitions
 * not memory friendly, all pointer returned must be freed after use
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
#include <stdio.h>

/**
 * initialize a struct _u_map
 * this function MUST be called after a declaration or allocation
 */
void u_map_init(struct _u_map * map) {
	if (map != NULL) {
		map->nb_values = 0;
		map->value_list = NULL;
	}
}

/**
 * free the struct _u_map and its components
 * return true if no error
 */
int u_map_clean(struct _u_map * u_map) {
	int i;
	if (u_map != NULL) {
		for (i = 0; i < u_map->nb_values; i++) {
			free(u_map->value_list[i].key);
			u_map->value_list[i].key = NULL;
			free(u_map->value_list[i].value);
			u_map->value_list[i].value = NULL;
		}
		free(u_map->value_list);
		u_map->value_list = NULL;
		free(u_map);
		return 1;
	} else {
		return 0;
	}
}

/**
 * free an enum return by functions u_map_enum_keys or u_map_enum_values
 * return true if no error
 */
int u_map_clean_enum(char ** array) {
	int i;
	if (array != NULL) {
		for (i=0; array[i] != NULL; i++) {
			free(array[i]);
			array[i] = NULL;
		}
		free(array);
		return 1;
	} else {
		return 0;
	}
}

/**
 * returns an array containing all the keys in the struct _u_map
 * return an array of char * ending with a NULL element
 * use u_map_clean_enum(char ** array) to clean a returned array
 */
char ** u_map_enum_keys(const struct _u_map * u_map) {
	char ** key_list = NULL;
	int i;
	if (u_map != NULL) {
		key_list = malloc((u_map->nb_values+1) * sizeof(char*));
		if (key_list == NULL) {
			return NULL;
		}
		for (i = 0; i < u_map->nb_values; i++) {
			key_list[i] = strdup(u_map->value_list[i].key);
		}
		key_list[u_map->nb_values] = NULL;
	}
	return key_list;
}

/**
 * returns an array containing all the values in the struct _u_map
 * return an array of char * ending with a NULL element
 * use u_map_clean_enum(char ** array) to clean a returned array
 */
char ** u_map_enum_values(const struct _u_map * u_map) {
	char ** value_list = NULL;
	int i;
	if (u_map != NULL) {
		value_list = malloc((u_map->nb_values+1) * sizeof(char*));
		if (value_list == NULL) {
			return NULL;
		}
		for (i = 0; i < u_map->nb_values; i++) {
			value_list[i] = strdup(u_map->value_list[i].value);
		}
		value_list[u_map->nb_values] = NULL;
	}
	return value_list;
}

/**
 * return true if the sprcified u_map contains the specified key
 * false otherwise
 * search is case sensitive
 */
int u_map_has_key(const struct _u_map * u_map, const char * key) {
	int has_key = 0, i;
	if (u_map != NULL && key != NULL) {
		char ** key_list = u_map_enum_keys(u_map);
		for (i=0; key_list[i] != NULL; i++) {
			if (0 == strcmp(key_list[i], key)) {
				has_key = 1;
			}
			free(key_list[i]);
			key_list[i] = NULL;
		}
		free(key_list);
		key_list = NULL;
	}
	return has_key;
}

/**
 * return true if the sprcified u_map contains the specified value
 * false otherwise
 * search is case sensitive
 */
int u_map_has_value(const struct _u_map * u_map, const char * value) {
	int has_value = 0, i;
	if (u_map != NULL && value != NULL) {
		char ** value_list = u_map_enum_values(u_map);
		for (i=0; value_list[i] != NULL; i++) {
			if (0 == strcmp(value_list[i], value)) {
				has_value = 1;
			}
			free(value_list[i]);
			value_list[i] = NULL;
		}
		free(value_list);
		value_list = NULL;
	}
	return has_value;
}

/**
 * add the specified key/value pair into the specified u_map
 * if the u_map already contains a pair with the same key, replace the value
 * return true if no error
 */
int u_map_put(struct _u_map * u_map, const char * key, const char * value) {
	int found = 0, i;
	if (u_map != NULL && key != NULL && value != NULL) {
		for (i=0; i < u_map->nb_values; i++) {
			if (0 == strcmp(u_map->value_list[i].key, key)) {
				found = 1;
				free(u_map->value_list[i].value);
				u_map->value_list[i].value = NULL;
				u_map->value_list[i].value = strdup(value);
			}
		}
		if (!found) {
			u_map->value_list = realloc(u_map->value_list, (u_map->nb_values+1)*sizeof(struct _u_map_value));
			if (u_map->value_list == NULL) {
				return 0;
			}
			u_map->value_list[u_map->nb_values].key = strdup(key);
			u_map->value_list[u_map->nb_values].value = strdup(value);
			if (u_map->value_list[u_map->nb_values].key == NULL || u_map->value_list[u_map->nb_values].value == NULL) {
				return 0;
			}
			u_map->nb_values++;
		}
		return 1;
	} else {
		return 0;
	}
}

/**
 * get the value corresponding to the specified key in the u_map
 * return NULL if no match found
 * search is case sensitive
 * returned value must be freed after use
 */
char * u_map_get(const struct _u_map * u_map, const char * key) {
	int i;
	if (u_map != NULL && key != NULL) {
		for (i=0; i < u_map->nb_values; i++) {
			if (0 == strcmp(u_map->value_list[i].key, key)) {
				return strdup(u_map->value_list[i].value);
			}
		}
		return NULL;
	} else {
		return NULL;
	}
}

/**
 * return true if the sprcified u_map contains the specified key
 * false otherwise
 * search is case insensitive
 */
int u_map_has_key_case(const struct _u_map * u_map, const char * key) {
	int has_key = 0, i;
	if (u_map != NULL && key != NULL) {
		char ** key_list = u_map_enum_keys(u_map);
		for (i=0; key_list[i] != NULL; i++) {
			if (0 == strcasecmp(key_list[i], key)) {
				has_key = 1;
			}
			free(key_list[i]);
			key_list[i] = NULL;
		}
		free(key_list);
		key_list = NULL;
	}
	return has_key;
}

/**
 * return true if the sprcified u_map contains the specified value
 * false otherwise
 * search is case insensitive
 */
int u_map_has_value_case(const struct _u_map * u_map, const char * value) {
	int has_value = 0, i;
	if (u_map != NULL && value != NULL) {
		char ** value_list = u_map_enum_values(u_map);
		for (i=0; value_list[i] != NULL; i++) {
			if (0 == strcasecmp(value_list[i], value)) {
				has_value = 1;
			}
			free(value_list[i]);
			value_list[i] = NULL;
		}
		free(value_list);
		value_list = NULL;
	}
	return has_value;
}

/**
 * get the value corresponding to the specified key in the u_map
 * return NULL if no match found
 * search is case insensitive
 * returned value must be freed after use
 */
char * u_map_get_case(const struct _u_map * u_map, const char * key) {
	int i;
	if (u_map != NULL && key != NULL) {
		for (i=0; i < u_map->nb_values; i++) {
			if (0 == strcasecmp(u_map->value_list[i].key, key)) {
				return strdup(u_map->value_list[i].value);
			}
		}
		return NULL;
	} else {
		return NULL;
	}
}

/**
 * Create an exact copy of the specified struct _u_map
 * return a reference to the copy, NULL otherwise
 * returned value must be freed after use
 */
struct _u_map * u_map_copy(const struct _u_map * source) {
	struct _u_map * copy = NULL;
	char ** keys, * value;
	int i;
	if (source != NULL) {
		copy = malloc(sizeof(struct _u_map));
		u_map_init(copy);
		keys = u_map_enum_keys(source);
		for (i=0; keys[i] != NULL; i++) {
			value = u_map_get(source, keys[i]);
			u_map_put(copy, keys[i], value);
			free(value);
		}
		u_map_clean_enum(keys);
	}
	return copy;
}
