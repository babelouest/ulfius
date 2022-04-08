# Token validation for resource service based on [Ulfius](https://github.com/babelouest/ulfius) framework

These files contain an authentication callback for Ulfius framework to validate a [JSON Web Token (JWT) Profile for OAuth 2.0 Access Tokens Draft 10](https://tools.ietf.org/html/draft-ietf-oauth-access-token-jwt-10) with the correct scope. It also verifies DPoP header and its conformity with the access_token, according to [OAuth 2.0 Demonstrating Proof-of-Possession at the Application Layer (DPoP) Draft 07](https://www.ietf.org/archive/id/draft-ietf-oauth-dpop-07.html).

[rhonabwy](https://github.com/babelouest/rhonabwy) and [iddawc](https://github.com/babelouest/iddawc) are required.

To use it, you must create a `struct _iddawc_resource_config` and initialize with the parameters:

```C
int i_jwt_profile_access_token_init_config(struct _iddawc_resource_config * config, // The config structure
                                           unsigned short method,                   // method to retrieve the access token, vlues are G_METHOD_HEADER, G_METHOD_BODY or G_METHOD_URL for the access_token location, see https://tools.ietf.org/html/rfc6750
                                           const char * realm,                      // Optional, a realm value that will be sent back to the client
                                           const char * aud,                        // Optional, the aud value to check in the claims
                                           const char * oauth_scope,                // Scope values required by the resource, multiple values must be separated by a space character
                                           const char * resource_url_root,          // root url of the resource server, required if DPoP is used
                                           time_t dpop_max_iat);                    // Maximum age in seconds for the DPoP iat value
```

Then, load the OIDC configuration url, or manually setup the public keys to validate the signatures:

```C
int i_jwt_profile_access_token_load_config(struct _iddawc_resource_config * config, const char * config_url, int verify_cert);

int i_jwt_profile_access_token_load_jwks(struct _iddawc_resource_config * config, json_t * j_jwks, const char * iss);
```

Then, you use `callback_check_jwt_profile_access_token` as authentication callback for your ulfius endpoints that need to validate an access_token, example:

```C
struct _iddawc_resource_config config;
ulfius_add_endpoint_by_val(instance, "GET", "/api", "/resource/*", &callback_check_jwt_profile_access_token, (void*)&config);
```
