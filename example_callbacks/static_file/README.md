# Static file server callback function for Ulfius Framework

Provides a simple static file server. `user_data` must be initialized with a `struct static_file_config` containing the following informations:

- `files_path`: path to the DocumentRoot folder, can be relative or absolute
- `url_prefix`: prefix used to access the callback function
- `mime_types`: a `struct _u_map` containing a set of mime-types with file extension as key and mime-type as value
- `map_header`: a `struct _u_map` containing a set of headers that will be added to all responses within the `static_file_callback`
- `redirect_on_404`: redirct uri on error 404, if NULL, send 404

Here is a sample code on how to use the callback function:

```C
struct static_file_config config;

// Add mime types
u_map_put(&config->mime_types, "*", "application/octet-stream");
u_map_put(&config->mime_types, ".html", "text/html");
u_map_put(&config->mime_types, ".css", "text/css");
u_map_put(&config->mime_types, ".js", "application/javascript");
u_map_put(&config->mime_types, ".json", "application/json");
u_map_put(&config->mime_types, ".png", "image/png");
u_map_put(&config->mime_types, ".gif", "image/gif");
u_map_put(&config->mime_types, ".jpeg", "image/jpeg");
u_map_put(&config->mime_types, ".jpg", "image/jpeg");
u_map_put(&config->mime_types, ".ttf", "font/ttf");
u_map_put(&config->mime_types, ".woff", "font/woff");
u_map_put(&config->mime_types, ".ico", "image/x-icon");

// Add callback function to all endpoints
ulfius_add_endpoint_by_val(instance, "GET", NULL, "*", 0, &callback_static_file, &config);
```
