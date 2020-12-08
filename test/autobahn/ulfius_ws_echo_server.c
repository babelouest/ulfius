/* Public domain, no copyright. Use at your own risk. */

#include <signal.h>
#include <string.h>
#include <ulfius.h>

#define PORT_HTTP  9010
#define PORT_HTTPS 9011

static pthread_mutex_t global_handler_close_lock;
static pthread_cond_t  global_handler_close_cond;

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

void websocket_echo_message_callback (const struct _u_request * request,
                                      struct _websocket_manager * websocket_manager,
                                      const struct _websocket_message * last_message,
                                      void * websocket_incoming_message_user_data) {
  if (ulfius_websocket_send_message(websocket_manager, last_message->opcode, last_message->data_len, last_message->data) != U_OK) {
    y_log_message(Y_LOG_LEVEL_ERROR, "Error sending echo message");
  }
}

int callback_websocket_echo (const struct _u_request * request, struct _u_response * response, void * user_data) {
  y_log_message(Y_LOG_LEVEL_INFO, "Client connected to echo websocket");
  if (ulfius_set_websocket_response(response, NULL, NULL, NULL, NULL, &websocket_echo_message_callback, NULL, NULL, NULL) == U_OK) {
    ulfius_add_websocket_deflate_extension(response);
    return U_CALLBACK_CONTINUE;
  } else {
    return U_CALLBACK_ERROR;
  }
}

void* signal_thread(void *arg) {
  sigset_t *sigs = arg;
  int res, signum;

  res = sigwait(sigs, &signum);
  if (res) {
    fprintf(stderr, "Glewlwyd - Waiting for signals failed\n");
    exit(1);
  }
  if (signum == SIGQUIT || signum == SIGINT || signum == SIGTERM || signum == SIGHUP) {
    y_log_message(Y_LOG_LEVEL_INFO, "Glewlwyd - Received close signal: %s", strsignal(signum));
    pthread_mutex_lock(&global_handler_close_lock);
    pthread_cond_signal(&global_handler_close_cond);
    pthread_mutex_unlock(&global_handler_close_lock);
    return NULL;
  } else if (signum == SIGBUS) {
    fprintf(stderr, "Glewlwyd - Received bus error signal\n");
    exit(256-signum);
  } else if (signum == SIGSEGV) {
    fprintf(stderr, "Glewlwyd - Received segmentation fault signal\n");
    exit(256-signum);
  } else if (signum == SIGILL) {
    fprintf(stderr, "Glewlwyd - Received illegal instruction signal\n");
    exit(256-signum);
  } else {
    y_log_message(Y_LOG_LEVEL_WARNING, "Glewlwyd - Received unexpected signal: %s", strsignal(signum));
  }

  return NULL;
}

int main(int argc, char ** argv) {
  struct _u_instance instance_http, instance_https;
  char * key_file, * cert_file;
  pthread_t signal_thread_id;
  static sigset_t close_signals;

  y_init_logs("websocket_example", Y_LOG_MODE_FILE, Y_LOG_LEVEL_DEBUG, "ulfius_ws_echo_server.log", "Starting websocket echo for autobahn testsuite");

  if (pthread_mutex_init(&global_handler_close_lock, NULL) || 
      pthread_cond_init(&global_handler_close_cond, NULL)) {
    y_log_message(Y_LOG_LEVEL_ERROR, "Error initializing global_handler_close_lock or global_handler_close_cond");
    return 1;
  }

  // Process end signals on dedicated thread
  if (sigemptyset(&close_signals) == -1 ||
      sigaddset(&close_signals, SIGQUIT) == -1 ||
      sigaddset(&close_signals, SIGINT) == -1 ||
      sigaddset(&close_signals, SIGTERM) == -1 ||
      sigaddset(&close_signals, SIGHUP) == -1 ||
      sigaddset(&close_signals, SIGBUS) == -1 ||
      sigaddset(&close_signals, SIGSEGV) == -1 ||
      sigaddset(&close_signals, SIGILL) == -1) {
    fprintf(stderr, "init - Error creating signal mask\n");
    return 1;
  }
  if (pthread_sigmask(SIG_BLOCK, &close_signals, NULL)) {
    fprintf(stderr, "init - Error setting signal mask\n");
    return 1;
  }
  if (pthread_sigmask(SIG_BLOCK, &close_signals, NULL)) {
    fprintf(stderr, "init - Error setting signal mask\n");
    return 1;
  }
  if (pthread_create(&signal_thread_id, NULL, &signal_thread, &close_signals)) {
    fprintf(stderr, "init - Error creating signal thread\n");
    return 1;
  }

  if (ulfius_init_instance(&instance_http, PORT_HTTP, NULL, NULL) != U_OK) {
    y_log_message(Y_LOG_LEVEL_ERROR, "Error ulfius_init_instance http, abort");
    return 1;
  }

  if (ulfius_init_instance(&instance_https, PORT_HTTPS, NULL, NULL) != U_OK) {
    y_log_message(Y_LOG_LEVEL_ERROR, "Error ulfius_init_instance https, abort");
    return 1;
  }

  ulfius_add_endpoint_by_val(&instance_http, "GET", NULL, "*", 0, &callback_websocket_echo, NULL);
  ulfius_start_framework(&instance_http);
  y_log_message(Y_LOG_LEVEL_INFO, "Start http websocket echo server on port %d", instance_http.port);

  if (argc > 2) {
    key_file = read_file(argv[1]);
    cert_file = read_file(argv[2]);
    if (key_file != NULL && cert_file != NULL) {
      ulfius_add_endpoint_by_val(&instance_https, "GET", NULL, "*", 0, &callback_websocket_echo, NULL);
      ulfius_start_secure_framework(&instance_https, key_file, cert_file);
      y_log_message(Y_LOG_LEVEL_INFO, "Start https websocket echo server on port %d", instance_https.port);
      o_free(key_file);
      o_free(cert_file);
      pthread_mutex_lock(&global_handler_close_lock);
      pthread_cond_wait(&global_handler_close_cond, &global_handler_close_lock);
      pthread_mutex_unlock(&global_handler_close_lock);
      ulfius_stop_framework(&instance_https);
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "Error https key or certificate file");
    }
  } else {
    pthread_mutex_lock(&global_handler_close_lock);
    pthread_cond_wait(&global_handler_close_cond, &global_handler_close_lock);
    pthread_mutex_unlock(&global_handler_close_lock);
  }
  ulfius_clean_instance(&instance_https);
  ulfius_stop_framework(&instance_http);
  ulfius_clean_instance(&instance_http);
  y_log_message(Y_LOG_LEVEL_INFO, "Stop http websocket echo server");
  y_close_logs();
}
