/* Public domain, no copyright. Use at your own risk. */

#include <check.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ulfius.h>
#include <zlib.h>
#include <curl/curl.h>

#define MIN(A, B) ((A)>(B)?(B):(A))

#define WEBSOCKET_URL "http://localhost:8378/websocket"
#define DEFAULT_PROTOCOL "proto"
#define DEFAULT_EXTENSION "ext"
#define DEFAULT_MESSAGE "message content with a few characters"
#define PORT_1 9275
#define PORT_2 9276
#define PORT_3 9277
#define PORT_4 9278
#define PORT_5 9279
#define PORT_6 9280
#define PORT_7 9281
#define PORT_8 9282
#define PORT_9 9283
#define PORT_10 9284
#define PORT_11 9285
#define PREFIX_WEBSOCKET "/websocket"
#define MESSAGE "HelloFrom"
#define MESSAGE_CLIENT "HelloFromClient"
#define MESSAGE_SERVER "HelloFromServer"
#define MESSAGE_EXT1 "Grut"
#define MESSAGE_EXT2 "Plop"
#define MESSAGE_EXT3 "Gnaa"
#define MESSAGE_EXT4 "Glop"
#define ORIGIN "http://client.ulfius.tld"
#define NO_ORIGIN "null"
#define _U_W_BUFF_LEN 256

#define U_W_FLAG_SERVER  0x00000001
#define U_W_FLAG_CONTEXT 0x00000010

#define UNUSED(x) (void)(x)

static pthread_mutex_t ws_lock;
static pthread_cond_t  ws_cond;

#ifndef U_DISABLE_WEBSOCKET

void websocket_manager_callback_empty (const struct _u_request * request, struct _websocket_manager * websocket_manager, void * websocket_manager_user_data) {
  UNUSED(request);
  UNUSED(websocket_manager);
  UNUSED(websocket_manager_user_data);
}

void websocket_incoming_message_callback_empty (const struct _u_request * request, struct _websocket_manager * websocket_manager, const struct _websocket_message * message, void * websocket_incoming_user_data) {
  UNUSED(request);
  UNUSED(websocket_manager);
  UNUSED(message);
  UNUSED(websocket_incoming_user_data);
}

void websocket_onclose_callback_empty (const struct _u_request * request, struct _websocket_manager * websocket_manager, void * websocket_onclose_user_data) {
  UNUSED(request);
  UNUSED(websocket_manager);
  UNUSED(websocket_onclose_user_data);
}

void websocket_onclose_callback_client (const struct _u_request * request, struct _websocket_manager * websocket_manager, void * websocket_onclose_user_data) {
  UNUSED(request);
  UNUSED(websocket_manager);
  ck_assert_ptr_ne(websocket_onclose_user_data, NULL);
}

void websocket_echo_message_callback (const struct _u_request * request,
                                       struct _websocket_manager * websocket_manager,
                                       const struct _websocket_message * last_message,
                                       void * websocket_incoming_message_user_data) {
  UNUSED(request);
  UNUSED(websocket_incoming_message_user_data);
  if (last_message->opcode == U_WEBSOCKET_OPCODE_TEXT) {
    ck_assert_int_eq(ulfius_websocket_send_fragmented_message(websocket_manager, U_WEBSOCKET_OPCODE_TEXT, last_message->data_len, last_message->data, last_message->fragment_len), U_OK);
  }
}

void websocket_origin_callback (const struct _u_request * request,
                                   struct _websocket_manager * websocket_manager,
                                   void * websocket_incoming_message_user_data) {
  UNUSED(request);
  if (websocket_incoming_message_user_data != NULL) {
    ulfius_websocket_send_message(websocket_manager, U_WEBSOCKET_OPCODE_TEXT, o_strlen((char *)websocket_incoming_message_user_data), (char *)websocket_incoming_message_user_data);
  } else {
    ulfius_websocket_send_message(websocket_manager, U_WEBSOCKET_OPCODE_TEXT, o_strlen(NO_ORIGIN), NO_ORIGIN);
  }
}

void websocket_onclose_message_callback (const struct _u_request * request, struct _websocket_manager * websocket_manager, void * websocket_onclose_user_data) {
  UNUSED(request);
  UNUSED(websocket_manager);
  o_free(websocket_onclose_user_data);
}

void websocket_manager_callback_client (const struct _u_request * request, struct _websocket_manager * websocket_manager, void * websocket_manager_user_data) {
  int i;
  UNUSED(request);
  UNUSED(websocket_manager_user_data);
  
  for (i=0; i<4; i++) {
    if (ulfius_websocket_wait_close(websocket_manager, 50) == U_WEBSOCKET_STATUS_OPEN) {
      if (i%2) {
        ck_assert_int_eq(ulfius_websocket_send_message(websocket_manager, U_WEBSOCKET_OPCODE_TEXT, o_strlen(DEFAULT_MESSAGE), DEFAULT_MESSAGE), U_OK);
      } else {
        ck_assert_int_eq(ulfius_websocket_send_fragmented_message(websocket_manager, U_WEBSOCKET_OPCODE_TEXT, o_strlen(DEFAULT_MESSAGE), DEFAULT_MESSAGE, (o_strlen(DEFAULT_MESSAGE)/4)), U_OK);
      }
    }
  }
}

void websocket_extension_callback_client (const struct _u_request * request, struct _websocket_manager * websocket_manager, void * websocket_manager_user_data) {
  UNUSED(request);
  UNUSED(websocket_manager_user_data);
  ck_assert_int_eq(ulfius_websocket_send_message(websocket_manager, U_WEBSOCKET_OPCODE_TEXT, o_strlen(MESSAGE_CLIENT), MESSAGE_CLIENT), U_OK);
  y_log_message(Y_LOG_LEVEL_DEBUG, "client message 1 '%s' sent", MESSAGE_CLIENT);
  ck_assert_int_eq(ulfius_websocket_send_message(websocket_manager, U_WEBSOCKET_OPCODE_TEXT, o_strlen(MESSAGE_CLIENT), MESSAGE_CLIENT), U_OK);
  y_log_message(Y_LOG_LEVEL_DEBUG, "client message 2 '%s' sent", MESSAGE_CLIENT);
  if (ulfius_websocket_wait_close(websocket_manager, 50) == U_WEBSOCKET_STATUS_OPEN) {
    ck_assert_int_eq(ulfius_websocket_send_close_signal(websocket_manager), U_OK);
  }
}

void websocket_incoming_message_callback_client (const struct _u_request * request, struct _websocket_manager * websocket_manager, const struct _websocket_message * message, void * websocket_incoming_user_data) {
  UNUSED(request);
  UNUSED(websocket_manager);
  UNUSED(websocket_incoming_user_data);
  ck_assert_int_eq(0, o_strncmp(message->data, DEFAULT_MESSAGE, message->data_len));
}

void websocket_incoming_json_valid_messages (const struct _u_request * request, struct _websocket_manager * websocket_manager, const struct _websocket_message * message, void * websocket_incoming_user_data) {
  json_t * j_message = ulfius_websocket_parse_json_message(message, NULL), * j_expected_message = json_pack("{ss}", "message", "Hello World!");
  UNUSED(request);
  UNUSED(websocket_manager);
  UNUSED(websocket_incoming_user_data);
  
  // Valid JSON
  ck_assert_ptr_ne(j_message, NULL);
  ck_assert_int_eq(1, json_equal(j_message, j_expected_message));
  ck_assert_int_ne(message->data_len, 0);

  json_decref(j_message);
  json_decref(j_expected_message);
}

void websocket_incoming_json_invalid_messages (const struct _u_request * request, struct _websocket_manager * websocket_manager, const struct _websocket_message * message, void * websocket_incoming_user_data) {
  json_t * j_message = ulfius_websocket_parse_json_message(message, NULL);
  UNUSED(request);
  UNUSED(websocket_manager);
  UNUSED(websocket_incoming_user_data);
  
  // Invalid JSON
  ck_assert_ptr_eq(j_message, NULL);
  ck_assert_int_ne(message->data_len, 0);

  json_decref(j_message);
}

void websocket_incoming_json_empty_messages (const struct _u_request * request, struct _websocket_manager * websocket_manager, const struct _websocket_message * message, void * websocket_incoming_user_data) {
  json_t * j_message = ulfius_websocket_parse_json_message(message, NULL);
  UNUSED(request);
  UNUSED(websocket_manager);
  UNUSED(websocket_incoming_user_data);
  
  // Empty payload
  ck_assert_ptr_eq(j_message, NULL);

  json_decref(j_message);
}

void websocket_manager_json_valid_callback(const struct _u_request * request, struct _websocket_manager * websocket_manager, void * websocket_manager_user_data) {
  json_t * j_message = json_pack("{ss}", "message", "Hello World!");
  ck_assert_ptr_ne(j_message, NULL);
  const char str_message[] = "{\"message\":\"Hello World!\"}";
  UNUSED(request);
  UNUSED(websocket_manager_user_data);
  
  ck_assert_int_eq(ulfius_websocket_send_json_message(websocket_manager, j_message), U_OK);
  ck_assert_int_eq(ulfius_websocket_send_message(websocket_manager, U_WEBSOCKET_OPCODE_TEXT, o_strlen(str_message), str_message), U_OK);
  ck_assert_int_eq(ulfius_websocket_send_message(websocket_manager, U_WEBSOCKET_OPCODE_BINARY, o_strlen(str_message), str_message), U_OK);

  json_decref(j_message);
}

