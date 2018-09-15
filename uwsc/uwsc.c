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
  struct _websocket_message * last_message;
  do {
    char message[257];
    if (fgets(message, 256, stdin) != NULL) {
      fprintf(stdout, "\b\bSend '%.*s'\n> ", (int)(o_strlen(message)-1), message);
      fflush(stdout);
      if (ulfius_websocket_send_message(websocket_manager, U_WEBSOCKET_OPCODE_TEXT, o_strlen(message)-1, message) != U_OK) {
        y_log_message(Y_LOG_LEVEL_ERROR, "Error sending message '%.*s'\n", (int)(o_strlen(message)-1), message);
      } else {
        last_message = ulfius_websocket_pop_first_message(websocket_manager->message_list_outcoming);
        if (last_message != NULL) {
          ulfius_clear_websocket_message(last_message);
        }
      }
    }
  } while (1);
}

void uwsc_manager_incoming (const struct _u_request * request, struct _websocket_manager * websocket_manager, const struct _websocket_message * message, void * websocket_incoming_user_data) {
  struct _websocket_message * last_message;
  if (message->opcode == U_WEBSOCKET_OPCODE_TEXT) {
    fprintf(stdout, "\b\bServer: '%.*s'\n> ", (int)message->data_len, message->data);
    fflush(stdout);
  } else if (message->opcode == U_WEBSOCKET_OPCODE_BINARY) {
    fprintf(stdout, "\b\bServer sent binary message, length %zu\n> ", message->data_len);
    fflush(stdout);
  }
  last_message = ulfius_websocket_pop_first_message(websocket_manager->message_list_incoming);
  if (last_message != NULL) {
    ulfius_clear_websocket_message(last_message);
  }
}

int main (int argc, char ** argv) {
  int next_option;
  const char * short_options = "o::x::b::t::i::l::f::p::e::v::h::";
  static const struct option long_options[]= {
    {"output-log-file", required_argument, NULL, 'o'},  // Sets an output file for logging messages
    {"add-header", required_argument, NULL, 'x'},       // Add the specified header of the form 'key:value'
    {"send-binary-file", required_argument, NULL, 'b'}, // Sends the content of a file at the beginning of the connection in a binary frame
    {"send-text-file", required_argument, NULL, 't'},   // Sends the content of a file at the beginning of the connection in a text frame
    {"non-interactive-send", no_argument, NULL, 'i'},   // Do not prompt for message sending
    {"non-listening", no_argument, NULL, 'l'},          // Do not listen for incoming messages
    {"fragmentation", required_argument, NULL, 'f'},    // Specify the max length of a frame and fragment the message to send if required
    {"protocol", required_argument, NULL, 'p'},         // Websocket protocol
    {"extensions", required_argument, NULL, 'e'},       // Websocket extensions
    {"version", no_argument, NULL, 'v'},                // Show version
    {"help", no_argument, NULL, 'h'},                   // print help
    {NULL, 0, NULL, 0}
  };
  char * log_path = NULL, * url = NULL;
  
  struct _u_request request;
  struct _u_response response;
  struct _websocket_client_handler websocket_client_handler;
  
  // TODO: Complete option manager
  do {
    next_option = getopt_long(argc, argv, short_options, long_options, NULL);
    
    switch (next_option) {
      case 'o':
        log_path = o_strdup(optarg);
        break;
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
  
  if (log_path != NULL) {
    y_init_logs("Ulfius Websocket Client", Y_LOG_MODE_FILE, Y_LOG_LEVEL_DEBUG, log_path, "Start uwsc");
  }

  if (optind < argc) {
    url = argv[optind];
  } else {
    fprintf(stderr, "No url\n");
    exit(0);
  }
  
  ulfius_init_request(&request);
  ulfius_init_response(&response);
  if (ulfius_set_websocket_request(&request, url, NULL, "permessage-deflate") == U_OK) {
    if (ulfius_open_websocket_client_connection(&request, &uwsc_manager_callback, NULL, &uwsc_manager_incoming, NULL, NULL, NULL, &websocket_client_handler, &response) == U_OK) {
      fprintf(stdout, "Websocket connected, you can send text messages of maximum 256 characters.\n> ");
      fflush(stdout);
      ulfius_websocket_client_connection_wait_close(&websocket_client_handler, 0);
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "Error connecting to websocket");
    }
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "Error initializing websocket request");
  }
  
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  if (log_path != NULL) {
    y_close_logs();
  }
}
