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

#include <orcania.h>
#include <yder.h>

#define U_DISABLE_JANSSON
#define U_DISABLE_CURL
#include <ulfius.h>

#include "../../example_callbacks/static_file/static_file_callback.h"

#define PORT 9275
#define PREFIX_WEBSOCKET "/websocket"
#define PREFIX_STATIC "/static"

#if defined(U_DISABLE_WEBSOCKET)
  #error Ulfius is not available with WebSockets support
#else

int callback_websocket (const struct _u_request * request, struct _u_response * response, void * user_data);
int callback_websocket_echo (const struct _u_request * request, struct _u_response * response, void * user_data);
int callback_websocket_file (const struct _u_request * request, struct _u_response * response, void * user_data);

static char * read_file(const char * filename) {
  char * buffer = NULL;
  long length;
  FILE * f;
  if (filename != NULL) {
    f = fopen (filename, "rb");
    if (f) {
      fseek (f, 0, SEEK_END);
      length = ftell (f);
      fseek (f, 0, SEEK_SET);
      buffer = o_malloc (length + 1);
      if (buffer) {
        fread (buffer, 1, length, f);
        buffer[length] = '\0';
      }
      fclose (f);
    }
    return buffer;
  } else {
    return NULL;
  }
}

/**
 * main function
 * open the wbservice on port 9275
 */
