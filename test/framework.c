/* Public domain, no copyright. Use at your own risk. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <ctype.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>

#ifndef _WIN32
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <netdb.h>
#else
  #include <unistd.h>
#endif
#include <inttypes.h>

#include <check.h>
#include <ulfius.h>

#define SMTP_FROM "sender@localhost"
#define SMTP_TO "recipient@localhost"
#define SMTP_HOST "localhost"
#define SMTP_PORT 2525
#define SMTP_SUBJECT "E-mail subject for the test"
#define SMTP_BODY "E-mail body for the test as well"

#define BACKLOG_MAX  (10)
#define BUF_SIZE  4096
#define STREQU(a,b)  (strcmp(a, b) == 0)

#define BODY_NOT_REDIRECTED "This is the blue pill"
#define BODY_REDIRECTED "Welcome to the Matrix, Neo!"

struct smtp_manager {
  char * mail_data;
  unsigned int port;
  int sockfd;
};

/**
 * 
 * Function that emulates a very simple SMTP server
 * Taken from Kenneth Finnegan's ccsmtp program
 * https://gist.github.com/PhirePhly/2914635
 * This function is under the GPL2 license
 * 
 */
static void handle_smtp (struct smtp_manager * manager) {
  int rc, i;
  char buffer[BUF_SIZE], bufferout[BUF_SIZE];
  int buffer_offset = 0;
  buffer[BUF_SIZE-1] = '\0';

  // Flag for being inside of DATA verb
  int inmessage = 0;

  sprintf(bufferout, "220 ulfius.tld SMTP CCSMTP\r\n");
  send(manager->sockfd, bufferout, strlen(bufferout), 0);
  //manager->mail_data = mstrcatf(manager->mail_data, bufferout);

  while (1) {
    fd_set sockset;
    struct timeval tv;

    FD_ZERO(&sockset);
    FD_SET(manager->sockfd, &sockset);
    tv.tv_sec = 120; // Some SMTP servers pause for ~15s per message
    tv.tv_usec = 0;

    // Wait tv timeout for the server to send anything.
    select(manager->sockfd+1, &sockset, NULL, NULL, &tv);

    if (!FD_ISSET(manager->sockfd, &sockset)) {
      y_log_message(Y_LOG_LEVEL_DEBUG, "%d: Socket timed out", manager->sockfd);
      break;
    }

    int buffer_left = BUF_SIZE - buffer_offset - 1;
    if (buffer_left == 0) {
      y_log_message(Y_LOG_LEVEL_DEBUG, "%d: Command line too long", manager->sockfd);
      sprintf(bufferout, "500 Too long\r\n");
      send(manager->sockfd, bufferout, strlen(bufferout), 0);
      //manager->mail_data = mstrcatf(manager->mail_data, bufferout);
      buffer_offset = 0;
      continue;
    }

    rc = recv(manager->sockfd, buffer + buffer_offset, buffer_left, 0);
    if (rc == 0) {
      y_log_message(Y_LOG_LEVEL_DEBUG, "%d: Remote host closed socket", manager->sockfd);
      break;
    }
    if (rc == -1) {
      y_log_message(Y_LOG_LEVEL_DEBUG, "%d: Error on socket", manager->sockfd);
      break;
    }

    buffer_offset += rc;

    char *eol;

    // Only process one line of the received buffer at a time
    // If multiple lines were received in a single recv(), goto
    // back to here for each line
    //
processline:
    eol = strstr(buffer, "\r\n");
    if (eol == NULL) {
      y_log_message(Y_LOG_LEVEL_DEBUG, "%d: Haven't found EOL yet", manager->sockfd);
      continue;
    }

    // Null terminate each line to be processed individually
    eol[0] = '\0';

    if (!inmessage) { // Handle system verbs
      manager->mail_data = mstrcatf(manager->mail_data, "%s\n", buffer);
      // Replace all lower case letters so verbs are all caps
      for (i=0; i<4; i++) {
        if (islower(buffer[i])) {
          buffer[i] += 'A' - 'a';
        }
      }
      // Null-terminate the verb for strcmp
      buffer[4] = '\0';

      // Respond to each verb accordingly.
      // You should replace these with more meaningful
      // actions than simply printing everything.
      //
      if (STREQU(buffer, "HELO")) { // Initial greeting
        sprintf(bufferout, "250 Ok\r\n");
        send(manager->sockfd, bufferout, strlen(bufferout), 0);
        //manager->mail_data = mstrcatf(manager->mail_data, bufferout);
      } else if (STREQU(buffer, "MAIL")) { // New mail from...
        sprintf(bufferout, "250 Ok\r\n");
        send(manager->sockfd, bufferout, strlen(bufferout), 0);
        //manager->mail_data = mstrcatf(manager->mail_data, bufferout);
      } else if (STREQU(buffer, "RCPT")) { // Mail addressed to...
        sprintf(bufferout, "250 Ok recipient\r\n");
        send(manager->sockfd, bufferout, strlen(bufferout), 0);
        //manager->mail_data = mstrcatf(manager->mail_data, bufferout);
      } else if (STREQU(buffer, "DATA")) { // Message contents...
        sprintf(bufferout, "354 Continue\r\n");
        send(manager->sockfd, bufferout, strlen(bufferout), 0);
        //manager->mail_data = mstrcatf(manager->mail_data, bufferout);
        inmessage = 1;
      } else if (STREQU(buffer, "RSET")) { // Reset the connection
        sprintf(bufferout, "250 Ok reset\r\n");
        send(manager->sockfd, bufferout, strlen(bufferout), 0);
        //manager->mail_data = mstrcatf(manager->mail_data, bufferout);
      } else if (STREQU(buffer, "NOOP")) { // Do nothing.
        sprintf(bufferout, "250 Ok noop\r\n");
        send(manager->sockfd, bufferout, strlen(bufferout), 0);
        //manager->mail_data = mstrcatf(manager->mail_data, bufferout);
      } else if (STREQU(buffer, "QUIT")) { // Close the connection
        sprintf(bufferout, "221 Ok\r\n");
        send(manager->sockfd, bufferout, strlen(bufferout), 0);
        //manager->mail_data = mstrcatf(manager->mail_data, bufferout);
        break;
      } else { // The verb used hasn't been implemented.
        sprintf(bufferout, "502 Command Not Implemented\r\n");
        send(manager->sockfd, bufferout, strlen(bufferout), 0);
        //manager->mail_data = mstrcatf(manager->mail_data, bufferout);
      }
    } else { // We are inside the message after a DATA verb.
      manager->mail_data = mstrcatf(manager->mail_data, "%s\n", buffer);
      if (STREQU(buffer, ".")) { // A single "." signifies the end
        sprintf(bufferout, "250 Ok\r\n");
        send(manager->sockfd, bufferout, strlen(bufferout), 0);
        //manager->mail_data = mstrcatf(manager->mail_data, bufferout);
        inmessage = 0;
      }
    }

    // Shift the rest of the buffer to the front
    memmove(buffer, eol+2, BUF_SIZE - (eol + 2 - buffer));
    buffer_offset -= (eol - buffer) + 2;

    // Do we already have additional lines to process? If so,
    // commit a horrid sin and goto the line processing section again.
    if (strstr(buffer, "\r\n")) 
      goto processline;
  }

  // All done. Clean up everything and exit.
  close(manager->sockfd);
}

