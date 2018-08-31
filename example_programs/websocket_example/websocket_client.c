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

#define PORT "9275"
#define PREFIX_WEBSOCKET "/websocket"

#define U_DISABLE_JANSSON
#define U_DISABLE_CURL
#include "../../include/ulfius.h"

#if defined(U_DISABLE_WEBSOCKET)
  #error Ulfius is not available with WebSockets support
#else

void websocket_manager_callback(const struct _u_request * request,
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
  y_log_message(Y_LOG_LEVEL_DEBUG, "Incoming message, opcode: %x, mask: %d, len: %zu", last_message->opcode, last_message->has_mask, last_message->data_len);
  if (last_message->opcode == U_WEBSOCKET_OPCODE_TEXT) {
    y_log_message(Y_LOG_LEVEL_DEBUG, "text payload '%.*s'", last_message->data_len, last_message->data);
  } else if (last_message->opcode == U_WEBSOCKET_OPCODE_BINARY) {
    y_log_message(Y_LOG_LEVEL_DEBUG, "binary payload");
  }
}

void websocket_onclose_callback (const struct _u_request * request,
                                struct _websocket_manager * websocket_manager,
                                void * websocket_onclose_user_data) {
  if (websocket_onclose_user_data != NULL) {
    y_log_message(Y_LOG_LEVEL_DEBUG, "websocket_onclose_user_data is %s", websocket_onclose_user_data);
    o_free(websocket_onclose_user_data);
  }
}

int main(int argc, char ** argv) {
  struct _u_request request;
  char * websocket_user_data = o_strdup("plop");
  
  y_init_logs("websocket_client", Y_LOG_MODE_CONSOLE, Y_LOG_LEVEL_DEBUG, NULL, "Starting websocket_client");
  ulfius_init_request(&request);
  if (ulfius_init_websocket_request(&request, "http://localhost:" PORT PREFIX_WEBSOCKET, "protocol", "extension") == U_OK) {
    if (ulfius_open_websocket_client_connection(&request, &websocket_manager_callback, websocket_user_data, &websocket_incoming_message_callback, websocket_user_data, &websocket_onclose_callback, websocket_user_data) == U_OK) {
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "Error ulfius_open_websocket_client_connection");
    }
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "Error ulfius_init_websocket_request");
  }
  
  ulfius_clean_request(&request);
  y_close_logs();
  return 0;
}
#endif
