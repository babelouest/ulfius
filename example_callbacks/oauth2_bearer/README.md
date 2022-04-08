# Token validation for resource service based on [Ulfius](https://github.com/babelouest/ulfius) framework

**(DEPRECATED)**

This callback function is deprecated and is not maintained anymore, please use [iddawc_jwt_profile](../iddawc_jwt_profile) instead.

These files contain an authentication callback for Ulfius framework to validate a Glewlwyd OAuth2 access token with the correct scope.

[rhonabwy](https://github.com/babelouest/rhonabwy) is required.

To use this file, you must create a `struct _glewlwyd_resource_config` with your specific parameters:

```C
struct _glewlwyd_resource_config {
  int            method;              // Values are G_METHOD_HEADER, G_METHOD_BODY or G_METHOD_URL for the access_token location, see https://tools.ietf.org/html/rfc6750
  char *         oauth_scope;         // Scope values required by the resource, multiple values must be separated by a space character
  jwt_t *        jwt;                 // The jwt used to decode an access token, the jwt must be initialized with the public key or jwks used to verify the signature
  jwa_alg        alg;                 // The algorithm used to encode a token, see https://babelouest.github.io/rhonabwy/
  char *         realm;               // Optional, a realm value that will be sent back to the client
  unsigned short accept_access_token; // required, accept type access_token
  unsigned short accept_client_token; // required, accept type client_token
};
```

Then, you use `callback_check_glewlwyd_access_token` as authentication callback for your ulfius endpoints that need to validate a glewlwyd access_token, example:

```C
struct _glewlwyd_resource_config g_config;
jwt_t * jwt;
r_jwt_init(&jwt);
r_jwt_add_sign_keys_json_str(jwt, NULL, "{\"kty\":\"EC\",\"crv\":\"P-256\",\"x\":\"MKBCTNIcKUSDii11ySs3526iDZ8AiTo7Tu6KPAqv7D4\","\
                                        "\"y\":\"4Etl6SRW2YiLUrN5vfvVHuhp7x8PxltmWWlbbM4IFyM\",\"use\":\"enc\",\"kid\":\"1\"}");
g_config.method = G_METHOD_HEADER;
g_config.oauth_scope = "scope1";
g_config.jwt = jwt;
g_config.alg = R_JWA_ALG_ES256;
g_config.realm = "example";
g_config.accept_access_token = 1;
g_config.accept_client_token = 0;

// Example, add an authentication callback callback_check_glewlwyd_access_token for the endpoint GET "/api/resource/*"
ulfius_add_endpoint_by_val(instance, "GET", "/api", "/resource/*", &callback_check_glewlwyd_access_token, (void*)g_config);
```