static void * simple_smtp(void * args) {
  struct smtp_manager * manager = (struct smtp_manager *)args;
  int server_fd; 
  struct sockaddr_in address; 
  int opt = 1; 
  int addrlen = sizeof(address); 
  
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) != 0) {
    if (!setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
      address.sin_family = AF_INET;
      address.sin_addr.s_addr = INADDR_ANY;
      address.sin_port = htons( manager->port );
         
      if (!bind(server_fd, (struct sockaddr *)&address, sizeof(address))) {
        if (listen(server_fd, 3) >= 0) {
          if ((manager->sockfd = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) >= 0) {
            handle_smtp(manager);
          } else {
            y_log_message(Y_LOG_LEVEL_ERROR, "simple_smtp - Error accept");
          }
        } else {
          y_log_message(Y_LOG_LEVEL_ERROR, "simple_smtp - Error listen");
        }
      } else {
        y_log_message(Y_LOG_LEVEL_ERROR, "simple_smtp - Error bind");
      }
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "simple_smtp - Error setsockopt");
    }
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "simple_smtp - Error socket");
  }

  pthread_exit(NULL);
}

static char * get_file_content(const char * file_path) {
  char * buffer = NULL;
  size_t length, res;
  FILE * f;

  f = fopen (file_path, "rb");
  if (f) {
    fseek (f, 0, SEEK_END);
    length = ftell (f);
    fseek (f, 0, SEEK_SET);
    buffer = o_malloc((length+1)*sizeof(char));
    if (buffer) {
      res = fread (buffer, 1, length, f);
      if (res != length) {
        fprintf(stderr, "fread warning, reading %zu while expecting %zu", res, length);
      }
      // Add null character at the end of buffer, just in case
      buffer[length] = '\0';
    }
    fclose (f);
  } else {
    fprintf(stderr, "error opening file %s\n", file_path);
  }
  
  return buffer;
}

/**
 * decode a u_map into a string
 */
char * print_map(const struct _u_map * map) {
  char * line, * to_return = NULL;
  const char **keys, * value;
  int len, i;
  if (map != NULL) {
    keys = u_map_enum_keys(map);
    for (i=0; keys[i] != NULL; i++) {
      value = u_map_get(map, keys[i]);
      len = snprintf(NULL, 0, "key is %s, value is %s", keys[i], value);
      line = o_malloc((len+1)*sizeof(char));
      snprintf(line, (len+1), "key is %s, value is %s", keys[i], value);
      if (to_return != NULL) {
        len = o_strlen(to_return) + o_strlen(line) + 1;
        to_return = o_realloc(to_return, (len+1)*sizeof(char));
        if (o_strlen(to_return) > 0) {
          strcat(to_return, "\n");
        }
      } else {
        to_return = o_malloc((o_strlen(line) + 1)*sizeof(char));
        to_return[0] = 0;
      }
      strcat(to_return, line);
      o_free(line);
    }
    return to_return;
  } else {
    return NULL;
  }
}

int callback_function_empty(const struct _u_request * request, struct _u_response * response, void * user_data) {
  return U_CALLBACK_CONTINUE;
}

int callback_function_return_url(const struct _u_request * request, struct _u_response * response, void * user_data) {
  ulfius_set_string_body_response(response, 200, request->http_url);
  return U_CALLBACK_CONTINUE;
}

int callback_function_error(const struct _u_request * request, struct _u_response * response, void * user_data) {
  return U_CALLBACK_ERROR;
}

int callback_function_unauthorized(const struct _u_request * request, struct _u_response * response, void * user_data) {
  return U_CALLBACK_UNAUTHORIZED;
}

int callback_function_check_auth(const struct _u_request * request, struct _u_response * response, void * user_data) {
  if (o_strcmp("user", request->auth_basic_user) == 0 && o_strcmp("password", request->auth_basic_password) == 0) {
    return U_CALLBACK_CONTINUE;
  } else {
    return U_CALLBACK_UNAUTHORIZED;
  }
}

int callback_function_param(const struct _u_request * request, struct _u_response * response, void * user_data) {
  char * param3, * body;
  
  if (u_map_has_key(request->map_url, "param3")) {
    param3 = msprintf(", param3 is %s", u_map_get(request->map_url, "param3"));
  } else {
    param3 = o_strdup("");
  }
  body = msprintf("param1 is %s, param2 is %s%s", u_map_get(request->map_url, "param1"), u_map_get(request->map_url, "param2"), param3);
  ulfius_set_string_body_response(response, 200, body);
  o_free(body);
  o_free(param3);
  return U_CALLBACK_CONTINUE;
}

int callback_function_body_param(const struct _u_request * request, struct _u_response * response, void * user_data) {
  char * body;
  
  body = msprintf("param1 is %s, param2 is %s", u_map_get(request->map_post_body, "param1"), u_map_get(request->map_post_body, "param2"));
  ulfius_set_string_body_response(response, 200, body);
  o_free(body);
  return U_CALLBACK_CONTINUE;
}

int callback_function_header_param(const struct _u_request * request, struct _u_response * response, void * user_data) {
  char * body;
  
  body = msprintf("param1 is %s, param2 is %s", u_map_get(request->map_header, "param1"), u_map_get(request->map_header, "param2"));
  ulfius_set_string_body_response(response, 200, body);
  o_free(body);
  return U_CALLBACK_CONTINUE;
}

int callback_function_cookie_param(const struct _u_request * request, struct _u_response * response, void * user_data) {
  char * body;
  
  body = msprintf("param1 is %s", u_map_get(request->map_cookie, "param1"));
  ck_assert_int_eq(ulfius_set_string_body_response(response, 200, body), U_OK);
  ck_assert_int_eq(ulfius_add_cookie_to_response(response, "param2", "value_cookie", NULL, 100, "localhost", "/cookie", 0, 1), U_OK);
  ck_assert_int_eq(ulfius_add_same_site_cookie_to_response(response, "cookieSameSiteStrict", "value_cookie", NULL, 100, "localhost", "/cookie", 0, 1, U_COOKIE_SAME_SITE_STRICT), U_OK);
  ck_assert_int_eq(ulfius_add_same_site_cookie_to_response(response, "cookieSameSiteLax", "value_cookie", NULL, 100, "localhost", "/cookie", 0, 1, U_COOKIE_SAME_SITE_LAX), U_OK);
  ck_assert_int_eq(ulfius_add_same_site_cookie_to_response(response, "cookieSameSiteNone", "value_cookie", NULL, 100, "localhost", "/cookie", 0, 1, U_COOKIE_SAME_SITE_NONE), U_OK);
  ck_assert_int_ne(ulfius_add_same_site_cookie_to_response(response, "cookieSameSiteError", "value_cookie", NULL, 100, "localhost", "/cookie", 0, 1, 42), U_OK);
  o_free(body);
  return U_CALLBACK_CONTINUE;
}

