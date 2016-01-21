/**
 * 
 * Ulfius Framework sheep_counter program
 * 
 * This program implements a small set of webservices and a tiny static files server
 * 
 * As a result, the index.html page will increment a sheep counter and 
 * shows a new sheep on the screen every time an increment is done (every 5 seconds)
 * 
 * Copyright 2015 Nicolas Mora <mail@babelouest.org>
 * 
 * License MIT
 *
 */

#include <string.h>
#include <jansson.h>

#include "../../src/ulfius.h"

#define PORT 7437
#define PREFIX "/sheep"
#define STATIC_FOLDER "static"

// Callback function used to serve static files that are present in the static folder
int callback_static_file (const struct _u_request * request, struct _u_response * response, void * user_data);

// Callback function used to start a new count by setting the number of sheeps
int callback_sheep_counter_start (const struct _u_request * request, struct _u_response * response, void * user_data);

// Callback function used to reset the number of sheeps to 0
int callback_sheep_counter_reset (const struct _u_request * request, struct _u_response * response, void * user_data);

// Callback function used to add one sheep to the counter
int callback_sheep_counter_add (const struct _u_request * request, struct _u_response * response, void * user_data);

/**
 * return the filename extension
 */
const char * get_filename_ext(const char *path) {
    const char *dot = strrchr(path, '.');
    if(!dot || dot == path) return "*";
    return dot + 1;
}

/**
 * Main function
 */
int main (int argc, char **argv) {
  
  // jansson integer type can vary
  #if JSON_INTEGER_IS_LONG_LONG
  long long nb_sheep = 0;
  #else
  long nb_sheep = 0;
  #endif
  
  // Initialize the instance
  struct _u_instance instance;
  
  if (ulfius_init_instance(&instance, PORT, NULL) != U_OK) {
    y_log_message(Y_LOG_LEVEL_ERROR, "Error ulfius_init_instance, abort");
    return(1);
  }
  
  // MIME types that will define the static files
  struct _u_map mime_types;
  u_map_init(&mime_types);
  u_map_put(&mime_types, ".html", "text/html");
  u_map_put(&mime_types, ".css", "text/css");
  u_map_put(&mime_types, ".js", "application/javascript");
  u_map_put(&mime_types, ".png", "image/png");
  u_map_put(&mime_types, ".jpeg", "image/jpeg");
  u_map_put(&mime_types, ".jpg", "image/jpeg");
  u_map_put(&mime_types, "*", "application/octet-stream");
  
  // Endpoint list declaration
  // The first 3 are webservices with a specific url
  // The last endpoint will be called for every GET call and will serve the static files
  ulfius_add_endpoint_by_val(&instance, "POST", PREFIX, NULL, &callback_sheep_counter_start, &nb_sheep);
  ulfius_add_endpoint_by_val(&instance, "PUT", PREFIX, NULL, &callback_sheep_counter_add, &nb_sheep);
  ulfius_add_endpoint_by_val(&instance, "DELETE", PREFIX, NULL, &callback_sheep_counter_reset, &nb_sheep);
  ulfius_add_endpoint_by_val(&instance, "GET", NULL, "*", &callback_static_file, &mime_types);
  
  // Start the framework
  if (ulfius_start_framework(&instance) == U_OK) {
    printf("Start sheep counter on port %d\n", instance.port);
    
    // Wait for the user to press <enter> on the console to quit the application
    getchar();
  } else {
    printf("Error starting framework\n");
  }

  // Clean the mime map
  u_map_clean(&mime_types);
  
  printf("End framework\n");
  ulfius_stop_framework(&instance);
  ulfius_clean_instance(&instance);
  
  return 0;
}

/**
 * Start a new counter by setting the number of sheeps to a specific value
 * return the current number of sheeps in a json object
 */
int callback_sheep_counter_start (const struct _u_request * request, struct _u_response * response, void * user_data) {
  json_t * json_nb_sheep = json_object_get(request->json_body, "nbsheep");
  
  #if JSON_INTEGER_IS_LONG_LONG
  long long * nb_sheep = user_data;
  #else
  long * nb_sheep = user_data;
  #endif

  if (json_nb_sheep != NULL) {
    * nb_sheep = json_integer_value(json_nb_sheep);
  } else {
    * nb_sheep = 0;
  }
  
  response->json_body = json_object();
  json_object_set_new(response->json_body, "nbsheep", json_integer(* nb_sheep));
  response->status = 200;
  json_decref(json_nb_sheep);
  return U_OK;
}

/**
 * Reset the number of sheeps to 0
 * return the current number of sheeps in a json object
 */
int callback_sheep_counter_reset (const struct _u_request * request, struct _u_response * response, void * user_data) {
  #if JSON_INTEGER_IS_LONG_LONG
  long long * nb_sheep = user_data;
  #else
  long * nb_sheep = user_data;
  #endif
  * nb_sheep = 0;
  
  response->json_body = json_object();
  json_object_set_new(response->json_body, "nbsheep", json_integer(0));
  response->status = 200;
  
  return U_OK;
}

/**
 * Adds one sheep
 * return the current number of sheeps in a json object
 */
int callback_sheep_counter_add (const struct _u_request * request, struct _u_response * response, void * user_data) {
  #if JSON_INTEGER_IS_LONG_LONG
  long long * nb_sheep = user_data;
  #else
  long * nb_sheep = user_data;
  #endif
  
  (*nb_sheep)++;
  
  response->json_body = json_object();
  json_object_set_new(response->json_body, "nbsheep", json_integer(*nb_sheep));
  response->status = 200;
  
  return U_OK;
}

/**
 * serve a static file to the client as a very simple http server
 */
int callback_static_file (const struct _u_request * request, struct _u_response * response, void * user_data) {
  void * buffer = NULL;
  long length;
  FILE * f;
  char  * file_path = malloc(strlen(request->http_url)+strlen(STATIC_FOLDER)+sizeof(char));
  const char * content_type;
  snprintf(file_path, (strlen(request->http_url)+strlen(STATIC_FOLDER)+sizeof(char)), "%s%s", STATIC_FOLDER, request->http_url);
  
  if (access(file_path, F_OK) != -1) {
    f = fopen (file_path, "rb");
    if (f) {
      fseek (f, 0, SEEK_END);
      length = ftell (f);
      fseek (f, 0, SEEK_SET);
      buffer = malloc(length*sizeof(void));
      if (buffer) {
        fread (buffer, 1, length, f);
      }
      fclose (f);
    }

    if (buffer) {
      content_type = u_map_get((struct _u_map *)user_data, get_filename_ext(request->http_url));
      response->binary_body = buffer;
      response->binary_body_length = length;
      u_map_put(response->map_header, "Content-Type", content_type);
      response->status = 200;
    } else {
      response->string_body = u_strdup("");
      response->status = 404;
    }
  } else {
    response->string_body = u_strdup("");
    response->status = 404;
  }
  free(file_path);
  return U_OK;
}
