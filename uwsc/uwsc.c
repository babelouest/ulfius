/**
 *
 * uwsc - Ulfius Websocket client
 *
 * Command-line application to run a websocket client
 *
 * Copyright 2018 Nicolas Mora <mail@babelouest.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU GENERAL PUBLIC LICENSE
 * License as published by the Free Software Foundation;
 * version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU GENERAL PUBLIC LICENSE for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <getopt.h>
#include <orcania.h>
#include <yder.h>
#include <ulfius.h>

void uwsc_manager_callback (const struct _u_request * request, struct _websocket_manager * websocket_manager, void * websocket_manager_user_data) {
  do {
    char message[257];
    if (fgets(message, 256, stdin) != NULL) {
      ulfius_websocket_send_message(websocket_manager, U_WEBSOCKET_OPCODE_TEXT, o_strlen(message), message);
    }
  } while (1);
}

void uwsc_manager_incoming (const struct _u_request * request, struct _websocket_manager * websocket_manager, const struct _websocket_message * message, void * websocket_incoming_user_data) {
  y_log_message(Y_LOG_LEVEL_DEBUG, "Incoming message: '%.*s'", message->data_len, message->data);
}

int main (int argc, char ** argv) {
  int next_option;
  const char * short_options = "x::b::t::i::l::f::p::e::v::h::";
  static const struct option long_options[]= {
    {"add-header", required_argument, NULL, 'x'},       // Add the specified header of the form 'key:value'
    {"send-binary-file", required_argument, NULL, 'b'}, // Sends the content of a file at the beginning of the connection in a binary frame
    {"send-text-file", required_argument, NULL, 't'},   // Sends the content of a file at the beginning of the connection in a text frame
    {"non-interactive-send", no_argument, NULL, 'i'},   // Do not prompt for message sending
    {"non-listening", no_argument, NULL, 'l'},          // Do not listen for incoming messages
    {"fragmentation", required_argument, NULL, 'f'},    // Specify the max length of a frame and fragment the message to send if required
    {"protocol", required_argument, NULL, 'p'},         // Websocket protocol
    {"extensions", required_argument, NULL, 'e'},       // Websocket extensions
    {"version", required_argument, NULL, 'v'},          // Show version
    {"help", required_argument, NULL, 'h'},             // print help
    {NULL, 0, NULL, 0}
  };
  
  struct _u_request request;
  struct _u_response response;
  struct _websocket_client_handler websocket_client_handler;
  
  // TODO: Complete option manager
  do {
    next_option = getopt_long(argc, argv, short_options, long_options, NULL);
    
    switch (next_option) {
      case 'x':
        break;
      case 'b':
        break;
      case 't':
        break;
      case 'i':
        break;
      case 'l':
        break;
      case 'f':
        break;
      case 'p':
        break;
      case 'e':
        break;
      case 'v':
        break;
      case 'h':
        break;
      default:
        break;
    }
  } while (next_option != -1);

  ulfius_init_request(&request);
  ulfius_init_response(&response);
  if (ulfius_set_websocket_request(&request, argv[0], NULL, NULL) == U_OK) {
    if (ulfius_open_websocket_client_connection(&request, &uwsc_manager_callback, NULL, &uwsc_manager_incoming, NULL, NULL, NULL, &websocket_client_handler, &response) == U_OK) {
      ulfius_websocket_client_connection_wait_close(&websocket_client_handler, 0);
    } else {
      printf("status %ld\n", response.status);
    }
  }
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
}
