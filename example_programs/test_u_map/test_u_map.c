/**
 * 
 * Ulfius Framework example program
 * 
 * This example program describes the main features 
 * that are available in struct _u_map
 * 
 * Copyright 2015-2022 Nicolas Mora <mail@babelouest.org>
 * 
 * License MIT
 *
 */

#include <stdio.h>
#include <string.h>
#include <ulfius.h>

#include "u_example.h"

/**
 * decode a u_map into a string
 */
char * print_map(const struct _u_map * map) {
  char * line, * to_return = NULL;
  const char **keys, * value;
  int len, i;
  if (map != NULL) {
    keys = u_map_enum_keys(map);
    for (i=0; keys[i] != NULL; i++) {
      value = u_map_get(map, keys[i]);
      len = snprintf(NULL, 0, "key is %s, length is %zu, value is %.*s", keys[i], u_map_get_length(map, keys[i]), (int)u_map_get_length(map, keys[i]), value);
      line = o_malloc((len+1)*sizeof(char));
      snprintf(line, (len+1), "key is %s, length is %zu, value is %.*s", keys[i], u_map_get_length(map, keys[i]), (int)u_map_get_length(map, keys[i]), value);
      if (to_return != NULL) {
        len = o_strlen(to_return) + o_strlen(line) + 1;
        to_return = o_realloc(to_return, (len+1)*sizeof(char));
        if (o_strlen(to_return) > 0) {
          strcat(to_return, "\n");
        }
      } else {
        to_return = o_malloc((o_strlen(line) + 1)*sizeof(char));
        to_return[0] = 0;
      }
      strcat(to_return, line);
      o_free(line);
    }
    return to_return;
  } else {
    return NULL;
  }
}

/**
 * put the content of a file in the u_map with the file_path as the key using u_map_put_binary
 */
int put_file_content_in_map (struct _u_map * map, const char * file_path, uint64_t offset) {
  void * buffer = NULL;
  long length;
  FILE * f;
  int res = U_OK;
  
  if (map != NULL) {
    f = fopen (file_path, "rb");
    if (f) {
      fseek (f, 0, SEEK_END);
      length = ftell (f);
      fseek (f, 0, SEEK_SET);
      buffer = o_malloc(length*sizeof(void));
      if (buffer) {
        fread (buffer, 1, length, f);
      }
      fclose (f);

      if (buffer) {
        res = u_map_put_binary(map,file_path,  (char *)buffer, offset, length);
        o_free(buffer);
      } else {
        res = U_ERROR;
      }
    } else {
      res = U_ERROR_PARAMS;
    }
  } else {
    res = U_ERROR_PARAMS;
  }
  return res;
}