int callback_function_multiple_continue(const struct _u_request * request, struct _u_response * response, void * user_data) {
  if (response->binary_body != NULL) {
    char * body = msprintf("%.*s\n%s", response->binary_body_length, (char*)response->binary_body, request->http_url);
    ulfius_set_string_body_response(response, 200, body);
    o_free(body);
  } else {
    ulfius_set_string_body_response(response, 200, request->http_url);
  }
  return U_CALLBACK_CONTINUE;
}

int callback_function_multiple_complete(const struct _u_request * request, struct _u_response * response, void * user_data) {
  if (response->binary_body != NULL) {
    char * body = msprintf("%.*s\n%s", response->binary_body_length, (char*)response->binary_body, request->http_url);
    ulfius_set_string_body_response(response, 200, body);
    o_free(body);
  } else {
    ulfius_set_string_body_response(response, 200, request->http_url);
  }
  return U_CALLBACK_COMPLETE;
}

ssize_t stream_data (void * cls, uint64_t pos, char * buf, size_t max) {
  usleep(100);
  if (pos <= 100) {
      snprintf(buf, max, "%s %" PRIu64 "\n", (char *)cls, pos + 1);
      return o_strlen(buf);
  } else {
    return MHD_CONTENT_READER_END_OF_STREAM;
  }
}

void free_stream_data(void * cls) {
  o_free(cls);
}

int callback_function_stream (const struct _u_request * request, struct _u_response * response, void * user_data) {
  ulfius_set_stream_response(response, 200, stream_data, free_stream_data, U_STREAM_SIZE_UNKOWN, 32 * 1024, o_strdup("stream test"));
  return U_CALLBACK_CONTINUE;
}

size_t my_write_body(void * contents, size_t size, size_t nmemb, void * user_data) {
  ck_assert_int_eq(o_strncmp((char *)contents, "stream test ", o_strlen("stream test ")), 0);
  ck_assert_int_ne(strtol((char *)contents + o_strlen("stream test "), NULL, 10), 0);
  return size * nmemb;
}

int callback_check_utf8_ignored(const struct _u_request * request, struct _u_response * response, void * user_data) {
  ck_assert_int_eq(u_map_has_key(request->map_header, "utf8_param"), 0);
  ck_assert_int_eq(u_map_has_key(request->map_url, "utf8_param1"), 0);
  ck_assert_int_eq(u_map_has_key(request->map_url, "utf8_param2"), 0);
  ck_assert_int_eq(u_map_has_key(request->map_post_body, "utf8_param"), 0);
  ck_assert_int_eq(u_map_has_key(request->map_header, "utf8_param_valid"), 1);
  ck_assert_int_eq(u_map_has_key(request->map_url, "utf8_param_valid1"), 1);
  ck_assert_int_eq(u_map_has_key(request->map_url, "utf8_param_valid2"), 1);
  ck_assert_int_eq(u_map_has_key(request->map_post_body, "utf8_param_valid"), 1);
  return U_CALLBACK_CONTINUE;
}

int callback_check_utf8_not_ignored(const struct _u_request * request, struct _u_response * response, void * user_data) {
  ck_assert_int_eq(u_map_has_key(request->map_header, "utf8_param"), 1);
  ck_assert_int_eq(u_map_has_key(request->map_url, "utf8_param1"), 1);
  ck_assert_int_eq(u_map_has_key(request->map_url, "utf8_param2"), 1);
  ck_assert_int_eq(u_map_has_key(request->map_post_body, "utf8_param"), 1);
  ck_assert_int_eq(u_map_has_key(request->map_header, "utf8_param_valid"), 1);
  ck_assert_int_eq(u_map_has_key(request->map_url, "utf8_param_valid1"), 1);
  ck_assert_int_eq(u_map_has_key(request->map_url, "utf8_param_valid2"), 1);
  ck_assert_int_eq(u_map_has_key(request->map_post_body, "utf8_param_valid"), 1);
  return U_CALLBACK_CONTINUE;
}

int callback_function_position_0(const struct _u_request * request, struct _u_response * response, void * user_data) {
  ck_assert_int_eq(request->callback_position, 0);
  return U_CALLBACK_CONTINUE;
}

int callback_function_position_1(const struct _u_request * request, struct _u_response * response, void * user_data) {
  ck_assert_int_eq(request->callback_position, 1);
  return U_CALLBACK_CONTINUE;
}

int callback_function_position_2(const struct _u_request * request, struct _u_response * response, void * user_data) {
  ck_assert_int_eq(request->callback_position, 2);
  return U_CALLBACK_CONTINUE;
}

int callback_function_redirect_first(const struct _u_request * request, struct _u_response * response, void * user_data) {
  u_map_put(response->map_header, "Location", "/last");
  ulfius_set_string_body_response(response, 302, BODY_NOT_REDIRECTED);
  return U_CALLBACK_CONTINUE;
}

int callback_function_redirect_last(const struct _u_request * request, struct _u_response * response, void * user_data) {
  ulfius_set_string_body_response(response, 200, BODY_REDIRECTED);
  return U_CALLBACK_CONTINUE;
}

int via_free_with_test = 0;

void free_with_test(void * ptr) {
  via_free_with_test++;
  free(ptr);
}

#ifndef U_DISABLE_GNUTLS
int callback_auth_client_cert (const struct _u_request * request, struct _u_response * response, void * user_data) {
  char * dn;
  size_t lbuf = 0;
  
  ck_assert_ptr_ne(request->client_cert, NULL);
  gnutls_x509_crt_get_dn(request->client_cert, NULL, &lbuf);
  dn = o_malloc(lbuf + 1);
  gnutls_x509_crt_get_dn(request->client_cert, dn, &lbuf);
  dn[lbuf] = '\0';
  ck_assert_ptr_ne(o_strstr(dn, "C=CA"), NULL);
  ck_assert_ptr_ne(o_strstr(dn, "ST=Quebec"), NULL);
  ck_assert_ptr_ne(o_strstr(dn, "L=Quebec"), NULL);
  ck_assert_ptr_ne(o_strstr(dn, "O=Ulfius"), NULL);
  ck_assert_ptr_ne(o_strstr(dn, "OU=test-client"), NULL);
  ck_assert_ptr_ne(o_strstr(dn, "CN=localhost"), NULL);
  ck_assert_ptr_ne(o_strstr(dn, "EMAIL=webmaster@localhost"), NULL);
  ck_assert_int_eq(ulfius_set_string_body_response(response, 200, dn), U_OK);
  o_free(dn);

  return U_CALLBACK_CONTINUE;
}

