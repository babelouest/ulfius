/**
 *
 * Iddawc OIDC Access Token token check
 *
 * Copyright 2021-2022 Nicolas Mora <mail@babelouest.org>
 *
 * Version 20220326
 *
 * The MIT License (MIT)
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */
#include <jansson.h>
#include <iddawc.h>

#define I_TOKEN_OK                       0
#define I_TOKEN_ERROR                    1
#define I_TOKEN_ERROR_INTERNAL           2
#define I_TOKEN_ERROR_INVALID_REQUEST    3
#define I_TOKEN_ERROR_INVALID_TOKEN      4
#define I_TOKEN_ERROR_INSUFFICIENT_SCOPE 5

#define I_METHOD_HEADER 0
#define I_METHOD_BODY   1
#define I_METHOD_URL    2

#define HEADER_PREFIX_BEARER "Bearer "
#define HEADER_RESPONSE      "WWW-Authenticate"
#define HEADER_AUTHORIZATION "Authorization"
#define BODY_URL_PARAMETER   "access_token"
#define HEADER_DPOP          "DPoP"

struct _iddawc_resource_config {
  unsigned short      method;
  char              * oauth_scope;
  char              * realm;
  char              * aud;
  struct _i_session * session;
  char              * resource_url_root;
  time_t              dpop_max_iat;
  unsigned short      accept_client_token;
  pthread_mutex_t     session_lock;
};

int jwt_profile_access_token_check_scope(struct _iddawc_resource_config * config, json_t * j_access_token);

/**
 * 
 * check if bearer token has some of the specified scope
 * Return I_TOKEN_OK on success
 * or I_TOKEN_ERROR* on any other case
 * 
 */
int callback_check_jwt_profile_access_token (const struct _u_request * request, struct _u_response * response, void * user_data);

int i_jwt_profile_access_token_init_config(struct _iddawc_resource_config * config, unsigned short method, const char * realm, const char * aud, const char * oauth_scope, const char * resource_url_root, unsigned short accept_client_token, time_t dpop_max_iat);

int i_jwt_profile_access_token_load_config(struct _iddawc_resource_config * config, const char * config_url, int verify_cert);

int i_jwt_profile_access_token_load_jwks(struct _iddawc_resource_config * config, json_t * j_jwks, const char * iss);

void i_jwt_profile_access_token_close_config(struct _iddawc_resource_config * config);