void websocket_manager_json_invalid_callback(const struct _u_request * request, struct _websocket_manager * websocket_manager, void * websocket_manager_user_data) {
  UNUSED(request);
  UNUSED(websocket_manager_user_data);
  ck_assert_int_eq(ulfius_websocket_send_message(websocket_manager, U_WEBSOCKET_OPCODE_TEXT, o_strlen(DEFAULT_MESSAGE), DEFAULT_MESSAGE), U_OK);
  ck_assert_int_eq(ulfius_websocket_send_message(websocket_manager, U_WEBSOCKET_OPCODE_BINARY, o_strlen(DEFAULT_MESSAGE), DEFAULT_MESSAGE), U_OK);
}

void websocket_manager_json_empty_callback(const struct _u_request * request, struct _websocket_manager * websocket_manager, void * websocket_manager_user_data) {
  UNUSED(request);
  UNUSED(websocket_manager_user_data);
  ck_assert_int_eq(ulfius_websocket_send_json_message(websocket_manager, NULL), U_ERROR_PARAMS);
  ck_assert_int_eq(ulfius_websocket_send_message(websocket_manager, U_WEBSOCKET_OPCODE_TEXT, 0, NULL), U_OK);
  ck_assert_int_eq(ulfius_websocket_send_message(websocket_manager, U_WEBSOCKET_OPCODE_BINARY, 0, NULL), U_OK);
}

void websocket_incoming_message_without_origin_callback_client (const struct _u_request * request, struct _websocket_manager * websocket_manager, const struct _websocket_message * message, void * websocket_incoming_user_data) {
  UNUSED(request);
  UNUSED(websocket_manager);
  UNUSED(websocket_incoming_user_data);
  ck_assert_int_eq(0, o_strncmp(message->data, NO_ORIGIN, message->data_len));
}

void websocket_incoming_message_with_origin_callback_client (const struct _u_request * request, struct _websocket_manager * websocket_manager, const struct _websocket_message * message, void * websocket_incoming_user_data) {
  UNUSED(request);
  UNUSED(websocket_manager);
  UNUSED(websocket_incoming_user_data);
  ck_assert_int_eq(0, o_strncmp(message->data, ORIGIN, message->data_len));
}

void websocket_manager_extension_callback (const struct _u_request * request, struct _websocket_manager * websocket_manager, void * websocket_manager_user_data) {
  UNUSED(request);
  UNUSED(websocket_manager_user_data);
  y_log_message(Y_LOG_LEVEL_DEBUG, "server message '%s' sent", MESSAGE_SERVER);
  ck_assert_int_eq(ulfius_websocket_send_message(websocket_manager, U_WEBSOCKET_OPCODE_TEXT, o_strlen(MESSAGE_SERVER), MESSAGE_SERVER), U_OK);
  while (ulfius_websocket_wait_close(websocket_manager, 50) == U_WEBSOCKET_STATUS_OPEN);
}

void websocket_manager_extension_deflate_callback (const struct _u_request * request, struct _websocket_manager * websocket_manager, void * websocket_manager_user_data) {
  UNUSED(request);
  UNUSED(websocket_manager_user_data);
  ck_assert_int_eq(ulfius_websocket_send_message(websocket_manager, U_WEBSOCKET_OPCODE_TEXT, o_strlen(MESSAGE_SERVER), MESSAGE_SERVER), U_OK);
  y_log_message(Y_LOG_LEVEL_DEBUG, "server message 1 sent");
  ck_assert_int_eq(ulfius_websocket_send_message(websocket_manager, U_WEBSOCKET_OPCODE_TEXT, o_strlen(MESSAGE_SERVER), MESSAGE_SERVER), U_OK);
  y_log_message(Y_LOG_LEVEL_DEBUG, "server message 2 sent");
  while (ulfius_websocket_wait_close(websocket_manager, 50) == U_WEBSOCKET_STATUS_OPEN);
}

void websocket_incoming_extension_callback (const struct _u_request * request,
                                            struct _websocket_manager * websocket_manager,
                                            const struct _websocket_message * last_message,
                                            void * user_data) {
  int * rsv = (int *)user_data;
  UNUSED(request);
  UNUSED(websocket_manager);
  
  if (last_message->opcode == U_WEBSOCKET_OPCODE_TEXT || last_message->opcode == U_WEBSOCKET_OPCODE_BINARY) {
    ck_assert_ptr_ne(NULL, o_strnstr(last_message->data, MESSAGE, last_message->data_len));
    if ((*rsv)&U_WEBSOCKET_RSV1) {
      ck_assert_int_ne(last_message->rsv&U_WEBSOCKET_RSV1, 0);
      ck_assert_ptr_ne(NULL, o_strnstr(last_message->data, MESSAGE_EXT1, last_message->data_len));
    }
    if ((*rsv)&U_WEBSOCKET_RSV2) {
      ck_assert_int_ne(last_message->rsv&U_WEBSOCKET_RSV2, 0);
      ck_assert_ptr_ne(NULL, o_strnstr(last_message->data, MESSAGE_EXT2, last_message->data_len));
    }
    if ((*rsv)&U_WEBSOCKET_RSV3) {
      ck_assert_int_ne(last_message->rsv&U_WEBSOCKET_RSV3, 0);
      ck_assert_ptr_ne(NULL, o_strnstr(last_message->data, MESSAGE_EXT3, last_message->data_len));
    }
  }
}

void websocket_incoming_extension_deflate_client_callback (const struct _u_request * request,
                                                    struct _websocket_manager * websocket_manager,
                                                    const struct _websocket_message * last_message,
                                                    void * user_data) {
  UNUSED(request);
  UNUSED(websocket_manager);
  UNUSED(user_data);
  ck_assert_int_eq(0, o_strncmp(last_message->data, MESSAGE_SERVER, last_message->data_len));
  if (last_message->opcode == U_WEBSOCKET_OPCODE_TEXT || last_message->opcode == U_WEBSOCKET_OPCODE_BINARY) {
    ck_assert_int_ne(last_message->rsv&U_WEBSOCKET_RSV1, 0);
  }
}

void websocket_incoming_extension_deflate_client_callback_disabled (const struct _u_request * request,
                                                    struct _websocket_manager * websocket_manager,
                                                    const struct _websocket_message * last_message,
                                                    void * user_data) {
  UNUSED(request);
  UNUSED(websocket_manager);
  UNUSED(user_data);
  ck_assert_int_eq(0, o_strncmp(last_message->data, MESSAGE_SERVER, last_message->data_len));
  ck_assert_int_eq(last_message->rsv&U_WEBSOCKET_RSV1, 0);
}

void websocket_incoming_extension_deflate_server_callback (const struct _u_request * request,
                                                    struct _websocket_manager * websocket_manager,
                                                    const struct _websocket_message * last_message,
                                                    void * user_data) {
  UNUSED(request);
  UNUSED(websocket_manager);
  UNUSED(user_data);
  ck_assert_int_eq(0, o_strncmp(last_message->data, MESSAGE_CLIENT, last_message->data_len));
  if (last_message->opcode == U_WEBSOCKET_OPCODE_TEXT || last_message->opcode == U_WEBSOCKET_OPCODE_BINARY) {
    ck_assert_int_ne(last_message->rsv&U_WEBSOCKET_RSV1, 0);
  }
}

void websocket_incoming_extension_deflate_server_callback_disabled (const struct _u_request * request,
                                                    struct _websocket_manager * websocket_manager,
                                                    const struct _websocket_message * last_message,
                                                    void * user_data) {
  UNUSED(request);
  UNUSED(websocket_manager);
  UNUSED(user_data);
  ck_assert_int_eq(0, o_strncmp(last_message->data, MESSAGE_CLIENT, last_message->data_len));
  ck_assert_int_eq(last_message->rsv&U_WEBSOCKET_RSV1, 0);
}

#ifndef U_DISABLE_WS_MESSAGE_LIST
void websocket_manager_callback_keep_messages (const struct _u_request * request, struct _websocket_manager * websocket_manager, void * websocket_manager_user_data) {
  UNUSED(request);
  if (websocket_manager_user_data != NULL) {
    websocket_manager->keep_messages = *((int*)(websocket_manager_user_data));
  }
  ck_assert_int_eq(ulfius_websocket_send_message(websocket_manager, U_WEBSOCKET_OPCODE_TEXT, o_strlen(MESSAGE_EXT1), MESSAGE_EXT1), U_OK);
  
  while (ulfius_websocket_wait_close(websocket_manager, 50) == U_WEBSOCKET_STATUS_OPEN);
}