int main(int argc, char ** argv) {
  int ret;
  struct _u_instance instance;
  struct _static_file_config file_config;
  char * cert_file = NULL, * key_file = NULL;
  
  y_init_logs("websocket_example", Y_LOG_MODE_CONSOLE, Y_LOG_LEVEL_DEBUG, NULL, "Starting websocket_example");
  
  file_config.mime_types = o_malloc(sizeof(struct _u_map));
  u_map_init(file_config.mime_types);
  u_map_put(file_config.mime_types, ".html", "text/html");
  u_map_put(file_config.mime_types, ".css", "text/css");
  u_map_put(file_config.mime_types, ".js", "application/javascript");
  u_map_put(file_config.mime_types, ".png", "image/png");
  u_map_put(file_config.mime_types, ".jpg", "image/jpeg");
  u_map_put(file_config.mime_types, ".jpeg", "image/jpeg");
  u_map_put(file_config.mime_types, ".ttf", "font/ttf");
  u_map_put(file_config.mime_types, ".woff", "font/woff");
  u_map_put(file_config.mime_types, ".woff2", "font/woff2");
  u_map_put(file_config.mime_types, ".map", "application/octet-stream");
  u_map_put(file_config.mime_types, "*", "application/octet-stream");
  file_config.files_path = "static";
  file_config.url_prefix = PREFIX_STATIC;
  
  if (ulfius_init_instance(&instance, PORT, NULL, NULL) != U_OK) {
    y_log_message(Y_LOG_LEVEL_ERROR, "Error ulfius_init_instance, abort");
    return(1);
  }
  
  u_map_put(instance.default_headers, "Access-Control-Allow-Origin", "*");
  
  // Endpoint list declaration
  ulfius_add_endpoint_by_val(&instance, "GET", PREFIX_WEBSOCKET, NULL, 0, &callback_websocket, NULL);
  ulfius_add_endpoint_by_val(&instance, "GET", PREFIX_WEBSOCKET, "/echo", 0, &callback_websocket_echo, NULL);
  ulfius_add_endpoint_by_val(&instance, "GET", PREFIX_WEBSOCKET, "/file", 0, &callback_websocket_file, NULL);
  ulfius_add_endpoint_by_val(&instance, "GET", PREFIX_STATIC, "*", 0, &callback_static_file, &file_config);
  
  // Start the framework
  if (argc > 3 && 0 == o_strcmp(argv[1], "-https")) {
    key_file = read_file(argv[2]);
    cert_file = read_file(argv[3]);
    if (key_file == NULL || cert_file == NULL) {
      printf("Error reading https certificate files\n");
      ret = U_ERROR_PARAMS;
    } else {
      ret = ulfius_start_secure_framework(&instance, key_file, cert_file);
    }
    o_free(key_file);
    o_free(cert_file);
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
  u_map_clean_full(file_config.mime_types);
  y_close_logs();
  
  return 0;
}

/**
 * websocket_onclose_callback
 * onclose callback function
 * Used to clear data after the websocket connection is closed
 */
void websocket_onclose_callback (const struct _u_request * request,
                                struct _websocket_manager * websocket_manager,
                                void * websocket_onclose_user_data) {
  if (websocket_onclose_user_data != NULL) {
    y_log_message(Y_LOG_LEVEL_DEBUG, "websocket_onclose_user_data is %s", websocket_onclose_user_data);
    o_free(websocket_onclose_user_data);
  }
}

void websocket_onclose_file_callback (const struct _u_request * request,
                                struct _websocket_manager * websocket_manager,
                                void * websocket_onclose_user_data) {
  y_log_message(Y_LOG_LEVEL_DEBUG, "websocket_onclose_file_callback");
}

/**
 * websocket_manager_callback
 * send 5 text messages and 1 ping for 11 seconds, then closes the websocket
 */
void websocket_manager_callback(const struct _u_request * request,
                               struct _websocket_manager * websocket_manager,
                               void * websocket_manager_user_data) {
  if (websocket_manager_user_data != NULL) {
    y_log_message(Y_LOG_LEVEL_DEBUG, "websocket_manager_user_data is %s", websocket_manager_user_data);
  }
  
  // Send text message without fragmentation
  if (ulfius_websocket_wait_close(websocket_manager, 2000) == U_WEBSOCKET_STATUS_OPEN) {
    if (ulfius_websocket_send_message(websocket_manager, U_WEBSOCKET_OPCODE_TEXT, o_strlen("Message without fragmentation from server"), "Message without fragmentation from server") != U_OK) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Error send message without fragmentation");
    }
  }
  
  // Send text message with fragmentation for ulfius clients only, browsers seem to dislike fragmented messages
  if (o_strncmp(u_map_get(request->map_header, "User-Agent"), U_WEBSOCKET_USER_AGENT, o_strlen(U_WEBSOCKET_USER_AGENT)) == 0 &&
      ulfius_websocket_wait_close(websocket_manager, 2000) == U_WEBSOCKET_STATUS_OPEN) {
    if (ulfius_websocket_send_fragmented_message(websocket_manager, U_WEBSOCKET_OPCODE_TEXT, o_strlen("Message with fragmentation from server"), "Message with fragmentation from server", 5) != U_OK) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Error send message with fragmentation");
    }
  }
  
  // Send ping message
  if (ulfius_websocket_wait_close(websocket_manager, 2000) == U_WEBSOCKET_STATUS_OPEN) {
    if (ulfius_websocket_send_message(websocket_manager, U_WEBSOCKET_OPCODE_PING, 0, NULL) != U_OK) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Error send ping message");
    }
  }
  
  // Send binary message without fragmentation
  if (ulfius_websocket_wait_close(websocket_manager, 2000) == U_WEBSOCKET_STATUS_OPEN) {
    if (ulfius_websocket_send_message(websocket_manager, U_WEBSOCKET_OPCODE_BINARY, o_strlen("Message without fragmentation from server"), "Message without fragmentation from server") != U_OK) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Error send binary message without fragmentation");
    }
  }
  
  y_log_message(Y_LOG_LEVEL_DEBUG, "Closing websocket_manager_callback");
}

void websocket_manager_file_callback(const struct _u_request * request,
                               struct _websocket_manager * websocket_manager,
                               void * websocket_manager_user_data) {
  y_log_message(Y_LOG_LEVEL_DEBUG, "Opening websocket_manager_file_callback");
  for (;;) {
    sleep(1);
    if (websocket_manager == NULL || !websocket_manager->connected) {
      break;
    }
  }
  y_log_message(Y_LOG_LEVEL_DEBUG, "Closing websocket_manager_file_callback");
}

/**
 * websocket_incoming_message_callback
 * Read incoming message and prints it on the console
 */
