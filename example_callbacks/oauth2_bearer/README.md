# Token validation for resource service based on [Ulfius](https://github.com/babelouest/ulfius) framework

These files contain an authentication callback for Ulfius framework to validate a Glewlwyd access token with the correct scope.

To use this file, you must create a `struct _glewlwyd_resource_config` with your specific parameters:

```C
struct _glewlwyd_resource_config {
  int            method;              // Values are G_METHOD_HEADER, G_METHOD_BODY or G_METHOD_URL for the access_token location, see https://tools.ietf.org/html/rfc6750
  char *         oauth_scope;         // Scope values required by the resource, multiple values must be separated by a space character
  char *         jwt_decode_key;      // The key used to decode an access token
  jwt_alg_t      jwt_alg;             // The algorithm used to encode a token, see http://benmcollins.github.io/libjwt/
  char *         realm;               // Optional, a realm value that will be sent back to the client
  unsigned short accept_access_token; // required, accept type acces_token
  unsigned short accept_client_token; // required, accept type client_token
};
```

Then, you use `callback_check_glewlwyd_access_token` as authentication callback for your ulfius endpoints that need to validate a glewlwyd access_token, example:

```C
struct _glewlwyd_resource_config g_config;
g_config.method = G_METHOD_HEADER;
g_config.oauth_scope = "scope1";
g_config.jwt_decode_key = "secret";
g_config.jwt_alg = JWT_ALG_HS512;
g_config.realm = "example";
g_config.accept_access_token = 1;
g_config.accept_client_token = 0;

// Example, add an authentication callback callback_check_glewlwyd_access_token for the endpoint GET "/api/resource/*"
ulfius_add_endpoint_by_val(instance, "GET", "/api", "/resource/*", 0, &callback_check_glewlwyd_access_token, (void*)g_config);
```
