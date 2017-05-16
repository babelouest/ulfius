/**
 * 
 * Ulfius Framework example program
 * 
 * This example program implements a websocket
 * 
 * Copyright 2017 Nicolas Mora <mail@babelouest.org>
 * 
 * License MIT
 *
 */

#include <string.h>
#include <unistd.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <ulfius.h>
#include <orcania.h>
#include <yder.h>
#include <static_file_callback.h>

#define PORT 9275
#define PREFIX_WEBSOCKET "/websocket"
#define PREFIX_STATIC "/static"

int callback_websocket (const struct _u_request * request, struct _u_response * response, void * user_data);

/**
 * decode a u_map into a string
 */
char * print_map(const struct _u_map * map) {
  char * line, * to_return = NULL;
  const char **keys, * value;
  int i;
  size_t len;
  if (map != NULL) {
    keys = u_map_enum_keys(map);
    for (i=0; keys[i] != NULL; i++) {
      value = u_map_get(map, keys[i]);
      line = msprintf("%s: %s", keys[i], value);
      if (to_return != NULL) {
        len = strlen(to_return) + strlen(line) + 1;
        to_return = o_realloc(to_return, (len+1)*sizeof(char));
        if (strlen(to_return) > 0) {
          strcat(to_return, "\n");
        }
      } else {
        to_return = o_malloc((strlen(line) + 1)*sizeof(char));
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

char * read_file(const char * filename) {
  char * buffer = NULL;
  long length;
  FILE * f = fopen (filename, "rb");
  if (filename != NULL) {

    if (f) {
      fseek (f, 0, SEEK_END);
      length = ftell (f);
      fseek (f, 0, SEEK_SET);
      buffer = malloc (length + 1);
      if (buffer) {
        fread (buffer, 1, length, f);
      }
      buffer[length] = '\0';
      fclose (f);
    }
    return buffer;
  } else {
    return NULL;
  }
}

int main(int argc, char ** argv) {
  int ret;
  struct _u_instance instance;
  struct static_file_config file_config;
  char * cert_file = NULL, * key_file = NULL;
  
  y_init_logs("websocket_example", Y_LOG_MODE_CONSOLE, Y_LOG_LEVEL_DEBUG, NULL, "Starting websocket_example");
  
  u_map_init(&file_config.mime_types);
  u_map_put(&file_config.mime_types, ".html", "text/html");
  u_map_put(&file_config.mime_types, ".css", "text/css");
  u_map_put(&file_config.mime_types, ".js", "application/javascript");
  u_map_put(&file_config.mime_types, ".png", "image/png");
  u_map_put(&file_config.mime_types, ".jpg", "image/jpeg");
  u_map_put(&file_config.mime_types, ".jpeg", "image/jpeg");
  u_map_put(&file_config.mime_types, ".ttf", "font/ttf");
  u_map_put(&file_config.mime_types, ".woff", "font/woff");
  u_map_put(&file_config.mime_types, ".woff2", "font/woff2");
  u_map_put(&file_config.mime_types, ".map", "application/octet-stream");
  u_map_put(&file_config.mime_types, "*", "application/octet-stream");
  file_config.files_path = "static";
  file_config.url_prefix = PREFIX_STATIC;
  
  if (ulfius_init_instance(&instance, PORT, NULL, NULL) != U_OK) {
    y_log_message(Y_LOG_LEVEL_ERROR, "Error ulfius_init_instance, abort");
    return(1);
  }
  
  u_map_put(instance.default_headers, "Access-Control-Allow-Origin", "*");
  
  // Endpoint list declaration
  ulfius_add_endpoint_by_val(&instance, "GET", PREFIX_WEBSOCKET, NULL, 0, &callback_websocket, NULL);
  ulfius_add_endpoint_by_val(&instance, "GET", PREFIX_STATIC, "*", 0, &callback_static_file, &file_config);
  
  // Start the framework
  if (argc > 3 && 0 == o_strcmp(argv[1], "-https")) {
    key_file = read_file(argv[1]);
    cert_file = read_file(argv[2]);
    if (key_file == NULL || cert_file == NULL) {
      printf("Error reading https certificate files\n");
      ret = U_ERROR_PARAMS;
    } else {
      ret = ulfius_start_secure_framework(&instance, key_file, cert_file);
    }
    free(key_file);
    free(cert_file);
  } else {
    ret = ulfius_start_framework(&instance);
  }
  
  if (ret == U_OK) {
    y_log_message(Y_LOG_LEVEL_INFO, "Start framework on port %d %s", instance.port, (argc > 1 && 0 == o_strcmp(argv[1], "-https"))?"https mode":"http mode");
    
    // Wait for the user to press <enter> on the console to quit the application
    getchar();
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "Error starting framework");
  }
  y_log_message(Y_LOG_LEVEL_INFO, "End framework");
  
  ulfius_stop_framework(&instance);
  ulfius_clean_instance(&instance);
  u_map_clean(&file_config.mime_types);
  y_close_logs();
  
  return 0;
}

void websocket_onclose_callback (const struct _u_request * request,
                                struct _websocket_manager * websocket_manager,
                                void * websocket_onclose_user_data) {
  if (websocket_onclose_user_data != NULL) {
    y_log_message(Y_LOG_LEVEL_DEBUG, "websocket_onclose_user_data is %s", websocket_onclose_user_data);
  }
}

void websocket_manager_callback(const struct _u_request * request,
                               struct _websocket_manager * websocket_manager,
                               void * websocket_manager_user_data) {
  int i, ret;
  char * my_message;
  if (websocket_manager_user_data != NULL) {
    y_log_message(Y_LOG_LEVEL_DEBUG, "websocket_manager_user_data is %s", websocket_manager_user_data);
  }
  for (i=0; i<5; i++) {
    sleep(2);
    if (websocket_manager != NULL && websocket_manager->connected) {
      my_message = msprintf("Send message #%d", i);
      ret = ulfius_websocket_send_message(websocket_manager, U_WEBSOCKET_OPCODE_TEXT, o_strlen(my_message), my_message);
      o_free(my_message);
      if (ret != U_OK) {
        y_log_message(Y_LOG_LEVEL_DEBUG, "Error send message");
        break;
      }
    } else {
      y_log_message(Y_LOG_LEVEL_DEBUG, "websocket not connected");
      break;
    }
  }
  y_log_message(Y_LOG_LEVEL_DEBUG, "Closing websocket_manager_callback");
}

void websocket_incoming_message_callback (const struct _u_request * request,
                                         struct _websocket_manager * websocket_manager,
                                         const struct _websocket_message * last_message,
                                         void * websocket_incoming_message_user_data) {
  if (websocket_incoming_message_user_data != NULL) {
    y_log_message(Y_LOG_LEVEL_DEBUG, "websocket_incoming_message_user_data is %s", websocket_incoming_message_user_data);
  }
  y_log_message(Y_LOG_LEVEL_DEBUG, "Incoming message, opcode: %x, mask: %d, len: %d", last_message->opcode, last_message->has_mask, last_message->data_len);
  if (last_message->opcode == U_WEBSOCKET_OPCODE_TEXT) {
    y_log_message(Y_LOG_LEVEL_DEBUG, "text payload '%.*s'", last_message->data_len, last_message->data);
  } else if (last_message->opcode == U_WEBSOCKET_OPCODE_BINARY) {
    y_log_message(Y_LOG_LEVEL_DEBUG, "binary payload");
  }
}

int callback_websocket (const struct _u_request * request, struct _u_response * response, void * user_data) {
  char * websocket_user_data = "my_user_data";
  if (ulfius_set_websocket_response(response, NULL, NULL, &websocket_manager_callback, websocket_user_data, &websocket_incoming_message_callback, websocket_user_data, &websocket_onclose_callback, websocket_user_data) == U_OK) {
    return U_CALLBACK_CONTINUE;
  } else {
    return U_CALLBACK_ERROR;
  }
}