void websocket_incoming_keep_messages (const struct _u_request * request,
                                       struct _websocket_manager * websocket_manager,
                                       const struct _websocket_message * last_message,
                                       void * user_data) {
  UNUSED(request);
  UNUSED(user_data);
  if (0 == o_strncmp(MESSAGE_EXT1, last_message->data, last_message->data_len)) {
    ck_assert_int_eq(ulfius_websocket_send_message(websocket_manager, U_WEBSOCKET_OPCODE_TEXT, o_strlen(MESSAGE_EXT2), MESSAGE_EXT2), U_OK);
  } else if (0 == o_strncmp(MESSAGE_EXT2, last_message->data, last_message->data_len)) {
    ck_assert_int_eq(ulfius_websocket_send_message(websocket_manager, U_WEBSOCKET_OPCODE_TEXT, o_strlen(MESSAGE_EXT3), MESSAGE_EXT3), U_OK);
  } else if (0 == o_strncmp(MESSAGE_EXT3, last_message->data, last_message->data_len)) {
    ck_assert_int_eq(ulfius_websocket_send_message(websocket_manager, U_WEBSOCKET_OPCODE_TEXT, o_strlen(MESSAGE_EXT4), MESSAGE_EXT4), U_OK);
  } else if (0 == o_strncmp(MESSAGE_EXT4, last_message->data, last_message->data_len)) {
    ck_assert_int_eq(ulfius_websocket_send_close_signal(websocket_manager), U_OK);
  }
}

void websocket_onclose_callback_keep_messages (const struct _u_request * request, struct _websocket_manager * websocket_manager, void * websocket_onclose_user_data) {
  UNUSED(request);
  UNUSED(websocket_onclose_user_data);
  if (websocket_manager->keep_messages & U_WEBSOCKET_KEEP_INCOMING) {
    ck_assert_int_gt(websocket_manager->message_list_incoming->len, 0);
  } else {
    ck_assert_int_eq(websocket_manager->message_list_incoming->len, 0);
  }
  if (websocket_manager->keep_messages & U_WEBSOCKET_KEEP_OUTCOMING) {
    ck_assert_int_gt(websocket_manager->message_list_outcoming->len, 0);
  } else {
    ck_assert_int_eq(websocket_manager->message_list_outcoming->len, 0);
  }
}
#endif

void websocket_origin_onclose_callback (const struct _u_request * request, struct _websocket_manager * websocket_manager, void * websocket_onclose_user_data) {
  UNUSED(request);
  UNUSED(websocket_manager);
  o_free(websocket_onclose_user_data);
}

int websocket_extension1_message_out_perform(const uint8_t opcode, const uint64_t data_len_in, const char * data_in, uint64_t * data_len_out, char ** data_out, const uint64_t fragment_len, void * user_data, void * context) {
  UNUSED(opcode);
  UNUSED(fragment_len);
  UNUSED(user_data);
  UNUSED(context);
  ck_assert_int_eq(opcode, U_WEBSOCKET_OPCODE_TEXT);
  if (((uintptr_t)user_data)&U_W_FLAG_CONTEXT) {
    ck_assert_ptr_ne(context, NULL);
  }
  *data_out = msprintf("%.*s%s", (int)data_len_in, data_in, MESSAGE_EXT1);
  ck_assert_ptr_ne(*data_out, NULL);
  *data_len_out = o_strlen(*data_out);
  y_log_message(Y_LOG_LEVEL_DEBUG, "inside websocket_extension1_message_out_perform");
  return U_OK;
}

int websocket_extension1_message_in_perform(const uint8_t opcode, const uint64_t data_len_in, const char * data_in, uint64_t * data_len_out, char ** data_out, const uint64_t fragment_len, void * user_data, void * context) {
  UNUSED(opcode);
  UNUSED(fragment_len);
  UNUSED(user_data);
  UNUSED(context);
  ck_assert_int_eq(opcode, U_WEBSOCKET_OPCODE_TEXT);
  if (((uintptr_t)user_data)&U_W_FLAG_CONTEXT) {
    ck_assert_ptr_ne(context, NULL);
  }
  *data_out = msprintf("%.*s%s", (int)data_len_in, data_in, MESSAGE_EXT1);
  *data_len_out = o_strlen(*data_out);
  y_log_message(Y_LOG_LEVEL_DEBUG, "inside websocket_extension1_message_in_perform: '%.*s'", (int)data_len_in, data_in);
  return U_OK;
}

int websocket_extension2_message_out_perform(const uint8_t opcode, const uint64_t data_len_in, const char * data_in, uint64_t * data_len_out, char ** data_out, const uint64_t fragment_len, void * user_data, void * context) {
  UNUSED(opcode);
  UNUSED(fragment_len);
  UNUSED(user_data);
  UNUSED(context);
  ck_assert_int_eq(opcode, U_WEBSOCKET_OPCODE_TEXT);
  if (((uintptr_t)user_data)&U_W_FLAG_CONTEXT) {
    ck_assert_ptr_ne(context, NULL);
  }
  *data_out = msprintf("%.*s%s", (int)data_len_in, data_in, MESSAGE_EXT2);
  *data_len_out = o_strlen(*data_out);
  y_log_message(Y_LOG_LEVEL_DEBUG, "inside websocket_extension2_message_out_perform");
  return U_OK;
}

int websocket_extension2_message_in_perform(const uint8_t opcode, const uint64_t data_len_in, const char * data_in, uint64_t * data_len_out, char ** data_out, const uint64_t fragment_len, void * user_data, void * context) {
  UNUSED(opcode);
  UNUSED(fragment_len);
  UNUSED(user_data);
  UNUSED(context);
  ck_assert_int_eq(opcode, U_WEBSOCKET_OPCODE_TEXT);
  if (((uintptr_t)user_data)&U_W_FLAG_CONTEXT) {
    ck_assert_ptr_ne(context, NULL);
  }
  *data_out = msprintf("%.*s%s", (int)data_len_in, data_in, MESSAGE_EXT2);
  *data_len_out = o_strlen(*data_out);
  y_log_message(Y_LOG_LEVEL_DEBUG, "inside websocket_extension2_message_in_perform");
  return U_OK;
}

int websocket_extension3_message_out_perform(const uint8_t opcode, const uint64_t data_len_in, const char * data_in, uint64_t * data_len_out, char ** data_out, const uint64_t fragment_len, void * user_data, void * context) {
  UNUSED(opcode);
  UNUSED(fragment_len);
  UNUSED(user_data);
  UNUSED(context);
  if (o_strlen(MESSAGE_SERVER) == data_len_in) {
    ck_assert_int_ne(0, memcmp(MESSAGE_SERVER, data_in, MIN(data_len_in, o_strlen(MESSAGE_SERVER))));
  }
  *data_out = o_malloc(data_len_in);
  memcpy(*data_out, data_in, data_len_in);
  *data_len_out = data_len_in;
  return U_OK;
}

int websocket_extension3_message_in_perform(const uint8_t opcode, const uint64_t data_len_in, const char * data_in, uint64_t * data_len_out, char ** data_out, const uint64_t fragment_len, void * user_data, void * context) {
  UNUSED(opcode);
  UNUSED(fragment_len);
  UNUSED(user_data);
  UNUSED(context);
  if (o_strlen(MESSAGE_SERVER) == data_len_in) {
    ck_assert_int_ne(0, memcmp(MESSAGE_SERVER, data_in, MIN(data_len_in, o_strlen(MESSAGE_SERVER))));
  }
  *data_out = o_malloc(data_len_in);
  memcpy(*data_out, data_in, data_len_in);
  *data_len_out = data_len_in;
  return U_OK;
}

int websocket_extension4_message_out_perform(const uint8_t opcode, const uint64_t data_len_in, const char * data_in, uint64_t * data_len_out, char ** data_out, const uint64_t fragment_len, void * user_data, void * context) {
  UNUSED(opcode);
  UNUSED(fragment_len);
  UNUSED(user_data);
  UNUSED(context);
  ck_assert_int_eq(o_strlen(MESSAGE_SERVER), data_len_in);
  ck_assert_int_eq(0, memcmp(MESSAGE_SERVER, data_in, MIN(data_len_in, o_strlen(MESSAGE_SERVER))));
  *data_out = o_malloc(data_len_in);
  memcpy(*data_out, data_in, data_len_in);
  *data_len_out = data_len_in;
  return U_OK;
}

int websocket_extension4_message_in_perform(const uint8_t opcode, const uint64_t data_len_in, const char * data_in, uint64_t * data_len_out, char ** data_out, const uint64_t fragment_len, void * user_data, void * context) {
  UNUSED(opcode);
  UNUSED(fragment_len);
  UNUSED(user_data);
  UNUSED(context);
  ck_assert_int_eq(o_strlen(MESSAGE_SERVER), data_len_in);
  ck_assert_int_eq(0, memcmp(MESSAGE_SERVER, data_in, MIN(data_len_in, o_strlen(MESSAGE_SERVER))));
  *data_out = o_malloc(data_len_in);
  memcpy(*data_out, data_in, data_len_in);
  *data_len_out = data_len_in;
  return U_OK;
}

int callback_websocket (const struct _u_request * request, struct _u_response * response, void * user_data) {
  int ret;
  char * websocket_allocated_data = o_strdup("grut");
  UNUSED(request);
  UNUSED(user_data);
  
  ret = ulfius_set_websocket_response(response, NULL, NULL, NULL, NULL, &websocket_echo_message_callback, websocket_allocated_data, &websocket_onclose_message_callback, websocket_allocated_data);
  ck_assert_int_eq(ret, U_OK);
  return (ret == U_OK)?U_CALLBACK_CONTINUE:U_CALLBACK_ERROR;
}

