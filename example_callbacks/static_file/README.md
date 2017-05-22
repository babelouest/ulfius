# static file server callback function for Ulfius Framework

Provides a simple static file server. `user_data` must be initialized with a `struct static_file_config` containing the following informations:

- `files_path`: path to the DocumentRoot folder, can be relative or absolute
- `mime_types`: a `struct _u_map` containing a set of mime-types with file extension as key and mime-type as value
