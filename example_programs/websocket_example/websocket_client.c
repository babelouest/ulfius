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
#include <ulfius.h>

#if defined(U_DISABLE_WEBSOCKET)
  #error Ulfius is not available with WebSockets support
#else

void websocket_manager_callback(const struct _u_request * request,
                               struct _websocket_manager * websocket_manager,
                               void * websocket_manager_user_data) {
  
  if (websocket_manager_user_data != NULL) {
    y_log_message(Y_LOG_LEVEL_DEBUG, "websocket_manager_user_data is %s", websocket_manager_user_data);
  }
  
  // Send text message without fragmentation
  if (ulfius_websocket_wait_close(websocket_manager, 2000) == U_WEBSOCKET_STATUS_OPEN) {
    if (ulfius_websocket_send_message(websocket_manager, U_WEBSOCKET_OPCODE_TEXT, o_strlen("Message without fragmentation from client"), "Message without fragmentation from client") != U_OK) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Error send message without fragmentation");
    }
  }
  
  // Send text message with fragmentation
  if (ulfius_websocket_wait_close(websocket_manager, 2000) == U_WEBSOCKET_STATUS_OPEN) {
    if (ulfius_websocket_send_fragmented_message(websocket_manager, U_WEBSOCKET_OPCODE_TEXT, o_strlen("Message with fragmentation from client"), "Message with fragmentation from client", 5) != U_OK) {
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
    if (ulfius_websocket_send_message(websocket_manager, U_WEBSOCKET_OPCODE_BINARY, o_strlen("Message without fragmentation from client"), "Message without fragmentation from client") != U_OK) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Error send binary message without fragmentation");
    }
  }
  
  y_log_message(Y_LOG_LEVEL_DEBUG, "Closing websocket_manager_callback");
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
  struct _u_response response;
  struct _websocket_client_handler websocket_client_handler;
  char * websocket_user_data = o_strdup("my user data");
  char * url = (argc>1&&0==o_strcmp("-https", argv[1]))?"wss://localhost:" PORT PREFIX_WEBSOCKET:"ws://localhost:" PORT PREFIX_WEBSOCKET;
  
  y_init_logs("websocket_client", Y_LOG_MODE_CONSOLE, Y_LOG_LEVEL_DEBUG, NULL, "Starting websocket_client");
  ulfius_init_request(&request);
  ulfius_init_response(&response);
  if (ulfius_set_websocket_request(&request, url, "protocol", "extension") == U_OK) {
    request.check_server_certificate = 0;
    if (ulfius_open_websocket_client_connection(&request, &websocket_manager_callback, websocket_user_data, &websocket_incoming_message_callback, websocket_user_data, &websocket_onclose_callback, websocket_user_data, &websocket_client_handler, &response) == U_OK) {
      y_log_message(Y_LOG_LEVEL_DEBUG, "Wait for user to press <enter> to close the program");
      getchar();
      ulfius_websocket_client_connection_close(&websocket_client_handler);
      y_log_message(Y_LOG_LEVEL_DEBUG, "Websocket closed");
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "Error ulfius_open_websocket_client_connection");
      o_free(websocket_user_data);
    }
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "Error ulfius_set_websocket_request");
    o_free(websocket_user_data);
  }
  
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  y_close_logs();
  return 0;
}
#endif
