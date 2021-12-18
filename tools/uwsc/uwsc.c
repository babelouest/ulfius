/**
 *
 * uwsc - Ulfius Websocket client
 *
 * Command-line application to connect to a websocket service
 *
 * Copyright 2018-2020 Nicolas Mora <mail@babelouest.org>
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
#include <stdio.h>
#include <stdlib.h>
#include <ulfius.h>

/** Define mock yder functions when yder is disabled **/
#ifdef U_DISABLE_YDER
int y_init_logs(const char * app, const unsigned long init_mode, const unsigned long init_level, const char * init_log_file, const char * message) {
  (void)(app);
  (void)(init_mode);
  (void)(init_level);
  (void)(init_log_file);
  (void)(message);
  return 1;
}

int y_set_logs_callback(void (* y_callback_log_message) (void * cls, const char * app_name, const time_t date, const unsigned long level, const char * message), void * cls, const char * message) {
  (void)(y_callback_log_message);
  (void)(cls);
  (void)(message);
  return 1;
}

void y_log_message(const unsigned long type, const char * message, ...) {
  (void)(type);
  (void)(message);
}

int y_close_logs() {
  return 1;
}
#endif

#define _UWSC_VERSION_ "0.11"

#ifndef U_DISABLE_WEBSOCKET

struct _config {
  char       * log_path;
  char       * binary_file_send;
  char       * text_file_send;
  int          non_interactive;
  int          non_listening;
  int          quiet;
  unsigned int fragmentation;
  char       * protocol;
  char       * extensions;
  struct _u_request * request;
  struct _u_response * response;
};

static char * read_file(const char * filename, size_t * filesize) {
  char * buffer = NULL;
  long length;
  FILE * f;
  if (filename != NULL) {
    f = fopen (filename, "rb");
    if (f) {
      fseek (f, 0, SEEK_END);
      length = ftell (f);
      if (filesize != NULL) {
        *filesize = length;
      }
      fseek (f, 0, SEEK_SET);
      buffer = o_malloc (length + 1);
      if (buffer != NULL) {
        if (fread (buffer, 1, length, f)) {
          buffer[length] = '\0';
        }
      }
      fclose (f);
    }
    return buffer;
  } else {
    return NULL;
  }
}

static void uwsc_manager_callback (const struct _u_request * request, struct _websocket_manager * websocket_manager, void * websocket_manager_user_data) {
  char message[257] = {0};
  struct _websocket_message * last_message;
  struct _config * config = (struct _config *)websocket_manager_user_data;
  char * file_content;
  size_t file_len;

  if (config->text_file_send != NULL) {
    file_content = read_file(config->text_file_send, &file_len);
    if (file_content != NULL && file_len > 0) {
      if (ulfius_websocket_send_message(websocket_manager, U_WEBSOCKET_OPCODE_TEXT, file_len, file_content) != U_OK) {
        y_log_message(Y_LOG_LEVEL_ERROR, "Error sending text file");
      } else {
        last_message = ulfius_websocket_pop_first_message(websocket_manager->message_list_outcoming);
        if (last_message != NULL) {
          ulfius_clear_websocket_message(last_message);
        }
      }
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "Error reading file %s", config->text_file_send);
    }
    o_free(file_content);
  } else if (config->binary_file_send != NULL) {
    file_content = read_file(config->binary_file_send, &file_len);
    if (file_content != NULL && file_len > 0) {
      if (ulfius_websocket_send_message(websocket_manager, U_WEBSOCKET_OPCODE_BINARY, file_len, file_content) != U_OK) {
        y_log_message(Y_LOG_LEVEL_ERROR, "Error sending binary file");
      } else {
        last_message = ulfius_websocket_pop_first_message(websocket_manager->message_list_outcoming);
        if (last_message != NULL) {
          ulfius_clear_websocket_message(last_message);
        }
      }
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "Error reading file %s", config->binary_file_send);
    }
    o_free(file_content);
  }
  do {
    if (!config->non_interactive) {
      if (fgets(message, 256, stdin) != NULL) {
        if (o_strlen(message)) {
          if (0 == o_strncmp(message, "!q", o_strlen("!q"))) {
            if (!config->quiet) {
              fprintf(stdout, "\b\bQuit uwsc\n");
            }
            if (ulfius_websocket_send_message(websocket_manager, U_WEBSOCKET_OPCODE_CLOSE, 0, NULL) != U_OK) {
              y_log_message(Y_LOG_LEVEL_ERROR, "Error sending close message");
            }
          } else {
            if (!config->quiet) {
              fprintf(stdout, "\b\bSend '%.*s'\n> ", (int)(o_strlen(message)-1), message);
              fflush(stdout);
            }
            if (ulfius_websocket_send_message(websocket_manager, U_WEBSOCKET_OPCODE_TEXT, o_strlen(message)-1, message) != U_OK) {
              y_log_message(Y_LOG_LEVEL_ERROR, "Error sending message '%.*s'", (int)(o_strlen(message)-1), message);
            } else {
              last_message = ulfius_websocket_pop_first_message(websocket_manager->message_list_outcoming);
              if (last_message != NULL) {
                ulfius_clear_websocket_message(last_message);
              }
            }
          }
        }
      }
    }
  } while (o_strncmp(message, "!q", o_strlen("!q")) != 0 && ulfius_websocket_status(websocket_manager) == U_WEBSOCKET_STATUS_OPEN);
}

