/**
 *
 * Ulfius Framework
 *
 * REST framework library
 *
 * u_umap.c: Simple map structure functions definitions
 * not memory friendly, all pointer returned must be freed after use
 *
 * Copyright 2015-2022 Nicolas Mora <mail@babelouest.org>
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

#include <stdio.h>
#include <string.h>
#include <u_private.h>
#include <ulfius.h>

int u_map_init(struct _u_map * u_map) {
  if (u_map != NULL) {
    u_map->nb_values = 0;
    u_map->keys = o_malloc(sizeof(char *));
    if (u_map->keys == NULL) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for u_map->keys");
      return U_ERROR_MEMORY;
    }
    u_map->keys[0] = NULL;

    u_map->values = o_malloc(sizeof(char *));
    if (u_map->values == NULL) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for u_map->values");
      o_free(u_map->keys);
      return U_ERROR_MEMORY;
    }
    u_map->values[0] = NULL;

    u_map->lengths = o_malloc(sizeof(size_t));
    if (u_map->lengths == NULL) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for u_map->lengths");
      o_free(u_map->keys);
      o_free(u_map->values);
      return U_ERROR_MEMORY;
    }
    u_map->lengths[0] = 0;

    return U_OK;
  } else {
    return U_ERROR_PARAMS;
  }
}

int u_map_clean(struct _u_map * u_map) {
  int i;
  if (u_map != NULL) {
    for (i=0; i<u_map->nb_values; i++) {
      o_free(u_map->keys[i]);
      o_free(u_map->values[i]);
    }
    o_free(u_map->keys);
    o_free(u_map->values);
    o_free(u_map->lengths);
    return U_OK;
  } else {
    return U_ERROR_PARAMS;
  }
}

int u_map_clean_full(struct _u_map * u_map) {
  if (u_map_clean(u_map) == U_OK) {
    o_free(u_map);
    return U_OK;
  } else {
    return U_ERROR_PARAMS;
  }
}

int u_map_clean_enum(char ** array) {
  int i;
  if (array != NULL) {
    for (i=0; array[i] != NULL; i++) {
      o_free(array[i]);
      array[i] = NULL;
    }
    o_free(array);
    return U_OK;
  } else {
    return U_ERROR_PARAMS;
  }
}

const char ** u_map_enum_keys(const struct _u_map * u_map) {
  return (const char **)u_map->keys;
}

const char ** u_map_enum_values(const struct _u_map * u_map) {
  return (const char **)u_map->values;
}

int u_map_has_key(const struct _u_map * u_map, const char * key) {
  int i;
  if (u_map != NULL && key != NULL) {
    for (i=0; u_map->keys[i] != NULL; i++) {
      if (0 == o_strcmp(u_map->keys[i], key)) {
        return 1;
      }
    }
  }
  return 0;
}

int u_map_has_value(const struct _u_map * u_map, const char * value) {
  return u_map_has_value_binary(u_map, value, o_strlen(value));
}

int u_map_has_value_binary(const struct _u_map * u_map, const char * value, size_t length) {
  int i;
  if (u_map != NULL && value != NULL) {
    for (i=0; u_map->values[i] != NULL; i++) {
      if (0 == memcmp(u_map->values[i], value, length)) {
        return 1;
      }
    }
  }
  return 0;
}

int u_map_put(struct _u_map * u_map, const char * key, const char * value) {
  if (value != NULL) {
    return u_map_put_binary(u_map, key, value, 0, o_strlen(value)+1);
  } else {
    return u_map_put_binary(u_map, key, NULL, 0, 0);
  }
}

int u_map_put_binary(struct _u_map * u_map, const char * key, const char * value, uint64_t offset, size_t length) {
  int i;
  char * dup_key, * dup_value;
  if (u_map != NULL && key != NULL && !o_strnullempty(key)) {
    for (i=0; i < u_map->nb_values; i++) {
      if (0 == o_strcmp(u_map->keys[i], key)) {
        // Key already exist, extend and/or replace value
        if (u_map->lengths[i] < (offset + length)) {
          u_map->values[i] = o_realloc(u_map->values[i], (offset + length + 1)*sizeof(char));
          if (u_map->values[i] == NULL) {
            y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for u_map->values");
            return U_ERROR_MEMORY;
          }
        }
        if (value != NULL) {
          memcpy(u_map->values[i]+offset, value, length);
          if (u_map->lengths[i] < (offset + length)) {
            u_map->lengths[i] = (offset + length);
            *(u_map->values[i]+offset+length) = '\0';
          }
        } else {
          o_free(u_map->values[i]);
          u_map->values[i] = o_strdup("");
          u_map->lengths[i] = 0;
        }
        return U_OK;
      }
    }
    if (u_map->values[i] == NULL) {
      // Not found, add key/value
      dup_key = o_strdup(key);
      if (dup_key == NULL) {
        y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for dup_key");
        return U_ERROR_MEMORY;
      }
      if (value != NULL) {
        dup_value = o_malloc((offset + length + 1)*sizeof(char));
        if (dup_value == NULL) {
          y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for dup_value");
          o_free(dup_key);
          return U_ERROR_MEMORY;
        }
        memcpy((dup_value + offset), value, length);
        *(dup_value + offset + length) = '\0';
      } else {
        dup_value = o_strdup("");
      }

      // Append key
      for (i = 0; u_map->keys[i] != NULL; i++);
      u_map->keys = o_realloc(u_map->keys, (i + 2)*sizeof(char *));
      if (u_map->keys == NULL) {
        y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for u_map->keys");
        o_free(dup_key);
        o_free(dup_value);
        return U_ERROR_MEMORY;
      }
      u_map->keys[i] = (char *)dup_key;
      u_map->keys[i+1] = NULL;

      // Append value
      u_map->values = o_realloc(u_map->values, (i + 2)*sizeof(char *));
      if (u_map->values == NULL) {
        y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for u_map->values");
        o_free(dup_key);
        o_free(dup_value);
        return U_ERROR_MEMORY;
      }
      u_map->values[i] = dup_value;
      u_map->values[i+1] = NULL;

      // Append length
      u_map->lengths = o_realloc(u_map->lengths, (i + 2)*sizeof(size_t));
      if (u_map->lengths == NULL) {
        y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for u_map->lengths");
        o_free(dup_key);
        o_free(dup_value);
        return U_ERROR_MEMORY;
      }
      u_map->lengths[i] = (offset + length);
      u_map->lengths[i+1] = 0;

      u_map->nb_values++;
    }
    return U_OK;
  } else {
    return U_ERROR_PARAMS;
  }
}

int u_map_remove_from_key(struct _u_map * u_map, const char * key) {
  int i, res, found = 0;

  if (u_map == NULL || key == NULL) {
    return U_ERROR_PARAMS;
  } else {
    for (i = u_map->nb_values-1; i >= 0; i--) {
      if (0 == o_strcmp(u_map->keys[i], key)) {
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

int u_map_remove_from_key_case(struct _u_map * u_map, const char * key) {
  int i, res, found = 0;

  if (u_map == NULL || key == NULL) {
    return U_ERROR_PARAMS;
  } else {
    for (i = u_map->nb_values-1; i >= 0; i--) {
      if (0 == o_strcasecmp(u_map->keys[i], key)) {
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

int u_map_remove_from_value(struct _u_map * u_map, const char * value) {
  return u_map_remove_from_value_binary(u_map, value, o_strlen(value));
}

int u_map_remove_from_value_binary(struct _u_map * u_map, const char * value, size_t length) {
  int i, res, found = 0;

  if (u_map == NULL || value == NULL) {
    return U_ERROR_PARAMS;
  } else {
    for (i = u_map->nb_values-1; i >= 0; i--) {
      if (0 == memcmp(u_map->values[i], value, length)) {
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

int u_map_remove_from_value_case(struct _u_map * u_map, const char * value) {
  int i, res, found = 0;

  if (u_map == NULL || value == NULL) {
    return U_ERROR_PARAMS;
  } else {
    for (i = u_map->nb_values-1; i >= 0; i--) {
      if (0 == o_strcasecmp(u_map->values[i], value)) {
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

int u_map_remove_at(struct _u_map * u_map, const int index) {
  int i;
  if (u_map == NULL || index < 0) {
    return U_ERROR_PARAMS;
  } else if (index >= u_map->nb_values) {
    return U_ERROR_NOT_FOUND;
  } else {
    o_free(u_map->keys[index]);
    o_free(u_map->values[index]);
    for (i = index; i < u_map->nb_values; i++) {
      u_map->keys[i] = u_map->keys[i + 1];
      u_map->values[i] = u_map->values[i + 1];
      u_map->lengths[i] = u_map->lengths[i + 1];
    }
    u_map->keys = o_realloc(u_map->keys, (u_map->nb_values)*sizeof(char *));
    if (u_map->keys == NULL) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for u_map->keys");
      return U_ERROR_MEMORY;
    }
    u_map->values = o_realloc(u_map->values, (u_map->nb_values)*sizeof(char *));
    if (u_map->values == NULL) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for u_map->values");
      return U_ERROR_MEMORY;
    }
    u_map->lengths = o_realloc(u_map->lengths, (u_map->nb_values)*sizeof(char *));
    if (u_map->lengths == NULL) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for u_map->lengths");
      return U_ERROR_MEMORY;
    }

    u_map->nb_values--;
    return U_OK;
  }
}

const char * u_map_get(const struct _u_map * u_map, const char * key) {
  int i;
  if (u_map != NULL && key != NULL) {
    for (i=0; u_map->keys[i] != NULL; i++) {
      if (0 == o_strcmp(u_map->keys[i], key)) {
        if (u_map->lengths[i] > 0) {
          return u_map->values[i];
        } else {
          return NULL;
        }
      }
    }
    return NULL;
  } else {
    return NULL;
  }
}

int u_map_has_key_case(const struct _u_map * u_map, const char * key) {
  int i;
  if (u_map != NULL && key != NULL) {
    for (i=0; u_map->keys[i] != NULL; i++) {
      if (0 == o_strcasecmp(u_map->keys[i], key)) {
        return 1;
      }
    }
  }
  return 0;
}

int u_map_has_value_case(const struct _u_map * u_map, const char * value) {
  int i;
  if (u_map != NULL && value != NULL) {
    for (i=0; u_map->values[i] != NULL; i++) {
      if (0 == o_strcasecmp(u_map->values[i], value)) {
        return 1;
      }
    }
  }
  return 0;
}

const char * u_map_get_case(const struct _u_map * u_map, const char * key) {
  int i;
  if (u_map != NULL && key != NULL) {
    for (i=0; u_map->keys[i] != NULL; i++) {
      if (0 == o_strcasecmp(u_map->keys[i], key)) {
        return u_map->values[i];
      }
    }
    return NULL;
  } else {
    return NULL;
  }
}

ssize_t u_map_get_length(const struct _u_map * u_map, const char * key) {
  int i;
  if (u_map != NULL && key != NULL) {
    for (i=0; u_map->keys[i] != NULL; i++) {
      if (0 == o_strcmp(u_map->keys[i], key)) {
        return u_map->lengths[i];
      }
    }
    return -1;
  } else {
    return -1;
  }
}

ssize_t u_map_get_case_length(const struct _u_map * u_map, const char * key) {
  int i;
  if (u_map != NULL && key != NULL) {
    for (i=0; u_map->keys[i] != NULL; i++) {
      if (0 == o_strcasecmp(u_map->keys[i], key)) {
        return u_map->lengths[i];
      }
    }
    return -1;
  } else {
    return -1;
  }
}

struct _u_map * u_map_copy(const struct _u_map * source) {
  struct _u_map * copy = NULL;
  const char ** keys, * value;
  int i;
  if (source != NULL) {
    copy = o_malloc(sizeof(struct _u_map));
    if (copy == NULL) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error allocating memory for u_map_copy.copy");
      return NULL;
    }
    if (u_map_init(copy) != U_OK) {
      o_free(copy);
      y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error u_map_init for u_map_copy.copy");
      return NULL;
    }
    keys = u_map_enum_keys(source);
    for (i=0; keys != NULL && keys[i] != NULL; i++) {
      value = u_map_get(source, keys[i]);
      if (u_map_put_binary(copy, keys[i], value, 0, source->lengths[i]) != U_OK) {
        y_log_message(Y_LOG_LEVEL_ERROR, "Ulfius - Error u_map_put_binary for u_map_copy.copy");
        u_map_clean_full(copy);
        return NULL;
      }
    }
  }
  return copy;
}

int u_map_copy_into(struct _u_map * dest, const struct _u_map * source) {
  const char ** keys;
  int i, res;

  if (source != NULL && dest != NULL) {
    keys = u_map_enum_keys(source);
    for (i=0; keys != NULL && keys[i] != NULL; i++) {
      res = u_map_put(dest, keys[i], u_map_get(source, keys[i]));
      if (res != U_OK) {
        return res;
      }
    }
    return U_OK;
  } else {
    return U_ERROR_PARAMS;
  }
}

int u_map_count(const struct _u_map * source) {
  if (source != NULL) {
    if (source->nb_values >= 0) {
      return source->nb_values;
    }
  }
  return -1;
}

int u_map_empty(struct _u_map * u_map) {
  int ret = u_map_clean(u_map);
  if (ret == U_OK) {
    return u_map_init(u_map);
  } else {
    return ret;
  }
}
