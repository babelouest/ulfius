/**
 * 
 * Ulfius Framework sheep_counter program
 * 
 * This program implements a small set of webservices and a tiny static files server
 * 
 * As a result, the index.html page will increment a sheep counter and 
 * shows a new sheep on the screen every time an increment is done (every 5 seconds)
 * 
 * Copyright 2015-2022 Nicolas Mora <mail@babelouest.org>
 * 
 * License MIT
 *
 */

#include <jansson.h>
#include <string.h>
#include <ulfius.h>

#include "http_compression_callback.h"
#include "static_compressed_inmemory_website_callback.h"
#include "u_example.h"

#define PORT 7437
#define PREFIX "/sheep"
#define FILE_PREFIX "/upload"
#define STATIC_FOLDER "static"

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
      len = snprintf(NULL, 0, "key is %s, length is %zu, value is %.*s", keys[i], u_map_get_length(map, keys[i]), (int)u_map_get_length(map, keys[i]), value);
      line = o_malloc((len+1)*sizeof(char));
      snprintf(line, (len+1), "key is %s, length is %zu, value is %.*s", keys[i], u_map_get_length(map, keys[i]), (int)u_map_get_length(map, keys[i]), value);
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

/**
 * Start a new counter by setting the number of sheeps to a specific value
 * return the current number of sheeps in a json object
 */
static int callback_sheep_counter_start (const struct _u_request * request, struct _u_response * response, void * user_data) {
  json_t * json_nb_sheep = ulfius_get_json_body_request(request, NULL), * json_body = NULL;
  
  json_int_t * nb_sheep = user_data;

  if (json_nb_sheep != NULL) {
    * nb_sheep = json_integer_value(json_object_get(json_nb_sheep,"nbsheep"));
  } else {
    * nb_sheep = 0;
  }
  
  json_body = json_object();
  json_object_set_new(json_body, "nbsheep", json_integer(* nb_sheep));
  ulfius_set_json_body_response(response, 200, json_body);
  json_decref(json_nb_sheep);
  json_decref(json_body);
  return U_CALLBACK_CONTINUE;
}

/**
 * Reset the number of sheeps to 0
 * return the current number of sheeps in a json object
 */
static int callback_sheep_counter_reset (const struct _u_request * request, struct _u_response * response, void * user_data) {
  json_t * json_body = NULL;
  json_int_t * nb_sheep = user_data;
  * nb_sheep = 0;
  
  json_body = json_object();
  json_object_set_new(json_body, "nbsheep", json_integer(0));
  ulfius_set_json_body_response(response, 200, json_body);
  json_decref(json_body);
  
  return U_CALLBACK_CONTINUE;
}

/**
 * Adds one sheep
 * return the current number of sheeps in a json object
 */
static int callback_sheep_counter_add (const struct _u_request * request, struct _u_response * response, void * user_data) {
  json_t * json_body = NULL;
  json_int_t * nb_sheep = user_data;
  
  (*nb_sheep)++;
  
  json_body = json_object();
  json_object_set_new(json_body, "nbsheep", json_integer(*nb_sheep));
  ulfius_set_json_body_response(response, 200, json_body);
  json_decref(json_body);
  
  return U_CALLBACK_CONTINUE;
}

/**
 * upload a file
 */
static int callback_upload_file (const struct _u_request * request, struct _u_response * response, void * user_data) {
  char * url_params = print_map(request->map_url), * headers = print_map(request->map_header), * cookies = print_map(request->map_cookie), 
       * post_params = print_map(request->map_post_body);

  char * string_body = msprintf("Upload file\n\n  method is %s\n  url is %s\n\n  parameters from the url are \n%s\n\n  cookies are \n%s\n\n  headers are \n%s\n\n  post parameters are \n%s\n\n",
                                  request->http_verb, request->http_url, url_params, cookies, headers, post_params);
  y_log_message(Y_LOG_LEVEL_DEBUG, "Post parameters:\n%s", post_params);
  y_log_message(Y_LOG_LEVEL_DEBUG, "URL parameters:\n%s", url_params);
  o_free(url_params);
  o_free(headers);
  o_free(cookies);
  o_free(post_params);
  o_free(string_body);
  return U_CALLBACK_CONTINUE;
}

/**
 * Submit a form
 */