int main (int argc, char **argv) {
    y_init_logs("test_u_map", Y_LOG_MODE_CONSOLE, Y_LOG_LEVEL_DEBUG, NULL, "Starting test_u_map");
    struct _u_map map, * map_copy;
    char * print, * print_copy;
    
    u_map_init(&map);
    
    print = print_map(&map);
    y_log_message(Y_LOG_LEVEL_DEBUG, "iteration 1, map is\n%s\n", print);
    o_free(print);
    
    u_map_put(&map, "key1", "value1");
    u_map_put(&map, "key2", "value2");
    u_map_put(&map, "key3", "value3");
    
    print = print_map(&map);
    y_log_message(Y_LOG_LEVEL_DEBUG, "iteration 2, map is\n%s\n", print);
    o_free(print);
    
    u_map_put(&map, "key2", "value4");
    
    print = print_map(&map);
    y_log_message(Y_LOG_LEVEL_DEBUG, "iteration 3, map is\n%s\n", print);
    o_free(print);
    
    map_copy = u_map_copy(&map);
    u_map_put(map_copy, "key4", "value5");
    
    print = print_map(&map);
    print_copy = print_map(map_copy);
    y_log_message(Y_LOG_LEVEL_DEBUG, "iteration 4, map is\n%s\nmap_copy is\n%s\n", print, print_copy);
    o_free(print);
    o_free(print_copy);
    
    u_map_remove_at(&map, 2);
    
    print = print_map(&map);
    y_log_message(Y_LOG_LEVEL_DEBUG, "iteration 5: remove item 2 from map, map is\n%s\n", print);
    o_free(print);
    
    u_map_remove_from_key(map_copy, "key1");
    
    print = print_map(map_copy);
    y_log_message(Y_LOG_LEVEL_DEBUG, "iteration 6: remove key:key1 from map_copy, map is\n%s\n", print);
    o_free(print);
    
    u_map_remove_from_key(map_copy, "key_nope");
    
    print = print_map(map_copy);
    y_log_message(Y_LOG_LEVEL_DEBUG, "iteration 7: remove key:key_nope from map_copy, map is\n%s\n", print);
    o_free(print);
    
    u_map_remove_from_key_case(map_copy, "Key2");
    
    print = print_map(map_copy);
    y_log_message(Y_LOG_LEVEL_DEBUG, "iteration 8: remove case key:Key2 from map_copy, map is\n%s\n", print);
    o_free(print);
    
    u_map_remove_from_value(map_copy, "value3");
    
    print = print_map(map_copy);
    y_log_message(Y_LOG_LEVEL_DEBUG, "iteration 9: remove value:value3 from map_copy, map is\n%s\n", print);
    o_free(print);
    
    u_map_remove_from_value_case(map_copy, "Value5");
    
    print = print_map(map_copy);
    y_log_message(Y_LOG_LEVEL_DEBUG, "iteration 10: remove case value:Value5 from map_copy, map is\n%s\n", print);
    o_free(print);
    
    y_log_message(Y_LOG_LEVEL_DEBUG, "map has key key1? %d", u_map_has_key(&map, "key1"));
    y_log_message(Y_LOG_LEVEL_DEBUG, "map has key key_nope? %d", u_map_has_key(&map, "key_nope"));
    y_log_message(Y_LOG_LEVEL_DEBUG, "map has key Key1? %d", u_map_has_key(&map, "Key1"));
    y_log_message(Y_LOG_LEVEL_DEBUG, "map has key Key1 (no case) ? %d", u_map_has_key_case(&map, "Key1"));
    y_log_message(Y_LOG_LEVEL_DEBUG, "map has key Key_nope (no case) ? %d", u_map_has_key_case(&map, "Key_nope"));
    
    y_log_message(Y_LOG_LEVEL_DEBUG, "map has value value1? %d", u_map_has_value(&map, "value1"));
    y_log_message(Y_LOG_LEVEL_DEBUG, "map has value value_nope? %d", u_map_has_value(&map, "value_nope"));
    y_log_message(Y_LOG_LEVEL_DEBUG, "map has value Value1? %d", u_map_has_value(&map, "Value1"));
    y_log_message(Y_LOG_LEVEL_DEBUG, "map has value Value1 (no case) ? %d", u_map_has_value_case(&map, "Value1"));
    y_log_message(Y_LOG_LEVEL_DEBUG, "map has value Value_nope (no case) ? %d", u_map_has_value_case(&map, "Value_nope"));
    
    y_log_message(Y_LOG_LEVEL_DEBUG, "get value from key key1: %s", u_map_get(&map, "key1"));
    y_log_message(Y_LOG_LEVEL_DEBUG, "get value from key key_nope: %s", u_map_get(&map, "key_nope"));
    y_log_message(Y_LOG_LEVEL_DEBUG, "get value from key Key1 (no case): %s", u_map_get_case(&map, "Key1"));
    y_log_message(Y_LOG_LEVEL_DEBUG, "get value from key Key_nope (no case): %s", u_map_get_case(&map, "Key_nope"));
    
    put_file_content_in_map(&map, "../sheep_counter/static/sheep.png", 0);
    
    print = print_map(&map);
    y_log_message(Y_LOG_LEVEL_DEBUG, "iteration 11, map is\n%s\n", print);
    o_free(print);
    
    u_map_remove_from_key(&map, "../sheep_counter/static/sheep.png");
    put_file_content_in_map(&map, "Makefile", 0);
    
    print = print_map(&map);
    y_log_message(Y_LOG_LEVEL_DEBUG, "iteration 12, map is\n%s\n", print);
    o_free(print);
    
    u_map_put_binary(&map, "Makefile", "Replace the first characters", 0, o_strlen("Replace the first characters"));
    
    print = print_map(&map);
    y_log_message(Y_LOG_LEVEL_DEBUG, "iteration 12, map is\n%s\n", print);
    o_free(print);
    
    u_map_put_binary(&map, "Makefile", "Append at the end of the value", u_map_get_length(&map, "Makefile"), o_strlen("Append at the end of the value"));
    
    print = print_map(&map);
    y_log_message(Y_LOG_LEVEL_DEBUG, "iteration 12, map is\n%s\n", print);
    o_free(print);
    
    u_map_clean(&map);
    u_map_clean_full(map_copy);
    
    y_close_logs();
    
    return 0;
}
