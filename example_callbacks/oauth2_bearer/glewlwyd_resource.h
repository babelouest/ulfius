/**
 *
 * Glewlwyd SSO Access Token token check
 * This code is deprecated and should not be used anymore
 *
 * Copyright 2016-2022 Nicolas Mora <mail@babelouest.org>
 *
 * Version 20220604
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
#include <rhonabwy.h>

#define G_TOKEN_OK                       0
#define G_TOKEN_ERROR                    1
#define G_TOKEN_ERROR_INTERNAL           2
#define G_TOKEN_ERROR_INVALID_REQUEST    3
#define G_TOKEN_ERROR_INVALID_TOKEN      4
#define G_TOKEN_ERROR_INSUFFICIENT_SCOPE 5

#define G_METHOD_HEADER 0
#define G_METHOD_BODY   1
#define G_METHOD_URL    2

#define HEADER_PREFIX_BEARER "Bearer "
#define HEADER_RESPONSE      "WWW-Authenticate"
#define HEADER_AUTHORIZATION "Authorization"
#define BODY_URL_PARAMETER   "access_token"

struct _glewlwyd_resource_config {
  int       method;
  char *    oauth_scope;
  jwt_t *   jwt;
  jwa_alg   alg;
  char *    realm;
  unsigned short accept_access_token;
  unsigned short accept_client_token;
};

/**
 * 
 * check if bearer token has some of the specified scope
 * 
 */
int callback_check_glewlwyd_access_token (const struct _u_request * request, struct _u_response * response, void * user_data);