int callback_websocket_with_origin (const struct _u_request * request, struct _u_response * response, void * user_data) {
  int ret;
  char * origin = o_strdup(u_map_get_case(request->map_header, "Origin"));
  UNUSED(user_data);

  if (u_map_get_case(request->map_header, "Origin") != NULL) {
    ulfius_set_string_body_response(response, 400, u_map_get_case(request->map_header, "Origin"));
  } else {
    response->status = 400;
  }
  ret = ulfius_set_websocket_response(response, NULL, NULL, &websocket_origin_callback, origin, NULL, NULL, &websocket_origin_onclose_callback, origin);
  ck_assert_int_eq(ret, U_OK);

  return (ret == U_OK)?U_CALLBACK_CONTINUE:U_CALLBACK_ERROR;
}

int callback_websocket_json_valid_messages (const struct _u_request * request, struct _u_response * response, void * user_data) {
  int ret;
  UNUSED(request);
  UNUSED(user_data);
  
  ret = ulfius_set_websocket_response(response, NULL, NULL, NULL, NULL, websocket_incoming_json_valid_messages, user_data, NULL, NULL);
  ck_assert_int_eq(ret, U_OK);
  return (ret == U_OK)?U_CALLBACK_CONTINUE:U_CALLBACK_ERROR;
}

int callback_websocket_json_invalid_messages (const struct _u_request * request, struct _u_response * response, void * user_data) {
  int ret;
  UNUSED(request);
  UNUSED(user_data);
  
  ret = ulfius_set_websocket_response(response, NULL, NULL, NULL, NULL, websocket_incoming_json_invalid_messages, user_data, NULL, NULL);
  ck_assert_int_eq(ret, U_OK);
  return (ret == U_OK)?U_CALLBACK_CONTINUE:U_CALLBACK_ERROR;
}

int callback_websocket_json_empty_messages (const struct _u_request * request, struct _u_response * response, void * user_data) {
  int ret;
  UNUSED(request);
  
  ret = ulfius_set_websocket_response(response, NULL, NULL, NULL, NULL, websocket_incoming_json_empty_messages, user_data, NULL, NULL);
  ck_assert_int_eq(ret, U_OK);
  return (ret == U_OK)?U_CALLBACK_CONTINUE:U_CALLBACK_ERROR;
}

#ifndef U_DISABLE_WS_MESSAGE_LIST
int callback_websocket_keep_messages (const struct _u_request * request, struct _u_response * response, void * user_data) {
  int ret;
  UNUSED(request);
  
  ret = ulfius_set_websocket_response(response, NULL, NULL, &websocket_manager_callback_keep_messages, user_data, websocket_incoming_keep_messages, NULL, &websocket_onclose_callback_keep_messages, user_data);
  ck_assert_int_eq(ret, U_OK);
  return (ret == U_OK)?U_CALLBACK_CONTINUE:U_CALLBACK_ERROR;
}
#endif

int callback_websocket_onclose (const struct _u_request * request, struct _u_response * response, void * user_data) {
  int ret;
  UNUSED(request);
  UNUSED(user_data);
  
  ret = ulfius_set_websocket_response(response, NULL, NULL, NULL, NULL, &websocket_echo_message_callback, NULL, NULL, NULL);
  ck_assert_int_eq(ret, U_OK);
  return (ret == U_OK)?U_CALLBACK_CONTINUE:U_CALLBACK_ERROR;
}

int callback_websocket_extension_no_match (const struct _u_request * request, struct _u_response * response, void * user_data) {
  UNUSED(request);
  ck_assert_int_eq(ulfius_set_websocket_response(response, NULL, NULL, &websocket_manager_extension_callback, NULL, &websocket_incoming_extension_callback, user_data, NULL, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_websocket_extension_message_perform(response, MESSAGE_EXT1, U_WEBSOCKET_RSV1, &websocket_extension1_message_out_perform, (void *)U_W_FLAG_SERVER, &websocket_extension1_message_in_perform, (void *)U_W_FLAG_SERVER, NULL, NULL, NULL, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_websocket_extension_message_perform(response, MESSAGE_EXT2, U_WEBSOCKET_RSV2, &websocket_extension2_message_out_perform, (void *)U_W_FLAG_SERVER, &websocket_extension2_message_in_perform, (void *)U_W_FLAG_SERVER, NULL, NULL, NULL, NULL), U_OK);
  return U_CALLBACK_CONTINUE;
}

void websocket_extension_match_free_context(void * user_data, void * context) {
  UNUSED(user_data);
  y_log_message(Y_LOG_LEVEL_DEBUG, "inside websocket_extension_match_free_context");
  ck_assert_ptr_ne(context, NULL);
  o_free(context);
}

int websocket_extension_server_match_ext1(const char * extension_server, const char ** extension_client_list, char ** extension_client, void * user_data, void ** context) {
  UNUSED(extension_client_list);
  UNUSED(user_data);
  y_log_message(Y_LOG_LEVEL_DEBUG, "check match server extension "MESSAGE_EXT1" with extension '%s'", extension_server);
  if (0 == o_strncmp(extension_server, MESSAGE_EXT1, o_strlen(MESSAGE_EXT1))) {
    *extension_client = o_strdup(MESSAGE_EXT1);
    *context = o_strdup("context for " MESSAGE_EXT1);
    return U_OK;
  } else {
    return U_ERROR;
  }
}

int websocket_extension_server_match_ext2(const char * extension_server, const char ** extension_client_list, char ** extension_client, void * user_data, void ** context) {
  UNUSED(extension_client_list);
  UNUSED(user_data);
  y_log_message(Y_LOG_LEVEL_DEBUG, "check match server extension "MESSAGE_EXT2" with extension '%s'", extension_server);
  if (0 == o_strncmp(extension_server, MESSAGE_EXT2, o_strlen(MESSAGE_EXT2))) {
    *extension_client = o_strdup(MESSAGE_EXT2);
    *context = o_strdup("context for " MESSAGE_EXT1);
    return U_OK;
  } else {
    return U_ERROR;
  }
}

int websocket_extension_client_match_ext1(const char * extension_server, void * user_data, void ** context) {
  UNUSED(user_data);
  y_log_message(Y_LOG_LEVEL_DEBUG, "check match client extension "MESSAGE_EXT1" with extension '%s'", extension_server);
  if (0 == o_strncmp(extension_server, MESSAGE_EXT1, o_strlen(MESSAGE_EXT1))) {
    *context = o_strdup("context for " MESSAGE_EXT1);
    return U_OK;
  } else {
    return U_ERROR;
  }
}

int websocket_extension_client_match_ext2(const char * extension_server, void * user_data, void ** context) {
  UNUSED(user_data);
  y_log_message(Y_LOG_LEVEL_DEBUG, "check match client extension "MESSAGE_EXT2" with extension '%s'", extension_server);
  if (0 == o_strncmp(extension_server, MESSAGE_EXT2, o_strlen(MESSAGE_EXT2))) {
    *context = o_strdup("context for " MESSAGE_EXT2);
    return U_OK;
  } else {
    return U_ERROR;
  }
}

int callback_websocket_extension_match (const struct _u_request * request, struct _u_response * response, void * user_data) {
  UNUSED(request);
  ck_assert_int_eq(ulfius_set_websocket_response(response, NULL, NULL, &websocket_manager_extension_callback, NULL, &websocket_incoming_extension_callback, user_data, NULL, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_websocket_extension_message_perform(response, MESSAGE_EXT1, U_WEBSOCKET_RSV1, &websocket_extension1_message_out_perform, (void *)(U_W_FLAG_SERVER|U_W_FLAG_CONTEXT), &websocket_extension1_message_in_perform, (void *)(U_W_FLAG_SERVER|U_W_FLAG_CONTEXT), &websocket_extension_server_match_ext1, NULL, &websocket_extension_match_free_context, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_websocket_extension_message_perform(response, MESSAGE_EXT2, U_WEBSOCKET_RSV2, &websocket_extension2_message_out_perform, (void *)(U_W_FLAG_SERVER|U_W_FLAG_CONTEXT), &websocket_extension2_message_in_perform, (void *)(U_W_FLAG_SERVER|U_W_FLAG_CONTEXT), &websocket_extension_server_match_ext2, NULL, &websocket_extension_match_free_context, NULL), U_OK);
  return U_CALLBACK_CONTINUE;
}

int websocket_extension_message_out_deflate_test(const uint8_t opcode,
                                                uint8_t * rsv,
                                                const uint64_t data_len_in,
                                                const char * data_in,
                                                uint64_t * data_len_out,
                                                char ** data_out,
                                                const uint64_t fragment_len,
                                                void * user_data,
                                                void * context) {
  struct _websocket_deflate_context * deflate_context = (struct _websocket_deflate_context *)context;
  int ret;
  (void)opcode;
  (void)fragment_len;
  (void)user_data;
  
  if (data_len_in) {
    if (deflate_context != NULL) {
      *data_out = NULL;
      *data_len_out = 0;
      
      deflate_context->defstream.avail_in = (uInt)data_len_in;
      deflate_context->defstream.next_in = (Bytef *)data_in;
      
      ret = U_OK;
      do {
        if ((*data_out = o_realloc(*data_out, (*data_len_out)+_U_W_BUFF_LEN)) != NULL) {
          deflate_context->defstream.avail_out = _U_W_BUFF_LEN;
          deflate_context->defstream.next_out = ((Bytef *)*data_out)+(*data_len_out);
          int res;
          switch ((res = deflate(&deflate_context->defstream, deflate_context->deflate_mask))) {
            case Z_OK:
            case Z_STREAM_END:
            case Z_BUF_ERROR:
              break;
            default:
              y_log_message(Y_LOG_LEVEL_ERROR, "websocket_extension_message_out_deflate - Error deflate");
              ret = U_ERROR;
              break;
          }
          (*data_len_out) += _U_W_BUFF_LEN - deflate_context->defstream.avail_out;
        } else {
          y_log_message(Y_LOG_LEVEL_ERROR, "websocket_extension_message_out_deflate - Error allocating resources for data_in_suffix");
          ret = U_ERROR;
        }
      } while (U_OK == ret && deflate_context->defstream.avail_out == 0);
      
      // https://github.com/madler/zlib/issues/149
      if (U_OK == ret && Z_BLOCK == deflate_context->deflate_mask) {
        if ((*data_out = o_realloc(*data_out, (*data_len_out)+_U_W_BUFF_LEN)) != NULL) {
          deflate_context->defstream.avail_out = _U_W_BUFF_LEN;
          deflate_context->defstream.next_out = ((Bytef *)*data_out)+(*data_len_out);
          switch (deflate(&deflate_context->defstream, Z_FULL_FLUSH)) {
            case Z_OK:
            case Z_STREAM_END:
            case Z_BUF_ERROR:
              break;
            default:
              y_log_message(Y_LOG_LEVEL_ERROR, "websocket_extension_message_out_deflate - Error inflate (2)");
              ret = U_ERROR;
              break;
          }
          (*data_len_out) += _U_W_BUFF_LEN - deflate_context->defstream.avail_out;
        } else {
          y_log_message(Y_LOG_LEVEL_ERROR, "websocket_extension_message_out_deflate - Error allocating resources for data_in_suffix (2)");
          ret = U_ERROR;
        }
      }

      if (U_OK != ret) {
        o_free(*data_out);
        *data_out = NULL;
        *data_len_out = 0;
      } else {
        if ((*(unsigned char **)data_out)[*data_len_out-1] == 0xff && (*(unsigned char **)data_out)[*data_len_out-2] == 0xff && (*(unsigned char **)data_out)[*data_len_out-3] == 0x00 && (*(unsigned char **)data_out)[*data_len_out-4] == 0x00) {
          *data_len_out -= 4;
        } else {
          (*(unsigned char **)data_out)[*data_len_out] = '\0';
          (*data_len_out)++;
        }
        *rsv |= U_WEBSOCKET_RSV1;
      }
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "websocket_extension_message_out_deflate - Error context is NULL");
      ret = U_ERROR;
    }
  } else {
    *data_len_out = 0;
    ret = U_OK;
  }
  ck_assert_int_eq(U_OK, ret);
  return ret;
}

int websocket_extension_message_in_inflate_test(const uint8_t opcode,
                                               uint8_t rsv,
                                               const uint64_t data_len_in,
                                               const char * data_in,
                                               uint64_t * data_len_out,
                                               char ** data_out,
                                               const uint64_t fragment_len,
                                               void * user_data,
                                               void * context) {
  struct _websocket_deflate_context * deflate_context = (struct _websocket_deflate_context *)context;
  unsigned char * data_in_suffix;
  unsigned char suffix[4] = {0x00, 0x00, 0xff, 0xff};
  int ret;
  (void)opcode;
  (void)fragment_len;
  (void)user_data;

  if (data_len_in && rsv&U_WEBSOCKET_RSV1) {
    if (deflate_context != NULL) {
      *data_out = NULL;
      *data_len_out = 0;
      if ((data_in_suffix = o_malloc(data_len_in+4)) != NULL) {
        memcpy(data_in_suffix, data_in, data_len_in);
        memcpy(data_in_suffix+data_len_in, suffix, 4);
        
        deflate_context->infstream.avail_in = (uInt)data_len_in+4;
        deflate_context->infstream.next_in = (Bytef *)data_in_suffix;
        
        ret = U_OK;
        do {
          if ((*data_out = o_realloc(*data_out, (*data_len_out)+_U_W_BUFF_LEN)) != NULL) {
            deflate_context->infstream.avail_out = _U_W_BUFF_LEN;
            deflate_context->infstream.next_out = ((Bytef *)*data_out)+(*data_len_out);
            switch (inflate(&deflate_context->infstream, deflate_context->inflate_mask)) {
              case Z_OK:
              case Z_STREAM_END:
              case Z_BUF_ERROR:
                break;
              default:
                y_log_message(Y_LOG_LEVEL_ERROR, "websocket_extension_message_in_inflate - Error inflate");
                ret = U_ERROR;
                break;
            }
            (*data_len_out) += _U_W_BUFF_LEN - deflate_context->infstream.avail_out;
          } else {
            y_log_message(Y_LOG_LEVEL_ERROR, "websocket_extension_message_in_inflate - Error allocating resources for data_in_suffix");
            ret = U_ERROR;
          }
        } while (U_OK == ret && deflate_context->infstream.avail_out == 0);
        
        o_free(data_in_suffix);
        if (U_OK != ret) {
          o_free(*data_out);
          *data_out = NULL;
          *data_len_out = 0;
        }
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "websocket_extension_message_in_inflate - Error allocating resources for data_in_suffix");
        ret = U_ERROR;
      }
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "websocket_extension_message_in_inflate - Error context is NULL");
      ret = U_ERROR;
    }
  } else {
    // If RSV1 flag isn't set, copy data as is (as seen in firefox)
    *data_len_out = data_len_in;
    *data_out = o_malloc(data_len_in);
    if (*data_out != NULL) {
      memcpy(*data_out, data_in, data_len_in);
      ret = U_OK;
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "websocket_extension_message_in_inflate - Error allocating resources for data_out");
      ret = U_ERROR;
    }
  }
  ck_assert_int_eq(U_OK, ret);
  return ret;
}

