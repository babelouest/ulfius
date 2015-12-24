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
 * return U_OK on success
 */
int u_map_init(struct _u_map * map) {
  if (map != NULL) {
    map->nb_values = 0;
    map->keys = malloc(sizeof(char *));
    if (map->keys == NULL) {
      return U_ERROR_MEMORY;
    }
    map->keys[0] = NULL;

    map->values = malloc(sizeof(char *));
    if (map->values == NULL) {
      free(map->keys);
      return U_ERROR_MEMORY;
    }
    map->values[0] = NULL;

    return U_OK;
  } else {
    return U_ERROR_PARAMS;
  }
}

/**
 * free the struct _u_map's inner components
 * return U_OK on success
 */
int u_map_clean(struct _u_map * u_map) {
  if (u_map != NULL) {
    u_map_clean_enum(u_map->keys);
    u_map_clean_enum(u_map->values);
    return U_OK;
  } else {
    return U_ERROR_PARAMS;
  }
}

/**
 * free the struct _u_map and its components
 * return U_OK on success
 */
int u_map_clean_full(struct _u_map * u_map) {
  if (u_map_clean(u_map) == U_OK) {
    free(u_map);
    return U_OK;
  } else {
    return U_ERROR_PARAMS;
  }
}

/**
 * free an enum return by functions u_map_enum_keys or u_map_enum_values
 * return U_OK on success
 */
int u_map_clean_enum(char ** array) {
  int i;
  if (array != NULL) {
    for (i=0; array[i] != NULL; i++) {
      free(array[i]);
      array[i] = NULL;
    }
    free(array);
    return U_OK;
  } else {
    return U_ERROR_PARAMS;
  }
}

/**
 * returns an array containing all the keys in the struct _u_map
 * return an array of char * ending with a NULL element
 */
const char ** u_map_enum_keys(const struct _u_map * u_map) {
  return (const char **)u_map->keys;
}

/**
 * returns an array containing all the values in the struct _u_map
 * return an array of char * ending with a NULL element
 */
const char ** u_map_enum_values(const struct _u_map * u_map) {
  return (const char **)u_map->values;
}

/**
 * return true if the sprcified u_map contains the specified key
 * false otherwise
 * search is case sensitive
 */
int u_map_has_key(const struct _u_map * u_map, const char * key) {
  int i;
  if (u_map != NULL && key != NULL) {
    for (i=0; u_map->keys[i] != NULL; i++) {
      if (0 == strcmp(u_map->keys[i], key)) {
        return 1;
      }
    }
  }
  return 0;
}

/**
 * return true if the sprcified u_map contains the specified value
 * false otherwise
 * search is case sensitive
 */
int u_map_has_value(const struct _u_map * u_map, const char * value) {
  int i;
  if (u_map != NULL && value != NULL) {
    for (i=0; u_map->values[i] != NULL; i++) {
      if (0 == strcmp(u_map->values[i], value)) {
        return 1;
      }
    }
  }
  return 0;
}

/**
 * add the specified key/value pair into the specified u_map
 * if the u_map already contains a pair with the same key, replace the value
 * return U_OK on success
 */
int u_map_put(struct _u_map * u_map, const char * key, const char * value) {
  int i;
  char * dup_key, * dup_value;
  if (u_map != NULL && key != NULL && strlen(key) > 0) {
    for (i=0; i < u_map->nb_values; i++) {
      if (0 == strcmp(u_map->keys[i], key)) {
        free(u_map->values[i]);
        dup_value = u_strdup(value);
        if (dup_value == NULL) {
          return U_ERROR_MEMORY;
        } else {
          u_map->values[i] = dup_value;
        }
        return U_OK;
      }
    }
    if (u_map->values[i] == NULL) {
      // Not found, append
      dup_key = u_strdup(key);
      if (dup_key == NULL) {
        return U_ERROR_MEMORY;
      }
      dup_value = u_strdup(value);
      if (dup_value == NULL) {
        free(dup_key);
        return U_ERROR_MEMORY;
      }
      
      // Append key
      for (i = 0; u_map->keys[i] != NULL; i++);
      u_map->keys = realloc(u_map->keys, (i + 2)*sizeof(char *));
      if (u_map->keys == NULL) {
        free(dup_key);
        free(dup_value);
        return U_ERROR_MEMORY;
      }
      u_map->keys[i] = (char *)dup_key;
      u_map->keys[i+1] = NULL;
      
      // Append value
      for (i = 0; u_map->values[i] != NULL; i++);
      u_map->values = realloc(u_map->values, (i + 2)*sizeof(char *));
      if (u_map->values == NULL) {
        free(dup_key);
        free(dup_value);
        return U_ERROR_MEMORY;
      }
      u_map->values[i] = (char *)dup_value;
      u_map->values[i+1] = NULL;
      
      u_map->nb_values++;
    }
    return U_OK;
  } else {
    return U_ERROR_PARAMS;
  }
}

/**
 * remove an pair key/value that has the specified key
 * return U_OK on success, U_NOT_FOUND if key was not found, error otherwise
 */
int u_map_remove_from_key(struct _u_map * u_map, const char * key) {
  int i, res, found = 0;
  
  if (u_map == NULL || key == NULL) {
    return U_ERROR_PARAMS;
  } else {
    for (i = u_map->nb_values-1; i >= 0; i--) {
      if (0 == strcmp(u_map->keys[i], key)) {
        found = 1;
        res = u_map_remove_at(u_map, i);
        if (res != U_OK) {
          return res;
        }
      }
    }
    if (found) {
      return U_OK;
    } else {
      return U_ERROR_NOT_FOUND;
    }
  }
}

/**
 * remove all pairs key/value that has the specified key (case insensitive search)
 * return U_OK on success, U_NOT_FOUND if key was not found, error otherwise
 */