int callback_no_auth_client_cert (const struct _u_request * request, struct _u_response * response, void * user_data) {
  ck_assert_ptr_eq(request->client_cert, NULL);
  
  return U_CALLBACK_CONTINUE;
}
#endif

START_TEST(test_ulfius_simple_endpoint)
{
  struct _u_instance u_instance;
  struct _u_request request;
  struct _u_response response;
  
  ck_assert_int_eq(ulfius_init_instance(&u_instance, 8080, NULL, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "empty", NULL, 0, &callback_function_empty, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "POST", "empty", NULL, 0, &callback_function_empty, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "PUT", "empty", NULL, 0, &callback_function_empty, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "DELETE", "empty", NULL, 0, &callback_function_empty, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "url", NULL, 0, &callback_function_return_url, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "*", "url", NULL, 0, &callback_function_return_url, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "error", NULL, 0, &callback_function_error, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "unauthorized", NULL, 0, &callback_function_unauthorized, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "check_auth", NULL, 0, &callback_function_check_auth, NULL), U_OK);
  ck_assert_int_eq(ulfius_start_framework(&u_instance), U_OK);
  
  ulfius_init_request(&request);
  request.http_url = o_strdup("http://localhost:8080/nope");
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 404);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  ulfius_init_request(&request);
  request.http_url = o_strdup("http://localhost:8080/empty");
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  ulfius_init_request(&request);
  request.http_verb = o_strdup("POST");
  request.http_url = o_strdup("http://localhost:8080/empty");
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  ulfius_init_request(&request);
  request.http_verb = o_strdup("PUT");
  request.http_url = o_strdup("http://localhost:8080/empty");
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  ulfius_init_request(&request);
  request.http_verb = o_strdup("DELETE");
  request.http_url = o_strdup("http://localhost:8080/empty");
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  ulfius_init_request(&request);
  request.http_verb = o_strdup("OPTION");
  request.http_url = o_strdup("http://localhost:8080/empty");
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 404);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  ulfius_init_request(&request);
  request.http_url = o_strdup("http://localhost:8080/url");
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_eq(o_strncmp(response.binary_body, "/url", o_strlen("/url")), 0);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  ulfius_init_request(&request);
  request.http_verb = o_strdup("OPTION");
  request.http_url = o_strdup("http://localhost:8080/url");
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_eq(o_strncmp(response.binary_body, "/url", o_strlen("/url")), 0);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  ulfius_init_request(&request);
  request.http_verb = o_strdup("test");
  request.http_url = o_strdup("http://localhost:8080/url");
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_eq(o_strncmp(response.binary_body, "/url", o_strlen("/url")), 0);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  ulfius_init_request(&request);
  request.http_url = o_strdup("http://localhost:8080/error");
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 500);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  ulfius_init_request(&request);
  request.http_url = o_strdup("http://localhost:8080/unauthorized");
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 401);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  ulfius_init_request(&request);
  request.http_url = o_strdup("http://localhost:8080/check_auth");
  request.auth_basic_user = o_strdup("nope");
  request.auth_basic_password = o_strdup("nope");
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 401);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  ulfius_init_request(&request);
  request.http_url = o_strdup("http://localhost:8080/check_auth");
  request.auth_basic_user = o_strdup("user");
  request.auth_basic_password = o_strdup("password");
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  ulfius_stop_framework(&u_instance);
  ulfius_clean_instance(&u_instance);
}
END_TEST

#if MHD_VERSION >= 0x00095208
START_TEST(test_ulfius_net_type_endpoint)
{
  struct _u_instance u_instance;
  struct _u_request request;
  struct _u_response response;
  struct sockaddr_in ipv4addr;
  struct sockaddr_in6 ipv6addr;
  
  // Test network accepting IPV4 and IPV6 connections
  ck_assert_int_eq(ulfius_init_instance_ipv6(&u_instance, 8080, NULL, U_USE_ALL, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "empty", NULL, 0, &callback_function_empty, NULL), U_OK);
  ck_assert_int_eq(ulfius_start_framework(&u_instance), U_OK);
  
  ulfius_init_request(&request);
  request.http_url = o_strdup("http://localhost:8080/empty");
  request.network_type = U_USE_IPV4;
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  ulfius_init_request(&request);
  request.http_url = o_strdup("http://[::1]:8080/empty");
  request.network_type = U_USE_IPV6;
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  ulfius_stop_framework(&u_instance);
  ulfius_clean_instance(&u_instance);
  
  // Test network accepting IPV6 only connections
  ck_assert_int_eq(ulfius_init_instance_ipv6(&u_instance, 8080, NULL, U_USE_IPV6, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "empty", NULL, 0, &callback_function_empty, NULL), U_OK);
  ck_assert_int_eq(ulfius_start_framework(&u_instance), U_OK);
  
  ulfius_init_request(&request);
  request.http_url = o_strdup("http://127.0.0.1:8080/empty");
  request.network_type = U_USE_IPV4;
  ck_assert_int_ne(ulfius_send_http_request(&request, NULL), U_OK);
  ulfius_clean_request(&request);
  
  ulfius_init_request(&request);
  request.http_url = o_strdup("http://[::1]:8080/empty");
  request.network_type = U_USE_IPV6;
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  ulfius_stop_framework(&u_instance);
  ulfius_clean_instance(&u_instance);
  
  // Test network accepting IPV4 only connections
  ck_assert_int_eq(ulfius_init_instance(&u_instance, 8080, NULL, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "empty", NULL, 0, &callback_function_empty, NULL), U_OK);
  ck_assert_int_eq(ulfius_start_framework(&u_instance), U_OK);
  
  ulfius_init_request(&request);
  request.http_url = o_strdup("http://localhost:8080/empty");
  request.network_type = U_USE_IPV4;
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  ulfius_init_request(&request);
  request.http_url = o_strdup("http://[::1]:8080/empty");
  request.network_type = U_USE_IPV6;
  ck_assert_int_ne(ulfius_send_http_request(&request, NULL), U_OK);
  ulfius_clean_request(&request);
  
  ulfius_stop_framework(&u_instance);
  ulfius_clean_instance(&u_instance);
  
  // Test network binding to an IPV4 address
  memset(&ipv4addr, 0, sizeof(ipv4addr));
  ipv4addr.sin_family = AF_INET;
  ipv4addr.sin_port = htons(8080);
  ipv4addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  ck_assert_int_eq(ulfius_init_instance(&u_instance, 8080, &ipv4addr, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "empty", NULL, 0, &callback_function_empty, NULL), U_OK);
  ck_assert_int_eq(ulfius_start_framework(&u_instance), U_OK);
  
  ulfius_init_request(&request);
  request.http_url = o_strdup("http://localhost:8080/empty");
  request.network_type = U_USE_IPV4;
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  ulfius_init_request(&request);
  request.http_url = o_strdup("http://[::1]:8080/empty");
  request.network_type = U_USE_IPV6;
  ck_assert_int_ne(ulfius_send_http_request(&request, NULL), U_OK);
  ulfius_clean_request(&request);
  
  ulfius_stop_framework(&u_instance);
  ulfius_clean_instance(&u_instance);
  
  // Test network binding to an IPV6 address
  memset(&ipv6addr, 0, sizeof(ipv6addr));
  ipv6addr.sin6_family = AF_INET6;
  ipv6addr.sin6_port = htons(8080);
  ipv6addr.sin6_addr = in6addr_loopback;
  ck_assert_int_eq(ulfius_init_instance_ipv6(&u_instance, 8080, &ipv6addr, U_USE_IPV6, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "empty", NULL, 0, &callback_function_empty, NULL), U_OK);
  ck_assert_int_eq(ulfius_start_framework(&u_instance), U_OK);

  ulfius_init_request(&request);
  request.http_url = o_strdup("http://127.0.0.1:8080/empty");
  request.network_type = U_USE_IPV4;
  ck_assert_int_ne(ulfius_send_http_request(&request, NULL), U_OK);
  ulfius_clean_request(&request);
  
  ulfius_init_request(&request);
  request.http_url = o_strdup("http://[::1]:8080/empty");
  request.network_type = U_USE_IPV6;
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  ulfius_stop_framework(&u_instance);
  ulfius_clean_instance(&u_instance);
}
END_TEST
#endif

