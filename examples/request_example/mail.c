/**
 * 
 * Ulfius Framework example program
 * 
 * This example program send several requests to describe
 * the function ulfius_request_http behaviour
 *  
 * Copyright 2015 Nicolas Mora <mail@babelouest.org>
 * 
 * License MIT
 *
 */

#include "../../src/ulfius.h"

int main(int argc, char ** argv) {
  printf("Send mail: %d\n", ulfius_send_smtp_email("localhost", 0, 0, 0, NULL, NULL, "test@example.org", "test@example.com", NULL, NULL, "test", "This is a test\nHello!!!"));
  return 0;
}