void websocket_incoming_message_callback (const struct _u_request * request,
                                         struct _websocket_manager * websocket_manager,
                                         const struct _websocket_message * last_message,
                                         void * websocket_incoming_message_user_data) {
  if (websocket_incoming_message_user_data != NULL) {
    y_log_message(Y_LOG_LEVEL_DEBUG, "websocket_incoming_message_user_data is %s", websocket_incoming_message_user_data);
  }
  y_log_message(Y_LOG_LEVEL_DEBUG, "Incoming message, opcode: 0x%02x, mask: %d, len: %zu", last_message->opcode, last_message->has_mask, last_message->data_len);
  if (last_message->opcode == U_WEBSOCKET_OPCODE_TEXT) {
    y_log_message(Y_LOG_LEVEL_DEBUG, "text payload '%.*s'", last_message->data_len, last_message->data);
  } else if (last_message->opcode == U_WEBSOCKET_OPCODE_BINARY) {
    y_log_message(Y_LOG_LEVEL_DEBUG, "binary payload");
  }
}

void websocket_echo_message_callback (const struct _u_request * request,
                                         struct _websocket_manager * websocket_manager,
                                         const struct _websocket_message * last_message,
                                         void * websocket_incoming_message_user_data) {
  y_log_message(Y_LOG_LEVEL_DEBUG, "Incoming message, opcode: 0x%02x, mask: %d, len: %zu, text payload '%.*s'", last_message->opcode, last_message->has_mask, last_message->data_len, last_message->data_len, last_message->data);
  if (ulfius_websocket_send_message(websocket_manager, last_message->opcode, last_message->data_len, last_message->data) != U_OK) {
    y_log_message(Y_LOG_LEVEL_ERROR, "Error ulfius_websocket_send_message");
  }
}

void websocket_incoming_file_callback (const struct _u_request * request,
                                         struct _websocket_manager * websocket_manager,
                                         const struct _websocket_message * last_message,
                                         void * websocket_incoming_message_user_data) {
  char * my_message = msprintf("Incoming file %p, opcode: 0x%02x, mask: %d, len: %zu", last_message, last_message->opcode, last_message->has_mask, last_message->data_len);
  y_log_message(Y_LOG_LEVEL_DEBUG, my_message);
  ulfius_websocket_send_message(websocket_manager, U_WEBSOCKET_OPCODE_TEXT, o_strlen(my_message), my_message);
  o_free(my_message);
}

/**
 * Ulfius main callback function that simply calls the websocket manager and closes
 */
int callback_websocket (const struct _u_request * request, struct _u_response * response, void * user_data) {
  char * websocket_user_data = o_strdup("my_user_data");
  int ret;
  
  if ((ret = ulfius_set_websocket_response(response, NULL, NULL, &websocket_manager_callback, websocket_user_data, &websocket_incoming_message_callback, websocket_user_data, &websocket_onclose_callback, websocket_user_data)) == U_OK) {
    return U_CALLBACK_CONTINUE;
  } else {
    return U_CALLBACK_ERROR;
  }
}

int callback_websocket_echo (const struct _u_request * request, struct _u_response * response, void * user_data) {
  char * websocket_user_data = o_strdup("my_user_data");
  int ret;
  
  y_log_message(Y_LOG_LEVEL_DEBUG, "Client connected to echo websocket");
  if ((ret = ulfius_set_websocket_response(response, NULL, NULL, NULL, NULL, &websocket_echo_message_callback, websocket_user_data, &websocket_onclose_callback, websocket_user_data)) == U_OK) {
    return U_CALLBACK_CONTINUE;
  } else {
    return U_CALLBACK_ERROR;
  }
}

int callback_websocket_file (const struct _u_request * request, struct _u_response * response, void * user_data) {
  int ret;
  
  if ((ret = ulfius_set_websocket_response(response, NULL, NULL, &websocket_manager_file_callback, NULL, &websocket_incoming_file_callback, NULL, &websocket_onclose_file_callback, NULL)) == U_OK) {
    return U_CALLBACK_CONTINUE;
  } else {
    return U_CALLBACK_ERROR;
  }
}
#endif
