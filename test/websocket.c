/* Public domain, no copyright. Use at your own risk. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include <check.h>
#include <ulfius.h>

#define WEBSOCKET_URL "http://localhost:8378/websocket"
#define DEFAULT_PROTOCOL "proto"
#define DEFAULT_EXTENSION "ext"
#define DEFAULT_MESSAGE "message content with a few characters"
#define PORT 9275
#define PREFIX_WEBSOCKET "/websocket"

#ifndef U_DISABLE_WEBSOCKET
void websocket_manager_callback_empty (const struct _u_request * request, struct _websocket_manager * websocket_manager, void * websocket_manager_user_data) {
}

void websocket_incoming_message_callback_empty (const struct _u_request * request, struct _websocket_manager * websocket_manager, const struct _websocket_message * message, void * websocket_incoming_user_data) {
}

void websocket_onclose_callback_empty (const struct _u_request * request, struct _websocket_manager * websocket_manager, void * websocket_onclose_user_data) {
}

void websocket_onclose_callback_client (const struct _u_request * request, struct _websocket_manager * websocket_manager, void * websocket_onclose_user_data) {
  ck_assert_ptr_ne(websocket_onclose_user_data, NULL);
}

void websocket_echo_message_callback (const struct _u_request * request,
                                         struct _websocket_manager * websocket_manager,
                                         const struct _websocket_message * last_message,
                                         void * websocket_incoming_message_user_data) {
  if (last_message->opcode == U_WEBSOCKET_OPCODE_TEXT) {
    ck_assert_int_eq(ulfius_websocket_send_message(websocket_manager, U_WEBSOCKET_OPCODE_TEXT, last_message->data_len, last_message->data), U_OK);
  }
}

void websocket_onclose_message_callback (const struct _u_request * request, struct _websocket_manager * websocket_manager, void * websocket_onclose_user_data) {
  o_free(websocket_onclose_user_data);
}

void websocket_manager_callback_client (const struct _u_request * request, struct _websocket_manager * websocket_manager, void * websocket_manager_user_data) {
  int i;
  
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

void websocket_incoming_message_callback_client (const struct _u_request * request, struct _websocket_manager * websocket_manager, const struct _websocket_message * message, void * websocket_incoming_user_data) {
  ck_assert_int_eq(0, o_strncmp(message->data, DEFAULT_MESSAGE, message->data_len));
}

int callback_websocket (const struct _u_request * request, struct _u_response * response, void * user_data) {
  int ret;
  char * websocket_allocated_data = o_strdup("grut");
  
  ret = ulfius_set_websocket_response(response, NULL, NULL, NULL, NULL, &websocket_echo_message_callback, websocket_allocated_data, &websocket_onclose_message_callback, websocket_allocated_data);
  ck_assert_int_eq(ret, U_OK);
  return (ret == U_OK)?U_CALLBACK_CONTINUE:U_CALLBACK_ERROR;
}

int callback_websocket_onclose (const struct _u_request * request, struct _u_response * response, void * user_data) {
  int ret;
  
  ret = ulfius_set_websocket_response(response, NULL, NULL, NULL, NULL, &websocket_echo_message_callback, NULL, NULL, NULL);
  ck_assert_int_eq(ret, U_OK);
  return (ret == U_OK)?U_CALLBACK_CONTINUE:U_CALLBACK_ERROR;
}

START_TEST(test_websocket_ulfius_set_websocket_response)
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

START_TEST(test_websocket_ulfius_set_websocket_request)
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

START_TEST(test_websocket_ulfius_open_websocket_client_connection_error)
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

START_TEST(test_websocket_ulfius_websocket_client)
{
  struct _u_instance instance;
  struct _u_request request;
  struct _u_response response;
  struct _websocket_client_handler websocket_client_handler;
  char url[64], * allocated_data = o_strdup("plop");

  ck_assert_int_eq(ulfius_init_instance(&instance, PORT, NULL, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&instance, "GET", PREFIX_WEBSOCKET, NULL, 0, &callback_websocket, allocated_data), U_OK);
  ck_assert_int_eq(ulfius_start_framework(&instance), U_OK);

  ulfius_init_request(&request);
  ulfius_init_response(&response);
  sprintf(url, "ws://localhost:%d/%s", PORT, PREFIX_WEBSOCKET);
  
  // Test correct websocket connection on correct websocket service
  ck_assert_int_eq(ulfius_set_websocket_request(&request, url, DEFAULT_PROTOCOL, DEFAULT_EXTENSION), U_OK);
  ck_assert_int_eq(ulfius_open_websocket_client_connection(&request, &websocket_manager_callback_client, NULL, &websocket_incoming_message_callback_client, NULL, websocket_onclose_callback_client, allocated_data, &websocket_client_handler, &response), U_OK);
  ck_assert_int_eq(ulfius_websocket_client_connection_wait_close(&websocket_client_handler, 0), U_WEBSOCKET_STATUS_CLOSE);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  // Test incorrect websocket connection on correct websocket service
  ulfius_init_request(&request);
  ulfius_init_response(&response);
  request.http_verb = o_strdup("GET");
  request.http_url = o_strdup(url);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_ERROR_LIBCURL); // On a websocket connection attempt, libcurl return 'Unsupported protocol'
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  // Test incorrect websocket connection on correct websocket service
  ulfius_init_request(&request);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_set_websocket_request(&request, url, DEFAULT_PROTOCOL, DEFAULT_EXTENSION), U_OK);
  ck_assert_int_eq(ulfius_open_websocket_client_connection(&request, &websocket_manager_callback_client, NULL, &websocket_incoming_message_callback_client, NULL, websocket_onclose_callback_client, allocated_data, &websocket_client_handler, &response), U_OK);
  ck_assert_int_eq(ulfius_websocket_client_connection_wait_close(&websocket_client_handler, 0), U_WEBSOCKET_STATUS_CLOSE);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  ck_assert_int_eq(ulfius_stop_framework(&instance), U_OK);
  ulfius_clean_instance(&instance);
  o_free(allocated_data);
}
END_TEST

START_TEST(test_websocket_ulfius_websocket_client_no_onclose)
{
  struct _u_instance instance;
  struct _u_request request;
  struct _u_response response;
  struct _websocket_client_handler websocket_client_handler;
  char url[64];
  ck_assert_int_eq(ulfius_init_instance(&instance, PORT, NULL, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&instance, "GET", PREFIX_WEBSOCKET, NULL, 0, &callback_websocket_onclose, NULL), U_OK);
  ck_assert_int_eq(ulfius_start_framework(&instance), U_OK);

  ulfius_init_request(&request);
  ulfius_init_response(&response);
  sprintf(url, "ws://localhost:%d/%s", PORT, PREFIX_WEBSOCKET);
  
  // Test correct websocket connection on correct websocket service
  ck_assert_int_eq(ulfius_set_websocket_request(&request, url, DEFAULT_PROTOCOL, DEFAULT_EXTENSION), U_OK);
  ck_assert_int_eq(ulfius_open_websocket_client_connection(&request, &websocket_manager_callback_client, NULL, &websocket_incoming_message_callback_client, NULL, NULL, NULL, &websocket_client_handler, &response), U_OK);
  ck_assert_int_eq(ulfius_websocket_client_connection_wait_close(&websocket_client_handler, 0), U_WEBSOCKET_STATUS_CLOSE);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  // Test incorrect websocket connection on correct websocket service
  ulfius_init_request(&request);
  ulfius_init_response(&response);
  request.http_verb = o_strdup("GET");
  request.http_url = o_strdup(url);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_ERROR_LIBCURL); // On a websocket connection attempt, libcurl return 'Unsupported protocol'
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  // Test incorrect websocket connection on correct websocket service
  ulfius_init_request(&request);
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_set_websocket_request(&request, url, DEFAULT_PROTOCOL, DEFAULT_EXTENSION), U_OK);
  ck_assert_int_eq(ulfius_open_websocket_client_connection(&request, &websocket_manager_callback_client, NULL, &websocket_incoming_message_callback_client, NULL, NULL, NULL, &websocket_client_handler, &response), U_OK);
  ck_assert_int_eq(ulfius_websocket_client_connection_wait_close(&websocket_client_handler, 0), U_WEBSOCKET_STATUS_CLOSE);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
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
	tcase_add_test(tc_websocket, test_websocket_ulfius_set_websocket_response);
	tcase_add_test(tc_websocket, test_websocket_ulfius_set_websocket_request);
	tcase_add_test(tc_websocket, test_websocket_ulfius_open_websocket_client_connection_error);
	tcase_add_test(tc_websocket, test_websocket_ulfius_websocket_client);
	tcase_add_test(tc_websocket, test_websocket_ulfius_websocket_client_no_onclose);
#endif
	tcase_set_timeout(tc_websocket, 30);
	suite_add_tcase(s, tc_websocket);

	return s;
}

int main(int argc, char *argv[])
{
  int number_failed;
  Suite *s;
  SRunner *sr;
  //y_init_logs("Ulfius", Y_LOG_MODE_CONSOLE, Y_LOG_LEVEL_DEBUG, NULL, "Starting Ulfius websocket tests");
  s = ulfius_suite();
  sr = srunner_create(s);

  srunner_run_all(sr, CK_VERBOSE);
  number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);
  
  //y_close_logs();
	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
