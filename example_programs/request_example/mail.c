/**
 * 
 * Ulfius Framework example program
 * 
 * This example program send several requests to describe
 * the function ulfius_request_http behaviour
 *  
 * Copyright 2015-2022 Nicolas Mora <mail@babelouest.org>
 * 
 * License MIT
 *
 */

#include <ulfius.h>

#include "u_example.h"

#define MAIL_SERVER "localhost"
#define MAIL_SENDER "test@example.org"
#define MAIL_RECIPIENT "test@example.com"

#if defined(U_DISABLE_CURL)
#error You must build ulfius with libcurl and jansson support enabled to compile this example, check the install documentation
#else

int main(void) {
  printf("Send mail: %d\n", ulfius_send_smtp_email(MAIL_SERVER, 0, 0, 0, NULL, NULL, MAIL_SENDER, MAIL_RECIPIENT, NULL, NULL, "test", "This is a test\nHello!!!"));
  return 0;
}
#endif