static void uwsc_manager_incoming (const struct _u_request * request, struct _websocket_manager * websocket_manager, const struct _websocket_message * message, void * websocket_incoming_user_data) {
  struct _websocket_message * last_message;
  struct _config * config = (struct _config *)websocket_incoming_user_data;

  if (!config->non_listening) {
    if (message->opcode == U_WEBSOCKET_OPCODE_TEXT) {
      if (!config->quiet) {
        fprintf(stdout, "\b\bServer message: '%.*s'\n> ", (int)message->data_len, message->data);
        fflush(stdout);
      } else {
        fprintf(stdout, "\b\b%.*s\n", (int)message->data_len, message->data);
      }
    } else if (message->opcode == U_WEBSOCKET_OPCODE_BINARY) {
      if (!config->quiet) {
        fprintf(stdout, "\b\bServer sent binary message, length %zu\n> ", message->data_len);
        fflush(stdout);
      } else {
        fprintf(stdout, "\b\bServer sent binary message, length %zu\n", message->data_len);
      }
    }
  }
  last_message = ulfius_websocket_pop_first_message(websocket_manager->message_list_incoming);
  if (last_message != NULL) {
    ulfius_clear_websocket_message(last_message);
  }
}

static void print_help(FILE * output) {
  fprintf(output, "\nuwsc - Ulfius Websocket Client\n");
  fprintf(output, "\n");
  fprintf(output, "Version %s\n", _UWSC_VERSION_);
  fprintf(output, "\n");
  fprintf(output, "Copyright 2018-2020 Nicolas Mora <mail@babelouest.org>\n");
  fprintf(output, "\n");
  fprintf(output, "This program is free software; you can redistribute it and/or\n");
  fprintf(output, "modify it under the terms of the MIT LICENSE\n");
  fprintf(output, "\n");
  fprintf(output, "Command-line options:\n");
  fprintf(output, "\n");
  fprintf(output, "-o --output-log-file=PATH\n");
  fprintf(output, "\tPrint yder log messages in a file\n");
  fprintf(output, "-x --add-header=HEADER\n");
  fprintf(output, "\tAdd the specified header of the form 'key:value'\n");
  fprintf(output, "-b --send-binary-file=PATH\n");
  fprintf(output, "\tSends the content of a file at the beginning of the connection in a binary frame\n");
  fprintf(output, "-t --send-text-file=PATH\n");
  fprintf(output, "\tSends the content of a file at the beginning of the connection in a text frame\n");
  fprintf(output, "-i --non-interactive-send\n");
  fprintf(output, "\tDo not prompt for message sending\n");
  fprintf(output, "-l --non-listening\n");
  fprintf(output, "\tDo not listen for incoming messages\n");
  fprintf(output, "-f --fragmentation=\n");
  fprintf(output, "\tSpecify the max length of a frame and fragment the message to send if required, 0 means no fragmentation, default 0\n");
  fprintf(output, "-p --protocol=PROTOCOL\n");
  fprintf(output, "\tSpecify the Websocket Protocol values, default none\n");
  fprintf(output, "-e --extensions=EXTENSION\n");
  fprintf(output, "\tSpecify the Websocket extensions values, default none\n");
  fprintf(output, "-s --non-secure\n");
  fprintf(output, "\tDo not check server certificate\n");
  fprintf(output, "-q --quiet\n");
  fprintf(output, "\tQuiet mode, show only websocket messages\n");
  fprintf(output, "-v --version\n");
  fprintf(output, "\tPrint uwsc's current version\n\n");
  fprintf(output, "-h --help\n");
  fprintf(output, "\tPrint this message\n\n");
}