int callback_websocket_extension_deflate (const struct _u_request * request, struct _u_response * response, void * user_data) {
  UNUSED(request);
  UNUSED(user_data);
  ck_assert_int_eq(ulfius_set_websocket_response(response, NULL, NULL, &websocket_manager_extension_deflate_callback, NULL, &websocket_incoming_extension_deflate_server_callback, NULL, NULL, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_websocket_deflate_extension(response), U_OK);
  ck_assert_int_eq(ulfius_add_websocket_extension_message_perform(response, MESSAGE_EXT3, U_WEBSOCKET_RSV3, websocket_extension3_message_out_perform, NULL, websocket_extension3_message_in_perform, NULL, NULL, NULL, NULL, NULL), U_OK);
  return U_CALLBACK_CONTINUE;
}

int callback_websocket_extension_deflate_disabled (const struct _u_request * request, struct _u_response * response, void * user_data) {
  UNUSED(request);
  UNUSED(user_data);
  ck_assert_int_eq(ulfius_set_websocket_response(response, NULL, NULL, &websocket_manager_extension_deflate_callback, NULL, &websocket_incoming_extension_deflate_server_callback_disabled, NULL, NULL, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_websocket_deflate_extension(response), U_OK);
  ck_assert_int_eq(ulfius_add_websocket_extension_message_perform(response, MESSAGE_EXT4, U_WEBSOCKET_RSV3, websocket_extension4_message_out_perform, NULL, websocket_extension4_message_in_perform, NULL, NULL, NULL, NULL, NULL), U_OK);
  return U_CALLBACK_CONTINUE;
}

START_TEST(test_ulfius_websocket_set_websocket_response)
{
  struct _u_response response;
  ulfius_init_response(&response);
  
  ck_assert_int_eq(ulfius_set_websocket_response(NULL, DEFAULT_PROTOCOL, DEFAULT_EXTENSION, &websocket_manager_callback_empty, NULL, NULL, NULL, NULL, NULL), U_ERROR_PARAMS);
  ck_assert_int_eq(ulfius_set_websocket_response(&response, DEFAULT_PROTOCOL, DEFAULT_EXTENSION, NULL, NULL, NULL, NULL, NULL, NULL), U_ERROR_PARAMS);
  ck_assert_int_eq(ulfius_set_websocket_response(&response, DEFAULT_PROTOCOL, DEFAULT_EXTENSION, NULL, NULL, NULL, NULL, &websocket_onclose_callback_empty, NULL), U_ERROR_PARAMS);
  ck_assert_int_eq(ulfius_set_websocket_response(&response, DEFAULT_PROTOCOL, DEFAULT_EXTENSION, &websocket_manager_callback_empty, NULL, NULL, NULL, NULL, NULL), U_OK);
  ck_assert_int_eq(ulfius_set_websocket_response(&response, DEFAULT_PROTOCOL, DEFAULT_EXTENSION, NULL, NULL, &websocket_incoming_message_callback_empty, NULL, NULL, NULL), U_OK);
  
  ulfius_clean_response(&response);
}
END_TEST

START_TEST(test_ulfius_websocket_set_websocket_request)
{
  struct _u_request request;
  ulfius_init_request(&request);
  
  ck_assert_int_eq(ulfius_set_websocket_request(NULL, WEBSOCKET_URL, DEFAULT_PROTOCOL, DEFAULT_EXTENSION), U_ERROR);
  ck_assert_int_eq(ulfius_set_websocket_request(&request, NULL, DEFAULT_PROTOCOL, DEFAULT_EXTENSION), U_ERROR);
  ck_assert_int_eq(ulfius_set_websocket_request(&request, WEBSOCKET_URL, DEFAULT_PROTOCOL, DEFAULT_EXTENSION), U_OK);
  ck_assert_int_eq(ulfius_set_websocket_request(&request, WEBSOCKET_URL, NULL, DEFAULT_EXTENSION), U_OK);
  ck_assert_int_eq(ulfius_set_websocket_request(&request, WEBSOCKET_URL, DEFAULT_PROTOCOL, NULL), U_OK);
  ck_assert_int_eq(ulfius_set_websocket_request(&request, WEBSOCKET_URL, NULL, NULL), U_OK);
  
  ulfius_clean_request(&request);
}
END_TEST

START_TEST(test_ulfius_websocket_open_websocket_client_connection_error)
{
  struct _u_request request;
  struct _u_response response;
  struct _websocket_client_handler websocket_client_handler;
  ulfius_init_request(&request);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_set_websocket_request(&request, WEBSOCKET_URL, DEFAULT_PROTOCOL, DEFAULT_EXTENSION), U_OK);
  
  ck_assert_int_eq(ulfius_open_websocket_client_connection(NULL, &websocket_manager_callback_empty, NULL, NULL, NULL, NULL, NULL, &websocket_client_handler, &response), U_ERROR_PARAMS);
  ck_assert_int_eq(ulfius_open_websocket_client_connection(&request, NULL, NULL, NULL, NULL, NULL, NULL, &websocket_client_handler, &response), U_ERROR_PARAMS);
  ck_assert_int_eq(ulfius_open_websocket_client_connection(&request, NULL, NULL, NULL, NULL, &websocket_onclose_callback_empty, NULL, &websocket_client_handler, &response), U_ERROR_PARAMS);
  
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
}
END_TEST

START_TEST(test_ulfius_websocket_client)
{
  struct _u_instance instance;
  struct _u_request request;
  struct _u_response response;
  struct _websocket_client_handler websocket_client_handler = {NULL, NULL};
  char url[64], * allocated_data = o_strdup("plop");

  ck_assert_int_eq(ulfius_init_instance(&instance, PORT_1, NULL, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&instance, "GET", PREFIX_WEBSOCKET, NULL, 0, &callback_websocket, allocated_data), U_OK);
  ck_assert_int_eq(ulfius_start_framework(&instance), U_OK);

  ulfius_init_request(&request);
  ulfius_init_response(&response);
  sprintf(url, "ws://localhost:%d/%s", PORT_1, PREFIX_WEBSOCKET);
  
  // Test correct websocket connection on correct websocket service
  ck_assert_int_eq(ulfius_set_websocket_request(&request, url, DEFAULT_PROTOCOL, DEFAULT_EXTENSION), U_OK);
  ck_assert_int_eq(ulfius_open_websocket_client_connection(&request, &websocket_manager_callback_client, NULL, &websocket_incoming_message_callback_client, NULL, websocket_onclose_callback_client, allocated_data, &websocket_client_handler, &response), U_OK);
  ck_assert_int_eq(ulfius_websocket_client_connection_wait_close(&websocket_client_handler, 0), U_WEBSOCKET_STATUS_CLOSE);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
#if LIBCURL_VERSION_NUM < 0x075600
  // Test incorrect websocket connection on correct websocket service for libcurl < 7.86.0
  ulfius_init_request(&request);
  ulfius_init_response(&response);
  request.http_verb = o_strdup("GET");
  request.http_url = o_strdup(url);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_ERROR_LIBCURL); // On a websocket connection attempt, libcurl return 'Unsupported protocol'
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
#endif
  
  // Test incorrect websocket connection on correct websocket service
  ulfius_init_request(&request);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_set_websocket_request(&request, url, DEFAULT_PROTOCOL, DEFAULT_EXTENSION), U_OK);
  ck_assert_int_eq(ulfius_open_websocket_client_connection(&request, &websocket_manager_callback_client, NULL, &websocket_incoming_message_callback_client, NULL, websocket_onclose_callback_client, allocated_data, &websocket_client_handler, &response), U_OK);
  ck_assert_int_eq(ulfius_websocket_client_connection_wait_close(&websocket_client_handler, 0), U_WEBSOCKET_STATUS_CLOSE);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  usleep(50);
  ck_assert_int_eq(ulfius_stop_framework(&instance), U_OK);

  ulfius_clean_instance(&instance);
  o_free(allocated_data);
}
END_TEST