START_TEST(test_ulfius_endpoint_parameters)
{
  struct _u_instance u_instance;
  struct _u_request request;
  struct _u_response response;
  const char * set_cookie;
  
  ck_assert_int_eq(ulfius_init_instance(&u_instance, 8080, NULL, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "param", "/:param1/@param2/", 0, &callback_function_param, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "param", "/:param1/@param2/:param3/:param3", 0, &callback_function_param, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "POST", "param", NULL, 0, &callback_function_body_param, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "header", NULL, 0, &callback_function_header_param, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "cookie", NULL, 0, &callback_function_cookie_param, NULL), U_OK);
  ck_assert_int_eq(ulfius_start_framework(&u_instance), U_OK);
  
  ulfius_init_request(&request);
  request.http_url = o_strdup("http://localhost:8080/param/value1/value2");
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_eq(o_strncmp(response.binary_body, "param1 is value1, param2 is value2", o_strlen("param1 is value1, param2 is value2")), 0);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  ulfius_init_request(&request);
  request.http_url = o_strdup("http://localhost:8080/param/value1/value2/value3.1/value3.2");
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_eq(o_strncmp(response.binary_body, "param1 is value1, param2 is value2, param3 is value3.1,value3.2", o_strlen("param1 is value1, param2 is value2, param3 is value3.1,value3.2")), 0);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  ulfius_init_request(&request);
  request.http_verb = o_strdup("POST");
  request.http_url = o_strdup("http://localhost:8080/param/");
  u_map_put(request.map_post_body, "param1", "value3");
  u_map_put(request.map_post_body, "param2", "value4");
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_eq(o_strncmp(response.binary_body, "param1 is value3, param2 is value4", o_strlen("param1 is value3, param2 is value4")), 0);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  ulfius_init_request(&request);
  request.http_url = o_strdup("http://localhost:8080/header");
  u_map_put(request.map_header, "param1", "value5");
  u_map_put(request.map_header, "param2", "value6");
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_eq(o_strncmp(response.binary_body, "param1 is value5, param2 is value6", o_strlen("param1 is value5, param2 is value6")), 0);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  ulfius_init_request(&request);
  request.http_url = o_strdup("http://localhost:8080/cookie");
  u_map_put(request.map_cookie, "param1", "value7");
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_eq(o_strncmp(response.binary_body, "param1 is value7", o_strlen("param1 is value7")), 0);

  set_cookie = u_map_get(response.map_header, "Set-Cookie");
  ck_assert_ptr_ne(o_strstr(set_cookie, "param2=value_cookie; Max-Age=100; Domain=localhost; Path=/cookie; HttpOnly"), NULL);
  ck_assert_ptr_ne(o_strstr(set_cookie, "cookieSameSiteStrict=value_cookie; Max-Age=100; Domain=localhost; Path=/cookie; HttpOnly; SameSite=Strict"), NULL);
  ck_assert_ptr_ne(o_strstr(set_cookie, "cookieSameSiteLax=value_cookie; Max-Age=100; Domain=localhost; Path=/cookie; HttpOnly; SameSite=Lax"), NULL);
  ck_assert_ptr_ne(o_strstr(set_cookie, "cookieSameSiteNone=value_cookie; Max-Age=100; Domain=localhost; Path=/cookie; HttpOnly"), NULL);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  ulfius_stop_framework(&u_instance);
  ulfius_clean_instance(&u_instance);
}
END_TEST

START_TEST(test_ulfius_endpoint_injection)
{
  struct _u_instance u_instance;
  struct _u_request request;
  struct _u_response response;
  
  ck_assert_int_eq(ulfius_init_instance(&u_instance, 8080, NULL, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "inject1", NULL, 0, &callback_function_empty, NULL), U_OK);
  ck_assert_int_eq(ulfius_start_framework(&u_instance), U_OK);
  
  ulfius_init_request(&request);
  request.http_url = o_strdup("http://localhost:8080/inject1");
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  ulfius_init_request(&request);
  request.http_url = o_strdup("http://localhost:8080/inject2");
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 404);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "inject2", NULL, 0, &callback_function_empty, NULL), U_OK);
  
  ulfius_init_request(&request);
  request.http_url = o_strdup("http://localhost:8080/inject2");
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  ck_assert_int_eq(ulfius_remove_endpoint_by_val(&u_instance, "GET", "inject2", NULL), U_OK);
  
  ulfius_init_request(&request);
  request.http_url = o_strdup("http://localhost:8080/inject2");
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 404);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  ulfius_stop_framework(&u_instance);
  ulfius_clean_instance(&u_instance);
}
END_TEST