static int callback_form_submit (const struct _u_request * request, struct _u_response * response, void * user_data) {
  char * post_params = print_map(request->map_post_body),
       * url_params = print_map(request->map_url);
  y_log_message(Y_LOG_LEVEL_DEBUG, "Post parameters:\n%s", post_params);
  y_log_message(Y_LOG_LEVEL_DEBUG, "URL parameters:\n%s", url_params);
  o_free(post_params);
  o_free(url_params);
  ulfius_set_string_body_response(response, 200, "Form submitted");
  return U_CALLBACK_CONTINUE;
}

/**
 * File upload callback function
 */
static int file_upload_callback (const struct _u_request * request, 
                          const char * key, 
                          const char * filename, 
                          const char * content_type, 
                          const char * transfer_encoding, 
                          const char * data, 
                          uint64_t off, 
                          size_t size, 
                          void * cls) {
  y_log_message(Y_LOG_LEVEL_DEBUG, "Got from file '%s' of the key '%s', offset %llu, size %zu, cls is '%s'", filename, key, off, size, cls);
  return U_OK;
}

/**
 * Main function
 */
int main (int argc, char **argv) {
  
  struct _u_compressed_inmemory_website_config file_config;
  json_int_t nb_sheep = 0;
  
  // Initialize the instance
  struct _u_instance instance;
  
  y_init_logs("sheep_counter", Y_LOG_MODE_CONSOLE, Y_LOG_LEVEL_DEBUG, NULL, "Starting sheep_counter");
  
  if (u_init_compressed_inmemory_website_config(&file_config) == U_OK) {
    u_map_put(&file_config.mime_types, ".html", "text/html");
    u_map_put(&file_config.mime_types, ".css", "text/css");
    u_map_put(&file_config.mime_types, ".js", "application/javascript");
    u_map_put(&file_config.mime_types, ".png", "image/png");
    u_map_put(&file_config.mime_types, ".jpg", "image/jpeg");
    u_map_put(&file_config.mime_types, ".jpeg", "image/jpeg");
    u_map_put(&file_config.mime_types, ".ttf", "font/ttf");
    u_map_put(&file_config.mime_types, ".woff", "font/woff");
    u_map_put(&file_config.mime_types, ".woff2", "font/woff2");
    u_map_put(&file_config.mime_types, ".map", "application/octet-stream");
    u_map_put(&file_config.mime_types, ".json", "application/json");
    u_map_put(&file_config.mime_types, "*", "application/octet-stream");
    file_config.files_path = "static";
    file_config.url_prefix = FILE_PREFIX;

    if (ulfius_init_instance(&instance, PORT, NULL, NULL) != U_OK) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Error ulfius_init_instance, abort");
      return(1);
    }
    
    // Max post param size is 16 Kb, which means an uploaded file is no more than 16 Kb
    instance.max_post_param_size = 16*1024;
    
    if (ulfius_set_upload_file_callback_function(&instance, &file_upload_callback, "my cls") != U_OK) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Error ulfius_set_upload_file_callback_function");
    }
    
    // Endpoint list declaration
    // The first 3 are webservices with a specific url
    // The last endpoint will be called for every GET call and will serve the static files
    ulfius_add_endpoint_by_val(&instance, "POST", PREFIX, NULL, 1, &callback_sheep_counter_start, &nb_sheep);
    ulfius_add_endpoint_by_val(&instance, "PUT", PREFIX, NULL, 1, &callback_sheep_counter_add, &nb_sheep);
    ulfius_add_endpoint_by_val(&instance, "DELETE", PREFIX, NULL, 1, &callback_sheep_counter_reset, &nb_sheep);
    ulfius_add_endpoint_by_val(&instance, "*", PREFIX, NULL, 2, &callback_http_compression, NULL);
    ulfius_add_endpoint_by_val(&instance, "*", STATIC_FOLDER, "/upload", 1, &callback_upload_file, NULL);
    ulfius_add_endpoint_by_val(&instance, "*", STATIC_FOLDER, "/submit", 1, &callback_form_submit, NULL);
    ulfius_add_endpoint_by_val(&instance, "GET", "*", NULL, 1, &callback_static_compressed_inmemory_website, &file_config);
    
    // Start the framework
    if (ulfius_start_framework(&instance) == U_OK) {
      printf("Start sheep counter on port %u\n", instance.port);
      
      // Wait for the user to press <enter> on the console to quit the application
      getchar();
    } else {
      printf("Error starting framework\n");
    }

    printf("End framework\n");
    ulfius_stop_framework(&instance);
    ulfius_clean_instance(&instance);
    u_clean_compressed_inmemory_website_config(&file_config);
  }
  
  y_close_logs();
  
  return 0;
}