START_TEST(test_ulfius_websocket_client_no_onclose)
{
  struct _u_instance instance;
  struct _u_request request;
  struct _u_response response;
  struct _websocket_client_handler websocket_client_handler = {NULL, NULL};
  char url[64];
  ck_assert_int_eq(ulfius_init_instance(&instance, PORT_2, NULL, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&instance, "GET", PREFIX_WEBSOCKET, NULL, 0, &callback_websocket_onclose, NULL), U_OK);
  ck_assert_int_eq(ulfius_start_framework(&instance), U_OK);

  ulfius_init_request(&request);
  ulfius_init_response(&response);
  sprintf(url, "ws://localhost:%d/%s", PORT_2, PREFIX_WEBSOCKET);
  
  // Test correct websocket connection on correct websocket service
  ck_assert_int_eq(ulfius_set_websocket_request(&request, url, DEFAULT_PROTOCOL, DEFAULT_EXTENSION), U_OK);
  ck_assert_int_eq(ulfius_open_websocket_client_connection(&request, &websocket_manager_callback_client, NULL, &websocket_incoming_message_callback_client, NULL, NULL, NULL, &websocket_client_handler, &response), U_OK);
  ck_assert_int_eq(ulfius_websocket_client_connection_wait_close(&websocket_client_handler, 0), U_WEBSOCKET_STATUS_CLOSE);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
#if LIBCURL_VERSION_NUM < 0x075600
  // Test incorrect websocket connection on correct websocket service for libcurl < 7.86.0
  ulfius_init_request(&request);
  ulfius_init_response(&response);
  request.http_verb = o_strdup("GET");
  request.http_url = o_strdup(url);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_ERROR_LIBCURL); // On a websocket connection attempt, libcurl return 'Unsupported protocol'
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
#endif
  
  // Test incorrect websocket connection on correct websocket service
  ulfius_init_request(&request);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_set_websocket_request(&request, url, DEFAULT_PROTOCOL, DEFAULT_EXTENSION), U_OK);
  ck_assert_int_eq(ulfius_open_websocket_client_connection(&request, &websocket_manager_callback_client, NULL, &websocket_incoming_message_callback_client, NULL, NULL, NULL, &websocket_client_handler, &response), U_OK);
  ck_assert_int_eq(ulfius_websocket_client_connection_wait_close(&websocket_client_handler, 0), U_WEBSOCKET_STATUS_CLOSE);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  usleep(50);
  ck_assert_int_eq(ulfius_stop_framework(&instance), U_OK);

  ulfius_clean_instance(&instance);
}
END_TEST