START_TEST(test_ulfius_endpoint_multiple)
{
  struct _u_instance u_instance;
  struct _u_request request;
  struct _u_response response;
  
  ck_assert_int_eq(ulfius_init_instance(&u_instance, 8080, NULL, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "multiple", "*", 0, &callback_function_multiple_continue, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "multiple", "/:param1/*", 1, &callback_function_multiple_continue, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "multiple", "/:param1/:param2/*", 2, &callback_function_multiple_continue, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "multiple", "/:param1/:param2/:param3", 3, &callback_function_multiple_continue, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "multiple_complete", "*", 0, &callback_function_multiple_continue, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "multiple_complete", "/:param1/*", 1, &callback_function_multiple_complete, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "multiple_complete", "/:param1/:param2/*", 2, &callback_function_multiple_continue, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "multiple_complete", "/:param1/:param2/:param3", 3, &callback_function_multiple_continue, NULL), U_OK);
  ck_assert_int_eq(ulfius_start_framework(&u_instance), U_OK);
  
  ulfius_init_request(&request);
  request.http_url = o_strdup("http://localhost:8080/multiple");
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_eq(o_strncmp(response.binary_body, "/multiple", o_strlen("/multiple")), 0);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  ulfius_init_request(&request);
  request.http_url = o_strdup("http://localhost:8080/multiple/value1");
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_eq(o_strncmp(response.binary_body, "/multiple/value1\n/multiple/value1", o_strlen("/multiple/value1\n/multiple/value1")), 0);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  ulfius_init_request(&request);
  request.http_url = o_strdup("http://localhost:8080/multiple/value1/value2");
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_eq(o_strncmp(response.binary_body, "/multiple/value1/value2\n/multiple/value1/value2\n/multiple/value1/value2", o_strlen("/multiple/value1/value2\n/multiple/value1/value2\n/multiple/value1/value2")), 0);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  ulfius_init_request(&request);
  request.http_url = o_strdup("http://localhost:8080/multiple/value1/value2/value3");
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_eq(o_strncmp(response.binary_body, "/multiple/value1/value2/value3\n/multiple/value1/value2/value3\n/multiple/value1/value2/value3\n/multiple/value1/value2/value3", o_strlen("/multiple/value1/value2/value3\n/multiple/value1/value2/value3\n/multiple/value1/value2/value3\n/multiple/value1/value2/value3")), 0);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  ulfius_init_request(&request);
  request.http_url = o_strdup("http://localhost:8080/nope/value1/value2/value3/value4");
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 404);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  ulfius_init_request(&request);
  request.http_url = o_strdup("http://localhost:8080/multiple_complete/value1/value2/value3");
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ck_assert_int_eq(o_strncmp(response.binary_body, "/multiple_complete/value1/value2/value3\n/multiple_complete/value1/value2/value3", o_strlen("/multiple_complete/value1/value2/value3\n/multiple_complete/value1/value2/value3")), 0);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  ulfius_stop_framework(&u_instance);
  ulfius_clean_instance(&u_instance);
}
END_TEST

START_TEST(test_ulfius_endpoint_stream)
{
  struct _u_instance u_instance;
  struct _u_request request;
  struct _u_response response;
  
  ck_assert_int_eq(ulfius_init_instance(&u_instance, 8080, NULL, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "stream", NULL, 0, &callback_function_stream, NULL), U_OK);
  ck_assert_int_eq(ulfius_start_framework(&u_instance), U_OK);
  
  ulfius_init_request(&request);
  request.http_url = o_strdup("http://localhost:8080/stream");
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_streaming_request(&request, &response, my_write_body, NULL), U_OK);
  ck_assert_int_eq(response.status, 200);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  ulfius_clean_instance(&u_instance);
}
END_TEST

START_TEST(test_ulfius_utf8_not_ignored)
{
  char * invalid_utf8_seq2 = msprintf("value %c%c", 0xC3, 0x28);
  char * invalid_utf8_seq3 = msprintf("value %c%c%c", 0xE2, 0x28, 0xA1);
  char * invalid_utf8_seq4 = msprintf("value %c%c%c%c", 0xF0, 0x90, 0x28, 0xBC);
  char * valid_utf8 = "valid value ȸ Ɇ  ɤ ¯\\_(ツ)_/¯";
  struct _u_instance u_instance;
  struct _u_request request;
  
  ck_assert_int_eq(ulfius_init_instance(&u_instance, 8080, NULL, NULL), U_OK);
  u_instance.check_utf8 = 0;
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "POST", "utf8", ":utf8_param1/:utf8_param_valid", 0, &callback_check_utf8_not_ignored, NULL), U_OK);
  ck_assert_int_eq(ulfius_start_framework(&u_instance), U_OK);
  
  ulfius_init_request(&request);
  request.http_url = msprintf("http://localhost:8080/utf8/%s/%s/?utf8_param3=%s&utf8_param_valid2=%s&utf8_param_empty=&utf8_param_null", invalid_utf8_seq3, valid_utf8, invalid_utf8_seq2, valid_utf8);
  request.http_verb = o_strdup("POST");
  u_map_put(request.map_header, "utf8_param", invalid_utf8_seq3);
  u_map_put(request.map_header, "utf8_param_valid", valid_utf8);
  u_map_put(request.map_post_body, "utf8_param", invalid_utf8_seq4);
  u_map_put(request.map_post_body, "utf8_param_valid", valid_utf8);
  u_map_put(request.map_post_body, "utf8_param_empty", "");
  ck_assert_int_eq(ulfius_send_http_request(&request, NULL), U_OK);
  ulfius_clean_request(&request);
  
  o_free(invalid_utf8_seq2);
  o_free(invalid_utf8_seq3);
  o_free(invalid_utf8_seq4);
  ulfius_stop_framework(&u_instance);
  ulfius_clean_instance(&u_instance);
}
END_TEST

START_TEST(test_ulfius_utf8_ignored)
{
  char * invalid_utf8_seq2 = msprintf("invalid value %c%c", 0xC3, 0x28);
  char * invalid_utf8_seq3 = msprintf("invalid value %c%c%c", 0xE2, 0x28, 0xA1);
  char * invalid_utf8_seq4 = msprintf("invalid value %c%c%c%c", 0xF0, 0x90, 0x28, 0xBC);
  char * valid_utf8 = "valid value ȸ Ɇ  ɤ ¯\\_(ツ)_/¯";
  struct _u_instance u_instance;
  struct _u_request request;
  
  ck_assert_int_eq(ulfius_init_instance(&u_instance, 8080, NULL, NULL), U_OK);
  ck_assert_int_eq(u_instance.check_utf8, 1);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "POST", "utf8", ":utf8_param1/:utf8_param_valid", 0, &callback_check_utf8_ignored, NULL), U_OK);
  ck_assert_int_eq(ulfius_start_framework(&u_instance), U_OK);
  
  ulfius_init_request(&request);
  request.http_url = msprintf("http://localhost:8080/utf8/%s/%s/?utf8_param3=%s&utf8_param_valid2=%s", invalid_utf8_seq3, valid_utf8, invalid_utf8_seq2, valid_utf8);
  request.http_verb = o_strdup("POST");
  u_map_put(request.map_header, "utf8_param", invalid_utf8_seq3);
  u_map_put(request.map_header, "utf8_param_valid", valid_utf8);
  u_map_put(request.map_post_body, "utf8_param", invalid_utf8_seq4);
  u_map_put(request.map_post_body, "utf8_param_valid", valid_utf8);
  ck_assert_int_eq(ulfius_send_http_request(&request, NULL), U_OK);
  ulfius_clean_request(&request);
  
  o_free(invalid_utf8_seq2);
  o_free(invalid_utf8_seq3);
  o_free(invalid_utf8_seq4);
  ulfius_stop_framework(&u_instance);
  ulfius_clean_instance(&u_instance);
}
END_TEST

