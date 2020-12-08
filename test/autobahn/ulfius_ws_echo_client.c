/* Public domain, no copyright. Use at your own risk. */

#include <ulfius.h>

#define PORT "9001"

static void websocket_echo_message_callback (const struct _u_request * request,
                                             struct _websocket_manager * websocket_manager,
                                             const struct _websocket_message * last_message,
                                             void * websocket_incoming_message_user_data) {
  //y_log_message(Y_LOG_LEVEL_DEBUG, "Got message opcode %d, %zu", last_message->opcode, last_message->data_len);
  if (ulfius_websocket_send_message(websocket_manager, last_message->opcode, last_message->data_len, last_message->data) != U_OK) {
    y_log_message(Y_LOG_LEVEL_ERROR, "Error send message from incoming");
  }
}

int main(int argc, char ** argv) {
  char * url;
  struct _u_request request;
  struct _u_response response;
  struct _websocket_client_handler websocket_client_handler;
  const char * url_pattern = (argc>2&&0==o_strcmp("-https", argv[2]))?"wss://localhost:" PORT "/runCase?case=%s&agent=ulfius":"ws://localhost:" PORT "/runCase?case=%s&agent=ulfius", * extensions;
  int test_case = atoi(argv[1]);

  y_init_logs("autobahn client tester", Y_LOG_MODE_FILE, Y_LOG_LEVEL_DEBUG, "ulfius_ws_echo_client.log", "Starting websocket client echo for autobahn testsuite");

  ulfius_init_request(&request);
  request.check_server_certificate = 0;

  if (test_case >= 394 && test_case <= 411) { // test cases 13.1.*
    extensions = "permessage-deflate; client_max_window_bits";
  } else if (test_case >= 412 && test_case <= 429) { // test cases 13.2.*
    extensions = "permessage-deflate; client_no_context_takeover; client_max_window_bits";
  } else if (test_case >= 430 && test_case <= 447) { // test cases 13.3.*
    extensions = "permessage-deflate; client_max_window_bits=8";
  } else if (test_case >= 448 && test_case <= 465) { // test cases 13.4.*
    extensions = "permessage-deflate; client_max_window_bits=15";
  } else if (test_case >= 466 && test_case <= 483) { // test cases 13.5.*
    extensions = "permessage-deflate; client_no_context_takeover; client_max_window_bits=8";
  } else if (test_case >= 484 && test_case <= 501) { // test cases 13.6.*
    extensions = "permessage-deflate; client_no_context_takeover; client_max_window_bits=15";
  } else if (test_case >= 502 && test_case <= 519) { // test cases 13.7.*
    extensions = "permessage-deflate; client_no_context_takeover; client_max_window_bits=8, permessage-deflate; client_no_context_takeover; client_max_window_bits, permessage-deflate; client_max_window_bits";
  } else {
    extensions = "permessage-deflate";
  }
  url = msprintf(url_pattern, argv[1]!=NULL?argv[1]:0);
  if (ulfius_set_websocket_request(&request, url, NULL, extensions) == U_OK) {
    websocket_client_handler.websocket = NULL;
    websocket_client_handler.response = NULL;
    ulfius_add_websocket_client_deflate_extension(&websocket_client_handler);
    ulfius_init_response(&response);
    y_log_message(Y_LOG_LEVEL_INFO, "Test case %s started", argv[1]);
    if (ulfius_open_websocket_client_connection(&request, NULL, NULL, &websocket_echo_message_callback, NULL, NULL, NULL, &websocket_client_handler, &response) == U_OK) {
      if (ulfius_websocket_client_connection_wait_close(&websocket_client_handler, 0) == U_WEBSOCKET_STATUS_ERROR) {
        y_log_message(Y_LOG_LEVEL_ERROR, "Test case %s error", argv[1]);
      } else {
        y_log_message(Y_LOG_LEVEL_INFO, "Test case %s closed", argv[1]);
      }
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "Error connecting to websocket");
    }
    ulfius_clean_response(&response);
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "Error setting request parameters");
  }
  o_free(url);
  ulfius_clean_request(&request);

  y_close_logs();
}
