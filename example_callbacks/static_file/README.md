# static file server callback function for Ulfius Framework

Provides a simple static file server. `user_data` must be initialized with a `struct static_file_config` containing the following informations:

- `files_path`: path to the DocumentRoot folder, can be relative or absolute
- `mime_types`: a `struct _u_map` containing a set of mime-types with file extension as key and mime-type as value
- `map_header`: a `struct _u_map` containing a set of headers that will be added to all responses within the `static_file_callback`
 * files_path: path (relative or absolute) to the DocumentRoot folder
 * url_prefix: prefix used to access the callback function
 * mime_types: a struct _u_map filled with all the mime-types needed for a static file server
 * redirect_on_404: redirct uri on error 404, if NULL, send 404