static int init_config(struct _config * config) {
  if (config != NULL) {
    config->log_path = NULL;
    config->binary_file_send = NULL;
    config->text_file_send = NULL;
    config->non_interactive = 0;
    config->non_listening = 0;
    config->quiet = 0;
    config->fragmentation = 0;
    config->protocol = NULL;
    config->extensions = NULL;
    config->request = NULL;
    config->response = NULL;
    if ((config->request = o_malloc(sizeof(struct _u_request))) == NULL) {
      return 0;
    }
    if ((config->response = o_malloc(sizeof(struct _u_response))) == NULL) {
      return 0;
    }
    if (ulfius_init_request(config->request) != U_OK) {
      return 0;
    }
    if (ulfius_init_response(config->response) != U_OK) {
      return 0;
    }
    return 1;
  } else {
    return 0;
  }
}

static void exit_program(struct _config ** config, int exit_value) {
  if (config != NULL) {
    o_free((*config)->log_path);
    o_free((*config)->binary_file_send);
    o_free((*config)->text_file_send);
    o_free((*config)->protocol);
    o_free((*config)->extensions);
    ulfius_clean_request_full((*config)->request);
    ulfius_clean_response_full((*config)->response);
    o_free(*config);
  }
  exit(exit_value);
}

int main (int argc, char ** argv) {
  struct _config * config;
  int next_option;
  const char * short_options = "o:x:b:t:i::l::f:p:e::s::q::v::h::";
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
    {"non-secure", no_argument, NULL, 's'},             // Do not check server certificate
    {"quiet", no_argument, NULL, 'v'},                  // Quiet mode
    {"version", no_argument, NULL, 'v'},                // Show version
    {"help", no_argument, NULL, 'h'},                   // print help
    {NULL, 0, NULL, 0}
  };
  char * url = NULL, * extensions = NULL;
  struct _websocket_client_handler websocket_client_handler = {NULL, NULL};

  config = o_malloc(sizeof(struct _config));
  if (config == NULL || !init_config(config)) {
    fprintf(stderr, "Error initialize configuration\n");
    exit_program(NULL, 1);
  }

  // TODO: Complete option manager
  do {
    char * key, * value;
    next_option = getopt_long(argc, argv, short_options, long_options, NULL);

    switch (next_option) {
      case 'o':
        config->log_path = o_strdup(optarg);
        break;
      case 'x':
        if (NULL != (value = o_strchr(optarg, ':'))) {
          key = o_strndup(optarg, (value-optarg));
          u_map_put(config->request->map_header, key, value+1);
          o_free(key);
        } else {
          fprintf(stderr, "Error, a header value must have the following format: <key>:<value>\n\nuse uwsc -h for help\n");
          exit_program(&config, 1);
          // Print help and exit
        }
        break;
      case 'b':
        if (config->text_file_send != NULL) {
          fprintf(stderr, "Error, you can't send a binary file and a text file\n\nuse uwsc -h for help\n");
          exit_program(&config, 1);
        }
        config->binary_file_send = o_strdup(optarg);
        break;
      case 't':
        if (config->binary_file_send != NULL) {
          fprintf(stderr, "Error, you can't send a binary file and a text file\n\nuse uwsc -h for help\n");
          exit_program(&config, 1);
        }
        config->text_file_send = o_strdup(optarg);
        break;
      case 'i':
        config->non_interactive = 1;
        break;
      case 'l':
        config->non_listening = 1;
        break;
      case 'f':
        config->fragmentation = strtol(optarg, NULL, 10);
        break;
      case 'p':
        config->protocol = o_strdup(optarg);
        break;
      case 'e':
        config->extensions = o_strdup(optarg);
        break;
      case 's':
        config->request->check_server_certificate = 0;
        break;
      case 'q':
        config->quiet = 1;
        break;
      case 'v':
        // Print version and exit
        fprintf(stdout, "%s\n", _UWSC_VERSION_);
        exit_program(&config, 0);
        break;
      case 'h':
        // Print help and exit
        print_help(stdout);
        exit_program(&config, 0);
        break;
      default:
        break;
    }
  } while (next_option != -1);

  if (config->log_path != NULL) {
    y_init_logs("Ulfius Websocket Client", Y_LOG_MODE_FILE, Y_LOG_LEVEL_DEBUG, config->log_path, "Start uwsc");
  }

  if (optind < argc) {
    url = argv[optind];
  } else {
    fprintf(stderr, "No url specified\n\n");
    print_help(stderr);
    exit_program(&config, 1);
  }

  if (!o_strlen(config->extensions)) {
    extensions = o_strdup("permessage-deflate");
  } else {
    extensions = msprintf("%s,permessage-deflate", config->extensions);
  }
  if (ulfius_set_websocket_request(config->request, url, config->protocol, extensions) == U_OK) {
    if (ulfius_add_websocket_client_deflate_extension(&websocket_client_handler) == U_OK) {
      if (ulfius_open_websocket_client_connection(config->request, &uwsc_manager_callback, config, &uwsc_manager_incoming, config, NULL, NULL, &websocket_client_handler, config->response) == U_OK) {
        if (!config->quiet) {
          fprintf(stdout, "Websocket connected, you can send text messages of maximum 256 characters.\nTo exit uwsc, type !q<enter>\n> ");
          fflush(stdout);
        }
        ulfius_websocket_client_connection_wait_close(&websocket_client_handler, 0);
        if (!config->quiet) {
          fprintf(stdout, "Websocket closed\n");
        }
        ulfius_websocket_client_connection_close(&websocket_client_handler);
      } else {
        fprintf(stderr, "Error connecting to websocket\n");
        y_log_message(Y_LOG_LEVEL_ERROR, "Error connecting to websocket");
      }
    } else {
      fprintf(stderr, "Error initializing websocket request\n");
      y_log_message(Y_LOG_LEVEL_ERROR, "Error initializing websocket request");
    }
  } else {
    fprintf(stderr, "Error ulfius_add_websocket_client_deflate_extension\n");
    y_log_message(Y_LOG_LEVEL_ERROR, "Error initializing websocket request");
  }
  o_free(extensions);

  if (config->log_path != NULL) {
    y_close_logs();
  }
  exit_program(&config, 0);
  return 0;
}

#else

#include <stdio.h>

int main() {
  fprintf(stderr, "Websocket not supported in your Ulfius library, you must rebuild ulfius and uwsc with Websocket support\n");
  return 1;
}

#endif // U_DISABLE_WEBSOCKET