START_TEST(test_ulfius_endpoint_callback_position)
{
  struct _u_instance u_instance;
  struct _u_request request;
  struct _u_response response;
  
  ck_assert_int_eq(ulfius_init_instance(&u_instance, 8080, NULL, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "callback_position", "*", 0, &callback_function_position_0, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "callback_position", "*", 0, &callback_function_position_1, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "callback_position", "*", 0, &callback_function_position_2, NULL), U_OK);
  ck_assert_int_eq(ulfius_start_framework(&u_instance), U_OK);
  
  ulfius_init_request(&request);
  request.http_url = o_strdup("http://localhost:8080/callback_position");
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  ulfius_stop_framework(&u_instance);
  ulfius_clean_instance(&u_instance);
}
END_TEST

START_TEST(test_ulfius_MHD_set_response_with_other_free)
{
  struct _u_instance u_instance;
  struct _u_request request;
  struct _u_response response;
  
  o_set_alloc_funcs(&malloc, &realloc, &free_with_test);
  ck_assert_int_eq(ulfius_init_instance(&u_instance, 8080, NULL, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "/", NULL, 0, &callback_function_return_url, NULL), U_OK);
  ck_assert_int_eq(ulfius_start_framework(&u_instance), U_OK);
  
  ulfius_init_request(&request);
  request.http_url = o_strdup("http://localhost:8080/");
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  ulfius_stop_framework(&u_instance);
  ulfius_clean_instance(&u_instance);
  // We can only test that via_free_with_test has been incremented.
  // The only way to know for sure now is to verify with valgrind and the memory consumption
  ck_assert_int_gt(via_free_with_test, 1);
}
END_TEST

#define PORT_PLAIN 2525
#define PORT_RICH 2526
#define FROM "from"
#define TO "to"
#define CC "cc"
#define BCC "bcc"
#define CONTENT_TYPE "text/ulfius; charset=utf-42"
#define SUBJECT "subject"
#define BODY "mail body"

START_TEST(test_ulfius_send_smtp)
{
  pthread_t thread;
  struct smtp_manager manager;

  manager.mail_data = NULL;
  manager.port = PORT_PLAIN;
  manager.sockfd = 0;

  ck_assert_int_eq(ulfius_send_smtp_email("localhost", PORT_PLAIN, 0, 0, NULL, NULL, FROM, TO, CC, BCC, SUBJECT, BODY), U_ERROR_LIBCURL);
  ck_assert_int_eq(ulfius_send_smtp_email(NULL, PORT_PLAIN, 0, 0, NULL, NULL, FROM, TO, CC, BCC, SUBJECT, BODY), U_ERROR_PARAMS);
  ck_assert_int_eq(ulfius_send_smtp_email("localhost", PORT_PLAIN, 0, 0, NULL, NULL, NULL, TO, CC, BCC, SUBJECT, BODY), U_ERROR_PARAMS);
  ck_assert_int_eq(ulfius_send_smtp_email("localhost", PORT_PLAIN, 0, 0, NULL, NULL, FROM, NULL, CC, BCC, SUBJECT, BODY), U_ERROR_PARAMS);
  ck_assert_int_eq(ulfius_send_smtp_email("localhost", PORT_PLAIN, 0, 0, NULL, NULL, FROM, TO, CC, BCC, SUBJECT, NULL), U_ERROR_PARAMS);
  
  pthread_create(&thread, NULL, simple_smtp, &manager);
  ck_assert_int_eq(ulfius_send_smtp_email("localhost", PORT_PLAIN, 0, 0, NULL, NULL, FROM, TO, CC, BCC, SUBJECT, BODY), U_OK);
  pthread_join(thread, NULL);
  ck_assert_ptr_ne(NULL, o_strstr(manager.mail_data, "HELO"));
  ck_assert_ptr_ne(NULL, o_strstr(manager.mail_data, "MAIL FROM:<" FROM ">"));
  ck_assert_ptr_ne(NULL, o_strstr(manager.mail_data, "RCPT TO:<" TO ">"));
  ck_assert_ptr_ne(NULL, o_strstr(manager.mail_data, "RCPT TO:<" CC ">"));
  ck_assert_ptr_ne(NULL, o_strstr(manager.mail_data, "RCPT TO:<" BCC ">"));
  ck_assert_ptr_ne(NULL, o_strstr(manager.mail_data, "Subject: " SUBJECT));
  ck_assert_ptr_ne(NULL, o_strstr(manager.mail_data, BODY));
  o_free(manager.mail_data);
  manager.mail_data = NULL;
}
END_TEST

START_TEST(test_ulfius_send_rich_smtp)
{
  pthread_t thread;
  struct smtp_manager manager;

  manager.mail_data = NULL;
  manager.port = PORT_RICH;
  manager.sockfd = 0;

  pthread_create(&thread, NULL, simple_smtp, &manager);
  ck_assert_int_eq(ulfius_send_smtp_rich_email("localhost", PORT_RICH, 0, 0, NULL, NULL, FROM, TO, CC, BCC, CONTENT_TYPE, SUBJECT, BODY), U_OK);
  pthread_join(thread, NULL);
  ck_assert_ptr_ne(NULL, o_strstr(manager.mail_data, "HELO"));
  ck_assert_ptr_ne(NULL, o_strstr(manager.mail_data, "MAIL FROM:<" FROM ">"));
  ck_assert_ptr_ne(NULL, o_strstr(manager.mail_data, "RCPT TO:<" TO ">"));
  ck_assert_ptr_ne(NULL, o_strstr(manager.mail_data, "RCPT TO:<" CC ">"));
  ck_assert_ptr_ne(NULL, o_strstr(manager.mail_data, "RCPT TO:<" BCC ">"));
  ck_assert_ptr_ne(NULL, o_strstr(manager.mail_data, "Content-Type: " CONTENT_TYPE));
  ck_assert_ptr_ne(NULL, o_strstr(manager.mail_data, "Subject: " SUBJECT));
  ck_assert_ptr_ne(NULL, o_strstr(manager.mail_data, BODY));
  o_free(manager.mail_data);
  manager.mail_data = NULL;
}
END_TEST