START_TEST(test_ulfius_websocket_extension_no_match_function)
{
  struct _u_instance instance;
  struct _u_request request;
  struct _u_response response;
  struct _websocket_client_handler websocket_client_handler = {NULL, NULL};
  char url[64];
  int user_data = U_WEBSOCKET_RSV1;

  ck_assert_int_eq(ulfius_init_instance(&instance, PORT_3, NULL, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&instance, "GET", PREFIX_WEBSOCKET, NULL, 0, &callback_websocket_extension_no_match, &user_data), U_OK);
  ck_assert_int_eq(ulfius_start_framework(&instance), U_OK);

  ulfius_init_request(&request);
  ulfius_init_response(&response);
  sprintf(url, "ws://localhost:%d/%s", PORT_3, PREFIX_WEBSOCKET);

  ck_assert_int_eq(ulfius_set_websocket_request(&request, url, NULL, MESSAGE_EXT1), U_OK);
  ck_assert_int_eq(ulfius_add_websocket_client_extension_message_perform(&websocket_client_handler, MESSAGE_EXT1, U_WEBSOCKET_RSV1, &websocket_extension1_message_out_perform, NULL, &websocket_extension1_message_in_perform, NULL, NULL, NULL, NULL, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_websocket_client_extension_message_perform(&websocket_client_handler, MESSAGE_EXT2, U_WEBSOCKET_RSV2, &websocket_extension2_message_out_perform, NULL, &websocket_extension2_message_in_perform, NULL, NULL, NULL, NULL, NULL), U_OK);
  ck_assert_int_eq(ulfius_open_websocket_client_connection(&request, &websocket_extension_callback_client, NULL, &websocket_incoming_extension_callback, &user_data, NULL, NULL, &websocket_client_handler, &response), U_OK);
  ck_assert_int_eq(ulfius_websocket_client_connection_wait_close(&websocket_client_handler, 0), U_WEBSOCKET_STATUS_CLOSE);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  usleep(50);
  ck_assert_int_eq(ulfius_stop_framework(&instance), U_OK);
  
  ulfius_clean_instance(&instance);
}
END_TEST

START_TEST(test_ulfius_websocket_extension_match_function)
{
  struct _u_instance instance;
  struct _u_request request;
  struct _u_response response;
  struct _websocket_client_handler websocket_client_handler = {NULL, NULL};
  char url[64];
  int user_data = U_WEBSOCKET_RSV1|U_WEBSOCKET_RSV2;

  ck_assert_int_eq(ulfius_init_instance(&instance, PORT_4, NULL, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&instance, "GET", PREFIX_WEBSOCKET, NULL, 0, &callback_websocket_extension_match, &user_data), U_OK);
  ck_assert_int_eq(ulfius_start_framework(&instance), U_OK);

  ulfius_init_request(&request);
  ulfius_init_response(&response);
  sprintf(url, "ws://localhost:%d/%s", PORT_4, PREFIX_WEBSOCKET);

  ck_assert_int_eq(ulfius_set_websocket_request(&request, url, NULL, MESSAGE_EXT1 ", " MESSAGE_EXT2), U_OK);
  ck_assert_int_eq(ulfius_add_websocket_client_extension_message_perform(&websocket_client_handler, MESSAGE_EXT1, U_WEBSOCKET_RSV1, &websocket_extension1_message_out_perform, (void *)U_W_FLAG_CONTEXT, &websocket_extension1_message_in_perform, (void *)U_W_FLAG_CONTEXT, &websocket_extension_client_match_ext1, NULL, &websocket_extension_match_free_context, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_websocket_client_extension_message_perform(&websocket_client_handler, MESSAGE_EXT2, U_WEBSOCKET_RSV2, &websocket_extension2_message_out_perform, (void *)U_W_FLAG_CONTEXT, &websocket_extension2_message_in_perform, (void *)U_W_FLAG_CONTEXT, &websocket_extension_client_match_ext2, NULL, &websocket_extension_match_free_context, NULL), U_OK);
  ck_assert_int_eq(ulfius_open_websocket_client_connection(&request, &websocket_extension_callback_client, NULL, &websocket_incoming_extension_callback, &user_data, NULL, NULL, &websocket_client_handler, &response), U_OK);
  ck_assert_int_eq(ulfius_websocket_client_connection_wait_close(&websocket_client_handler, 0), U_WEBSOCKET_STATUS_CLOSE);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  usleep(50);
  ck_assert_int_eq(ulfius_stop_framework(&instance), U_OK);

  ulfius_clean_instance(&instance);
}
END_TEST

START_TEST(test_ulfius_websocket_extension_deflate)
{
  struct _u_instance instance;
  struct _u_request request;
  struct _u_response response;
  struct _websocket_client_handler websocket_client_handler = {NULL, NULL};
  char url[64];

  ck_assert_int_eq(ulfius_init_instance(&instance, PORT_5, NULL, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&instance, "GET", PREFIX_WEBSOCKET, NULL, 0, &callback_websocket_extension_deflate, NULL), U_OK);
  ck_assert_int_eq(ulfius_start_framework(&instance), U_OK);

  ulfius_init_request(&request);
  ulfius_init_response(&response);
  sprintf(url, "ws://localhost:%d/%s", PORT_5, PREFIX_WEBSOCKET);

  ck_assert_int_eq(ulfius_set_websocket_request(&request, url, NULL, "permessage-deflate,"MESSAGE_EXT3), U_OK);
  ck_assert_int_eq(ulfius_add_websocket_client_deflate_extension(&websocket_client_handler), U_OK);
  ck_assert_int_eq(ulfius_add_websocket_client_extension_message_perform(&websocket_client_handler, MESSAGE_EXT3, U_WEBSOCKET_RSV3, &websocket_extension3_message_out_perform, NULL, websocket_extension3_message_in_perform, NULL, NULL, NULL, NULL, NULL), U_OK);
  ck_assert_int_eq(ulfius_open_websocket_client_connection(&request, &websocket_extension_callback_client, NULL, &websocket_incoming_extension_deflate_client_callback, NULL, NULL, NULL, &websocket_client_handler, &response), U_OK);
  ck_assert_int_eq(ulfius_websocket_client_connection_wait_close(&websocket_client_handler, 0), U_WEBSOCKET_STATUS_CLOSE);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  usleep(50);
  ck_assert_int_eq(ulfius_stop_framework(&instance), U_OK);

  ulfius_clean_instance(&instance);
}
END_TEST

START_TEST(test_ulfius_websocket_extension_deflate_with_all_params)
{
  struct _u_instance instance;
  struct _u_request request;
  struct _u_response response;
  struct _websocket_client_handler websocket_client_handler = {NULL, NULL};
  char url[64];

  ck_assert_int_eq(ulfius_init_instance(&instance, PORT_6, NULL, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&instance, "GET", PREFIX_WEBSOCKET, NULL, 0, &callback_websocket_extension_deflate, NULL), U_OK);
  ck_assert_int_eq(ulfius_start_framework(&instance), U_OK);

  ulfius_init_request(&request);
  ulfius_init_response(&response);
  sprintf(url, "ws://localhost:%d/%s", PORT_6, PREFIX_WEBSOCKET);

  ck_assert_int_eq(ulfius_set_websocket_request(&request, url, NULL, "permessage-deflate; server_max_window_bits=10; client_max_window_bits=12; server_no_context_takeover; client_no_context_takeover, "MESSAGE_EXT3), U_OK);
  ck_assert_int_eq(ulfius_add_websocket_client_deflate_extension(&websocket_client_handler), U_OK);
  ck_assert_int_eq(ulfius_add_websocket_client_extension_message_perform(&websocket_client_handler, MESSAGE_EXT3, U_WEBSOCKET_RSV3, &websocket_extension3_message_out_perform, NULL, websocket_extension3_message_in_perform, NULL, NULL, NULL, NULL, NULL), U_OK);
  ck_assert_int_eq(ulfius_open_websocket_client_connection(&request, &websocket_extension_callback_client, NULL, &websocket_incoming_extension_deflate_client_callback, NULL, NULL, NULL, &websocket_client_handler, &response), U_OK);
  ck_assert_int_eq(ulfius_websocket_client_connection_wait_close(&websocket_client_handler, 0), U_WEBSOCKET_STATUS_CLOSE);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  usleep(50);
  ck_assert_int_eq(ulfius_stop_framework(&instance), U_OK);

  ulfius_clean_instance(&instance);
}
END_TEST

START_TEST(test_ulfius_websocket_extension_deflate_error_params)
{
  struct _u_instance instance;
  struct _u_request request;
  struct _u_response response;
  struct _websocket_client_handler websocket_client_handler = {NULL, NULL};
  char url[64];

  ck_assert_int_eq(ulfius_init_instance(&instance, PORT_7, NULL, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&instance, "GET", PREFIX_WEBSOCKET, NULL, 0, &callback_websocket_extension_deflate_disabled, NULL), U_OK);
  ck_assert_int_eq(ulfius_start_framework(&instance), U_OK);

  sprintf(url, "ws://localhost:%d/%s", PORT_7, PREFIX_WEBSOCKET);

  ulfius_init_request(&request);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_set_websocket_request(&request, url, NULL, "permessage-deflate; server_max_window_bits=1; client_max_window_bits=12; server_no_context_takeover; client_no_context_takeover"), U_OK);
  ck_assert_int_eq(ulfius_add_websocket_client_deflate_extension(&websocket_client_handler), U_OK);
  ck_assert_int_eq(ulfius_open_websocket_client_connection(&request, &websocket_extension_callback_client, NULL, &websocket_incoming_extension_deflate_client_callback_disabled, NULL, NULL, NULL, &websocket_client_handler, &response), U_OK);
  ck_assert_int_eq(ulfius_websocket_client_connection_wait_close(&websocket_client_handler, 0), U_WEBSOCKET_STATUS_CLOSE);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  usleep(50);
  ck_assert_int_eq(ulfius_stop_framework(&instance), U_OK);

  ulfius_clean_instance(&instance);
}
END_TEST

#ifndef U_DISABLE_WS_MESSAGE_LIST
START_TEST(test_ulfius_websocket_keep_messages)
{
  struct _u_instance instance;
  struct _u_request request;
  struct _u_response response;
  struct _websocket_client_handler websocket_client_handler = {NULL, NULL};
  char url[64];
  int keep_messages = 0;

  ck_assert_int_eq(ulfius_init_instance(&instance, PORT_9, NULL, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&instance, "GET", PREFIX_WEBSOCKET, NULL, 0, &callback_websocket_keep_messages, &keep_messages), U_OK);
  ck_assert_int_eq(ulfius_start_framework(&instance), U_OK);
  sprintf(url, "ws://localhost:%d/%s", PORT_9, PREFIX_WEBSOCKET);

  keep_messages = U_WEBSOCKET_KEEP_NONE;
  ulfius_init_request(&request);
  ulfius_init_response(&response);
  // Test correct websocket connection on correct websocket service
  ck_assert_int_eq(ulfius_set_websocket_request(&request, url, NULL, NULL), U_OK);
  ck_assert_int_eq(ulfius_open_websocket_client_connection(&request, NULL, NULL, websocket_incoming_keep_messages, NULL, NULL, NULL, &websocket_client_handler, &response), U_OK);
  ck_assert_int_eq(ulfius_websocket_client_connection_wait_close(&websocket_client_handler, 0), U_WEBSOCKET_STATUS_CLOSE);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  keep_messages = U_WEBSOCKET_KEEP_INCOMING|U_WEBSOCKET_KEEP_OUTCOMING;
  ulfius_init_request(&request);
  ulfius_init_response(&response);
  // Test correct websocket connection on correct websocket service
  ck_assert_int_eq(ulfius_set_websocket_request(&request, url, NULL, NULL), U_OK);
  ck_assert_int_eq(ulfius_open_websocket_client_connection(&request, NULL, NULL, websocket_incoming_keep_messages, NULL, NULL, NULL, &websocket_client_handler, &response), U_OK);
  ck_assert_int_eq(ulfius_websocket_client_connection_wait_close(&websocket_client_handler, 0), U_WEBSOCKET_STATUS_CLOSE);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  keep_messages = U_WEBSOCKET_KEEP_INCOMING;
  ulfius_init_request(&request);
  ulfius_init_response(&response);
  // Test correct websocket connection on correct websocket service
  ck_assert_int_eq(ulfius_set_websocket_request(&request, url, NULL, NULL), U_OK);
  ck_assert_int_eq(ulfius_open_websocket_client_connection(&request, NULL, NULL, websocket_incoming_keep_messages, NULL, NULL, NULL, &websocket_client_handler, &response), U_OK);
  ck_assert_int_eq(ulfius_websocket_client_connection_wait_close(&websocket_client_handler, 0), U_WEBSOCKET_STATUS_CLOSE);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  keep_messages = U_WEBSOCKET_KEEP_OUTCOMING;
  ulfius_init_request(&request);
  ulfius_init_response(&response);
  // Test correct websocket connection on correct websocket service
  ck_assert_int_eq(ulfius_set_websocket_request(&request, url, NULL, NULL), U_OK);
  ck_assert_int_eq(ulfius_open_websocket_client_connection(&request, NULL, NULL, websocket_incoming_keep_messages, NULL, NULL, NULL, &websocket_client_handler, &response), U_OK);
  ck_assert_int_eq(ulfius_websocket_client_connection_wait_close(&websocket_client_handler, 0), U_WEBSOCKET_STATUS_CLOSE);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  usleep(50);
  ck_assert_int_eq(ulfius_stop_framework(&instance), U_OK);

  ulfius_clean_instance(&instance);
}
END_TEST
#endif

START_TEST(test_ulfius_websocket_origin)
{
  struct _u_instance instance;
  struct _u_request request;
  struct _u_response response;
  struct _websocket_client_handler websocket_client_handler = {NULL, NULL};
  char url[64];

  ck_assert_int_eq(ulfius_init_instance(&instance, PORT_10, NULL, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&instance, "GET", PREFIX_WEBSOCKET, NULL, 0, &callback_websocket_with_origin, NULL), U_OK);
  ck_assert_int_eq(ulfius_start_framework(&instance), U_OK);

  sprintf(url, "ws://localhost:%d/%s", PORT_10, PREFIX_WEBSOCKET);
  
  // Test websocket connection without Origin header
  ulfius_init_request(&request);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_set_websocket_request(&request, url, DEFAULT_PROTOCOL, DEFAULT_EXTENSION), U_OK);
  ck_assert_int_eq(ulfius_open_websocket_client_connection(&request, NULL, NULL, &websocket_incoming_message_without_origin_callback_client, NULL, NULL, NULL, &websocket_client_handler, &response), U_OK);
  ck_assert_int_eq(ulfius_websocket_client_connection_wait_close(&websocket_client_handler, 0), U_WEBSOCKET_STATUS_CLOSE);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  // Test websocket connection with Origin header
  ulfius_init_request(&request);
  ulfius_init_response(&response);
  websocket_client_handler.websocket = NULL;
  websocket_client_handler.response = NULL;
  ck_assert_int_eq(ulfius_set_request_properties(&request, U_OPT_HEADER_PARAMETER, "Origin", ORIGIN, U_OPT_NONE), U_OK);
  ck_assert_int_eq(ulfius_set_websocket_request(&request, url, DEFAULT_PROTOCOL, DEFAULT_EXTENSION), U_OK);
  ck_assert_int_eq(ulfius_open_websocket_client_connection(&request, NULL, NULL, &websocket_incoming_message_with_origin_callback_client, NULL, NULL, NULL, &websocket_client_handler, &response), U_OK);
  ck_assert_int_eq(ulfius_websocket_client_connection_wait_close(&websocket_client_handler, 0), U_WEBSOCKET_STATUS_CLOSE);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  ck_assert_int_eq(ulfius_stop_framework(&instance), U_OK);
  ulfius_clean_instance(&instance);
}
END_TEST

