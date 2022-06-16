# Static file server with compression callback function for Ulfius Framework

*Note: it is not recommended to use this callback function for public access or untrusted networks (i.e. internet), you should use a better web server like Apache or NGINX instead*

Provides a static file server where HTTP compress is allowed if specified in the mime-types. A memory cache system can be used to retrieve compressed files more easily. This cache system can be disabled if you don't want to overload the memory with a huge static website.

`user_data` must be initialized with a `struct _u_compressed_inmemory_website_config` containing the following informations:

- `files_path`: path to the DocumentRoot folder, can be relative or absolute
- `url_prefix`: prefix used to access the callback function
- `mime_types`: a `struct _u_map` containing a set of mime-types with file extension as key and mime-type as value
- `mime_types_compressed`: A `string_array` structure containing the list of mime-types allowed for compression
- `mime_types_compressed_size`: The number of elements in `mime_types_compressed`
- `map_header`: a `struct _u_map` containing a set of headers that will be added to all responses within the `static_file_callback`
- `redirect_on_404`: redirct uri on error 404, if NULL, send 404
- `allow_gzip`: Set to true if you want to allow gzip compression (default true)
- `allow_deflate`: Set to true if you want to allow deflate compression (default true)
- `allow_cache_compressed`: set to true if you want to allow memory cache for compressed files (default true)
- `lock`: mutex lock (do not touch this variable)
- `gzip_files`: a `struct _u_map` containing cached gzip files
- `deflate_files`: a `struct _u_map` containing cached deflate files

To use the callback function callback_static_compressed_inmemory_website, you must pass an initialized `struct _u_compressed_inmemory_website_config` as user_data with your configuration.

The functions `u_init_compressed_inmemory_website_config`, `u_clean_compressed_inmemory_website_config` and `u_add_mime_types_compressed` are dedicated to manipulate the `struct _u_compressed_inmemory_website_config`.

Here is a sample code on how to use the callback function:

```C
struct _u_compressed_inmemory_website_config config;

if (u_init_compressed_inmemory_website_config(&config) == U_OK) {
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

  // specify compressed mime types
  u_add_mime_types_compressed(&config, "text/html");
  u_add_mime_types_compressed(&config, "text/css");
  u_add_mime_types_compressed(&config, "application/javascript");
  u_add_mime_types_compressed(&config, "application/json");

  // Add callback function to all endpoints
  ulfius_add_endpoint_by_val(instance, "GET", NULL, "*", 0, &callback_static_compressed_inmemory_website, &config);
} else {
  // Error
}
```
