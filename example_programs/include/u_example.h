/**
 *
 * Ulfius example programs
 *
 * Common header for all example programs
 *
 * Copyright 2018-2020 Nicolas Mora <mail@babelouest.org>
 *
 * License MIT
 *
 */

#ifndef _U_EXAMPLE_
#define _U_EXAMPLE_

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
#endif // U_DISABLE_YDER

#endif // _U_EXAMPLE_
