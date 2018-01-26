/**
 * 
 * Ulfius Framework example program
 * 
 * This example program send several requests to describe
 * the function ulfius_request_http behaviour
 *  
 * Copyright 2015-2017 Nicolas Mora <mail@babelouest.org>
 * 
 * License MIT
 *
 */

#define U_DISABLE_WEBSOCKET
#include "../../include/ulfius.h"

#define MAIL_SERVER "localhost"
#define MAIL_SENDER "test@example.org"
#define MAIL_RECIPIENT "test@example.com"

int main(int argc, char ** argv) {
  printf("Send mail: %d\n", ulfius_send_smtp_email(MAIL_SERVER, 0, 0, 0, NULL, NULL, MAIL_SENDER, MAIL_RECIPIENT, NULL, NULL, "test", "This is a test\nHello!!!"));
  return 0;
}