START_TEST(test_ulfius_follow_redirect)
{
  struct _u_instance u_instance;
  struct _u_request request;
  struct _u_response response;
  
  ck_assert_int_eq(ulfius_init_instance(&u_instance, 8080, NULL, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "first", "*", 0, &callback_function_redirect_first, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "last", "*", 0, &callback_function_redirect_last, NULL), U_OK);
  ck_assert_int_eq(ulfius_start_framework(&u_instance), U_OK);
  
  ulfius_init_request(&request);
  request.http_url = o_strdup("http://localhost:8080/first");
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 302);
  ck_assert_str_eq(u_map_get(response.map_header, "Location"), "/last");
  ck_assert_int_eq(o_strncmp(response.binary_body, BODY_NOT_REDIRECTED, o_strlen(BODY_NOT_REDIRECTED)), 0);
  ulfius_clean_response(&response);
  
  ulfius_init_response(&response);
  request.follow_redirect = 1;
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(o_strncmp(response.binary_body, BODY_REDIRECTED, o_strlen(BODY_REDIRECTED)), 0);
  ck_assert_int_eq(response.status, 200);
  ulfius_clean_response(&response);
  
  ulfius_clean_request(&request);
  ulfius_stop_framework(&u_instance);
  ulfius_clean_instance(&u_instance);
}
END_TEST

#ifndef U_DISABLE_GNUTLS
START_TEST(test_ulfius_server_ca_trust)
{
  struct _u_instance u_instance;
  char * cert = get_file_content("cert/server.crt"), * key = get_file_content("cert/server.key"), * ca = get_file_content("cert/root1.crt");

  ck_assert_ptr_ne(cert, NULL);
  ck_assert_ptr_ne(key, NULL);
  ck_assert_ptr_ne(ca, NULL);
  ck_assert_int_eq(ulfius_init_instance(&u_instance, 8080, NULL, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "stream", NULL, 0, &callback_function_stream, NULL), U_OK);
  ck_assert_int_ne(ulfius_start_secure_ca_trust_framework(&u_instance, NULL, cert, ca), U_OK);
  ck_assert_int_ne(ulfius_start_secure_ca_trust_framework(&u_instance, key, NULL, ca), U_OK);
  ck_assert_int_eq(ulfius_start_secure_ca_trust_framework(&u_instance, key, cert, ca), U_OK);
  
  o_free(cert);
  o_free(key);
  o_free(ca);
  ulfius_stop_framework(&u_instance);
  ulfius_clean_instance(&u_instance);
}
END_TEST

START_TEST(test_ulfius_client_certificate)
{
  struct _u_instance u_instance;
  struct _u_request request;
  struct _u_response response;
  char * cert = get_file_content("cert/server.crt"), * key = get_file_content("cert/server.key"), * ca = get_file_content("cert/root1.crt");
  
  ck_assert_ptr_ne(cert, NULL);
  ck_assert_ptr_ne(key, NULL);
  ck_assert_ptr_ne(ca, NULL);
  ck_assert_int_eq(ulfius_init_instance(&u_instance, 8080, NULL, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "cert_auth", NULL, 0, &callback_auth_client_cert, NULL), U_OK);
  ck_assert_int_eq(ulfius_add_endpoint_by_val(&u_instance, "GET", "nocert_auth", NULL, 0, &callback_no_auth_client_cert, NULL), U_OK);
  ck_assert_int_eq(ulfius_start_secure_ca_trust_framework(&u_instance, key, cert, ca), U_OK);

  ulfius_init_request(&request);
  request.http_url = o_strdup("https://localhost:8080/nocert_auth");
  request.check_server_certificate = 0;
  request.client_key_file = o_strdup("cert/client1.key");
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  ulfius_init_request(&request);
  request.http_url = o_strdup("https://localhost:8080/nocert_auth");
  request.check_server_certificate = 0;
  request.client_cert_file = o_strdup("cert/client1.crt");
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  // Test with a valid certificate signed by the expected CA (root1)
  ulfius_init_request(&request);
  request.http_url = o_strdup("https://localhost:8080/cert_auth");
  request.check_server_certificate = 0;
  request.client_cert_file = o_strdup("cert/client1.crt");
  request.client_key_file = o_strdup("cert/client1.key");
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);
  
  // Test with a valid certificate signed by another CA (root2)
  ulfius_init_request(&request);
  request.http_url = o_strdup("https://localhost:8080/nocert_auth");
  request.check_server_certificate = 0;
  request.client_cert_file = o_strdup("cert/client2.crt");
  request.client_key_file = o_strdup("cert/client2.key");
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ck_assert_int_eq(response.status, 200);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  // Test with a certificate self-signed (client3)
  ulfius_init_request(&request);
  request.http_url = o_strdup("https://localhost:8080/nocert_auth");
  request.check_server_certificate = 0;
  request.client_cert_file = o_strdup("cert/client3.crt");
  request.client_key_file = o_strdup("cert/client3.key");
  ulfius_init_response(&response);
  ck_assert_int_eq(ulfius_send_http_request(&request, &response), U_OK);
  ulfius_clean_request(&request);
  ulfius_clean_response(&response);

  o_free(cert);
  o_free(key);
  o_free(ca);
  ulfius_stop_framework(&u_instance);
  ulfius_clean_instance(&u_instance);
}
END_TEST
#endif

static Suite *ulfius_suite(void)
{
  Suite *s;
  TCase *tc_core;

  s = suite_create("Ulfius framework function tests");
  tc_core = tcase_create("test_ulfius_framework");
  tcase_add_test(tc_core, test_ulfius_simple_endpoint);
#if MHD_VERSION >= 0x00095208
  tcase_add_test(tc_core, test_ulfius_net_type_endpoint);
#endif
  tcase_add_test(tc_core, test_ulfius_endpoint_parameters);
  tcase_add_test(tc_core, test_ulfius_endpoint_injection);
  tcase_add_test(tc_core, test_ulfius_endpoint_multiple);
  tcase_add_test(tc_core, test_ulfius_endpoint_stream);
  tcase_add_test(tc_core, test_ulfius_utf8_not_ignored);
  tcase_add_test(tc_core, test_ulfius_utf8_ignored);
  tcase_add_test(tc_core, test_ulfius_endpoint_callback_position);
  tcase_add_test(tc_core, test_ulfius_MHD_set_response_with_other_free);
  tcase_add_test(tc_core, test_ulfius_send_smtp);
  tcase_add_test(tc_core, test_ulfius_send_rich_smtp);
  tcase_add_test(tc_core, test_ulfius_follow_redirect);
#ifndef U_DISABLE_GNUTLS
  tcase_add_test(tc_core, test_ulfius_server_ca_trust);
  tcase_add_test(tc_core, test_ulfius_client_certificate);
#endif
  tcase_set_timeout(tc_core, 30);
  suite_add_tcase(s, tc_core);

  return s;
}

int main(int argc, char *argv[])
{
  int number_failed;
  Suite *s;
  SRunner *sr;
  //y_init_logs("Ulfius", Y_LOG_MODE_CONSOLE, Y_LOG_LEVEL_DEBUG, NULL, "Starting Ulfius framework tests");
  s = ulfius_suite();
  sr = srunner_create(s);

  srunner_run_all(sr, CK_VERBOSE);
  number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);
  
  //y_close_logs();
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