START_TEST(test_ulfius_websocket_json_messages)
{
  struct _u_instance instance;
  struct _u_request request;
  struct _u_response response;
  struct _websocket_client_handler websocket_client_handler = {NULL, NULL};
  char url[64];

  ck_assert_int_eq(ulfius_init_instance(&instance, PORT_11, NULL, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&instance, "GET", PREFIX_WEBSOCKET, "/valid", 0, &callback_websocket_json_valid_messages, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&instance, "GET", PREFIX_WEBSOCKET, "/invalid", 0, &callback_websocket_json_invalid_messages, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&instance, "GET", PREFIX_WEBSOCKET, "/empty", 0, &callback_websocket_json_empty_messages, NULL), U_OK);
  ck_assert_int_eq(ulfius_start_framework(&instance), U_OK);

  // Valid JSON
  sprintf(url, "ws://localhost:%d/%s/valid", PORT_11, PREFIX_WEBSOCKET);
  ulfius_init_request(&request);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_set_websocket_request(&request, url, NULL, NULL), U_OK);
  ck_assert_int_eq(ulfius_open_websocket_client_connection(&request, &websocket_manager_json_valid_callback, NULL, NULL, NULL, NULL, NULL, &websocket_client_handler, &response), U_OK);
  ck_assert_int_eq(ulfius_websocket_client_connection_wait_close(&websocket_client_handler, 0), U_WEBSOCKET_STATUS_CLOSE);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  usleep(50);

  // Invalid JSON
  sprintf(url, "ws://localhost:%d/%s/invalid", PORT_11, PREFIX_WEBSOCKET);
  ulfius_init_request(&request);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_set_websocket_request(&request, url, NULL, NULL), U_OK);
  ck_assert_int_eq(ulfius_open_websocket_client_connection(&request, &websocket_manager_json_invalid_callback, NULL, NULL, NULL, NULL, NULL, &websocket_client_handler, &response), U_OK);
  ck_assert_int_eq(ulfius_websocket_client_connection_wait_close(&websocket_client_handler, 0), U_WEBSOCKET_STATUS_CLOSE);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  usleep(50);

  // Empty JSON
  sprintf(url, "ws://localhost:%d/%s/empty", PORT_11, PREFIX_WEBSOCKET);
  ulfius_init_request(&request);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_set_websocket_request(&request, url, NULL, NULL), U_OK);
  ck_assert_int_eq(ulfius_open_websocket_client_connection(&request, &websocket_manager_json_empty_callback, NULL, NULL, NULL, NULL, NULL, &websocket_client_handler, &response), U_OK);
  ck_assert_int_eq(ulfius_websocket_client_connection_wait_close(&websocket_client_handler, 0), U_WEBSOCKET_STATUS_CLOSE);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  usleep(50);
  ck_assert_int_eq(ulfius_stop_framework(&instance), U_OK);

  ulfius_clean_instance(&instance);
}
END_TEST

#endif

static Suite *ulfius_suite(void)
{
	Suite *s;
	TCase *tc_websocket;

	s = suite_create("Ulfius websocket functions tests");
	tc_websocket = tcase_create("test_ulfius_websocket");
#ifndef U_DISABLE_WEBSOCKET
  tcase_add_test(tc_websocket, test_ulfius_websocket_set_websocket_response);
	tcase_add_test(tc_websocket, test_ulfius_websocket_set_websocket_request);
	tcase_add_test(tc_websocket, test_ulfius_websocket_open_websocket_client_connection_error);
	tcase_add_test(tc_websocket, test_ulfius_websocket_client);
	tcase_add_test(tc_websocket, test_ulfius_websocket_client_no_onclose);
	tcase_add_test(tc_websocket, test_ulfius_websocket_extension_no_match_function);
	tcase_add_test(tc_websocket, test_ulfius_websocket_extension_match_function);
	tcase_add_test(tc_websocket, test_ulfius_websocket_extension_deflate);
	tcase_add_test(tc_websocket, test_ulfius_websocket_extension_deflate_with_all_params);
	tcase_add_test(tc_websocket, test_ulfius_websocket_extension_deflate_error_params);
#ifndef U_DISABLE_WS_MESSAGE_LIST
	tcase_add_test(tc_websocket, test_ulfius_websocket_keep_messages);
#endif
	tcase_add_test(tc_websocket, test_ulfius_websocket_origin);
	tcase_add_test(tc_websocket, test_ulfius_websocket_json_messages);
#endif
	tcase_set_timeout(tc_websocket, 30);
	suite_add_tcase(s, tc_websocket);

	return s;
}

int main(void)
{
  int number_failed;
  Suite *s;
  SRunner *sr;
  //y_init_logs("Ulfius", Y_LOG_MODE_CONSOLE, Y_LOG_LEVEL_DEBUG, NULL, "Starting Ulfius websocket tests");
  ulfius_global_init();
  s = ulfius_suite();
  sr = srunner_create(s);

  pthread_mutex_init(&ws_lock, NULL);
  pthread_cond_init(&ws_cond, NULL);

  srunner_run_all(sr, CK_VERBOSE);
  number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);
  ulfius_global_close();
  
  pthread_mutex_destroy(&ws_lock);
  pthread_cond_destroy(&ws_cond);

  //y_close_logs();
	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