int u_map_remove_from_key_case(struct _u_map * u_map, const char * key) {
  int i, res, found = 0;
  
  if (u_map == NULL || key == NULL) {
    return U_ERROR_PARAMS;
  } else {
    for (i = u_map->nb_values-1; i >= 0; i--) {
      if (0 == strcasecmp(u_map->keys[i], key)) {
        found = 1;
        res = u_map_remove_at(u_map, i);
        if (res != U_OK) {
          return res;
        }
      }
    }
    if (found) {
      return U_OK;
    } else {
      return U_ERROR_NOT_FOUND;
    }
  }
}

/**
 * remove all pairs key/value that has the specified value
 * return U_OK on success, U_NOT_FOUND if key was not found, error otherwise
 */
int u_map_remove_from_value(struct _u_map * u_map, const char * key) {
  int i, res, found = 0;
  
  if (u_map == NULL || key == NULL) {
    return U_ERROR_PARAMS;
  } else {
    for (i = u_map->nb_values-1; i >= 0; i--) {
      if (0 == strcmp(u_map->values[i], key)) {
        found = 1;
        res = u_map_remove_at(u_map, i);
        if (res != U_OK) {
          return res;
        }
      }
    }
    if (found) {
      return U_OK;
    } else {
      return U_ERROR_NOT_FOUND;
    }
  }
}

/**
 * remove all pairs key/value that has the specified value (case insensitive search)
 * return U_OK on success, U_NOT_FOUND if key was not found, error otherwise
 */
int u_map_remove_from_value_case(struct _u_map * u_map, const char * key) {
  int i, res, found = 0;
  
  if (u_map == NULL || key == NULL) {
    return U_ERROR_PARAMS;
  } else {
    for (i = u_map->nb_values-1; i >= 0; i--) {
      if (0 == strcasecmp(u_map->values[i], key)) {
        found = 1;
        res = u_map_remove_at(u_map, i);
        if (res != U_OK) {
          return res;
        }
      }
    }
    if (found) {
      return U_OK;
    } else {
      return U_ERROR_NOT_FOUND;
    }
  }
}

/**
 * remove the pair key/value at the specified index
 * return U_OK on success, U_NOT_FOUND if index is out of bound, error otherwise
 */
int u_map_remove_at(struct _u_map * u_map, const int index) {
  int i;
  if (u_map == NULL || index < 0) {
    return U_ERROR_PARAMS;
  } else if (index >= u_map->nb_values) {
    return U_ERROR_NOT_FOUND;
  } else {
    free(u_map->keys[index]);
    free(u_map->values[index]);
    for (i = index; i < u_map->nb_values; i++) {
      u_map->keys[i] = u_map->keys[i + 1];
      u_map->values[i] = u_map->values[i + 1];
    }
    u_map->keys = realloc(u_map->keys, (u_map->nb_values)*sizeof(char *));
    if (u_map->keys == NULL) {
      return U_ERROR_MEMORY;
    }
    u_map->values = realloc(u_map->values, (u_map->nb_values)*sizeof(char *));
    if (u_map->values == NULL) {
      return U_ERROR_MEMORY;
    }
    
    u_map->nb_values--;
    return U_OK;
  }
}

/**
 * get the value corresponding to the specified key in the u_map
 * return NULL if no match found
 * search is case sensitive
 */
const char * u_map_get(const struct _u_map * u_map, const char * key) {
  int i;
  if (u_map != NULL && key != NULL) {
    for (i=0; u_map->keys[i] != NULL; i++) {
      if (0 == strcmp(u_map->keys[i], key)) {
        return u_map->values[i];
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
  int i;
  if (u_map != NULL && key != NULL) {
    for (i=0; u_map->keys[i] != NULL; i++) {
      if (0 == strcasecmp(u_map->keys[i], key)) {
        return 1;
      }
    }
  }
  return 0;
}

/**
 * return true if the sprcified u_map contains the specified value
 * false otherwise
 * search is case insensitive
 */
int u_map_has_value_case(const struct _u_map * u_map, const char * value) {
  int i;
  if (u_map != NULL && value != NULL) {
    for (i=0; u_map->values[i] != NULL; i++) {
      if (0 == strcasecmp(u_map->values[i], value)) {
        return 1;
      }
    }
  }
  return 0;
}

/**
 * get the value corresponding to the specified key in the u_map
 * return NULL if no match found
 * search is case insensitive
 */
const char * u_map_get_case(const struct _u_map * u_map, const char * key) {
  int i;
  if (u_map != NULL && key != NULL) {
    for (i=0; u_map->keys[i] != NULL; i++) {
      if (0 == strcasecmp(u_map->keys[i], key)) {
        return u_map->values[i];
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
 * returned value must be cleaned after use
 */
struct _u_map * u_map_copy(const struct _u_map * source) {
  struct _u_map * copy = NULL;
  const char ** keys, * value;
  int i;
  if (source != NULL) {
    copy = malloc(sizeof(struct _u_map));
    if (copy == NULL) {
      return NULL;
    }
    if (u_map_init(copy) != U_OK) {
      free(copy);
      return NULL;
    }
    keys = u_map_enum_keys(source);
    for (i=0; keys != NULL && keys[i] != NULL; i++) {
      value = u_map_get(source, keys[i]);
      if (value == NULL || u_map_put(copy, keys[i], value) != U_OK) {
        return NULL;
      }
    }
  }
  return copy;
}

/**
 * Return the number of key/values pair in the specified struct _u_map
 * Return -1 on error
 */
int u_map_count(const struct _u_map * source) {
  if (source != NULL) {
    if (source->nb_values >= 0) {
      return source->nb_values;
    }
  }
  return -1;
}
